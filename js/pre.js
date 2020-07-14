(function () {
  /**
   * Set up the user folder with a backing store.
   *
   * Adding as run dependency, because this is an async operation.
   */
  function initUserFolder() {
    // Create the user folder and mount IndexedDB on it.
    FS.mkdir('/neverball');
    FS.mount(IDBFS, {}, '/neverball');

    // Tell Emscripten to wait for us.
    Module.addRunDependency('neverball:user-folder');

    console.log('Synchronizing from backing store...');

    FS.syncfs(true, function (err) {
      if (err)
        console.error('Failure to synchronize from backing store: ' + err);
      else
        console.log('Successfully synced from backing store.');

      // Tell Emscripten to stop waiting.
      Module.removeRunDependency('neverball:user-folder');
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

  if (Module['preRun'] === undefined) {
    Module['preRun'] = [];
  }
  Module['preRun'].push(initUserFolder);
})();
