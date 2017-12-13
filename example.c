// Example program loading bakefont3 data and using it to display text
// COMPILE:
//     gcc -std=c99 example.c bakefont3.c -Wall -Wextra -o example
// USAGE:
//     example data.bf3 atlas.png

#include "bakefont3.h"
#include <stdlib.h> // malloc, free
#include <string.h> // strcmp
#include <stdio.h>
#include <errno.h>


// bakefont3 doesn't know anything, or care, about reading from disk or from
// memory or wherever. You give bakefont a pointer to whatever represents your
// data and a pointer to a function that can read from that data at a given
// offset. Here's an implementation for reading from a FILE *.

size_t (read_FILE)(bf3_filelike *filelike, char *dest, size_t offset, size_t numbytes)
{
    FILE *src = (void *) filelike->ptr;
    
    if (0 != fseek(src, (long) offset, SEEK_SET))
    {
        // NOTE error message will be incorrect for offsets > 4GB
        fprintf(stderr, "Seek error (%u:%u)\n",
            (unsigned int) offset, (unsigned int) numbytes);
        return 0;
    }
    
    size_t total_read = 0;
    while (numbytes > total_read)
    {
        size_t was_read = fread(dest, 1, numbytes, src);
        total_read += was_read;
        if (!was_read)
        {
            if (feof(src)) { break; }
            int err = ferror(src);
            if (err == EAGAIN) { continue; }
            fprintf(stderr, "Read error %x\n", err);
            break;
        }
    }
    
    return total_read;
}


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("USAGE: %s data.bf3 atlas.png\n", argv[0]);
        return -1;
    }

    // open the bf3 data file
    FILE *fdata = fopen(argv[1], "rb");
    if (!fdata) { fprintf(stderr, "Could not open %s\n", argv[1]); return -1; }
    
    // open the texture atlas
    FILE *fimage = fopen(argv[2], "rb");
    if (!fimage) { fprintf(stderr, "Could not open %s\n", argv[2]); return -1; }
    
    // tell bakefont3 how to read the data
    bf3_filelike data_reader = {(void *)fdata, read_FILE};

    // peek at the the header telling us what's in the bakefont3 file
    // to see how big the first chunk of information we need is
    size_t header_size = bf3_header_peek(&data_reader);
    printf("header size %u (bytes)\n", (unsigned int) header_size);
    if (!header_size) { fprintf(stderr, "Not a bf3 file %s\n", argv[1]); return -1; }
    
    // allocate a buffer to hold the header information
    char *hdr = malloc(header_size);
    if (!hdr) { fprintf(stderr, "Malloc error\n"); return -1; }
    
    // parse the header
    bf3_header header = bf3_header_load(&data_reader, hdr, header_size);
    printf("header dimensions = %dx%dx%d\n",
        header.width, header.height, header.depth);
    printf("number of fonts: %d\n", header.num_fonts);
    printf("number of modes: %d\n", header.num_modes);

    // iterate through the fonts table
    // and let's look for a font named Sans
    // (you may this line if you have given the fonts different names)
    const char *wanted_font = "Sans";
    
    bf3_font font;
    bf3_font font_sans = (bf3_font) {0};
    for (font = bf3_fonts_start(hdr);
         font.id < header.num_fonts;
         font = bf3_fonts_next(hdr, font))
    {
        printf("Font: ID: %d, Name: %s\n", font.id, font.name);
        
        if (0 == strcmp(font.name, wanted_font)) { font_sans = font; }
    }

    if (!font_sans.name)
    {
        fprintf(stderr, "Couldn't find the '%s' font\n", wanted_font);
        return -1;
    }

    free(hdr);
    fclose(fimage);
    fclose(fdata);
}
