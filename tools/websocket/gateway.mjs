import http from 'node:http';
import net from 'node:net';
import { readFile } from 'node:fs/promises';
import { pathToFileURL } from 'node:url';
import { WebSocket, WebSocketServer } from 'ws';

const MAX_MESSAGE = 1024 * 1024;
const MAX_QUEUE = 2 * MAX_MESSAGE;
const ipv4 = address => address?.startsWith('::ffff:') ? address.slice(7) : address;

export function createGateway(config) {
  if (!config || !Array.isArray(config.origins) || config.origins.length === 0 ||
      !config.routes || !Object.keys(config.routes).length)
    throw new Error('Configure allowed browser origins and advertised host:port routes.');
  const routes = new Map(Object.entries(config.routes));
  for (const [route, port] of routes) {
    if (!/^[a-zA-Z0-9.-]+:[1-9][0-9]{0,4}$/.test(route) || Number(route.split(':')[1]) > 65535 ||
        !Number.isInteger(port) || port < 1 || port > 65535)
      throw new Error('Each route must map an advertised host:port to a local GatewayProxyPort.');
  }
  const origins = new Set(config.origins);
  const trustedProxies = new Set(config.trustedProxies ?? []);
  const maxConnections = config.maxConnections ?? 2000;
  if (!Number.isInteger(maxConnections) || maxConnections < 1 || maxConnections > 100000)
    throw new Error('Invalid maxConnections.');
  const server = http.createServer((_request, response) => {
    response.writeHead(200, { 'content-type': 'text/plain' });
    response.end('DarkEden WebSocket gateway\n');
  });
  const sockets = new Set();
  const wss = new WebSocketServer({ noServer: true, maxPayload: MAX_MESSAGE,
    perMessageDeflate: false, handleProtocols: protocols => protocols.has('binary') ? 'binary' : false });
  server.on('connection', socket => {
    // Count HTTP handshakes too, including connections that never send headers.
    if (sockets.size >= maxConnections) { socket.destroy(); return; }
    sockets.add(socket);
    socket.setTimeout(10000, () => socket.destroy());
    socket.once('close', () => sockets.delete(socket));
    socket.on('error', () => {});
  });
  server.on('upgrade', (request, socket, head) => {
    const reject = status => {
      socket.end(`HTTP/1.1 ${status}\r\nConnection: close\r\nContent-Length: 0\r\n\r\n`);
    };
    let url;
    try { url = new URL(request.url, 'http://gateway'); }
    catch { reject('400 Bad Request'); return; }
    if (url.pathname !== '/game' || [...url.searchParams.keys()].some(key => !['host', 'port'].includes(key)) ||
        url.searchParams.getAll('host').length !== 1 || url.searchParams.getAll('port').length !== 1) {
      reject('404 Not Found'); return;
    }
    const destination = routes.get(`${url.searchParams.get('host')}:${url.searchParams.get('port')}`);
    if (!destination) { reject('403 Forbidden'); return; }
    const origin = request.headers.origin;
    // Native clients omit Origin; browser clients must match an explicit origin.
    if (origin ? !origins.has(origin) : !config.allowNative) { reject('403 Forbidden'); return; }
    const peer = ipv4(socket.remoteAddress);
    let clientIP = peer;
    if (trustedProxies.has(peer)) {
      // The reverse proxy must overwrite this header with ONE verified address.
      clientIP = request.headers['x-forwarded-for'];
      if (typeof clientIP !== 'string' || net.isIP(clientIP) !== 4) { reject('400 Bad Request'); return; }
    }
    if (net.isIP(clientIP) !== 4) { reject('400 Bad Request'); return; }
    const tcp = net.createConnection({ host: '127.0.0.1', port: destination });
    tcp.pause();
    tcp.setNoDelay(true);
    tcp.setTimeout(5000);
    let ws;
    let heartbeat;
    let alive = true;
    let stopped = false;
    const cleanup = () => {
      if (stopped) return;
      stopped = true;
      clearInterval(heartbeat);
      tcp.destroy();
      if (ws) ws.terminate();
      else socket.destroy();
    };
    tcp.on('error', cleanup);
    tcp.on('timeout', cleanup);
    socket.once('close', cleanup);
    tcp.once('connect', () => {
      if (stopped || socket.destroyed) { cleanup(); return; }
      tcp.setTimeout(0);
      tcp.write(`PROXY TCP4 ${clientIP} 127.0.0.1 ${socket.remotePort} ${destination}\r\n`);
      wss.handleUpgrade(request, socket, head, connected => {
        ws = connected;
        socket.setTimeout(0);
        ws.on('error', cleanup);
        ws.on('close', cleanup);
        ws.on('pong', () => { alive = true; });
        ws.on('message', (bytes, binary) => {
          if (!binary || tcp.writableLength + bytes.length > MAX_QUEUE) { cleanup(); return; }
          if (!tcp.write(bytes)) ws.pause();
        });
        tcp.on('drain', () => { if (ws.readyState === WebSocket.OPEN) ws.resume(); });
        tcp.on('data', bytes => {
          if (ws.readyState !== WebSocket.OPEN || ws.bufferedAmount + bytes.length > MAX_QUEUE) {
            cleanup(); return;
          }
          // One outstanding TCP chunk at a time bounds memory for a slow browser.
          tcp.pause();
          ws.send(bytes, { binary: true }, error => error ? cleanup() : tcp.resume());
        });
        tcp.on('end', () => ws.close(1000, 'Server disconnected'));
        tcp.on('close', () => { if (!tcp.readableEnded) cleanup(); });
        heartbeat = setInterval(() => {
          if (!alive) { cleanup(); return; }
          alive = false;
          ws.ping();
        }, 30000);
        heartbeat.unref();
        tcp.resume();
      });
    });
  });
  return {
    server,
    async close() {
      for (const socket of sockets) socket.destroy();
      await new Promise(resolve => server.close(resolve));
      wss.close();
    },
  };
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const config = JSON.parse(await readFile(process.argv[2] ?? 'config.json', 'utf8'));
  const gateway = createGateway(config);
  gateway.server.listen(config.port ?? 8080, config.bind ?? '127.0.0.1', () => {
    console.log(`DarkEden gateway listening on ${config.bind ?? '127.0.0.1'}:${config.port ?? 8080}`);
  });
  for (const signal of ['SIGINT', 'SIGTERM']) process.once(signal, () => gateway.close());
}
