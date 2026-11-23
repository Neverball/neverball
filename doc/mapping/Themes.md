# Themes

Neverball features a themeable GUI, which allows designers to give custom look and feel to GUI elements.

The default Neverball GUI theme can be found in `data/gui/classic`.

Theme selection is controlled by the `theme` option in the `neverballrc` configuration file.

## Images

Backgrounds for GUI elements are defined by four images, each representing a GUI state.

* `back-plain.png` gives the default background for all elements.
* `back-plain-focus.png` gives the background for focused elements, for example, when the cursor is hovering over a button.
* `back-hilite.png` gives the background for highlighted elements, such as a toggle button that has been enabled.
* `back-hilite-focus.png` gives the background for focused *and* highlighted elements.

## Theme description

The way in which these images are displayed is defined in a file called `theme.txt`. Currently it contains a single directive.

    slice 0.25 0.25 0.25 0.25

The `slice` directive defines what portion of the GUI images should be used for each border using four parameters. These parameters are ordered left, right, top then bottom, and defined in values between `0.0` (none of the image) and `1.0` (the entirety of the image).
