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
    bool found;

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
    // don't free() this until you're done using bakefont3
    char *hdr = malloc(header_size);
    if (!hdr) { fprintf(stderr, "Malloc error (header)\n"); return -1; }
    
    // parse the header
    bf3_info info;
    if (!bf3_header_load(&info, &data_reader, hdr, header_size))
        { fprintf(stderr, "Error reading header\n"); return -1; }
    printf("header dimensions = %dx%dx%d\n",
        info.width, info.height, info.depth);
    printf("number of fonts: %d\n", info.num_fonts);
    printf("number of modes: %d\n", info.num_modes);
    printf("number of tables: %d\n", info.num_tables);

    // iterate through the fonts table
    // and let's look for a font named Sans
    // (please edit this line if you have given the fonts different names)
    const char *wanted_font = "Sans";
    bf3_font font_sans;
    found = false;
    
    for (int i = 0; i < info.num_fonts; i++)
    {
        bf3_font font;
        bf3_font_get(&font, hdr, i);
        printf("Font: ID: %d, Name: %s\n", font.id, font.name);
        
        if (0 == strcmp(font.name, wanted_font))
            { font_sans = font; found = true; }
    }

    if (!found)
    {
        fprintf(stderr, "Couldn't find the '%s' font\n", wanted_font);
        return -1;
    }
    
    // iterate through the mode table
    // and lets look for a mode using this font, size 16, and antialiasing
    // (please edit this line if you used different sizes)
    const bf3_fp26 wanted_size = BF3_ENCODE_FP26(16);
    bf3_mode mode_sans16;
    found = false;
    
    for (int i = 0; i < info.num_modes; i++)
    {
        bf3_mode mode;
        bf3_mode_get(&mode, hdr, i);
        float size = BF3_DECODE_FP26(mode.size);
        
        printf("Mode: ID: %d, Font ID: %d, Size: %.2f\n",
            mode.mode_id, mode.font_id, size);
        
        if ((mode.font_id == font_sans.id) && (mode.size == wanted_size) && (mode.antialias))
            { mode_sans16 = mode; found=true; }
    }
    
    if (!found)
    {
        fprintf(stderr, "Couldn't find the size 16 AA font mode we wanted\n");
        return -1;
    }
    
    printf("Found the font mode we wanted.\n");
    printf("Lineheight: %.2fpx\n", BF3_DECODE_FP26(mode_sans16.lineheight));
    printf("Underline center position relative to baesline: %.2fpx\n",
        BF3_DECODE_FP26(mode_sans16.underline_position));
    printf("Underline thickness: %.2fpx\n",
        BF3_DECODE_FP26(mode_sans16.underline_thickness));
    
    // iterate through the glyph set table
    // and lets look for a glyph set using our chosen mode with the name
    // "ALL" (you might use a different name, like "FPS", if you had a reduced
    // character set as an optimisation for a specific purpose)
    // (please edit this line if you used different names)
    const char *wanted_table_name = "ALL";
    bf3_table table_sans16_all;
    found = false;
    
    for (int i = 0; i < info.num_tables; i++)
    {
        bf3_table table;
        bf3_table_get(&table, hdr, i);
        
        printf("Table: for mode ID: %d, glyph set name: %s\n",
            table.mode_id, table.name);
        
        if ((table.mode_id == mode_sans16.mode_id) && (0 == strcmp(table.name, wanted_table_name)))
            { table_sans16_all = table; found=true; }
    }
    
    if (!found)
    {
        fprintf(stderr, "Couldn't find the table %s for the font mode we wanted\n", wanted_table_name);
        return -1;
    }

    // allocate room for the metrics and kerning information
    char *metrics = malloc(table_sans16_all.metrics_size);
    if (!metrics) { fprintf(stderr, "Malloc error (metrics)\n"); return -1; }
    char *kerning = malloc(table_sans16_all.kerning_size);
    if (!kerning) { fprintf(stderr, "Malloc error (kerning)\n"); return -1; }
    
    // (debugging - you don't need to care about these next two commented lines)
    // printf("metrics offset %d size %d\n", table_sans16_all.metrics_offset, table_sans16_all.metrics_size);
    // printf("kerning offset %d size %d\n", table_sans16_all.kerning_offset, table_sans16_all.kerning_size);
    
    // load the metrics and kerning information
    if (!bf3_metrics_load(&data_reader, metrics, &table_sans16_all))
        { fprintf(stderr, "Error reading font metrics\n"); return -1; }
    
    if (!bf3_kerning_load(&data_reader, kerning, &table_sans16_all))
        { fprintf(stderr, "Error reading font kerning information\n"); return -1; }
    
    
    
    free(kerning);
    free(metrics);
    free(hdr);
    fclose(fimage);
    fclose(fdata);
}


