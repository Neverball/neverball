# Adding Materials

Neverball uses the standard OpenGL lighting model rather than a script-based shading system like Quake's. So, to add a new material you must create both a texture image and an OpenGL material specification. 

1. The texture must be an image in either JPG or PNG format. In general, use PNG for images that require an alpha channel, and JPG otherwise. Image width and height must both be powers of two. For this discussion, let us assume you have an image named `wood.jpg`. Place in it `textures/mtrl/`. 

2. Create a material file in `textures/mtrl/`, in this case `textures/mtrl/wood`. This name must match the texture name, minus the image suffix. The file must contain a material specification, explained below. If you don't care about the material properties, the following values suffice in most any case.

```
diffuse   0.8 0.8 0.8 1.0
ambient   0.2 0.2 0.2 1.0
specular  0.0 0.0 0.0 1.0
emissive  0.0 0.0 0.0 1.0
shininess 0.0
flags     shadowed lit
angle     45.0
```

Do a "Flush & Reload Shaders" in Radiant and your new material should appear among the mtrl shaders. Use the F7 key in Neverball to reload materials from material files without having to recompile your map.

Those directives define the material in terms of the standard OpenGL material model. They define not only the color of the material, but how it reacts to different types of lighting and different points of view. Their meaning is a follows: 

* `diffuse` gives the diffuse material color (R G B A). This is the basic color of the material resulting from directional light falling upon it. The fourth component is the alpha channel. 0 is transparent, 1 is opaque. 
* `ambient` gives the ambient material color. It gives the color of the material resulting from non-directional light. It doesn't make much of a contribution in practice. 
* `specular` gives the specular material color. This is the color of the material when directional light is glinting directly off of it into the eye of the user. A bright white here will make an object look shiny, while a darker color will give the object a matte appearance. 
* `emissive` gives the emissive material color. This color contributes to the appearance of the object regardless of the light falling upon it. A bright color here will make an object appear to glow in darkness, or at least appear to be unaffected by external light. 
* `shininess` gives the material shininess. Simply put, it defines the size of the specularity on an object. 0 will produce a dull finish, while 128 will produce a very tight specularity. 

The five of these together determine the base color of the material. This base color is then blended with (added to) the texture color. To disable lighting computation for a given material (e.g., for a billboard material), omit the `lit` material flag, in which case the above values have no effect.

* `flags` gives the material flags. Multiple flags may be specified, separated by spaces. The recognized material flags are enumerated below.
* `angle` gives the cutoff angle for lump smoothing. Surfaces of the same material that share an edge with a crease angle below this value will have their surface normals adjusted for lighting computations, making the surface transition appear smoother. A value of 0 will disable lump smoothing.
* `alpha-test <func> <value>` turns on alpha testing and gives a comparison function and a value to compare against. Pixels with an alpha value that fails the comparison are not rendered. For example, `alpha-test gequal 0.5` only renders pixels with alpha value greater or equal to 0.5. All of the standard comparison functions are available: `always`, `equal`, `gequal`, `greater`, `lequal`, `less`, `never`, `notequal`.

## Material Flags

Flag          | Description
--------------|------------
`additive`    | Additively blended material
`clamp-s`     | Material texture is non-repeating horizontally
`clamp-t`     | Material texture is non-repeating vertically
`decal`       | Surface decals are automatically offset from the base surface
`environment` | Environment-mapped material
`reflective`  | Reflective material
`shadowed`    | Shadow-receiving material
`transparent` | Transparent or translucent material
`two-sided`   | Two-sided material visible from behind
`particle`    | Meshes using materials of this type are rendered as a series of point sprites
`lit`         | Enable lighting for this material
