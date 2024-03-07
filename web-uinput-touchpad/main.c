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

#define ABS_MAXVAL 1650 // 65535
#define MAX_16_BIT_INT 65535

static const unsigned int allow_event_type[] = {
	EV_KEY,
	EV_SYN,
	EV_ABS,
};
#define ALLOWED_EVENT_TYPES (sizeof allow_event_type / sizeof allow_event_type[0])

static const unsigned int allow_key_code[] = {
	BTN_TOUCH,
	BTN_TOOL_FINGER,
	BTN_TOOL_DOUBLETAP,
	BTN_TOOL_TRIPLETAP,
	BTN_TOOL_QUADTAP,
	BTN_TOOL_QUINTTAP,
};
#define ALLOWED_KEY_CODES (sizeof allow_key_code / sizeof allow_key_code[0])

static int uinput_fd;

static void uinput_close()
{
	if (uinput_fd == -1)
		return;

	ioctl(uinput_fd, UI_DEV_DESTROY);
	close(uinput_fd);

	uinput_fd = -1;
}

void setup_abs(int code, int minimum, int maximum, int resolution)
{
	if (uinput_fd == -1)
		uinput_close();

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

static int uinput_open(const char *name, const unsigned int vendor, const unsigned int product, const unsigned int version, int height, int width)
{
	if (uinput_fd != -1)
		uinput_close();

	struct uinput_user_dev dev;
	size_t i;

	if (!name || strlen(name) < 1 || strlen(name) >= UINPUT_MAX_NAME_SIZE)
	{
		errno = EINVAL;
		return -1;
	}

	uinput_fd = open("/dev/uinput", O_RDWR);
	if (uinput_fd == -1)
		return -1;

	memset(&dev, 0, sizeof dev);
	strncpy(dev.name, name, UINPUT_MAX_NAME_SIZE - 1);
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

		setup_abs(ABS_X, 0, width, 1);
		setup_abs(ABS_Y, 0, height, 1);
		setup_abs(ABS_MT_SLOT, 0, 10, 0);					  // Only two slots are needed for two fingers
		setup_abs(ABS_MT_TRACKING_ID, -1, MAX_16_BIT_INT, 0); // Initialize slots with -1
		setup_abs(ABS_MT_POSITION_X, 0, width, 1);
		setup_abs(ABS_MT_POSITION_Y, 0, height, 1);
		setup_abs(ABS_MT_TOOL_TYPE, 0, 2, 0);

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

void send_uinput_event(int type, int code, int value)
{
	if (uinput_fd == -1)
		return;

	struct input_event ev;
	// Zero out the input_event structure
	memset(&ev, 0, sizeof(ev));

	ev.type = type;
	ev.code = code;
	ev.value = value;

	write(uinput_fd, &ev, sizeof(ev));
}

/**
 * Protocol (angle brackets are to be omitted):
 * - 0 <width> <height> 0\n: Initialize a new device with dimensions width and height
 * - 1 <type> <code> <value>\n: Call send_uinput_event with type, code, and value
 */
int main()
{
	int command;

	// Parse the protocol from stdin. They may arrive in any order
	while (scanf("%d", &command) == 1)
	{
		switch (command)
		{
		case 0:
			// Initialize a new device with dimensions width and height
			int width = 0, height = 0;
			scanf("%d %d", &width, &height);

			// Example vendor, product, version
			uinput_fd = uinput_open("uinput-example", 0x1, 0x2, 0x3, width, height);

			if (uinput_fd == -1)
			{
				perror("Unable to open uinput device");
				return 1;
			}

			// Required - unsure why
			usleep(2000000);
			break;
		case 1:
			// Call send_uinput_event with type, code, and value
			int type = 0, code = 0, value = 0;
			scanf("%d %d %d", &type, &code, &value);

			send_uinput_event(type, code, value);
			break;
		default:
			printf("Unknown command: %d\n", command);
			break;
		}
	}

	// uinput_fd = uinput_open("uinput-example", 0x1, 0x2, 0x3, ABS_MAXVAL, ABS_MAXVAL); // Example vendor, product, version

	// if (uinput_fd == -1)
	// {
	// 	perror("Unable to open uinput device");
	// 	return 1;
	// }

	// int delta_x = ABS_MAXVAL / 4;
	// int delta_y = ABS_MAXVAL / 100;

	// int current_x = ABS_MAXVAL / 2;
	// int current_y = ABS_MAXVAL / 2;

	// // Required - unsure why
	// // usleep(2000000);

	// send_uinput_event(EV_ABS, ABS_MT_SLOT, 0);
	// send_uinput_event(EV_ABS, ABS_MT_TRACKING_ID, 0);
	// send_uinput_event(EV_ABS, ABS_MT_POSITION_X, current_x);
	// send_uinput_event(EV_ABS, ABS_MT_POSITION_Y, current_y);
	// send_uinput_event(EV_KEY, BTN_TOUCH, 1);
	// send_uinput_event(EV_KEY, BTN_TOOL_FINGER, 1);
	// send_uinput_event(EV_SYN, SYN_REPORT, 0);

	// // usleep(8000);

	// send_uinput_event(EV_ABS, ABS_MT_SLOT, 1);
	// send_uinput_event(EV_ABS, ABS_MT_TRACKING_ID, 1);
	// send_uinput_event(EV_ABS, ABS_MT_POSITION_X, current_x + delta_x); // offset for second finger
	// send_uinput_event(EV_ABS, ABS_MT_POSITION_Y, current_y);
	// // send_uinput_event(uinput_fd, EV_KEY, BTN_TOUCH, 1);
	// send_uinput_event(EV_KEY, BTN_TOOL_FINGER, 0);
	// send_uinput_event(EV_KEY, BTN_TOOL_DOUBLETAP, 1);

	// send_uinput_event(EV_SYN, SYN_REPORT, 0);
	// // usleep(8000);

	// while (current_y >= 0)
	// {
	// 	send_uinput_event(EV_ABS, ABS_MT_SLOT, 0);
	// 	send_uinput_event(EV_ABS, ABS_MT_POSITION_Y, current_y);
	// 	// send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

	// 	send_uinput_event(EV_ABS, ABS_MT_SLOT, 1);
	// 	send_uinput_event(EV_ABS, ABS_MT_POSITION_Y, current_y);
	// 	send_uinput_event(EV_SYN, SYN_REPORT, 0);

	// 	current_y -= delta_y;
	// 	usleep(80000);
	// }

	// send_uinput_event(EV_ABS, ABS_MT_SLOT, 0);
	// send_uinput_event(EV_ABS, ABS_MT_TRACKING_ID, -1);
	// send_uinput_event(EV_KEY, BTN_TOUCH, 0);
	// send_uinput_event(EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	// send_uinput_event(EV_SYN, SYN_REPORT, 0);

	// send_uinput_event(EV_ABS, ABS_MT_SLOT, 1);
	// send_uinput_event(EV_ABS, ABS_MT_TRACKING_ID, -1);
	// send_uinput_event(EV_KEY, BTN_TOUCH, 0);
	// send_uinput_event(EV_KEY, BTN_TOOL_FINGER, 0);
	// send_uinput_event(EV_SYN, SYN_REPORT, 0);

	// // usleep(8000);

	uinput_close(uinput_fd);
	return 0;
}
