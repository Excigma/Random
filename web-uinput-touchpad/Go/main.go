package main

import (
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/bendahl/uinput"
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
			 * - 0 <width> <height> 0\n: Initialize a new device with dimensions width and height
			 * - 1 <type> <code> <value>\n: Call send_uinput_event with type, code, and value
			 */
			_, msg, err := conn.ReadMessage()
			if err != nil {
				return
			}

			// Parse command, one two and three from the message
			// Split the string into individual numbers
			msgSplit := strings.Split(string(msg), " ")

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

			if numbers[0] == 0 {
				touchpad, err = uinput.CreateMultiTouch(
					"/dev/uinput",
					[]byte("virtual-touchpad"),
					0,
					numbers[1],
					0,
					numbers[2],
					10)
				if err != nil {
					return
				}
				defer touchpad.Close()

				// touchpad.GetContacts()[0].id
				fmt.Printf("%s sent: %s\n", conn.RemoteAddr(), string(msg))
			} else if numbers[0] == 1 {

			}
		}
	})

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "index.html")
	})

	http.ListenAndServe(":3000", nil)
}
