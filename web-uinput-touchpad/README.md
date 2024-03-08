# web-uinput-touchpad

Doesn't work at all yet. Work in progress. Or maybe not in progress anymore.

This is a simple web interface hosted with a Node.js to control a mouse using a phone or web browser on a device with a touchscreen as a touchpad. It uses a small .c program interacting with the `uinput` kernel module to create a virtual mouse.

You need to create some udev rules: https://github.com/H-M-H/Weylus/blob/8c5ad71b8e7a3628d9e988f9adbdc1bb214b205f/Readme.md#linux

Security is non-existent.
