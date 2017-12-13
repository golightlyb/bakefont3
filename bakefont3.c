#include "bakefont3.h"
#include <string.h> // memcpy


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
    if (0 != memcmp(buf, "BAKEFONTv3r0", 12)) { return 0; }
    
    memcpy(&size, buf + 18, 2);
    
    return (size_t) size;
}


bf3_header bf3_header_load(bf3_filelike *filelike, char *hdr, size_t header_size)
{
    // HEADER - 24 byte block
    // b"BAKEFONTv3r0"  #  0 | 12 | magic bytes, version 3 revision 0
    // uint16(width)    # 12 |  2 | texture atlas width
    // uint16(height)   # 14 |  2 | texture atlas height
    // uint16(depth)    # 16 |  2 | texture atlas depth (1, 3, 4)
    // uint16(bytesize) # 18 |  2 | ...
    // b'\0\0\0\0'      # 20 |  4 | padding (realign to 8 bytes)

    int w, h, d, num_fonts, num_modes;

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
    
    size_t offset = 32 + (48 * (size_t ) num_fonts);
    if (0 != memcmp(hdr + offset, "MODE", 4)) { goto fail; }
    memcpy(v, hdr + 4 + offset, 2);
    num_modes = v[0];
    
    return (bf3_header) {w, h, d, num_fonts, num_modes};
    
    fail:
        return (bf3_header) {0, 0, 0, 0, 0};
}


bf3_font bf3_font_at(int index, char *buf)
{
    // 32+48n |  4 | attributes
    // 36+48n | 44 | name for font with FontID=n (null terminated string)
    size_t i = (size_t) index;
    char horizontal = *(buf + 32 + 0 + (48 * i)) == 'H';
    char vertical   = *(buf + 32 + 1 + (48 * i)) == 'V';
    const char *name = buf + 32 + 4 + (48 * i);
    
    return (bf3_font) {index, horizontal, vertical, name};
}


bf3_font bf3_fonts_start(char *hdr)
{
    // FONT TABLE HEADER - 6 bytes
    // b"FONT"                   # 24 | 4 | debugging marker
    // uint16(len(result.fonts)) # 28 | 2 | number of fonts
    // b"\0\0"                   # 30 | 2 | padding (realign to 8 bytes)

    // FONT RECORDS - 48 bytes * number of fonts
    // (for n = 0; n => n + 1; each record is at offset 32 + 48n)
    // the FontID is implicit by the order e.g. the first font has FontID 0
    // 32+48n |  4 | attributes
    // 36+48n | 44 | name for font with FontID=n (null terminated string)
    
    uint16_t num;
    if (0 != memcmp(hdr + 24, "FONT", 4)) { goto fail; }
    memcpy(&num, hdr + 28, 2);
    if (!num) { goto fail; }
    
    return bf3_font_at(0, hdr);
    
    fail:
        return (bf3_font) {num, 0, 0, NULL};
}


bf3_font bf3_fonts_next(char *hdr, bf3_font prev_font)
{
    return bf3_font_at(prev_font.id + 1, hdr);
}
