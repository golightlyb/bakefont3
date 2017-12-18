// Example program loading bakefont3 data and using it to display text

// COMPILE:
//     gcc -std=c99 example.c bakefont3.c -lm -Wall -Wextra -o example.bin
// USAGE:
//     ./example.bin example/test.bf3 example/test-rgba.png


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


void print_metric_info(bf3_metric *metric)
{
    printf("Metric:");
    printf("\tCodepoint %u\n", metric->codepoint);
    printf("\tTexture position (x, y, channel): %d, %d, %d\n",
        metric->tex_x, metric->tex_y, metric->tex_z);
    printf("\tTexture size (x, y, depth): %d, %d, %d\n",
        metric->tex_w, metric->tex_h, metric->tex_d);
    
    printf("\tHorizontal left side bearing: %.2f\n",  BF3_DECODE_FP26(metric->hbx));
    printf("\tHorizontal top side bearing: %.2f\n",   BF3_DECODE_FP26(metric->hby));
    printf("\tHorizontal advance: %.2f\n",            BF3_DECODE_FP26(metric->hadvance));
    printf("\tVertical left side bearing: %.2f\n",    BF3_DECODE_FP26(metric->vbx));
    printf("\tVertical top side bearing: %.2f\n",     BF3_DECODE_FP26(metric->vby));
    printf("\tVertical advance: %.2f\n",              BF3_DECODE_FP26(metric->vadvance));
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
    
    // a simple structure that tells bakefont3 how to read the data
    // (bakefont3 doesn't care about the file system)
    bf3_filelike data_reader = {(void *)fdata, read_FILE};

    // peek at the the header telling us what's in the bakefont3 file
    // to see how big the first chunk of information we need is
    size_t header_size = bf3_header_peek(&data_reader);
    printf("header size %u (bytes)\n", (unsigned int) header_size);
    if (!header_size) { fprintf(stderr, "Not a bf3 file %s\n", argv[1]); return -1; }
    
    // allocate a buffer to hold the header information
    // don't free() this until you're done using bakefont3 for anything
    // related to this file
    char *hdr = malloc(header_size);
    if (!hdr) { fprintf(stderr, "Malloc error (header)\n"); return -1; }
    
    // parse the header and store some initial info
    bf3_info info;
    if (!bf3_header_load(&info, &data_reader, hdr, header_size))
        { fprintf(stderr, "Error reading header\n"); return -1; }
    printf("header dimensions = %dx%dx%d\n",
        info.width, info.height, info.depth);
    printf("number of fonts: %d\n", info.num_fonts);
    printf("number of modes: %d\n", info.num_modes);
    printf("number of tables: %d\n", info.num_tables);

    // Iterate through the fonts table.
    // and let's look for a font named "Sans"
    // (please edit this line if you have given the fonts different names)
    const char *wanted_font = "Sans";
    bf3_font font_sans;
    found = false;
    
    // Fonts are basically "Font ID" (number) and "Font Name" (string), where
    // the Font Name has been chosen by the person who generated the bakefont3
    // data file. Each Font ID is unique in the file.
    
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
    
    // Font Modes are basically "Mode ID" (number), a "Font ID" (number)
    // referencing the previous fonts table, the font size (1pt==1px at 72dpi)
    // (a Real number encoded as FP26), and whether or not the font was
    // rasterised with antialiasing/hinting (boolean).
    // Each Mode ID is unique in a file.
    
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
    
    
    // Font Modes also give you some information that's true across all glyphs
    // for that Mode. Again these are Real numbers encoded as FP26.
    printf("Found the font mode we wanted.\n");
    printf("Lineheight: %.2fpx\n", BF3_DECODE_FP26(mode_sans16.lineheight));
    printf("Underline center position relative to baesline: %.2fpx\n",
        BF3_DECODE_FP26(mode_sans16.underline_position)); // down is negative?
    printf("Underline thickness: %.2fpx\n",
        BF3_DECODE_FP26(mode_sans16.underline_thickness));
    
    // Iterate through the glyphset table
    // and lets look for a glyph set using our chosen mode with the name "ALL"
    // (please edit this line if you used different names)
    const char *wanted_table_name = "ALL";
    bf3_table table_sans16_all;
    found = false;
    
    // The glyphset table tells us, for a given Mode ID, how we are later going
    // to load information about the Glyph metrics (like the size of a glyph,
    // where its position is in the texture atlas) and kerning (when you
    // display two characters next to eachother, how you tweak the offset
    // to make them look nicer).
    
    // Glyphset tables are identified by a name, and have information on
    // different sets of glyphs. A single font mode might have several tables
    // for different purposes, so that looking up information can be fast.
    // e.g. a FPS display mode that only needs the glyphs for "FPS: 0123456789"
    // e.g. a fancy large title mode that only needs the glyphs for A-Z
    // e.g. a general-purpose mode that needs all glyphs supported by a font
    //      (as a convention, please use the name "ALL" for this one)
    // e.g. a tileset mode for graphics, like how the game Dwarf Fortress only
    //      uses Codepage 437 to render its graphics.
    
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
    
    // we now have lookup tables that we can quickly index by a unicode code point
    
    // lets query a couple of glyph metrics...
    // these tell us how to draw the glyph from the texture atlas
    
    // (a function that returns code points from utf-8 would be useful)
    uint32_t codepoint_a = (unsigned char) 'a';
    uint32_t codepoint_omega = 0x03A9; // Ω;
    
    bf3_metric metric;
    if (bf3_metric_get(&metric, metrics, codepoint_a))
    {
        printf("Found a!\n");
        print_metric_info(&metric);
    }
    else
    {
        printf("Didn't find a\n");
    }
    
    
    if (bf3_metric_get(&metric, metrics, codepoint_omega))
    {
        printf("Found Ω!\n");
        print_metric_info(&metric);
    }
    else
    {
        printf("Didn't find Ω\n");
    }
    
    
    // lets query a couple of kerning pairs...
    // these tell us how to adjust two characters so that they look nice
    // when combined.
    
    // WWW. <- the dot moves closer to the W in some fonts
    uint32_t codepoint_W = (unsigned char) 'W';
    uint32_t codepoint_period = (unsigned char) '.';
    
    // BRAVO <- the A and V move closer in some fonts
    uint32_t codepoint_A = (unsigned char) 'A';
    uint32_t codepoint_V = (unsigned char) 'V';
    
    bf3_kpair kpair;
    if (bf3_kpair_get(&kpair, kerning, codepoint_W, codepoint_period))
    {
        printf("Found ('W','.') kerning pair!\n");
        printf("X offset: %d (grid fit) or %.2f (no grid fit)\n",
            kpair.x, BF3_DECODE_FP26(kpair.xf));
    }
    else
    {
        printf("Didn't find ('W','.') kerning pair\n");
    }
    
    if (bf3_kpair_get(&kpair, kerning, codepoint_A, codepoint_V))
    {
        printf("Found ('A','V') kerning pair!\n");
        printf("X offset: %d (grid fit) or %.2f (no grid fit)\n",
            kpair.x, BF3_DECODE_FP26(kpair.xf));
    }
    else
    {
        printf("Didn't find ('A','V') kerning pair\n");
    }
    
    
    free(kerning);
    free(metrics);
    free(hdr);
    fclose(fimage);
    fclose(fdata);
}


