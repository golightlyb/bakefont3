# bakefont3 #

*bakefont3* is a python3 module for rasterising font glyphs into a RGBA texture
atlas.

![Example](./docs/example.png)

TODO data export too


## Features ##

* Unicode support
* customisable character ranges
* exports kerning and glyph metrics
* efficient packing to reduce physical texture size
* uses all four colour channels to share space with other RGBA textures
* supports square and rectangle texture atlases of any size


## Example Usage ##

See `example.py`, which drives the module.

```python
#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import bakefont3 as bf

# define a set of characters that you want to be able to rasterise
charset_ascii            = bf.charset((32, 126))
charset_latin_1          = bf.charset(charset_ascii, (161, 255))
charset_windows_1252     = bf.charset(charset_latin_1, "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ")


# define the fonts we want to be able to rasterise with
# bf.font(name, file)
#     * name - a name of the font (client code will use this to refer to it)
#     * file - path to a TrueType font

font_liberation_sans_regular      = bf.font("Sans Serif",      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")
font_liberation_sans_bold         = bf.font("Sans Serif Bold", "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf")


# a list of (font, size, character set) tuples to pack
# optional sizes argument: e.g. sizes=[(256,256),(512,256),(512,512)]
result = bf.pack([
    (font_liberation_sans_regular, 12, charset_windows_1252),
    (font_liberation_sans_bold,    12, charset_windows_1252),
    (font_liberation_sans_regular, 14, charset_windows_1252),
    (font_liberation_sans_bold,    14, charset_windows_1252),
    (font_liberation_sans_regular, 16, charset_windows_1252),
    (font_liberation_sans_bold,    16, charset_windows_1252),
])

width, height = result.size
print("> The atlas fits into a rectangle of size %dx%d (4 channel)" % (width, height))

filename = "font-atlas"
result.image().save(outfilename+".png") # the texture atlas
result.data().save(outfilename+".bf3")  # kerning, lookup, size data etc.
```


## Dependencies ##

    sudo pip3 install Pillow numpy freetype-py


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

