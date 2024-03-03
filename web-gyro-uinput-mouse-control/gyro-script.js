const socket = new WebSocket("wss://" + location.host);

const messagesDiv = document.getElementById('messages');
const messageInput = document.getElementById('messageInput');
const sendButton = document.getElementById('sendButton');
const deviceMotion = document.getElementById('deviceMotion');
const deviceOrientation = document.getElementById('deviceOrientation');
const info = document.getElementById('info');

let interval;
let beta = 0, gamma = 0;

// Event listener for WebSocket open
socket.addEventListener('open', (event) => {
	console.log('WebSocket connection opened');
	info.textContent = 'WebSocket connection opened';
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

	if (!beta || !gamma) return;
	if (!socket.readyState === WebSocket.OPEN) return;

	socket.send(`${Math.round(gamma)} ${Math.round(beta)}\n`);
});