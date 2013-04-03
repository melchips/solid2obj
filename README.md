solid2obj
=========

Wolfire's Black Shades solid file converter to obj file

Current status
--------------

Supported :

* Converting mesh to obj file (vertices and triangles only)

Unsupported :

* color of each face
* textures (solid files don't support them in their state)
* conversion from obj to solid file

Usage
-----

    solid2obj <input_file> <output_file>

    input file  :   a valid solid mesh file
    output_file :   name of the output obj file to create

**WARNING** output file WILL be **OVERWRITTEN** !

Building
--------

On GNU/Linux:

    $ make

Links
-----

http://www.wolfire.com/black-shades
