# bakefont3 #

*bakefont3* is a python3 module for rasterising font glyphs into a texture
atlas.

TODO 90% complete


## Features

* Unicode support
* customisable character ranges
* exports kerning and glyph metrics
* efficient packing to reduce physical texture size
* uses all four colour channels to share space with other RGBA textures


## Example Usage

See `example.py`, which drives the module.



## Dependencies ##

**freetype-py - Python binding for the freetype library**

    git clone https://github.com/rougier/freetype-py
    cd freetype-py
    sudo python3 ./setup.py install

**pillow - The friendly PIL fork**

    git clone https://github.com/python-pillow/Pillow.git
    cd Pillow
    sudo python3 ./setup.py install

