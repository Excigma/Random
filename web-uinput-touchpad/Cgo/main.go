package main

/*
#include <main.h>
*/
import "C"

import (
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}

func main() {
	http.HandleFunc("/ws", func(w http.ResponseWriter, r *http.Request) {
		var uinput_fp C.int = -1

		conn, wsErr := upgrader.Upgrade(w, r, nil)
		if wsErr != nil {
			return
		}

		for {
			/**
			 * Protocol (angle brackets are to be omitted):
			 * - 0 <width> <height> 0\n: Initialize a new device with dimensions width and height
			 * - 1 <type> <code> <value>\n: Call send_uinput_event with type, code, and value
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
			// fmt.Printf("%s sent: %s\n", conn.RemoteAddr(), string(msg))

			if numbers[0] == 0 {
				// Init packet to initialize a new touchpad device
				// width, height
				if uinput_fp != -1 {
					C.uinput_close(uinput_fp)
				}

				uinput_fp = C.uinput_open(C.int(numbers[1]), C.int(numbers[2]))
				defer C.uinput_close(uinput_fp)
			} else if numbers[0] == 1 {
				// Touchdown packet to simulate a new touch contact
				// slot, x, y
				C.send_uinput_event(uinput_fp, C.int(numbers[1]), C.int(numbers[2]), C.int(numbers[3]))
			}
		}
	})

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "index.html")
	})

	http.ListenAndServe(":3000", nil)
}
