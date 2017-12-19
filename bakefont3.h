/*

    bakefont3 - low-level C loader

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

*/


#ifndef BAKEFONT3_H
#define BAKEFONT3_H

#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdint.h>
#include <math.h>


// Fixed point 26.6 encoding is used by freetype and bakefont3 for precise
// floating point values with precision of 1/64pt.

// use BF3_DECODE_FP26((int32_t) x) to get a FP26 value as a float value
// use BF3_ENCODE_FP26((float) x) to encode a float value to a FP26 value

// use BF3_DECODE_FP26_FLOOR((int32_t) x) to quickly get a FP26 value as an
// integer value, truncating any floating point information (the effect of this
// is to always round down). You might use this if you only ever use integer
// font sizes, for example, and don't need to support fractional font sizes.

// use BF3_DECODE_FP26_NEAREST((int32_t) x) to get a result as an integer,
// rounded to the nearest integer (rounding away from 0 if exactly half way).

// use BF3_DECODE_FP26_CEIL((int32_t) x, (bf3_fp26) tolerance) to get the
// result as an integer rounded towards +/-ve infinity, with a bit of tolerance
// where the value is rounded to the nearest integer instead. This is useful
// for rounding up to the next pixel, except when the value is almost exact.
// e.g.
//     bf3_fp26 x = BF3_ENCODE_FP26(-0.171875)
//     bf3_fp26 tolerance = BF3_ENCODE_FP26(12f/64f) // 0.1875
//     int value = BF3_DECODE_FP26_CEIL(x, tolerance)


typedef int32_t bf3_fp26;

#define BF3_DECODE_FP26(x) ( ((float) (x)) / 64.0f )
#define BF3_ENCODE_FP26(x) ((bf3_fp26) (x  * 64.0f))

#define BF3_DECODE_FP26_NEAREST(x) ((int) lroundf(BF3_DECODE_FP26(x)))
#define BF3_DECODE_FP26_CEIL(x, tolerance) BF3_DECODE_FP26_CEIL_impl(x, tolerance)

int BF3_DECODE_FP26_CEIL_impl(bf3_fp26 x, bf3_fp26 tolerance);



// The bf3_filelike structure holds a pointer to some FILE-like stream or
// buffer and pointers to functions that implement basic I/O operations
// on that FIlE-like object.

typedef struct bf3_filelike bf3_filelike;

struct bf3_filelike
{
    // a pointer to a FILE-like stream or buffer
    void *ptr;
    
    // copy at most `numbytes` from ptr + offset into dest, and return
    // the number of bytes copied.
    size_t (*read)(bf3_filelike *filelike, char *dest, size_t offset, size_t numbytes);
};


// The bf3_info structure holds the read-only properties width, height, depth,
// describing the texture atlas, num_fonts which defines the number of
// unique font names, and num_modes which defines the number of unique
// (FontID, fontsize, antialias?) tuples, and num_tables, which has information
// about where to load the glyph metrics and kerning information for a given
// name identifying a specific set of glyphs unique to that Mode ID.

typedef struct bf3_info bf3_info;

struct bf3_info
{
    uint16_t width;  // texture atlas width
    uint16_t height; // texture atlas height
    uint16_t depth;  // texture atlas depth 4: RGBA, 3: RGB, 1: Greyscale
    uint16_t num_fonts; // A list of font names; the index is the FontID
    uint16_t num_modes; // ModeID => (FontID, size, antialias)
    uint16_t num_tables; // A list of (ModeID, Glyphsetname) mappings to offsets
};


// The bf3_font structure describes a font with read-only properties

typedef struct bf3_font bf3_font;

struct bf3_font
{
    // the unique font ID, from 0 to (num_fonts-1)
    uint16_t id;
    
    int horizontal:1; // 0 or 1 (e.g. Latin fonts)
    int vertical:1;   // 0 or 1 (e.g. some East Asian fonts)
    
    // the Latin-1 encoded name (null terminated, strlen < 44)
    // This is a pointer into the `char *hdr` argument of `bf3_header_load`
    const char *name;
};


// The bf3_mode structure describes a (FontID, size, antialias) record

typedef struct bf3_mode bf3_mode;

struct bf3_mode
{
    // the unique mode ID, from 0 to (num_modes-1)
    uint16_t mode_id;
    
    // the ID of the font used, from 0 to (num_fonts-1)
    uint16_t font_id;
    
    int antialias:1; // was hinting used?
    
    // fixed float values (1/64th precision)
    // use BF3_DECODE_FP26(size) to get an actual float
    bf3_fp26 size;                // font size (pt/px; 1pt == 1px at 72 DPI)
    bf3_fp26 lineheight;          // e.g. for linespacing
    bf3_fp26 underline_position;  // relative to baseline
    bf3_fp26 underline_thickness; // centered on the position
};


// The bf3_table structure describes how to load glyph metrics and kerning
// information for a "glpyh set", a named set of characters for a given Mode ID

typedef struct bf3_table bf3_table;

struct bf3_table
{
    // the table ID, from 0 to (num_tables - 1)
    uint16_t table_id;
    
    // the mode ID, from 0 to (num_modes - 1)
    uint16_t mode_id;
    
    uint32_t metrics_offset;
    uint32_t metrics_size;
    uint32_t kerning_offset;
    uint32_t kerning_size;
    
    // the Latin-1 encoded name of the glyph set (null terminated, strlen < 20)
    // This is a pointer into the `char *hdr` argument of `bf3_header_load`
    const char *name;
};


// The bf3_metric structure describes how to render a single glyph
// see e.g. https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html

typedef struct bf3_metric bf3_metric;

struct bf3_metric
{
    uint32_t codepoint;
    
    // pixel position of rasterised image in texture atlas
    // relative to top-left
    uint16_t tex_x;
    uint16_t tex_y;
    uint8_t  tex_z; // channel
    
    // size of the rasterised image in texture atlas
    uint8_t  tex_w;
    uint8_t  tex_h;
    uint8_t  tex_d; // always 0 or 1; if none, no image
    
    // horizontal left side bearing
    bf3_fp26 hbx;
    // horizontal top side bearing
    bf3_fp26 hby;
    // horizontal advance
    bf3_fp26 hadvance;
    
    // note - right side bearing
    // = advance_width - left_side_bearing - tex_w
    
    // vertical left side bearing
    bf3_fp26 vbx;
    // vertical top side bearing
    bf3_fp26 vby;
    // vertical advance
    bf3_fp26 vadvance;
};



// The bf3_kpair structure describes kerning information for a pair of two
// glyphs
// see e.g. https://www.freetype.org/freetype2/docs/glyphs/glyphs-4.html

typedef struct bf3_kpair bf3_kpair;

struct bf3_kpair
{
    int32_t x;   // grid-fitted
    bf3_fp26 xf; // "fine": not-grid fitted
};


// convention - destination is always the first argument

// Get the size of the bf3 header to read
// Returns 0 if not a bf3 file.
size_t bf3_header_peek(bf3_filelike *filelike);

// Read the bf3 header into a buf, `hdr`, of at least size `header_size`.
// Use the header size returned previously by `bf3_header_peek`.
bool bf3_header_load(bf3_info *info, bf3_filelike *filelike, char *hdr, size_t header_size);

// Get a font by Font ID. The font ID is between 0 and (num_fonts - 1), where
// `num_fonts` is the `num_fonts` property of the `bf3_info` structure returned
// previously by `bf3_header_load`.
void bf3_font_get(bf3_font *font, char *hdr, int index);

// Get a mode by Mode ID. The mode ID is between 0 and (num_modes - 1), where
// `num_modes` is the `num_modes` property of the `bf3_info` structure returned
// previously by `bf3_header_load`.
void bf3_mode_get(bf3_mode *mode, char *hdr, int index);

// Get a table by Table ID. The mode ID is between 0 and (num_tables - 1), where
// `num_tables` is the `num_tables` property of the `bf3_info` structure returned
// previously by `bf3_header_load`.
void bf3_table_get(bf3_table *table, char *hdr, int index);

// Read font metrics for a given table into a buf, `metrics`, of at least size
// `table->metrics_size`. Use a table structure initialised previously
// by `bf3_table_get`.
bool bf3_metrics_load(char *metrics, bf3_filelike *filelike,bf3_table *table);

// Read kerning metrics for a given table into a buf, `kerning`, of at least size
// `table->kerning_size`. Use a table structure initialised previously
// by `bf3_table_get`.
bool bf3_kerning_load(char *kerning, bf3_filelike *filelike, bf3_table *table);

// Read font metrics for a given glyph codepoint from the buf `metrics`
// previously filled by bf3_metrics_load.
bool bf3_metric_get(bf3_metric *metric, const char *metrics, uint32_t codepoint);

// Read kerning information metrics for a given codepoint pair from the buf
// `kerning` previously filled by bf3_kerning_load.
bool bf3_kpair_get(bf3_kpair *kpair, const char *kerning,
    uint32_t codepoint_left, uint32_t codepoint_right);

#endif // ifndef BAKEFONT3_H
