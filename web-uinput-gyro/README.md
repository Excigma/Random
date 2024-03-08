# web-uinput-gyro
 
This is a simple web interface hosted with a Node.js to control a mouse using a phone's gyroscope. It uses a small .c program interacting with the `uinput` kernel module to create a virtual mouse.

HTTPS is required to access the gyroscope data from the phone. This was made with the intention of it running behind a reverse proxy.

Security is non-existent.

You need to create some udev rules: https://github.com/H-M-H/Weylus/blob/8c5ad71b8e7a3628d9e988f9adbdc1bb214b205f/Readme.md#linux