const socket = new WebSocket("wss://" + location.host);

const messagesDiv = document.getElementById('messages');
const messageInput = document.getElementById('messageInput');
const sendButton = document.getElementById('sendButton');
const deviceMotion = document.getElementById('deviceMotion');
const deviceOrientation = document.getElementById('deviceOrientation');
const info = document.getElementById('info');

let interval;
let beta = 0, gamma = 0;
let xAccel = 0, yAccel = 0;

// Event listener for WebSocket open
socket.addEventListener('open', (event) => {
	console.log('WebSocket connection opened');
	info.textContent = 'WebSocket connection opened';

	interval = setInterval(() => {
		// if (!beta || !gamma) return;
		// socket.send(`${Math.round(gamma)} ${Math.round(beta)}\n`);

		if (!xAccel || !yAccel) return;
		socket.send(`${Math.round(xAccel)} ${Math.round(yAccel)}\n`);
	}, 16);
});

socket.addEventListener('message', (event) => {
	console.log('Message received:', event.data);
	const messageDiv = document.createElement('div');
	messageDiv.textContent = event.data;
	messagesDiv.appendChild(messageDiv);
});

socket.addEventListener('close', (event) => {
	console.log('WebSocket connection closed');
	clearInterval(interval);
});

socket.addEventListener('error', (error) => {
	console.error('WebSocket error:', error);
	clearInterval(interval);
});

window.addEventListener('deviceorientation', (event) => {
	beta = event.beta;
	gamma = event.gamma;

	console.log('Device motion:', beta, gamma);
	deviceMotion.textContent = `Beta: ${beta}, Gamma: ${gamma}`;
});

window.addEventListener('devicemotion', (event) => {
	xAccel = event.accelerationIncludingGravity.x * 10;
	yAccel = event.accelerationIncludingGravity.y * 10;

	console.log('Device motion:', xAccel, yAccel);
	deviceOrientation.textContent = `X: ${xAccel}, Y: ${yAccel}`;
});