int uinput_open(int height, int width);
void uinput_close(int uinput_fd);
void send_uinput_event(int uinput_fd, int type, int code, int value);