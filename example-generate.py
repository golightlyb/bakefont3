#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import bakefont3 as bf
import sys
import os.path
home = os.path.expanduser("~")

if len(sys.argv) == 2:
    outfilename = sys.argv[1]
else:
    print("USAGE: python3 %s outfilename" % sys.argv[0])
    sys.exit(1)


# Step One
# Define the set of characters we want fonts to be able to use.

# bf.charset(name, ...) takes a list of values.
# The result bf.charset represents a set of all the given values.

# Each value in the list of arguments is interpreted as follows:
#     * a number - a Unicode codepoint/value
#     * a letter - a Unicode character
#     * a 2-tuple - a range between two values (inclusive)
#     * a string - a string of Unicode characters
#     * a bf.charset object - a previously defined set of values

print("Loading character sets")
charset_simple = bf.charset(('A', 'Z'), ('0','9'), ".,:!?\"()")
charset_ascii  = bf.charset((32, 126))

charset_latin_1 = bf.charset(charset_ascii, (161, 255))

charset_latin_2 = bf.charset(charset_ascii,
    "Ą˘Ł¤ĽŚ§¨ŠŞŤŹŽŻ°ą˛ł´ľśˇ¸šşťź˝žżŔÁÂĂÄĹĆÇČÉĘËĚÍÎĎĐŃŇÓÔŐÖ×ŘŮÚŰÜÝŢßŕáâăäĺćçčé",
    "ęëěíîďđńňóôőö÷řůúűüýţ˙")

charset_latin_3 = bf.charset(charset_ascii,
    "Ħ˘£¤Ĥ§¨İŞĞĴŻ°ħ²³´µĥ·¸ışğĵ½żÀÁÂÄĊĈÇÈÉÊËÌÍÎÏÑÒÓÔĠÖ×ĜÙÚÛÜŬŜßàáâäċĉçèéêëìíîï",
    "ñòóôġö÷ĝùúûüŭŝ˙")

charset_latin_4 = bf.charset(charset_ascii,
    "ĄĸŖ¤ĨĻ§¨ŠĒĢŦŽ¯°ą˛ŗ´ĩļˇ¸šēģŧŊžŋĀÁÂÃÄÅÆĮČÉĘËĖÍÎĪĐŅŌĶÔÕÖ×ØŲÚÛÜŨŪßāáâãäåæįčé",
    "ęëėíîīđņōķôõö÷øųúûüũū˙")

charset_latin_cryillic = bf.charset(charset_ascii,
    "ЁЂЃЄЅІЇЈЉЊЋЌЎЏАБВГДЕЖИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъ",
    "ыьэюя№ёђѓєѕіїјљњћќ§ўџ") # ISO 8859-5

charset_latin_greek = bf.charset(charset_ascii,
    "‘’£€₯¦§¨©ͺ«¬―°±²³΄΅Ά·ΈΉΊ»Ό½ΎΏΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩΪΫάέήίΰαβγδεζηθικλ",
    "μνξοπρςστυφχψωϊϋόύώ") # ISO 8859-7

charset_latin_6 = bf.charset(charset_ascii,
    "ĄĒĢĪĨĶ§ĻĐŠŦŽŪŊ°ąēģīĩķ·ļđšŧž―ūŋĀÁÂÃÄÅÆĮČÉĘËĖÍÎÏÐŅŌÓÔÕÖŨØŲÚÛÜÝÞßāáâãäåæįčé",
    "ęëėíîïðņōóôõöũøųúûüýþĸ") # ISO 8859-10

charset_latin_7 = bf.charset(charset_ascii,
    "”¢£¤„¦§Ø©Ŗ«¬®Æ°±²³“µ¶·ø¹ŗ»¼½¾æĄĮĀĆÄÅĘĒČÉŹĖĢĶĪĻŠŃŅÓŌÕÖ×ŲŁŚŪÜŻŽßąįāćäåęēč",
    "éźėģķīļšńņóōõö÷ųłśūüżž’")

charset_latin_hebrew = bf.charset(charset_ascii,
    (0x00A2, 0x00AF), (0x00B0, 0x00BE), 0x2017, (0x05D0, 0x05EA)) # ISO 8859-8

charset_latin_8 = bf.charset(charset_ascii,
    "Ḃḃ£ĊċḊ§Ẁ©ẂḋỲ®ŸḞḟĠġṀṁ¶ṖẁṗẃṠỳẄẅṡÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏŴÑÒÓÔÕÖṪØÙÚÛÜÝŶßàáâãäåæçè",
    "éêëìíîïŵñòóôõöṫøùúûüýŷÿ") # Celtic / ISO 8859-14

charset_latin_10 = bf.charset(charset_ascii,
    "ĄąŁ€„Š§š©Ș«ŹSHYźŻ°±ČłŽ”¶·žčș»ŒœŸżÀÁÂĂÄĆÆÇÈÉÊËÌÍÎÏĐŃÒÓÔŐÖŚŰÙÚÛÜĘȚßàáâăäćæ",
    "çèéêëìíîïđńòóôőöśűùúûüęțÿ") # ISO 8859-16

charset_page_437 = bf.charset(charset_ascii,
    "☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿",
    "⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀αßΓπΣσµτΦΘΩδ∞φ",
    "ε∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■") # ♥ dwarf fortress ♥

charset_windows_1252 = bf.charset(charset_latin_1, "€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ")

# extra characters (depends on font support)

# e.g. Esperanto, Hungarian, Czech, ...
block_latin_extended_a = bf.charset((0x0100, 0x017F))

# some that may not be supported by most fonts...
block_arrows                 = bf.charset((0x2190, 0x21FF))
block_emoticons              = bf.charset((0x1F600, 0x1F64F))
block_geometric_shapes       = bf.charset((0x25A0, 0x25FF))
block_mathematical_operators = bf.charset((0x2200, 0x22FF))
block_letterlike_symbols     = bf.charset((0x2100, 0x214F))
block_greek_and_coptic       = bf.charset(
    (0x0370, 0x0377), (0x037A, 0x037F), (0x0384, 0x038A), 0x038C,
    (0x038E, 0x03A1), (0x03A3, 0x03FF))

charset_custom = bf.charset(
    charset_windows_1252, # superset of charset_latin_1
    charset_latin_2,
    charset_latin_3,
    charset_latin_4,
    charset_latin_cryillic,
    charset_latin_greek,
    charset_latin_6,
    charset_latin_7,
    charset_latin_8,
    # charset_latin_hebrew, # needs font support
    charset_latin_10,
    block_latin_extended_a,

    "ẀẁẂẃŴŵŶŷ",  # Welsh
    "©®™",
    "¶§",
    "¿¡•·ªº“”…",
    "✔✖",
    "°√ⁿ±≥≤∞",
    "ΑαΒβΓγΔδΕεΖζΗηΘθΙιΚκΛλΜμΝνΞξΟοΠπΡρΣσςΤτΥυΦφΧχΨψΩω", # Greek alphabet
    (0x0400,0x04FF), # Crylic

    # https://en.wikipedia.org/wiki/ISO/IEC_8859-1#Languages_with_incomplete_coverage
    "Ŀŀ",                   # Catalan
    "ČčĎďĚěŇňŘřŠšŤťŮůŽž",   # Czech
    'Ĳ', 'ĳ',               # Dutch ligatures
    "ŠšŽž",                 # Estonian
    "ŠšŽž",                 # Finnish
    "ŒœŸ",                  # French
    "ẼẽĨĩŨũỸỹGg̃",           # Guarani
    "ŐőŰű",                 # Hungarian
    #"ḂḃĊċḊḋḞḟĠġṀṁṠṡṪṫ",     # Irish (traditional orthography) (missing from most fonts)
    "ĀāĒēĪīŌōŪū",           # Latin with macrons
    "ĀāĒēĪīŌōŪū",           # Māori
    "ĂăȘșȚțŢţ",             # Romanian
    "İıĞğŞş",               # Turkish
    "ĞİŞğışÐÝÞðýþ",         # Turkish Latin-5 extras
    #7930, 7931,            # Welsh Ll/ll ligatures (missing from most fonts)
    "Ґґ",                   # Ukrainian CP1124
)

# e.g.
# print(charset_welsh.chars)



# define the fonts we may want to rasterise
# bf.font(name, file)
#     * name - a name of the font (client code will use this to refer to it)
#     * file - path to a TrueType font

print("Loading fonts")
try:
    font_monospace = bf.font("Monospace",   home+"/.fonts/RobotoMono-Regular.ttf")
    font_sans      = bf.font("Sans Serif",  home+"/.fonts/Roboto-Regular.ttf")
except Exception:
    print("You can download these fonts for free from Google Fonts")
    raise



# FIXME bad things happen if you give two different fonts the same name


tasks = [
    (font_monospace,    16, bf.charset("FPS: 0123456789")),
    (font_sans,         16, charset_custom),

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



