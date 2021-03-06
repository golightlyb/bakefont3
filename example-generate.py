import freetype
import os.path
import bakefont3
import sys

fontdir = os.path.expanduser("~") + "/.fonts" # e.g. /home/me/.fonts

try:
    fontface_mono       = freetype.Face(fontdir + "/RobotoMono-Regular.ttf")
    fontface_mono_bold  = freetype.Face(fontdir + "/RobotoMono-Bold.ttf")
    fontface_sans       = freetype.Face(fontdir + "/Roboto-Regular.ttf")
    fontface_sans_bold  = freetype.Face(fontdir + "/Roboto-Bold.ttf")
    fontface_serif      = freetype.Face(fontdir + "/RobotoSlab-Regular.ttf")
    fontface_serif_bold = freetype.Face(fontdir + "/RobotoSlab-Bold.ttf")

    # Roboto fonts don't have any kerning information, so we'll use an example
    # from Microsoft that has kerning information
    fontface_sans       = freetype.Face("/usr/share/fonts/truetype/msttcorefonts/arial.ttf")
except freetype.FT_Exception:
    print("You can download these fonts for free from Google Fonts and Microsoft")
    raise

def allchars(fontface):
    for charcode, index in fontface.get_chars():
        yield chr(charcode)


# A mapping of font name => font face
#   * name - how the font is looked up in the exported data file
#   * face - a freetype.Face object
fonts = {
    "Mono":      fontface_mono,
    "Mono Bold": fontface_mono_bold,
    "Sans":      fontface_sans,
    "Sans Bold": fontface_sans_bold,
    "Serif":     fontface_serif,
    "Serif Bold":fontface_serif_bold,
}

# Font modes (name, size, antialias?)
#   * name - a key from the table defined above
#   * size - font size in pixels (at typographic DPI where 1px = 1pt),
#            may be a fraction e.g. 11.5.
#   * antialias - True for nice hinting, False for 1-bit
fontmode_mono14   = ("Mono",      14, True) # (monospace)
fontmode_mono14b  = ("Mono Bold", 14, True)
fontmode_sans14   = ("Sans",      14, True)
fontmode_sans14b  = ("Sans Bold", 14, True)
fontmode_serif14  = ("Serif",     14, True)
fontmode_serif14b = ("Serif Bold",14, True)

fontmode_mono16   = ("Mono",      16, True)
fontmode_mono16b  = ("Mono Bold", 16, True)
fontmode_sans16   = ("Sans",      16, True)
fontmode_sans16b  = ("Sans Bold", 16, True)
fontmode_serif16  = ("Serif",     16, True)
fontmode_serif16b = ("Serif Bold",16, True)



# Font mode and character set combinations as a a list of (mode, charset name,
# charset) tuples.
#   * mode - a font mode tuple defined above
#   * name - how the table is looked up in the exported data file
#   * charset - a sequence of Unicode characters (don't worry about duplicates)
#               we're using EVERY character the font supports, but you might
#               want fewer, like just ASCII, depending on circumstances.

tasks = [
    # quick version for testing
    # (fontmode_sans14,  "ALL", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"),

    (fontmode_mono14,   "ALL", allchars(fontface_mono)),
    (fontmode_mono14b,  "ALL", allchars(fontface_mono_bold)),
    (fontmode_sans14,   "ALL", allchars(fontface_sans)),
    (fontmode_sans14b,  "ALL", allchars(fontface_sans_bold)),
    (fontmode_serif14,  "ALL", allchars(fontface_serif)),
    (fontmode_serif14b, "ALL", allchars(fontface_serif_bold)),

    (fontmode_sans16,   "ALL", allchars(fontface_sans)),
    (fontmode_sans16b,  "ALL", allchars(fontface_sans_bold)),
    (fontmode_serif16,  "ALL", allchars(fontface_serif)),
    (fontmode_serif16b, "ALL", allchars(fontface_serif_bold)),

    # a small character set we want a separate efficient lookup table for
    (fontmode_sans14b,  "FPS", "FPS: 0123456789"),

    # and just to test some characters that the font probably won't support
    # Like the Welsh Ll ligatures: U+1EFA and U+1EFB.
    (fontmode_sans14,   "EXTRA", (0x1EFA, 0x1EFB))
]

# Notice that one fontmode is used twice. This is so that it can be used to
# generate two lookup tables as an optimisation. The first ("ALL") has any
# character. The second only contains characters for efficiently showing a FPS
# display. (This is only worth doing for extremely small sets). No extra video
# texture memory is used.

# suitable sizes for our texture atlas, in order of prefence
# as any (possibly infinite) sequence of (width, height, depth) tuples
# width, height - pixels
# depth - 4 to use Red, Green, Blue, Alpha channels individually
#       - 3 to just use Red, Green, Blue, with full alpha
#       - 1 to make a single channel greyscale image

# e.g. the finite sequence (64, 64) (128, 128), (256, 256), ... (64k, 64k)
depth = 4
suitable_texture_sizes = [(2**x, 2**x, depth) for x in range(6, 17)]


# a callback for getting progress
class progress:
    STEP = 5.0

    def __init__(self):
        self.percent = 0.0
    def stage(self, msg):
        print("%s..." % msg)
        self.percent = 0.0
    def step(self, current, total):
        percent = 100 * current / total
        if (percent - self.percent) > self.STEP:
            self.percent = percent
            print("    %d of %d (%d%%)" % (current, total, percent))
    def info(self, msg):
        print("    (%s)" % msg)


# Use bakefont3 to rasterise the glyphs, tightly pack them, and collect
# kerning data
result = bakefont3.pack(fonts, tasks, suitable_texture_sizes, cb=progress())

if not result.image:
    print("No fit :-(")
    sys.exit(0)


# result.image => a PIL (Pillow) image with the method:
#   * result.image.save(filename) - saves to a file
#   * result.image.split() - splits a RGB or RGBA image into channels

# result.data  => a bakefont3.saveable object with the methods:
#   * result.data.bytes - raw bytes of the data file
#   * result.data.save(filename) - saves to a file

result.data.save("test.bf3")

width, height, depth = result.size
if depth == 4:
    result.image.save("test-rgba.png")
    red,green,blue,alpha = result.image.split()
    red.save("test-4r.png")
    green.save("test-4g.png")
    blue.save("test-4b.png")
    alpha.save("test-4a.png")
elif depth == 3:
    result.image.save("test-rgb.png")
    red,green,blue = result.image.split()
    red.save("test-3r.png")
    green.save("test-3g.png")
    blue.save("test-3b.png")
elif depth == 1:
    result.image.save("test-greyscale.png")


