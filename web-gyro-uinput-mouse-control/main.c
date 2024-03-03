#include <stdio.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

static const unsigned int allow_event_type[] = {
	EV_KEY,
	EV_SYN,
	EV_REL,
};
#define ALLOWED_EVENT_TYPES (sizeof allow_event_type / sizeof allow_event_type[0])

static const unsigned int allow_key_code[] = {
	KEY_SPACE,
	BTN_LEFT,
	BTN_MIDDLE,
	BTN_RIGHT,
};
#define ALLOWED_KEY_CODES (sizeof allow_key_code / sizeof allow_key_code[0])

static const unsigned int allow_rel_code[] = {
	REL_X,
	REL_Y,
	REL_WHEEL,
};
#define ALLOWED_REL_CODES (sizeof allow_rel_code / sizeof allow_rel_code[0])

static int uinput_open(const char *name, const unsigned int vendor, const unsigned int product, const unsigned int version)
{
	struct uinput_user_dev dev;
	int fd;
	size_t i;

	if (!name || strlen(name) < 1 || strlen(name) >= UINPUT_MAX_NAME_SIZE)
	{
		errno = EINVAL;
		return -1;
	}

	fd = open("/dev/uinput", O_RDWR);
	if (fd == -1)
		return -1;

	memset(&dev, 0, sizeof dev);
	strncpy(dev.name, name, UINPUT_MAX_NAME_SIZE);
	dev.id.bustype = BUS_USB;
	dev.id.vendor = vendor;
	dev.id.product = product;
	dev.id.version = version;

	do
	{
		for (i = 0; i < ALLOWED_EVENT_TYPES; i++)
			if (ioctl(fd, UI_SET_EVBIT, allow_event_type[i]) == -1)
				break;
		if (i < ALLOWED_EVENT_TYPES)
			break;

		for (i = 0; i < ALLOWED_KEY_CODES; i++)
			if (ioctl(fd, UI_SET_KEYBIT, allow_key_code[i]) == -1)
				break;
		if (i < ALLOWED_KEY_CODES)
			break;

		for (i = 0; i < ALLOWED_REL_CODES; i++)
			if (ioctl(fd, UI_SET_RELBIT, allow_rel_code[i]) == -1)
				break;
		if (i < ALLOWED_REL_CODES)
			break;

		if (write(fd, &dev, sizeof dev) != sizeof dev)
			break;

		if (ioctl(fd, UI_DEV_CREATE) == -1)
			break;

		/* Success. */
		return fd;

	} while (0);

	/* FAILED: */
	{
		const int saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}
}

static void uinput_close(const int fd)
{
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);
}

static void send_event(int fd, int type, int code, int value)
{
	struct input_event event;
	memset(&event, 0, sizeof(event));
	event.type = type;
	event.code = code;
	event.value = value;
	write(fd, &event, sizeof(event));
}

int main()
{
	int uinput_fd = uinput_open("uinput-example", 0x1, 0x2, 0x3); // Example vendor, product, version

	if (uinput_fd == -1)
	{
		perror("Unable to open uinput device");
		return 1;
	}

	char input_buffer[30];
	int x_amount = 0, y_amount = 0;

	while (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL)
	{
		// Parse input for x and y coordinates
		if (sscanf(input_buffer, "%d %d", &x_amount, &y_amount) != 2)
		{
			// fprintf(stderr, "Invalid input format. Expected: x y\n");
			continue;
		}

		// Move mouse based on x and y coordinates
		send_event(uinput_fd, EV_REL, REL_X, x_amount);
		send_event(uinput_fd, EV_REL, REL_Y, y_amount);
		send_event(uinput_fd, EV_SYN, SYN_REPORT, 0); // Sync the event
	}

	uinput_close(uinput_fd);

	return 0;
}
