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

#include "bakefont3.h"
#include <string.h> // memcpy


// NOTE - this implementation works for LITTLE ENDIAN HOSTS only
// (its quite trivial to fix but I don't have anything to test on)


// Decode a fixed point 2.6 number, rounded towards +/-ve infinity, but rounded
// towards zero if the fractional portion is below a given tolerance.
// Used to "round up" to the next integer, unless its "almost exact".
int BF3_DECODE_FP26_CEIL_impl(bf3_fp26 x, bf3_fp26 tolerance)
{
    // TODO (PERF) this could be clever enough to operate on the integers
    
    float fx = BF3_DECODE_FP26(x);
    float ftolerance = BF3_DECODE_FP26(tolerance);
    ftolerance = copysignf(ftolerance, fx);
    
    float sgn_ceil = copysignf(0.5f, fx); // round towards +/-ve infinity
    
    float result = (fx - ftolerance + sgn_ceil);
    return (int) lround(result);
}


size_t bf3_header_peek(bf3_filelike *filelike)
{
    // HEADER - 24 byte block
    // b"BAKEFONTv3r0"  #  0 | 12 | magic bytes, version 3 revision 0
    // uint16(width)    # 12 |  2 | texture atlas width
    // uint16(height)   # 14 |  2 | texture atlas height
    // uint16(depth)    # 16 |  2 | texture atlas depth (1, 3, 4)
    // uint16(bytesize) # 18 |  2 | ...
    // b'\0\0\0\0'      # 20 |  4 | padding (realign to 8 bytes)

    char buf[20];
    uint16_t size;
    
    size_t was_read = filelike->read(filelike, buf, 0, 20);
    if (was_read < 20) { return 0; }
    if (0 != memcmp(buf, "BAKEFONTv3r1", 12)) { return 0; }
    
    memcpy(&size, buf + 18, 2);
    
    return (size_t) size;
}


bool bf3_header_load(bf3_info *info, bf3_filelike *filelike, char *hdr, size_t header_size)
{
    // HEADER - 24 byte block
    // b"BAKEFONTv3r0"  #  0 | 12 | magic bytes, version 3 revision 0
    // uint16(width)    # 12 |  2 | texture atlas width
    // uint16(height)   # 14 |  2 | texture atlas height
    // uint16(depth)    # 16 |  2 | texture atlas depth (1, 3, 4)
    // uint16(bytesize) # 18 |  2 | ...
    // b'\0\0\0\0'      # 20 |  4 | padding (realign to 8 bytes)

    int w, h, d, num_fonts, num_modes, num_tables;

    size_t was_read = filelike->read(filelike, hdr, 0, header_size);
    if (was_read < header_size) { goto fail; }
    
    uint16_t v[3];
    memcpy(v, hdr + 12, 6);
    w = v[0]; h = v[1]; d = v[2];
    
    // FONT TABLE HEADER - 6 bytes
    // b"FONT"                   # 24 | 4 | debugging marker
    // uint16(len(result.fonts)) # 28 | 2 | number of fonts
    // b"\0\0"                   # 30 | 2 | padding (realign to 8 bytes)
    
    // FONT RECORDS
    // [48 bytes] * number_of_fonts
    
    if (0 != memcmp(hdr + 24, "FONT", 4)) { goto fail; }
    memcpy(v, hdr + 28, 2);
    num_fonts = v[0];
    
    // offset is relative to r = 32 + (48 * number of fonts)
    
    // FONT MODE TABLE HEADER - 8 bytes
    // "MODE"                    # r+0 | 4 | debugging marker
    // uint16(len(result.modes)) # r+4 | 2 | number of modes
    // b"\0\0"                   # r+6 | 2 | padding (realign to 8 bytes)
    
    size_t offset = 32 + (48 * num_fonts);
    if (0 != memcmp(hdr + offset, "MODE", 4)) { goto fail; }
    memcpy(v, hdr + 4 + offset, 2);
    num_modes = v[0];
    
    //offset is relative to r = 24 + 8 + (48 * number of fonts)
    //                              + 8 + (32 * number of modes)

    // GLYPH TABLE HEADER - 8 bytes
    // b"GTBL"                       # r+0 | 4 | debugging marker
    // uint16(len(result.modeTable)) # r+4 | 2 | number of (modeID, charsetname) pairs
    offset = 32 + (48 * num_fonts) + 8 + (32 * num_modes);
    if (0 != memcmp(hdr + offset, "GTBL", 4)) { goto fail; }
    memcpy(v, hdr + 4 + offset, 2);
    num_tables = v[0];
    
    
    bf3_info _info = {w, h, d, num_fonts, num_modes, num_tables};
    memcpy(info, &_info, sizeof(bf3_info));
    return true;
    
    fail:
        return false;
}


void bf3_font_get(bf3_font *font, char *buf, int index)
{
    // 32+48n |  4 | attributes
    // 36+48n | 44 | name for font with FontID=n (null terminated string)
    size_t i = (size_t) index;
    char horizontal = *(buf + 32 + 0 + (48 * i)) == 'H';
    char vertical   = *(buf + 32 + 1 + (48 * i)) == 'V';
    const char *name = buf + 32 + 4 + (48 * i);
    
    bf3_font _font = {index, horizontal, vertical, name};
    memcpy(font, &_font, sizeof(bf3_font));
}


void bf3_mode_get(bf3_mode *mode, char *buf, int index)
{
    uint16_t num_fonts;
    memcpy(&num_fonts, buf + 28, 2);
    
    size_t offset = 24 + 8 + (48 * num_fonts); // past font table
    offset += 8; // past mode header
    offset += (32 * index);
    
    
    // o +0 |  2 | font ID
    // o +2 |  1 | flag: 'A' if the font is antialiased, otherwise 'a'
    // o +3 |  1 | RESERVED
    // o +4 |  4 | font size - fixed point 26.6
    //            (divide the signed int32 by 64 to get original float)
    // o +8 |  4 | lineheight aka linespacing - (fixed point 26.6)
    // o+12 |  4 | underline position relative to baseline (fixed point 26.6)
    // o+16 |  4 | underline vertical thickness, centered on position (fp26.6)
    // o+20 | 12 | RESERVED
    
    uint16_t font_id;
    memcpy(&font_id, buf + offset + 0, 2);
    
    char antialias = *(buf + offset + 2) == 'A';
    
    uint32_t pts[4];
    memcpy(pts, buf + offset + 4, 16);
    
    bf3_fp26 size                = { pts[0] };
    bf3_fp26 lineheight          = { pts[1] };
    bf3_fp26 underline_position  = { pts[2] };
    bf3_fp26 underline_thickness = { pts[3] };
    
    bf3_mode _mode = {index, font_id, antialias,
        size, lineheight, underline_position, underline_thickness};
    
    memcpy(mode, &_mode, sizeof(bf3_mode));
}


void bf3_table_get(bf3_table *table, char *buf, int index)
{
    // 40 byte records
    // 40n +0 |  2 | mode ID
    // 40n +2 |  2 | RESERVED
    // 40n +4 |  4 | absolute byte offset of glyph metrics data
    // 40n +8 |  4 | byte size of glyph metrics data
    //             (subtract 4, divide by 36 to get number of entries)
    // 40n+12 |  4 | absolute byte offset of glyph kerning data
    // 40n+16 |  4 | byte size of glyph kerning data
    //             (subtract 4, divide by 16 to get number of entries)
    // 40n+20 | 20 | charset name (string, null terminated)
    
    uint16_t num_fonts, num_modes;
    
    memcpy(&num_fonts, buf + 28, 2);
    memcpy(&num_modes, buf + 32 + (48 * num_fonts) + 4, 2);
    
    size_t offset = 32 + (48 * num_fonts) + 8 + (32 * num_modes) + 8;
    offset += (40 * index);
    
    uint16_t mode_id;
    uint32_t metrics_offset, metrics_size, kerning_offset, kerning_size;
    
    memcpy(&mode_id,        buf + offset +  0, 2);
    memcpy(&metrics_offset, buf + offset +  4, 4);
    memcpy(&metrics_size,   buf + offset +  8, 4);
    memcpy(&kerning_offset, buf + offset + 12, 4);
    memcpy(&kerning_size,   buf + offset + 16, 4);
    const char *name = buf + offset + 20;
    
    bf3_table _table = {index, mode_id, metrics_offset, metrics_size,
        kerning_offset, kerning_size, name};
    
    memcpy(table, &_table, sizeof(bf3_table));
}


bool bf3_metrics_load(char *metrics, bf3_filelike *filelike, bf3_table *table)
{
    if (table->metrics_size < 4) { goto fail; }
    
    size_t was_read = filelike->read(filelike, metrics, table->metrics_offset, table->metrics_size);
    if (was_read < table->metrics_size) { goto fail; }
    
    if (0 != memcmp(metrics, "GSET", 4)) { goto fail; }
    
    // patch over the SGET header with nmemb
    uint32_t num = (table->metrics_size - 4) / 36;
    memcpy(metrics, &num, 4);
    
    return true;
    
    fail:
        return false;
}


bool bf3_kerning_load(char *kerning, bf3_filelike *filelike, bf3_table *table)
{
    if (table->kerning_size < 4) { goto fail; }
    
    size_t was_read = filelike->read(filelike, kerning, table->kerning_offset, table->kerning_size);
    if (was_read < table->kerning_size) { goto fail; }
    
    if (0 != memcmp(kerning, "KERN", 4)) { goto fail; }
    
    // patch over the KERN header with nmemb
    uint32_t num = (table->kerning_size - 4) / 16;
    memcpy(kerning, &num, 4);
    
    return true;
    
    fail:
        return false;
}


static void bf3_metric_decode(bf3_metric *metric, const char *buf)
{
    // the metric structure is tightly packed so this works
    memcpy(metric, buf, 36);
}


bool bf3_metric_get(bf3_metric *metric, const char *metrics, uint32_t codepoint)
{
    // read nmemb we stashed earlier
    uint32_t nmemb;
    memcpy(&nmemb, metrics, 4);

    // for a record, n, where is the offset to its codepoint relative to the
    // start of the metrics buffer?
#   define RECORD(n) (4 + (36*(n)))
    
    // binary search for a matching codepoint
    size_t start = 0;
    size_t pos   = nmemb / 2;
    size_t end   = nmemb;
    
    while ((pos >= start) && (pos < end))
    {
        size_t offset = RECORD(pos);
        uint32_t current;
        memcpy(&current, metrics + offset, 4);
        
        int cmp = (codepoint == current) ? 0 : (codepoint > current) ? 1 : -1;
        if (cmp == 0)
        {
            bf3_metric_decode(metric, metrics + offset);
            return true;
        }
        else if (cmp < 0) { end = pos; }
        else if (cmp > 0) { start = pos + 1; }
        
        pos = start + ((end - start) / 2);
    }
    
    return false;
    
#   undef RECORD
}


static void bf3_kpair_decode(bf3_kpair *kpair, const char *buf)
{
    bf3_fp26 x, xf;
    
    memcpy(&x,  buf + 8, 4);
    memcpy(&xf, buf + 12, 4);
    
    kpair->x  = BF3_DECODE_FP26_NEAREST(x);
    kpair->xf = xf;
}


bool bf3_kpair_get(bf3_kpair *kpair, const char *kerning,
    uint32_t codepoint_left, uint32_t codepoint_right)
{
    // read nmemb we stashed earlier
    uint32_t nmemb;
    memcpy(&nmemb, kerning, 4);

    // for a record, n, where is the offset to its codepoint relative to the
    // start of the kerning buffer?
#   define RECORD(n) (4 + (16*(n)))
    
    // binary search for a matching (left, right) pair
    size_t start = 0;
    size_t pos   = nmemb / 2;
    size_t end   = nmemb;
    
    while ((pos >= start) && (pos < end))
    {
        size_t offset = RECORD(pos);
        uint32_t current_left, current_right;
        memcpy(&current_left,  kerning + offset,     4);
        memcpy(&current_right, kerning + offset + 4, 4);
        
        int cmp0 = (codepoint_left  == current_left)  ? 0 :
            (codepoint_left > current_left) ? 1 : -1;
        int cmp1 = (codepoint_right == current_right) ? 0 :
            (codepoint_right > current_right) ? 1 : -1;
        
        if ((cmp0 == 0) && (cmp1 == 0))
        {
            bf3_kpair_decode(kpair, kerning + offset);
            return true;
        }
        else if ((cmp0 == 0) && (cmp1 < 0)) { end = pos; }
        else if ((cmp0 == 0) && (cmp1 > 0)) { start = pos + 1; }
        else if (cmp0 < 0) { end = pos; }
        else if (cmp0 > 0) { start = pos + 1; }
        
        pos = start + ((end - start) / 2);
    }
    
    return false;
    
#   undef RECORD
}
