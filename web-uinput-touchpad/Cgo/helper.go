package main

/*
#include <main.h>
*/
import "C"

import (
	"fmt"
	"strconv"
	"strings"
)

// function to parse the string message to an array of ints
func parse_message(msg string) []int32 {
	// Split the string into individual numbers
	msgSplit := strings.Split(strings.Trim(msg, "\n"), " ")

	// Initialize a slice to store parsed integers
	var numbers []int32

	// Parse each number string into an integer
	for _, numStr := range msgSplit {
		num, err := strconv.Atoi(numStr)
		if err != nil {
			fmt.Println("Error parsing number:", err)
			return nil
		}
		numbers = append(numbers, int32(num))
	}

	return numbers
}

func find_slot(touch_identifiers []int32, identifier int32) int {
	for i, touch_identifier := range touch_identifiers {
		if touch_identifier == identifier {
			return i
		}
	}

	return -1
}

func touch_count(touch_identifiers []int32) int {
	count := 0
	for _, touch_identifier := range touch_identifiers {
		if touch_identifier != -1 {
			count++
		}
	}

	return count
}

func update_touches(uinput_fp C.int, touch_identifiers []int32, touch_down bool) {
	touch_count := touch_count(touch_identifiers)
	// There has to be a better way lol

	if touch_down {
		// There has to be a better way lol
		switch touch_count {
		case 1:
			// Touch started?
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOUCH), C.int(1))
			// Finger down
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_FINGER), C.int(1))
		case 2:
			// Switch the events
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_FINGER), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_DOUBLETAP), C.int(1))
		case 3:
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_DOUBLETAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_TRIPLETAP), C.int(1))
		case 4:
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_TRIPLETAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_QUADTAP), C.int(1))
		case 5:
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_QUADTAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_QUINTTAP), C.int(1))
		}
	} else {
		switch touch_count {
		case 0:
			// Touch ended?
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOUCH), C.int(0))
			// Finger up
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_FINGER), C.int(0))
		case 1:
			// Switch the events
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_DOUBLETAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_FINGER), C.int(1))
		case 2:
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_TRIPLETAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_DOUBLETAP), C.int(1))
		case 3:
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_QUADTAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_TRIPLETAP), C.int(1))
		case 4:
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_QUINTTAP), C.int(0))
			C.send_uinput_event(uinput_fp, C.int(EV_KEY), C.int(BTN_TOOL_QUADTAP), C.int(1))
		}
	}
}
