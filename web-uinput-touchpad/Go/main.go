package main

import (
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/excigma/uinput-mt-touchpad"
	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func main() {
	http.HandleFunc("/ws", func(w http.ResponseWriter, r *http.Request) {
		var touchpad uinput.MultiTouch

		conn, wsErr := upgrader.Upgrade(w, r, nil)
		if wsErr != nil {
			return
		}

		for {
			/**
			 * Protocol (angle brackets are to be omitted):
			 * - 0 <width> <height>: Initialize a new device with dimensions width and height
			 * - 1 <slot> <x> <y>: Simulate a new touch contact at slot with coordinates x and y
			 * - 2 <slot> <x> <y>: Simulate a touch contact moving to coordinates x and y
			 * - 3 <slot>: Simulate a touch contact being lifted
			 */
			_, msg, err := conn.ReadMessage()
			if err != nil {
				return
			}

			// Parse command, one two and three from the message
			// Split the string into individual numbers
			msgSplit := strings.Split(strings.Trim(string(msg), "\n"), " ")

			// Initialize a slice to store parsed integers
			var numbers []int32

			// Parse each number string into an integer
			for _, numStr := range msgSplit {
				num, err := strconv.Atoi(numStr)
				if err != nil {
					fmt.Println("Error parsing number:", err)
					return
				}
				numbers = append(numbers, int32(num))
			}

			// Print the parsed numbers
			fmt.Printf("%s sent: %s\n", conn.RemoteAddr(), string(msg))

			if numbers[0] == 0 {
				// Init packet to initialize a new touchpad device
				// width, height
				// Close the previous touchpad if it exists
				if touchpad != nil {
					touchpad.Close()
				}

				// Create a new touchpad with the given dimensions
				touchpad, err = uinput.CreateMultiTouch(
					"/dev/uinput",
					[]byte("virtual-touchpad"),
					0,
					numbers[1],
					0,
					numbers[2],
					20)
				if err != nil {
					return
				}
				defer touchpad.Close()
			} else if numbers[0] == 1 || numbers[0] == 2 {
				// Touchdown packet to simulate a new touch contact
				// slot, x, y
				// Simulate a new touch contact
				touchpad.GetContacts()[numbers[1]].TouchDownAt(numbers[2], numbers[3])
			} else if numbers[0] == 3 {
				// Touchup packet to simulate a touch contact being lifted
				// slot
				// Simulate a touch contact being lifted
				touchpad.GetContacts()[numbers[1]].TouchUp()
			}
		}
	})

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "index.html")
	})

	http.ListenAndServe(":3000", nil)
}
