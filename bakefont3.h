/*

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

*/


#ifndef BAKEFONT3_H
#define BAKEFONT3_H

#include <stdbool.h>
#include <stddef.h> // size_t
#include <stdint.h>


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


// The bf3_header structure holds the read-only properties width, height, depth,
// describing the texture atlas.

typedef struct bf3_header bf3_header;

struct bf3_header
{
    int width;  // texture atlas width
    int height; // texture atlas height
    int depth;  // texture atlas depth 4: RGBA, 3: RGB, 1: Greyscale
    int num_fonts; // A list of font names; the index is the FontID
    int num_modes; // ModeID => (FontID, size, antialias)
};


// The bf3_font structure describes a font with read-only properties

typedef struct bf3_font bf3_font;

struct bf3_font
{
    // the unique font ID, from 0 to (num_fonts-1)
    int id;
    
    char horizontal:1; // 0 or 1 (e.g. Latin fonts)
    char vertical:1;   // 0 or 1 (e.g. some East Asian fonts)
    
    // the Latin-1 encoded name (null terminated, strlen < 44)
    const char *name;
};


// Get the size of the bf3 header to read
size_t bf3_header_peek(bf3_filelike *filelike);

// Read the bf3 header into a buf, `hdr`, of at least size `header_size`.
// Use the header size returned by `bf3_header_peek`.
bf3_header bf3_header_load(bf3_filelike *filelike, char *hdr, size_t header_size);

// Get the first font
// NOTE - the result is undefined if bf3_header.num_fonts
bf3_font bf3_fonts_start(char *hdr);

bf3_font bf3_fonts_next(char *hdr, bf3_font prev_font);


#endif // ifndef BAKEFONT3_H
