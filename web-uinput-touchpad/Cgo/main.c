#include <stdio.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>

#define MAX_16_BIT_INT 65535

// Changes the sensitivity of the touchpad. Lower is more sensitive.
#define RESOLUTION 5

const unsigned int allow_event_type[] = {
	EV_KEY,
	EV_SYN,
	EV_ABS,
};
#define ALLOWED_EVENT_TYPES (sizeof allow_event_type / sizeof allow_event_type[0])

const unsigned int allow_key_code[] = {
	BTN_TOUCH,
	BTN_TOOL_FINGER,
	BTN_TOOL_DOUBLETAP,
	BTN_TOOL_TRIPLETAP,
	BTN_TOOL_QUADTAP,
	BTN_TOOL_QUINTTAP,
};
#define ALLOWED_KEY_CODES (sizeof allow_key_code / sizeof allow_key_code[0])

void uinput_close(int uinput_fd)
{
	if (uinput_fd == -1)
		return;

	ioctl(uinput_fd, UI_DEV_DESTROY);
	close(uinput_fd);
}

void setup_abs(int uinput_fd, int code, int minimum, int maximum, int resolution)
{
	if (uinput_fd == -1)
		uinput_close(uinput_fd);

	struct uinput_abs_setup abs_setup;
	// Zero out the abs_setup structure
	memset(&abs_setup, 0, sizeof(abs_setup));

	abs_setup.code = code;
	abs_setup.absinfo.value = 0;
	abs_setup.absinfo.minimum = minimum;
	abs_setup.absinfo.maximum = maximum;
	abs_setup.absinfo.resolution = resolution;

	ioctl(uinput_fd, UI_SET_ABSBIT, code);
	ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
}

int uinput_open(int width, int height)
{
	struct uinput_user_dev dev;
	size_t i;

	int uinput_fd = open("/dev/uinput", O_RDWR);
	if (uinput_fd == -1)
		return -1;

	memset(&dev, 0, sizeof dev);
	strncpy(dev.name, "web-uinput-touchpad", UINPUT_MAX_NAME_SIZE - 1);
	dev.id.bustype = BUS_VIRTUAL;
	dev.id.vendor = 0x1;
	dev.id.product = 0x2;
	dev.id.version = 0x3;
	dev.ff_effects_max = 0;

	do
	{
		for (i = 0; i < ALLOWED_EVENT_TYPES; i++)
			if (ioctl(uinput_fd, UI_SET_EVBIT, allow_event_type[i]) == -1)
				break;
		if (i < ALLOWED_EVENT_TYPES)
			break;

		for (i = 0; i < ALLOWED_KEY_CODES; i++)
			if (ioctl(uinput_fd, UI_SET_KEYBIT, allow_key_code[i]) == -1)
				break;
		if (i < ALLOWED_KEY_CODES)
			break;

		setup_abs(uinput_fd, ABS_X, 0, width, RESOLUTION);
		setup_abs(uinput_fd, ABS_Y, 0, height, RESOLUTION);
		setup_abs(uinput_fd, ABS_MT_SLOT, 0, 20, 0);					 // Only two slots are needed for two fingers
		setup_abs(uinput_fd, ABS_MT_TRACKING_ID, -1, MAX_16_BIT_INT, 0); // Initialize slots with -1
		setup_abs(uinput_fd, ABS_MT_POSITION_X, 0, width, RESOLUTION);
		setup_abs(uinput_fd, ABS_MT_POSITION_Y, 0, height, RESOLUTION);

		if (ioctl(uinput_fd, UI_DEV_SETUP, &dev) == -1)
			break;

		if (ioctl(uinput_fd, UI_DEV_CREATE) == -1)
			break;

		/* Success. */
		return uinput_fd;
	} while (0);

	/* FAILED: */
	{
		const int saved_errno = errno;
		close(uinput_fd);
		errno = saved_errno;
		return -1;
	}
}

void send_uinput_event(int uinput_fd, int type, int code, int value)
{
	if (uinput_fd == -1)
		return;

	struct input_event ev;

	ev.type = type;
	ev.code = code;
	ev.value = value;

	write(uinput_fd, &ev, sizeof(ev));
}