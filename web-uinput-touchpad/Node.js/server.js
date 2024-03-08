const http = require('http');
const WebSocket = require('ws');
const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');
const { networkInterfaces } = require('os');

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

let writing = false;
const wss = new WebSocket.Server({ server });

wss.on('connection', (ws) => {
	console.log('Client connected');

	const cProcess = spawn('./main.bin', [], { stdio: 'pipe' });

	ws.on('message', (message) => {
		const text = message.toString('utf8');
		// console.log('Received:', text);
		cProcess.stdin.write(text);
	});

	cProcess.stdout.on('data', (data) => {
		console.log(`stdout: ${data}`);
	});

	cProcess.stderr.on('data', (data) => {
		console.error(`stderr: ${data}`);
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

	const nets = networkInterfaces();
	const results = []

	for (const name of Object.keys(nets)) {
		for (const net of nets[name]) {
			const familyV4Value = typeof net.family === 'string' ? 'IPv4' : 4
			if (net.family === familyV4Value && !net.internal) {
				results.push(net.address);
			}
		}
	}

	console.log(`Try visiting: ${results.map(ip => `http://${ip}:${PORT}/`)} on your phone!`)
});
