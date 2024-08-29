const cacheName = 'neverball-v1';

const urls = [
  '/index.html',
  '/favicon.ico',
  '/favicon.svg',
  '/icon_194.png',
  '/neverball.data',
  '/neverball.js',
  '/neverball.wasm',
  '/screenshot.png',
];

async function installWorker(event) {
  const cache = await caches.open(cacheName);
  return cache.addAll(urls);
}

async function serveResponse(event) {
  let response = await caches.match(event.request);

  if (response) {
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
  event.respondWith(serveResponse(event));
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
    }),
  );
});