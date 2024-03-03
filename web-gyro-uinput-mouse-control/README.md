# web-gyro-uinput-mouse-control
 
This is a simple web interface hosted with a Node.js to control a mouse using a phone's gyroscope. It uses a small .c program interacting with the `uinput` kernel module to create a virtual mouse.

HTTPS is required to access the gyroscope data from the phone. This was made with the intention of it running behind a reverse proxy.

Security is non-existent.