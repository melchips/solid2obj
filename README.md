solid2obj
=========

Wolfire's Black Shades solid file converter to obj file

Screenshots
-----------

Screenshots of game models imported successfully in blender :

![Image](screenshots/blender_sniper_rifle.png?raw=true)

![Image](screenshots/blender_glock.png?raw=true)


Current status
--------------

Supported :

* Converting solid file to OBJ file (vertices, triangles and corresponding colors)
* Exporting colors to a mtl file
* Conversion from obj to solid file

Unsupported :

* QUADS unsupported in OBJ files (using [blender](http://www.blender.org/), the option 'Triangulate Faces" has to be checked in the OBJ export options panel)
* textures (solid files don't support them in their state)

Limitation(s) :

* solid meshes can't have more than 1200 vertices and 400 triangles (without altering and recompiling the Black Shades sourcecode)

Usage
-----

### solid -> obj (with 3 args)

    ./solid2obj <input_solid_file> <output_obj_file> <output_mtl_file>

    input_solid_file    :   a valid solid mesh file
    output_obj_file     :   name of the output obj file to create
    output_mtl_file     :   name of the output material file to create

**! WARNING** output files **WILL** be **OVERWRITTEN !**

### obj -> solid (with 2 args)

    ./solid2obj <input_obj_file> <output_solid_file>

    input_obj_file      :   a valid obj mesh file
    output_solid_file   :   name of the output solid file to create

**! WARNING** output file **WILL** be **OVERWRITTEN !**


Building
--------

### On GNU/Linux:

Version for GNU/Linux

    $ make


Version for Windows
    
    $ make -f Makefile.win32


Links
-----

[Wolfire's Black Shades page](http://www.wolfire.com/black-shades)

[Black Shades modding thread](http://forums.wolfire.com/viewtopic.php?f=2&t=15294)
