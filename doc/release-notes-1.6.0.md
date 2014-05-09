# Neverball 1.6.0 release notes

After what's been a long, long wait since 1.5.4 released way back in 2009, we're finally releasing Neverball 1.6.0. The changes and improvements are too many to count, this is just a summary of everything that's been going on. (If you really miss the old change list, be sure to check out our [Github project][git].) Thanks to everyone who has contributed to this release, both those who have stuck around for all this time and those who just occasionally stop by to contribute something awesome.

## Highlights

* Neverball is using Git [on Github][git]!
* Oculus Rift support. This is a compile-time option. Precompiled Windows builds are included in the official download. HMD-enabled builds contain an option to toggle VR mode in the Options screen. Both the Oculus SDK (`make ENABLE_HMD=libovr`) and OpenHMD (`make ENABLE_HMD=openhmd`) backends are supported.
* An OpenGL ES-compatible renderer. All of the Neverball rendering code has been rewritten for OpenGL ES 1.1 compliance to ease porting efforts to mobile/embedded platforms.
* Port to SDL 2. Amongst other things, this fixes a number of fullscreen-related issues and vastly improves multiple display support.
* [Mapping documentation][mapping]!

[git]: https://github.com/neverball
[mapping]: https://github.com/Neverball/neverball-docs/blob/master/README.md

## Gameplay / controls

* Camera behavior tweaks
    * Configurable responsiveness of all three cameras via `camera_*_speed` options in *neverballrc*
    * Intelligent manual rotation behavior
* Gamepad handling improvements
    * Standardized controls
    * Reworked configuration options via `joystick_axis_*` and `joystick_button_*` options in *neverballrc*
* Configurable tilt responsiveness via `mouse_response` and `joystick_response` options in *neverballrc*
* Move default camera keys from F1-F3 to 1-3
* Automatically invalidate scores for older levels
* Adjustable replay playback speed using the mouse wheel
* Interpolate game state between updates for smooth display.
* Simulation and collision detection-related fixes

## Mapping

* Rotating platforms!
* *NeverballPack* gamepack for Radiant
    * Tailored defaults
    * Built-in entity documentation
    * Custom *Build* menu options
* Ability to load levels in standalone mode from the command line
* Alpha-tested materials
* Particle materials
* *mapc* output to the Radiant message window. This is a compile time option (`make ENABLE_RADIANT_CONSOLE=1`) and is enabled by default in the official builds.
* Recognize *.pk3* (a renamed *.zip*) as a package extension
* Limit entity timer precision to milliseconds to fix a number of timing-related bugs
* Fix mapc not to merge some planes, fixing a number of collision bugs

## Content updates

### Levels

* Countless highscore and bug fixes
* Some completely reworked levels
* Four all-new Nevermania levels
* Tones Levels, a level set by Ian Walker
* Golf Jambalaya, a golf course by vidski
* Reshaun's Easy-Putt, a golf course by Reshaun Francis

### Balls

* Rift Ball!
* Octocat Ball!
* Melon Ball
* Orange Ball
* Diagonal Ball
* Cheese Ball
* Catseye Ball
* UFO Ball
* Updated Eyeball
* Updated Blinky

## Localization

* Transifex translation integration
* Countless translation updates
* Language selection screen

## Other noteworthy changes

* On Windows, the user folder is created in *Documents\My Games\Neverball*
* Support for custom GUI themes. A GUI theme can be installed as any Neverball package and selected via the *theme* option in *neverballrc*.
* Hillcrest Labs Loop device support (`make ENABLE_TILT=loop`)
* A stdio-based backend for the VFS with no archive support and no PhysicsFS dependency (`make ENABLE_FS=stdio`)
* Lighting adjustments to improve visibility in levels
* Error logging to a text file in the user folder
* Incremental loading of replays
* Graphics performance improvements
