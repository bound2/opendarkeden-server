// Start beside the production gateway tests; native and browser transport probes
// connect to these controlled endpoints without a database or an account.
import net from 'node:net';
import http from 'node:http';
import { once } from 'node:events';
import { WebSocketServer } from 'ws';
import { createGateway } from './gateway.mjs';

const backend = net.createServer(socket => {
  let header = Buffer.alloc(0);
  let ready = false;
  socket.on('error', () => {});
  socket.on('data', bytes => {
    if (ready) { socket.write(bytes); return; }
    header = Buffer.concat([header, bytes]);
    const end = header.indexOf('\r\n');
    if (end < 0) return;
    if (!/^PROXY TCP4 127\.0\.0\.1 127\.0\.0\.1 \d+ \d+$/.test(header.subarray(0, end).toString())) {
      socket.destroy(); return;
    }
    ready = true;
    socket.write(header.subarray(end + 2));
  });
});
backend.listen(0, '127.0.0.1');
await once(backend, 'listening');
const gateway = createGateway({ origins: ['http://127.0.0.1:18739'], allowNative: true,
  routes: { '192.0.2.2:9999': backend.address().port, '192.0.2.3:9998': backend.address().port } });
gateway.server.listen(18740, '127.0.0.1');

const adversary = http.createServer();
const wss = new WebSocketServer({ server: adversary, perMessageDeflate: false });
wss.on('connection', (ws, request) => {
  ws.on('error', () => {});
  if (new URL(request.url, 'http://fixture').searchParams.get('port') === '9997') {
    ws.send('text must never become game packet bytes');
    console.log('Adversarial text frame sent');
  } else {
    ws.on('message', bytes => ws.send(bytes, { binary: true }));
  }
});
adversary.listen(18741, '127.0.0.1');
console.log('Gateway fixture: ws://127.0.0.1:18740/game');
console.log('Text rejection fixture: ws://127.0.0.1:18741/game');

for (const signal of ['SIGINT', 'SIGTERM']) process.once(signal, async () => {
  await gateway.close();
  for (const ws of wss.clients) ws.terminate();
  wss.close();
  adversary.close();
  backend.close();
});
