solid2obj
=========

Wolfire's Black Shades solid file converter to obj file

Current status
--------------

Supported :

* Converting mesh to obj file (vertices, triangles and corresponding colors)
* Exporting colors to a mtl file

Unsupported :

* textures (solid files don't support them in their state)
* conversion from obj to solid file

Usage
-----

    ./solid2obj <input_solid_file> <output_obj_file> <output_mtl_file>
    input_solid_file    :   a valid solid mesh file
    output_obj_file     :   name of the output obj file to create
    output_mtl_file     :   name of the output material file to create

**! WARNING** output files **WILL** be **OVERWRITTEN !**

Building
--------

On GNU/Linux:

    # Version for GNU/Linux
    $ make

    # Version for Windows
    $ make -f Makefile.win32


Links
-----

http://www.wolfire.com/black-shades
