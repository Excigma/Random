const socket = new WebSocket("wss://" + location.host);

const messagesDiv = document.getElementById('messages');
const messageInput = document.getElementById('messageInput');
const sendButton = document.getElementById('sendButton');
const deviceMotion = document.getElementById('deviceMotion');
const deviceOrientation = document.getElementById('deviceOrientation');
const info = document.getElementById('info');

let interval;
let lastTimestamp;
let speedX = 0, speedY = 0;

// Event listener for WebSocket open
socket.addEventListener('open', (event) => {
	console.log('WebSocket connection opened');
	info.textContent = 'WebSocket connection opened';

	interval = setInterval(() => {
		if (!speedX || !speedY) return;
		socket.send(`${Math.round(speedX)} ${Math.round(speedY)}\n`);
	}, 16); // Adjust interval as needed
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

window.addEventListener('devicemotion', function (event) {
	var currentTime = new Date().getTime();
	if (lastTimestamp === undefined) {
		lastTimestamp = new Date().getTime();
		return;
	}
	speedX += event.acceleration.x / 1000 * ((currentTime - lastTimestamp) / 1000) / 3600;
	speedY += event.acceleration.y / 1000 * ((currentTime - lastTimestamp) / 1000) / 3600;
	lastTimestamp = currentTime;
}, false);