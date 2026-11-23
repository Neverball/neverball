# Setup

## Setting up the level editor on Windows

Here's a quick guide on setting up everything you need to start making levels.

1. Grab the [latest Neverball build][neverball]. Make sure it works.
2. Visit [Ingar's NetRadiant site][radiant] and grab an appropriate NetRadiant build. Get the 32-bit build, as the 64-bit build is unstable and tends to crash.
3. Install the Neverball gamepack for Radiant: Navigate to the `NeverballPack` folder in `neverball`, select `games` and `neverball.game` and copy those two to your `netradiant` folder.
4. Launch NetRadiant using `radiant.exe`. In the *Global Preferences* window that appears select *Neverball*, then, if asked, set the engine path to the location of your `neverball` directory. If you don't receive the prompt, you should set the engine path later, in *Edit->Preferences->Settings->Paths*. At this point Radiant should launch. If it crashes, which may happen on occasion, just start it again. You may also want to disable desktop composition in `radiant.exe` properties, which seems to resolve a few visual issues and may help with the crashes.

To confirm that things are working, at this point you can hit *Ctrl+S* to save the (currently empty) map. Make sure to save it under `Documents\My Games\Neverball\`, for instance, as `Documents\My Games\Neverball\Maps\empty.map`. Then, in the *Build* menu, select *Neverball: compile+play*. Ignore any firewall and BSP monitoring warnings. Neverball should launch and you should arrive at a black screen that says *Standalone level*, which is your completely empty map. If you get this far, everything is set up properly and you're ready to start mapping.

[neverball]: http://neverball.org/download.php
[radiant]: http://ingar.intranifty.net/gtkradiant/
