/**
 * Initialize persistent store.
 *
 * Trying to do this as early as possible to sidestep the async nature of it.
 */
function initPersistentStore() {
  FS.mkdir('/neverball');
  FS.mount(IDBFS, {}, '/neverball');
  console.log('Synchronizing from persistent storage...');
  FS.syncfs(true, function (err) {
    if (err)
      console.error('Failure to synchronize from persistent storage: ' + err);
    else
      console.log('Successfully synced from persistent storage.');
  });
}

Module['preInit'] = [initPersistentStore];
