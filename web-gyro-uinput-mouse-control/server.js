const http = require('http');
const WebSocket = require('ws');
const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

const htmlFile = fs.readFileSync(path.join(__dirname, 'index.html'));

const server = http.createServer((req, res) => {
	if (req.url === '/') {
		res.writeHead(200, { 'Content-Type': 'text/html' });
		res.end(htmlFile);
	} else {
		res.writeHead(404);
		res.end('404 Not Found');
	}
});

const wss = new WebSocket.Server({ server });

wss.on('connection', (ws) => {
	console.log('Client connected');

	const cProcess = spawn('./build/main', [], { stdio: 'pipe' });

	ws.on('message', (message) => {
		gcc
		const text = message.toString('utf8');
		console.log('Received:', text);
		cProcess.stdin.write(text);
	});

	cProcess.on('close', (code) => {
		console.log('C program exited with code', code);
	});

	cProcess.on('error', (err) => {
		console.error('Error spawning C program:', err);
	});

	ws.on('close', () => {
		console.log('Client disconnected');
		cProcess.kill();
	});
});

// Start the server
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
	console.log(`Server listening on port ${PORT}`);
});
