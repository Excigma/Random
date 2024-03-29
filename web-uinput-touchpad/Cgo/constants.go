package main

const (
	TOUCH_DOWN = iota
	TOUCH_UP   = iota
)

const (
	EV_ABS = 0x03
	EV_KEY = 0x01
	EV_SYN = 0x00

	ABS_MT_SLOT        = 0x2f
	ABS_MT_TRACKING_ID = 0x39
	ABS_MT_POSITION_X  = 0x35
	ABS_MT_POSITION_Y  = 0x36

	BTN_TOUCH          = 0x14a
	BTN_TOOL_FINGER    = 0x145
	BTN_TOOL_DOUBLETAP = 0x14d
	BTN_TOOL_TRIPLETAP = 0x14e
	BTN_TOOL_QUADTAP   = 0x14f
	BTN_TOOL_QUINTTAP  = 0x148

	SYN_REPORT = 0x00
)
