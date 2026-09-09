const cacheName = 'neverball-@BUILD_VERSION@';

const urls = [
  '/',
  '/index.html',
  '/neverball.data',
  '/neverball.js',
  '/neverball.wasm',
  '/anim.mp4',
  '/anim.jpg',
  '/favicon.svg',
];

async function installWorker(event) {
  const cache = await caches.open(cacheName);

  for (const url of urls) {
    const response = await fetch(url, { cache: 'reload' });
    if (!response.ok) {
      throw new Error(`Failed to fetch ${url}: ${response.statusText}`);
    }

    // GitHub Pages (Fastly) naively gzips large .data archives (application/octet-stream),
    // sending Content-Encoding: gzip with the compressed Content-Length.
    // In Chromium, passing this decompressed streaming response directly into cache.put()
    // triggers an internal stream length mismatch (net::ERR_FAILED), aborting installation.
    // Buffering via arrayBuffer() and removing Content-Encoding and Content-Length ensures
    // CacheStorage writes clean uncompressed bytes and prevents Firefox from attempting
    // double-decompression (NS_ERROR_CORRUPTED_CONTENT).
    const buffer = await response.arrayBuffer();
    const headers = new Headers(response.headers);
    headers.delete('content-encoding');
    headers.delete('content-length');

    const cachedResponse = new Response(buffer, {
      status: response.status,
      statusText: response.statusText,
      headers: headers,
    });

    await cache.put(url, cachedResponse);
  }
}

async function serveCachedResponse(event) {
  let response = await caches.match(event.request);

  if (response) {
    const rangeHeader = event.request.headers.get('range');
    const matches = rangeHeader && rangeHeader.match(/^bytes=(\d+)-(\d+)?$/);

    // Safari requires HTTP 206 Partial Content for media range requests.
    if (matches) {
      const buffer = await response.arrayBuffer();
      const start = parseInt(matches[1], 10);
      const end = matches[2] ? parseInt(matches[2], 10) : buffer.byteLength - 1;

      if (start < buffer.byteLength && end < buffer.byteLength && start <= end) {
        const chunk = buffer.slice(start, end + 1);
        return new Response(chunk, {
          status: 206,
          statusText: 'Partial Content',
          headers: {
            'Content-Range': `bytes ${start}-${end}/${buffer.byteLength}`,
            'Content-Length': String(chunk.byteLength),
            'Content-Type': response.headers.get('Content-Type') || 'video/mp4',
            'Accept-Ranges': 'bytes',
          },
        });
      }
    }

    return response;
  }

  response = await fetch(event.request);

  if (response.ok) {
    const cache = await caches.open(cacheName);
    cache.put(event.request, response.clone());
  }

  return response;
}

self.addEventListener('install', event => {
  event.waitUntil(installWorker(event));
});

self.addEventListener('fetch', event => {
  const url = new URL(event.request.url);

  if (urls.includes(url.pathname)) {
    event.respondWith(serveCachedResponse(event));
  }
});

self.addEventListener('message', event => {
  if (event.data && event.data.type === 'SKIP_WAITING') {
    self.skipWaiting();
  } else if (event.data && event.data.type === 'GET_VERSION') {
    if (event.ports && event.ports[0]) {
      event.ports[0].postMessage({ version: cacheName });
    }
  }
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then(cacheNames => {
      return Promise.all(
        cacheNames.map(key => {
          if (key === cacheName) {
            return;
          }
          return caches.delete(key);
        }),
      );
    }).then(() => self.clients.claim())
  );
});