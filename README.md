# bakefont3 #

*bakefont3* is a python3 module for rasterising font glyphs into a RGBA texture
atlas and exporting metrics and kerning information, and a C library for
loading that information.

![Example](./docs/example.png)

## TODO ##

* haven't tested anything yet
* haven't done example of reading the data yet
* docs


## Features ##

* Unicode support
* customisable character ranges
* exports font metrics and kerning data as binary optimised for quick lookup
* efficient packing to reduce physical texture size
* can use all four colour channels to share space with other textures (can
    export RGBA, RGB, greyscale)
* supports square and rectangle texture atlases of any size
* metrics accurate up to 1/64th of a pixel (e.g. for supersampling)


## Example Usage ##

### Generate a font atlas and metrics using bakefont3 ###

A sample script, `example-generate.py`, is provided. You may like to edit it
to select the fonts, sizes and characters to generate.

    $ python3 example-generate.py

This file generates the following files:

    test.bf3
    test-rgba.png
    test-r.png
    test-g.png
    test-b.png
    test-a.png

`test-rgba.png` is a texture atlas containing rasterised glyphs in each colour
channel. The other images show the individual channels, but you don't need them.

`test.bf3` contains information about the fonts used, how to find characters
in the texture atlas, metrics and kerning information for laying out text
on a screen.

### Draw text with files generated by bakefont3 ###

A sample program, `example.c` is provided. You may like to edit it to
change the font data it tries to load.

`bakefont3.h` and `bakefont3.c` are an example interface for loading and
querying bakefont3 data.

    $ # Compile
    $ gcc -std=c99 example.c bakefont3.h bakefont3.c -o example.bin
    # # Run
    $ ./example test.bf3 test-rgba.png


## Dependencies ##

### For generating fonts using bakefont3:

    * Roboto and Roboto Mono fonts from [fonts.google.com](https://fonts.google.com/)
    * Python3
    * libfreetype
    * Python3 modules Pillow, numpy, freetype-py

Example:

    sudo apt-get install libfreetype6
    sudo pip3 install Pillow numpy freetype-py

### For the C example program:

    * libpng
    * libglfw3
    * a C compiler (e.g. gcc, clang)

Example:

    sudo apt-get install libpng12-0


## Useful notes ##

**When talking about font faces and glyphs, what do terms like ascent, descent,
etc. mean?**

See [1](https://www.microsoft.com/typography/otspec/TTCH01.htm),
[2](https://www.freetype.org/freetype2/docs/tutorial/step2.html)
[3](https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html)

**Font sizes are given in pixels.**

Typographic dpi is always 72. At 72ppi, 1pt == 1px.
Convert as necessary for different devices.

**What is the "26.6 fixed float" format?**

This encodes a [Real number](https://en.wikipedia.org/wiki/Real_number) as a
signed 32 bit integer. Divide by 64.0 to get the Real value. Or, multiply
a Real value by 64.0 and cast to a signed 32 bit integer to encode it.

(Don't worry about this unless you are directly parsing Bakefont3 output or
interfacing with Freetype yourself. Freetype also has a "2.14 fixed float"
format).



## COPYING ##

    bakefont3

    Copyright © 2015 - 2017 Ben Golightly <golightly.ben@googlemail.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction,  including without limitation the rights
    to use,  copy, modify,  merge,  publish, distribute, sublicense,  and/or sell
    copies  of  the  Software,  and  to  permit persons  to whom  the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice  and this permission notice  shall be  included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
    IMPLIED,  INCLUDING  BUT  NOT LIMITED TO THE WARRANTIES  OF  MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS  OR COPYRIGHT HOLDERS  BE LIABLE  FOR ANY  CLAIM,  DAMAGES  OR  OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

