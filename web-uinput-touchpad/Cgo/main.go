package main

/*
#include <main.h>
*/
import "C"

import (
	"net/http"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func main() {
	http.HandleFunc("/ws", func(writer http.ResponseWriter, req *http.Request) {
		uinput_fp := C.int(-1)

		// A map of slot => identifier
		touch_identifiers := make([]int32, 10)
		for i := range touch_identifiers {
			touch_identifiers[i] = -1
		}

		conn, wsErr := upgrader.Upgrade(writer, req, nil)
		if wsErr != nil {
			return
		}

		/**
		 * Protocol (angle brackets are to be omitted):
		 * - 0 <width> <height>: Initialize a new device with dimensions width and height
		 * - 1 <identifier> <x> <y>: Simulate a new touch contact at slot with coordinates x and y
		 * - 2 <identifier>: Simulate a touch contact being lifted
		 */
		for {
			_, msg, err := conn.ReadMessage()
			if err != nil {
				return
			}

			// Print received message
			// fmt.Printf("%s sent: %s", conn.RemoteAddr(), string(msg))
			numbers := parse_message(string(msg))

			switch numbers[0] {
			case 0:
				// Init packet to initialize a new touchpad device
				// <width> <height>
				width := numbers[1]
				height := numbers[2]

				if uinput_fp != -1 {
					C.uinput_close(uinput_fp)
				}

				uinput_fp = C.uinput_open(C.int(width), C.int(height))
				defer C.uinput_close(uinput_fp)
			case 1:
				// Touchdown packet to simulate a new touch contact
				// <identifier> <x> <y>
				identifier := numbers[1]
				x := numbers[2]
				y := numbers[3]

				// Check to see if the touch contact already exists
				slot := find_slot(touch_identifiers, identifier)

				new_touch := slot == -1
				if new_touch {
					// Find the first empty slot
					for i, touch_identifier := range touch_identifiers {
						if touch_identifier == -1 {
							slot = i
							touch_identifiers[i] = identifier
							break
						}
					}
				}

				C.send_uinput_event(uinput_fp, C.int(EV_ABS), C.int(ABS_MT_SLOT), C.int(slot))
				C.send_uinput_event(uinput_fp, C.int(EV_ABS), C.int(ABS_MT_TRACKING_ID), C.int(identifier))
				C.send_uinput_event(uinput_fp, C.int(EV_ABS), C.int(ABS_MT_POSITION_X), C.int(x))
				C.send_uinput_event(uinput_fp, C.int(EV_ABS), C.int(ABS_MT_POSITION_Y), C.int(y))

				if new_touch {
					update_touches(uinput_fp, touch_identifiers, true)
				}

				C.send_uinput_event(uinput_fp, C.int(EV_SYN), C.int(SYN_REPORT), C.int(0))
			case 2:
				// Simulate a touch contact being lifted
				// <identifier>
				identifier := numbers[1]

				// Find the slot of the touch contact
				slot := find_slot(touch_identifiers, identifier)
				if slot == -1 {
					continue
				}

				// Clear the slot
				touch_identifiers[slot] = -1

				C.send_uinput_event(uinput_fp, C.int(EV_ABS), C.int(ABS_MT_SLOT), C.int(slot))
				C.send_uinput_event(uinput_fp, C.int(EV_ABS), C.int(ABS_MT_TRACKING_ID), C.int(-1))

				update_touches(uinput_fp, touch_identifiers, false)

				C.send_uinput_event(uinput_fp, C.int(EV_SYN), C.int(SYN_REPORT), C.int(0))
			}
		}
	})

	http.HandleFunc("/", func(writer http.ResponseWriter, req *http.Request) {
		http.ServeFile(writer, req, "index.html")
	})

	http.ListenAndServe(":3000", nil)
}
