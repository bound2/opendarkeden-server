import assert from 'node:assert/strict';
import { once } from 'node:events';
import net from 'node:net';
import { test } from 'node:test';
import { WebSocket } from 'ws';
import { createGateway } from './gateway.mjs';

async function fixture(t, extra = {}) {
  const received = [];
  const peers = new Set();
  const backend = net.createServer(socket => {
    peers.add(socket);
    socket.once('close', () => peers.delete(socket));
    let pending = Buffer.alloc(0);
    let headerRead = false;
    socket.on('data', bytes => {
      if (headerRead) { socket.write(bytes); return; }
      pending = Buffer.concat([pending, bytes]);
      const end = pending.indexOf('\r\n');
      if (end < 0) return;
      received.push(pending.subarray(0, end).toString());
      headerRead = true;
      const payload = pending.subarray(end + 2);
      if (payload.length) socket.write(payload);
    });
  });
  backend.listen(0, '127.0.0.1');
  await once(backend, 'listening');
  const gateway = createGateway({ origins: ['https://game.example'], allowNative: true,
    routes: { '192.0.2.2:9999': backend.address().port, '192.0.2.3:9998': backend.address().port }, ...extra });
  gateway.server.listen(0, '127.0.0.1');
  await once(gateway.server, 'listening');
  t.after(async () => {
    await gateway.close();
    for (const socket of peers) socket.destroy();
    await new Promise(resolve => backend.close(resolve));
  });
  return { received, url: `ws://127.0.0.1:${gateway.server.address().port}/game?host=192.0.2.2&port=9999` };
}

test('binary stream survives WebSocket fragmentation, arbitrary chunks, and reconnect handoffs', async t => {
  const { url, received } = await fixture(t);
  for (const endpoint of [url, url.replace('192.0.2.2', '192.0.2.3').replace('9999', '9998'), url]) {
    const ws = new WebSocket(endpoint, 'binary', { origin: 'https://game.example' });
    await once(ws, 'open');
    const result = [];
    let count = 0;
    const bytes = Buffer.from(Array.from({ length: 200000 }, (_, i) => i & 255));
    const complete = new Promise(resolve => ws.on('message', (data, binary) => {
      assert.equal(binary, true);
      result.push(data);
      count += data.length;
      if (count === bytes.length) resolve();
    }));
    ws.send(bytes.subarray(0, 7), { binary: true, fin: false });
    ws.send(bytes.subarray(7, 70000), { binary: true, fin: true });
    ws.send(bytes.subarray(70000), { binary: true });
    await complete;
    assert.deepEqual(Buffer.concat(result), bytes);
    ws.close();
    await once(ws, 'close');
  }
  assert.equal(received.length, 3);
  assert.ok(received.every(line => /^PROXY TCP4 127\.0\.0\.1 127\.0\.0\.1 \d+ \d+$/.test(line)));
});

test('only an explicit trusted reverse proxy can supply the original IPv4 address', async t => {
  for (const trusted of [false, true]) {
    await t.test(String(trusted), async t => {
      const { url, received } = await fixture(t, { trustedProxies: trusted ? ['127.0.0.1'] : [] });
      const ws = new WebSocket(url, 'binary', { headers: { 'X-Forwarded-For': '203.0.113.44' } });
      await once(ws, 'open');
      const echo = once(ws, 'message');
      ws.send(Buffer.from([1]));
      await echo;
      assert.ok(received[0].startsWith(`PROXY TCP4 ${trusted ? '203.0.113.44' : '127.0.0.1'} `));
      ws.close();
      await once(ws, 'close');
    });
  }
});

test('denies unknown destinations, browser origins, ambiguous forwarding, and text payloads', async t => {
  const { url } = await fixture(t);
  for (const [endpoint, options] of [
    [url.replace('9999', '22'), {}], [url, { origin: 'https://attacker.example' }],
    [url + '&port=9999', {}], [url.replace('/game?', '/other?'), {}],
  ]) {
    const ws = new WebSocket(endpoint, 'binary', options);
    await assert.rejects(once(ws, 'open'));
  }
  const ws = new WebSocket(url, 'binary');
  await once(ws, 'open');
  const closed = once(ws, 'close');
  ws.send('text is not the game protocol');
  await closed;
});

test('rejects an oversized frame and closes both sides', async t => {
  const { url } = await fixture(t);
  const ws = new WebSocket(url, 'binary');
  await once(ws, 'open');
  const closed = once(ws, 'close');
  ws.send(Buffer.alloc(1024 * 1024 + 1));
  await closed;
});
