const cacheName = 'neverball-v9';

const urls = [
  '/',
  '/index.html',
  '/neverball.data',
  '/neverball.js',
  '/neverball.wasm',
  '/screenshot.png',
  '/favicon.svg',
];

async function installWorker(event) {
  const cache = await caches.open(cacheName);
  return cache.addAll(urls);
}

async function serveCachedResponse(event) {
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
  self.skipWaiting();

  event.waitUntil(installWorker(event));
});

self.addEventListener('fetch', event => {
  const url = new URL(event.request.url);

  if (urls.includes(url.pathname)) {
    event.respondWith(serveCachedResponse(event));
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
    }),
  );
});