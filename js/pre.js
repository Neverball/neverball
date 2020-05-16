(function () {
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


  /**
   * Detect browser back navigation via history states.
   */
  function initBackButton() {
    window.history.replaceState({initial: true}, null);
    window.history.pushState({initial: false}, null);

    window.addEventListener('popstate', function (event) {
      if (event.state && event.state.initial) {
        // Notify Neverball via our entry point.
        Module._push_user_event(-1);

        if (!Module['neverball'].isTitleScreen) {
          // Re-add sentinel immediately.
          window.history.pushState({initial: false}, null);
        } else {
          // User probably wants to leave.
          window.history.back();
        }
      }
    });

    // Also check and re-add sentinel on user action.
    Module['canvas'].addEventListener('click', function (event) {
      if (window.history.state && window.history.state.initial) {
        window.history.pushState({initial: false}, null);
      }
    });
  }

  Module['neverball'] = {
    isTitleScreen: false
  };

  initBackButton();

  Module['preInit'] = [initPersistentStore];
})();
