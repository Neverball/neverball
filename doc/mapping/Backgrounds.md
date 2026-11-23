# Backgrounds

This is just an updated version of a post rlk wrote back in 2006. It serves dual purpose: it documents background creation as well as the *info_null* entity as it relates to backgrounds. This is a must-read for anyone wanting to gain an understanding of billboards and their usage, whether they intend to create a new background, a new ball that uses billboards or add custom billboards to their level.

***

Ah the backgrounds.  How I love them.  I had so much fun doing that.  I really think they complete the look of the game.  The background system is simple, but there is huge potential for artistic expression there.

The key to understanding backgrounds is to hit F6 and look at them in wireframe.  This shows you that a background is nothing but a set of small sprites positioned, rotated, scaled, and blended with each other.  Some are placed randomly, some are animated.  This allows a great deal of visual detail to be represented at high resolution using a very small amount of image data.

Despite the fact that a background is simply a .MAP file editable in Radiant, all of the background files were automatically generated.  This includes the evenly-spaced mountains and clouds of clouds.map, the random distribution of stars in jupiter.map and alien.map, and the scattering of waves in ocean.map and alien.map.  This code is long lost, as I considered it disposable from the outset.  The point is that anybody looking to define new backgrounds is going to have to do some programming.

A background is a collection of billboards.  A billboard is represented in a .MAP as an info_null entity.  This entity has several custom properties:

"dist" is the distance from the center of the world to the billboard.  I used a value of 250 most of the time.

"flag" is a bitmap setting display options for each billboard.  A 1 indicates the billboard is positioned relative to its bottom edge, like a cloud on the horizon, rather than its center, like stars.  A 2 indicates the billboard lies flat, like the streets of the city, rather than facing the viewer.

"image" selects the image.  Duh.

"xrot", "yrot", and "zrot" are where the magic happens.  Each of these is a vector of 3 angles that determine where the billboard is located in the sky.  The X angle varies from -90 to 90 and determines how high off the horizon the billboard is position.  The Y angle varies from 0 to 360 and gives the compass direction to the billboard (0 degrees is due south).  The Z angle gives the rotation of the billboard about its center.  The first value of the 3-vector gives a static rotation about the X, Y, or Z axis.  The second value gives a dynamic rotation value that varies linearly with time.  The third value gives a dynamic rotation value that varies quadratically with time.  These dynamic values are what produce the animation.  In effect, the first value gives position, the second value gives velocity, and the third value gives accelleration.  Every background animation in the game is the result of these values.

"height" and "width" give the size of the billboard, which also implies the desired aspect ratio of the image.  At a distance of 250, a billboard needs to be around 100 in extent to be reasonably scaled.  Width and height are 3-component vectors just as the rotation values.  This allows the size and shape of a billboard to vary dynamically with time, linearly or quadratically.

"time" gives the time in seconds before the animation repeats.

There are a couple of tricks.  For example, ocean waves seem to appear and disappear.  In reality, their height varies quadratically with time and so they have negative height for much of their animation cycle.  A billboard with negative width or height faces away from the viewer, thus it cannot be seen.  Also, the little jet plane in clouds.map appears to circle endlessly.  In reality its animation cycle length is tuned to reset just as it reaches 360 degrees, so no discontinuity is visible.  In addition, good billboarding is all about blending, and good blending is all about render order.  Billboards are rendered in the order in which they appear in the file.

I hope that's everything.  After having typed all this in, I realize it might be good to put on the wiki.

I'm looking forward to seeing what you come up with.

Oh I should mention the OTHER key to dealing with backgrounds is F5.  This will freeze the game and let you mouselook around.  This is significant, as it is generally not possible to see all of the background easily with the in-game camera.  Remember thought that background areas which are commonly out of view can come into view on mirror levels.  For example, the stars in the sky of the alien background.
