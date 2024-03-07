#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ABS_MAXVAL 1650 // 65535

void setup_abs(int fd, int code, int minimum, int maximum, int resolution)
{
	struct uinput_abs_setup abs_setup;
	// Zero out the abs_setup structure
	memset(&abs_setup, 0, sizeof(abs_setup));

	abs_setup.code = code;
	abs_setup.absinfo.value = 0;
	abs_setup.absinfo.minimum = minimum;
	abs_setup.absinfo.maximum = maximum;
	abs_setup.absinfo.resolution = resolution;

	ioctl(fd, UI_SET_ABSBIT, code);
	ioctl(fd, UI_ABS_SETUP, &abs_setup);
}

int init_uinput_touchpad(const char *name)
{
	int device = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (device < 0)
	{
		perror("open /dev/uinput");
		exit(EXIT_FAILURE);
	}

	ioctl(device, UI_SET_EVBIT, EV_SYN);
	ioctl(device, UI_SET_EVBIT, EV_KEY);
	ioctl(device, UI_SET_EVBIT, EV_ABS);
	ioctl(device, UI_SET_PROPBIT, INPUT_PROP_POINTER);
	ioctl(device, UI_SET_PROPBIT, INPUT_PROP_BUTTONPAD);
	ioctl(device, UI_SET_KEYBIT, BTN_TOUCH);
	ioctl(device, UI_SET_KEYBIT, BTN_TOOL_FINGER);
	ioctl(device, UI_SET_KEYBIT, BTN_LEFT);
	ioctl(device, UI_SET_KEYBIT, BTN_TOOL_DOUBLETAP);
	ioctl(device, UI_SET_KEYBIT, BTN_TOOL_TRIPLETAP);
	ioctl(device, UI_SET_KEYBIT, BTN_TOOL_QUADTAP);
	ioctl(device, UI_SET_KEYBIT, BTN_TOOL_QUINTTAP);

	setup_abs(device, ABS_X, 0, ABS_MAXVAL, 13);
	setup_abs(device, ABS_Y, 0, ABS_MAXVAL, 13);
	setup_abs(device, ABS_MT_SLOT, 0, 1, 0);		 // Only two slots are needed for two fingers
	setup_abs(device, ABS_MT_TRACKING_ID, -1, 1, 0); // Initialize slots with -1
	setup_abs(device, ABS_MT_POSITION_X, 0, ABS_MAXVAL, 13);
	setup_abs(device, ABS_MT_POSITION_Y, 0, ABS_MAXVAL, 13);
	setup_abs(device, ABS_MT_TOOL_TYPE, 0, 2, 0);

	struct uinput_setup setup;
	// Zero out the setup structure
	memset(&setup, 0, sizeof(setup));
	strncpy(setup.name, name, UINPUT_MAX_NAME_SIZE - 1);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = 0x1;
	setup.id.product = 0x2;
	setup.id.version = 0x3;
	setup.ff_effects_max = 0;

	ioctl(device, UI_DEV_SETUP, &setup);
	ioctl(device, UI_DEV_CREATE);

	return device;
}

void send_uinput_event(int device, int type, int code, int value)
{
	struct input_event ev;
	// Zero out the input_event structure
	memset(&ev, 0, sizeof(ev));

	ev.type = type;
	ev.code = code;
	ev.value = value;

	write(device, &ev, sizeof(ev));
}

int main()
{
	printf("Starting");

	int uinput_fd = init_uinput_touchpad("uinput-example");
	if (uinput_fd == -1)
	{
		perror("Unable to open uinput device");
		return 1;
	}

	int delta_x = ABS_MAXVAL / 4;
	int delta_y = ABS_MAXVAL / 100;

	int current_x = ABS_MAXVAL / 2;
	int current_y = ABS_MAXVAL / 2;

	// Required
	usleep(2000000);

	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_SLOT, 0);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, 0);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_POSITION_X, current_x);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, current_y);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOUCH, 1);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOOL_FINGER, 1);
	send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

	usleep(8000);

	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_SLOT, 1);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, 1);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_POSITION_X, current_x + delta_x); // offset for second finger
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, current_y);
	// send_uinput_event(uinput_fd, EV_KEY, BTN_TOUCH, 1);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOOL_FINGER, 0);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOOL_DOUBLETAP, 1);

	send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);
	usleep(8000);

	while (current_y >= 0)
	{
		send_uinput_event(uinput_fd, EV_ABS, ABS_MT_SLOT, 0);
		send_uinput_event(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, current_y);
		// send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

		send_uinput_event(uinput_fd, EV_ABS, ABS_MT_SLOT, 1);
		send_uinput_event(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, current_y);
		send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

		current_y -= delta_y;
		usleep(8000);
	}

	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_SLOT, 0);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOUCH, 0);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
	send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_SLOT, 1);
	send_uinput_event(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOUCH, 0);
	send_uinput_event(uinput_fd, EV_KEY, BTN_TOOL_FINGER, 0);
	send_uinput_event(uinput_fd, EV_SYN, SYN_REPORT, 0);

	usleep(8000);

	ioctl(uinput_fd, UI_DEV_DESTROY);
	close(uinput_fd);

	printf("Done");

	return 0;
}
