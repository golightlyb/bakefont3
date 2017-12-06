#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import bakefont3 as bf
import sys

if len(sys.argv) == 2:
    outfilename = sys.argv[1]
else:
    print("USAGE: python3 %s outfilename" % sys.argv[0])
    sys.exit(1)


# define sets of characters we may want to render
# bf.charset(name, ...) takes a list of values:
#     * a number - a Unicode value
#     * a letter - a Unicode character
#     * a 2-tuple - a range between two values (inclusive)
#     * a string - a string of Unicode characters
#     * a bf.charset object - a previously defined set of values
# The result is a set of the union of all the given values

print("Loading character sets")
charset_simple           = bf.charset(('A', 'Z'), ('0','9'), ".,:!?\"()")
charset_ascii            = bf.charset((32, 126))
charset_latin_1          = bf.charset(charset_ascii, (161, 255))
charset_windows_1252     = bf.charset(charset_latin_1, "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ")
charset_welsh            = bf.charset(charset_latin_1, "ẀẁẂẃŴŵŶŷ", 7930, 7931)

charset_arrows           = bf.charset((0x2190, 0x21FF))
charset_emoticons        = bf.charset((0x1F600, 0x1F64F))
charset_geometric_shapes = bf.charset((0x25A0, 0x25FF))
charset_mathematical_operators = bf.charset((0x2200, 0x22FF))

charset_latin_extended_a = bf.charset((0x0100, 0x017F))
charset_latin_extended_b = bf.charset((0x0180, 0x024F))
charset_latin_extended_c = bf.charset((0x2C60, 0x2C7F))
charset_latin_extended_additional = bf.charset((0x1E00, 0x1EFF))

charset_greek_and_coptic = bf.charset(
    (0x0370, 0x0377), (0x037A, 0x037F), (0x0384, 0x038A), 0x038C,
    (0x038E, 0x03A1), (0x03A3, 0x03FF))

# https://en.wikipedia.org/wiki/ISO/IEC_8859-1#Languages_with_incomplete_coverage
charset_custom = bf.charset(
    charset_windows_1252,

    "Ŀŀ",                   # Catalan
    "ČčĎďĚěŇňŘřŠšŤťŮůŽž",   # Czech
    'Ĳ', 'ĳ',               # Dutch ligatures
    "ŠšŽž",                 # Estonian
    "ŠšŽž",                 # Finnish
    "ŒœŸ",                  # French
    "ẼẽĨĩŨũỸỹGg̃",           # Guarani
    "ŐőŰű",                 # Hungarian
    "ḂḃĊċḊḋḞḟĠġṀṁṠṡṪṫ",     # Irish (traditional orthography)
    "ĀāĒēĪīŌōŪū",           # Latin with macrons
    "ĀāĒēĪīŌōŪū",           # Māori
    "ĂăȘșȚțŢţ",             # Romanian
    "İıĞğŞş",               # Turkish
    "ẀẁẂẃŴŵŶŷ",             # Welsh
    7930, 7931,             # Welsh Ll/ll ligatures
    '€',                    # Euro
)

# e.g.
# print(charset_welsh.chars)



# define the fonts we may want to rasterise
# bf.font(name, file)
#     * name - a name of the font (client code will use this to refer to it)
#     * file - path to a TrueType font

print("Loading fonts")
font_liberation_sans_regular      = bf.font("Sans Serif",      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")
font_liberation_sans_bold         = bf.font("Sans Serif Bold", "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf")
font_liberation_monospace_regular = bf.font("Monospace",       "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf")
font_liberation_monospace_bold    = bf.font("Monospace Bold",  "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf")

# FIXME bad things happen if you give two different fonts the same name


tasks = [
    (font_liberation_monospace_bold,    14, bf.charset("FPS: 0123456789")),
    (font_liberation_sans_regular,      14, charset_simple),

    # as a test, note that overlapping sets don't waste any extra space
    # (font_liberation_sans_regular,      14, charset_ascii),
]

"""
(font_liberation_monospace_regular, 14, charset_simple),
(font_liberation_monospace_regular, 16, charset_simple),
(font_liberation_monospace_regular, 18, charset_simple),

(font_liberation_sans_regular, 11, charset_custom),
(font_liberation_sans_regular, 11.5, charset_custom), # fractional sizes you betcha
(font_liberation_sans_regular, 12, charset_custom),
(font_liberation_sans_regular, 14, charset_custom),
(font_liberation_sans_regular, 16, charset_custom),

(font_liberation_sans_bold, 12, charset_custom),
(font_liberation_sans_bold, 14, charset_custom),
(font_liberation_sans_bold, 16, charset_custom),
(font_liberation_sans_bold, 18, charset_custom),
(font_liberation_sans_bold, 22, charset_custom),
"""


print("Rasterising glyphs for each (font, size, charset) task")
results = []
for i, task in enumerate(tasks):
    print("    task %d of %d" % (i+1, len(tasks)))
    font, size, charset = task
    results.append(bf.render(font, size, charset))



# Let's bake! We combine and pack everything into one image
# bg.pack(render results..., [size_sequence]) -> pack result

# size_sequence defaults to the sequence (64, 64), (128, 128), (256, 256)...
# and defines what sizes are acceptable in order of preference
# e.g. result = bf.pack(results, [(200, 200), (512, 128)])

print("Packing...")
result = bf.pack(results) # default 2^n square sizes

width, height = result.size
print("> The atlas fits into a rectangle of size %dx%d (4 channel)" % (width, height))

print("Saving results...")

# result.image() => a PIL image, supports .save(filename)
# result.data() => encode object, .data => bytes, .save(filename)

img = result.image()
img.save(outfilename+".png")

data = result.data()
data.save(outfilename+".bf3")

# split into individual channels for testing
r,g,b,a = img.split()
r.save(outfilename+"-r.png")
g.save(outfilename+"-g.png")
b.save(outfilename+"-b.png")
a.save(outfilename+"-a.png")



# workaround to suppress deconstructor warnings in freetype-py
# https://github.com/rougier/freetype-py/issues/44
del font_liberation_sans_regular._face
del font_liberation_sans_bold._face
del font_liberation_monospace_regular._face
del font_liberation_monospace_bold._face

