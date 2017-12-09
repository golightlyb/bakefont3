# bakefont3 #

*bakefont3* is a python3 module for rasterising font glyphs into a RGBA texture
atlas.

![Example](./docs/example.png)

## TODO ##

* kerning tables aren't saved yet
* haven't tested anything yet
* docs
* clean up


## Features ##

* Unicode support
* customisable character ranges
* exports font metrics and kerning data
* efficient packing to reduce physical texture size
* uses all four colour channels to share space with other RGBA textures
* supports square and rectangle texture atlases of any size
* metrics accurate up to 1/64th of a pixel (e.g. for supersampling)


## Example Usage ##

### example-generate.py

This Python script demonstrates how to define the set of characters you want
and the font faces and font sizes you want, then generate a PNG texture atlas as
and a bakefont3 data file.

    # generates test.bf3 and test.png
    python3 example-generate.py test

### example-write.c

**TODO TODO TODO**

This program demonstrates loading and parsing a bakefont data file in C, then
using it to write some text and display it on screen using GLFW3.

    # Select a "Sans Serif" size 14 font generated earlier,
    # read text from stdin,
    # and render it to out.png

    cat text.txt | ./example-write test.png test.bf3 "Sans Serif" 14 out.png



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

