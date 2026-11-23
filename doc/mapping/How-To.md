# Level Creation Howto

## Requirements

  * Neverball installation
  * **Radiant**, the level design tool. Recommended: [NetRadiant](http://ingar.satgnu.net/gtkradiant/)

## Quake mapping / Neverball mapping

For those familiar with Quake engine mapping, go ahead and forget most of it. Neverball mapping has significantly fewer restrictions than Quake mapping. 

Some important differences are as follows: 

  * The scale differs. In Quake there are 8 units to a foot. In Neverball there are 64 units to a meter. The default major grid in radiant marks 64 units, so meters are easy to work with. 
  * Neverball maps need not be closed. Neverball environments are generally entirely visible all of the time, so all the attention paid to visibility by Quake becomes unnecessary. Maps don't leak. 
  * Brushes are referred to as "lumps". They may overlap. In fact, it is sometimes advantageous to overlap lumps. 
  * Shaders do not modify geometry. The textures you apply are the textures that appear in Neverball. 
  * No curves. Neverball physics doesn't consider them. See [Curve.c](Curve.md) for info on generating curves in Radiant.
  * Most of Quake's entities are meaningless in Neverball. A few of them are used, but in modified form. See [Entities](Entities.md). 
  * No lights. All lighting is dynamic.

## Creating Brushes

Brushes are handled primarily with the mouse buttons, see below in the [Shortcuts](#shortcuts) section. Create rectangular brushes (blocks) by dragging the mouse. When you create a brush, it's initially 6 sided. In the "Brush" menu, you can change it into a prism, sphere or cone.

## Editing Brushes

Reshape brushes (blocks) with the clipper tool (xyz icon). This truncates brushes, helpful when forming non-rectangular shapes.

Manipulate brushes by moving edges or vertices:

  * Press E to modify the edges, click and drag points to move the edges.
  * Press V to modify the vertices, as described above.

Radiant does not allow creation of concave brushes.

Use numerical keys "1 - 9" to change grid resolution. By default, all points snap to the grid. Deactivate this feature in the "Grid" menu if desired.

Rotate brushes in the menu "Selection", "Flip", "Rotate" or "Arbitrary Rotation".

Merge two or more brushes into one (if allowed) in the menu "Selection", "CGS", "CGS Merge".

One may also substract, but exercise caution with this feature as it can do strange things, particularly with curves.

Please refer to the [Editing recommendations](Editing.md) for instructions on what **NOT TO DO!** and advanced techniques.

## Textures

Apply textures to brush faces (only visible surfaces need textures, all other surfaces should have the invisible texture applied - (appears as white with green and red stripes in Radiant).

Add textures by selecting the desired faces, click a texture in the texture list or alternatively press T to open the texture menu and select.

If no textures appear, you have forgotten to load them. Menu "Textures", "mtrl".

Paint a map's first brush with 'invisible' texture. From then on, all new brushes will be textured 'invisible'. **Only apply textures on faces that will be visible**. This will simplify rendering for the final compiled .map file.

If the textures are not placed as desired, press S to launch the surface inspector.

Adjust the texture's placement by offsetting the H and V shift.

Increase/decrease the texture's size by adjusting the 'scale' value.

The increment values are the numbers increased/decreased when clicking on the arrows.

It is recommended to align brushes to the grid for easier texture placement. See [Editing recommendations](Editing.md) for details about texture placement.

See [Adding Material](Material.md) section if you want create new textures.

## Entities

Create [entities](Entities.md) with a right click in the desired location, then select the entity name from the contextual menu.

Press `N` to open the entities menu, and set up your entity; to add a key, type its name, its value and press enter.

All brushes are entities of classname "worldspawn".


## <a name="shortcuts"></a>Shortcuts

### 3-Button Wheel Mouse

#### Left Mouse Button

The left mouse button is used to manipulate brushes. A single click does nothing...
  * _If you click in empty space in a 2D view and nothing is selected:_  dragging the mouse creates a brush (6 sides).
  * _If you click inside a selected brush:_  dragging the mouse moves the brush.
  * _If you click outside a selected brush:_  dragging the mouse resizes the selected brush.

#### Right Mouse Button

  * _If you click in a 2D view:_  drag mouse to pan the view.
  * _If you click in a 3D view:_  a single click puts you in "navigation mode". Turn view with the mouse, and move in/out with mouse wheel and arrows, right click again to exit this mode.

#### Middle Mouse Button

  * _If you cilck in a 2D view:_  dragging the mouse moves the camera direction (not very usefull).

#### Mouse Wheel

  * In all views it zooms in & out.

### Keyboard

There are many shortcuts, a few important ones:

| Keys | Action |
| :--- | :----- |
| `Escape` | deselect all |
| `Space` | duplcate selected brush(es) |
| `E` | change selected brush(es) to edges editing mode. You can manipulate the brush edges |
| `V` | change selected brush(es) to vertex editing mode. You can manipulate the brush vertices |
| `X` | select the 'Clipper' tool for cutting and reshaping rectangles |
| `N` | open the "entites" dialog (see above) |
| `S` | launch "Surface inspector" dialog. Used to adjust textures on brush faces (see above) |
| `Backspace` | delete the selected brush(es) |
| `Ctrl` + `G` | snap to grid |

### Keyboard + Mouse

| Keys | Action |
| :---- | :------ |
| `Shift` + `left click` | select/deselect a brush |
| `Ctrl + shift` + `left click` | select a face in 3D view |
| `Ctrl + shift + alt` + `left click` | select/deselect several faces in 3D view |


You can also have a look at the original radiant [command reference](http://web.archive.org/web/20090620063403/http://zerowing.idsoftware.com/files/radiant/docs/1.5/commands.html) or [Shortcut keys and mouse functions of GTKRadiant](http://icculus.org/gtkradiant/documentation/q3radiant_manual/appndx/sskey_dl.htm)

## Compiling maps

Assume the map is named funkyball.map. Process it into a .sol with the following command. The first item specified is the input .map from Radiant, the second gives the location of the mtrl directory.

    $mapc funkyball.map ~/neverball/data
    
 Output should be similar to the following: 
 
    mtrl  vert  edge  side  texc  geom  lump  path  node
    10   552  1086   130   277   854   132     0    33
    body  coin  goal  view  jump  swch  ball  char  indx
    1    18     1     1     0     0     1    84  4286   115

## Adding maps to Neverball

Neverball levels are organized into sets which are defined in .txt files in the "data/" directory. These text files are a bit complex, so take example on the existing files of the initial sets. Any files referenced in these text files must reside in the "data/" diretory. The main directories in "data" are by convention:

    data/
    |
    +--- back/                     (static backgrounds)
    |
    +--- bgm/                      (music)
    |
    +--- map-name_of_the_set/      (compiled maps)
    |
    +--- map-back/                 (animated backgrounds)
    |
    +--- shot-name_of_the_set/     (screenshots)

### Set File

The set file works differently in 1.5.4 and later than in earlier versions.

#### For 1.5.4

You have a file called "sets.txt". It may be referred to as the "master set file" as it links to all the other sets.

It contains one line each containing a link to an existing set file. A new set may be established simply by making a new set file and typing it full name in the master set file.

The format for each set file is as follows:

    {%SETNAME%}
    {%DESCRIPTION%}
    {%SETSHOT%}
    {%HARD_TIMES%} {%MEDIUM_TIMES%} {%EASY_TIMES%} {%HARD_COINS%} {%MEDIUM_COINS%} {%EASY_COINS%}

Subsequent lines are following by a link to each level using the full name relative to the _data_ directory.

### Level shots

A level shot is a screen-shot of a level without the ball or any coins visible. It is used to preview a level in the level selection screen. There are two ways to acquire a level shot: per level, and per set.

To acquire a single level shot, press F12 at the level's intro screen. This will clear the HUD and all entities. Then, press F10 to take a screen-shot normally. Rename the screen-shot as needed and prepare it as described below. 

To capture level shots for an entire set, go to the set's selection screen and press F12. This will iterate through all levels and automatically take a screen-shot of each. The resulting BMP files will have the same name as the source SOL file as listed in the levels.txt file. 

_NOTE: If the SOL file has the name "sol/foo.sol" then the resulting screen-shot will have the name "sol/foo.bmp". Thus, if your normal screen-shot directory is your home directory, you must ensure that $(HOME)/sol exists and that the level shots may be written there._

To appear properly in level selection screen, a level shot must have power-of-two size. Screen-shots generally don't, thus they must be non-uniformly scaled. 256x256 in jpg format is recommended. Both the size and format of the image are optional. 

The author uses the following ImageMagick command to convert an entire set of level shots to jpg. 

    mogrify -resize 256x256! -format jpg -quality 95 *.png
