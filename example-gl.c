// Example program loading bakefont3 data and using it to display text

// COMPILE:
//     gcc -std=c99 example-gl.c bakefont3.c lib/png.c lib/gl3w.c lib/utf8.c -lm -lglfw -lGL -lpng -ldl  -Wall -Wextra -o example-gl.bin
// USAGE:
//     ./example-gl.bin example/test.bf3 example/test-rgba.png

// NOTE FOR AUTHOR - OpenGL functionality was butchered from previous projects
// * web/___/galaxy-fleet-tactics/src/typescript/*.ts
// * Code/OLD/LIDS/src/graphics/*.c

#include "bakefont3.h"

#include "lib/gl3w.h" // modern GL.h
#include <GLFW/glfw3.h>

#include <stdlib.h> // malloc, free
#include <string.h> // strcmp
#include <stdio.h>
#include <errno.h>
#include <png.h>
#include <assert.h>

// from png.c
typedef struct image image;
struct image
{
    int width;
    int height;
    unsigned char *rgba;
};

bool check_if_png(FILE *fp);
bool read_png(image *img, FILE *fp);

// utf8_decode.h

#define UTF8_END   -1
#define UTF8_ERROR -2

int  utf8_decode_at_byte();
int  utf8_decode_at_character();
void utf8_decode_init(const char *p, int length);
int  utf8_decode_next();

// simple utf8-aware strlen
// not the fastest, but surely the simplest
// from http://canonical.org/~kragen/strlen-utf8.html
// (see also http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html)
size_t utf8len(char *s)
{
  int i = 0, j = 0;
  while (s[i]) {
    if ((s[i] & 0xc0) != 0x80) j++;
    i++;
  }
  return (size_t) j;
}


static const char *fragment_shader = "\
 \
 #version 130\n\
 precision mediump float;\
 \
 uniform sampler2D sample0; \
 \
 in vec2 uv;\
 in vec4 mask;\
 in vec4 color;\
 out vec4 frag_color;\
 \
 void main(void)\
 {\
    vec4 alpha = texture(sample0, uv) * mask; \
    float opacity = min(1.0, alpha.x + alpha.y + alpha.z + alpha.w); \
    frag_color = vec4(color.xyz, color.w * opacity); \
 }\
\n";

static const char *vertex_shader = "\
 \
 #version 130\n\
 \
 uniform mat4 projection_matrix; \
 \
 in vec2 attrib_xy; \
 in vec2 attrib_uv; \
 in vec4 attrib_mask; \
 in vec4 attrib_color; \
 \
 out vec2 uv; \
 out vec4 mask; \
 out vec4 color; \
 \
 void main(void) \
 { \
     uv = attrib_uv; \
     mask = attrib_mask; \
     color = attrib_color; \
     gl_Position = vec4(attrib_xy, 0.0, 1.0) * projection_matrix; \
 } \
\n";


// make an OpenGL texture from image data
GLuint new_texture2D(int width, int height, /*const */ unsigned char *pixel_data)
{
    GLuint name[1];
    
    glGenTextures(1, name);
    glBindTexture(GL_TEXTURE_2D, name[0]);
    
    glTexImage2D
    (
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        pixel_data
    );
    
    return name[0];
}


// compile a shader program
GLuint make_shader
(
    const GLchar *frag_src,
    const GLint  frag_src_len,
    const GLchar *vertex_src,
    const GLint  vertex_src_len
)
{
    // create two shaders
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    
    // attach sources
    glShaderSource(fragment, 1, &frag_src,   &frag_src_len);
    glShaderSource(vertex,   1, &vertex_src, &vertex_src_len);
    
    // compile the shaders
    glCompileShader(fragment);
    glCompileShader(vertex);
    
    // check they compiled okay
    GLint frag_result, vertex_result;

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &frag_result);
    glGetShaderiv(vertex,   GL_COMPILE_STATUS, &vertex_result);

    assert(GL_TRUE == frag_result);
    assert(GL_TRUE == vertex_result);
    
    // Create and link the program
    GLuint program = glCreateProgram();
    assert(program);
    
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    
    assert(glIsProgram(program));
    
    return program;
}


// create a vertex buffer object
GLuint create_vbo(size_t size)
{
    GLuint name[1];
    glCreateBuffers(1, name);
    glBindBuffer(GL_ARRAY_BUFFER, name[0]);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);

    return name[0];
}


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
    image atlas;
    FILE *fimage = fopen(argv[2], "rb");
    if (!fimage) { fprintf(stderr, "Could not open %s\n", argv[2]); return -1; }
    if (!check_if_png(fimage)) { fprintf(stderr, "Not a PNG: %s\n", argv[2]); return -1; }
    if (!read_png(&atlas, fimage)) { fprintf(stderr, "Failed to laod PNG: %s\n", argv[2]); return -1; }
    fclose(fimage);
    printf("Texture atlas size: %dx%d\n", atlas.width, atlas.height);
    
    // a simple structure that tells bakefont3 how to read the data
    // (bakefont3 doesn't care about the file system)
    bf3_filelike data_reader = {(void *)fdata, read_FILE};

    // peek at the the header telling us what's in the bakefont3 file
    // to see how big the first chunk of information we need is
    size_t header_size = bf3_header_peek(&data_reader);
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

    if ((info.width < atlas.width) && (info.height < atlas.height))
        { fprintf(stderr, "WARNING: Supplied texture atlas is smaller than bakefont3 expected\n"); }

    // Iterate through the fonts table.
    // and let's look for a font named "Sans"
    // (please edit this line if you have given the fonts different names)
    const char *wanted_font = "Sans Bold";
    bf3_font font_sans;
    found = false;
    
    // Fonts are basically "Font ID" (number) and "Font Name" (string), where
    // the Font Name has been chosen by the person who generated the bakefont3
    // data file. Each Font ID is unique in the file.
    
    for (int i = 0; i < info.num_fonts; i++)
    {
        bf3_font font;
        bf3_font_get(&font, hdr, i);
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
        if ((mode.font_id == font_sans.id) && (mode.size == wanted_size) && (mode.antialias))
            { mode_sans16 = mode; found=true; }
    }
    
    if (!found)
    {
        fprintf(stderr, "Couldn't find the size 16 AA font mode we wanted\n");
        return -1;
    }
    
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
    
    // load the metrics and kerning information
    if (!bf3_metrics_load(metrics, &data_reader, &table_sans16_all))
        { fprintf(stderr, "Error reading font metrics\n"); return -1; }
    
    if (!bf3_kerning_load(kerning, &data_reader, &table_sans16_all))
        { fprintf(stderr, "Error reading font kerning information\n"); return -1; }
    
    // done with the underlying file now
    fclose(fdata);
    
    // we now have lookup tables that we can quickly index by a unicode code point
    
    // Lets move onto graphics...
    gl3wInit();

    GLFWwindow* window;
    
    // Initialize the library
    if (!glfwInit())
        { fprintf(stderr, "Couldn't initialise GLFW"); exit(-1); }

    // Create a windowed mode window and its OpenGL 3.0 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(640, 480, "Bakefont3 Example", NULL, NULL);
    if (!window)
        { fprintf(stderr, "Couldn't initialise window"); exit(-1); }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glClearColor(0.3, 0.3, 0.3, 1.0);

    // Upload the texture atlas
    GLuint atlas_texture = new_texture2D(atlas.width, atlas.height, atlas.rgba);
    
    // don't need a copy of this in memory any more
    free(atlas.rgba);
    
    // Make the texture active in texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // create a shader for drawing
    GLuint shader = make_shader(fragment_shader, strlen(fragment_shader),
        vertex_shader, strlen(vertex_shader));

    // make it the active shader (we only have one anyway)
    glUseProgram(shader);

    // get the shader uniform and attribute indexes
    GLint shader_attrib_xy = glGetAttribLocation(shader, "attrib_xy");
    GLint shader_attrib_uv = glGetAttribLocation(shader, "attrib_uv");
    GLint shader_attrib_mask = glGetAttribLocation(shader, "attrib_mask");
    GLint shader_attrib_color = glGetAttribLocation(shader, "attrib_color");
    GLint shader_uniform_sample0 = glGetUniformLocation(shader, "sample0");
    GLint shader_uniform_proj    = glGetUniformLocation(shader, "projection_matrix");

    // create a vbo for position and color data
    size_t max_vertexes = 16 * 1024;
    size_t max_glyphs = max_vertexes / 6; // ~85
    size_t vertex_bytes =  6 * sizeof(GL_FLOAT); // XYUV, MASK, COLOUR
    GLuint vbo_xy = create_vbo(max_vertexes * 2);
    GLuint vbo_uv = create_vbo(max_vertexes * 2);
    GLuint vbo_mask = create_vbo(max_vertexes);
    GLuint vbo_color = create_vbo(max_vertexes);

    // a 2D projection
    GLfloat projection_matrix[16] =
    {
        2.0f,  0.0f,  0.0f,      -1.0f,
        0.0f, -2.0f,  0.0f,       1.0f,
        0.0f,  0.0f, -2.0f/1.0f, -1.0f,
        0.0f,  0.0f,  0.0f,       1.0f
    };

    projection_matrix[0] /= 640.0f; // window width
    projection_matrix[5] /= 480.0f; // window height

    // set uniforms
    glUniformMatrix4fv(shader_uniform_proj, 1, GL_FALSE, projection_matrix);
    glUniform1i(shader_uniform_sample0, 0); // 0 => at GL_TEXTURE0

    // config OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // a buffer for uploading data
    float *tmp = malloc(max_vertexes * vertex_bytes);
    assert(tmp);

    // convert utf8 into a Unicode string
    char string_utf8[(max_glyphs * 4) + 1];
    strcpy(string_utf8, "Hello, world!\nThis is the Bakefont 3 test!\nDAVE DOVE www.example.org\nGBP £ Euro €\nWelsh: Ga i fynd i'r tŷ bach os gwelwch yn dda?");
    uint32_t string_utf32[max_glyphs];

    size_t len = strlen(string_utf8);
    if (len > max_glyphs) { len = max_glyphs; }
    utf8_decode_init(string_utf8, len);
    size_t index = 0;
    while (index < max_glyphs)
    {
        int scodepoint = utf8_decode_next();
        if (scodepoint < 0) { break; }
        
        string_utf32[index] = (uint32_t) scodepoint;
        
        index++;
    }
    size_t string_utf32_len = index;

    // loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // check for errors
        while (true)
        {
            GLenum error = glGetError();
            if (!error) { break; }
            printf("glGetError: %d\n", (int) error);
        }
    
        glClear(GL_COLOR_BUFFER_BIT);
        
        size_t vertexes = 0;
        
        int xoffset = 20;
        int yoffset = 20;
        int wordspacing = 6;
        
        // get the lineheight and round up to the nearest pixel
        int lineheight = BF3_DECODE_FP26_NEAREST(mode_sans16.lineheight);
        yoffset += lineheight;
        
        float *p = tmp;
        
        // write a densely packed structure with information about the
        // triangles to draw (counter-clockwise triangles)
        for (size_t i = 0; i < string_utf32_len; i++)
        {
            // look up the glyph metrics
            bf3_metric metric;
            uint32_t codepoint = string_utf32[i];
            
            if (codepoint == '\n') { yoffset += lineheight; xoffset = 20; continue; }
            
            if (!bf3_metric_get(&metric, metrics, codepoint)) { continue; }
            
            // nothing to render? e.g. space
            if (!metric.tex_d) { xoffset += wordspacing; continue; }
            
            // based on the layer (metric.tex_z),
            // choose a mask colour that will extract from the channel we want
            unsigned char mask[4] = {0, 0, 0, 0};
            switch (metric.tex_z)
            {
                case 0: mask[0] = 255; break; // red
                case 1: mask[1] = 255; break; // green
                case 2: mask[2] = 255; break; // blue
                case 3: mask[3] = 255; break; // alpha
            }
            
            // layout - see
            // https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
            // right side bearing = advance_width - left_side_bearing - (xMax-xMin)
            int advance = BF3_DECODE_FP26_NEAREST(metric.hadvance);
            int lsb = BF3_DECODE_FP26_NEAREST(metric.hbx);
            //int rsb = advance - lsb - metric.tex_w;
            int tsb = BF3_DECODE_FP26_NEAREST(metric.hby);
            int bitmap_left = metric.bitmap_left;
            int bitmap_top = metric.bitmap_top;
            
            // kerning
            int xkern = 0;
            if (i > 0) // don't kern the first letter
            {
                uint32_t last_codepoint = string_utf32[i-1];
                bf3_kpair kpair;
                
                // not all fonts have kerning information
                if (bf3_kpair_get(&kpair, kerning, last_codepoint, codepoint))
                {
                    xkern = kpair.x;
                }
            }
            
            // compute x/y, size, and texture u/v
            float x0 = ((float) xoffset + lsb + xkern);
            float y0 = ((float) yoffset - tsb); // realtive to baseline
            float x1 = x0 + metric.tex_w;
            float y1 = y0 + metric.tex_h;
            float u0 = ((float) metric.tex_x) / ((float)info.width); // could be optimised
            float v0 = ((float) metric.tex_y) / ((float)info.height); // could be optimised
            float u1 = ((float) (metric.tex_x + metric.tex_w)) / ((float)info.width); // could be optimised
            float v1 = ((float) (metric.tex_y + metric.tex_h)) / ((float)info.height); // could be optimised
            
            // lets compute per-vertex colors (any linear gradient you like!)
            unsigned char color_top[4] = {255, 255, 0, 255};
            unsigned char color_bottom[4] = {255, 0, 255, 255};
            
            // move the cursor across
            xoffset += advance + xkern;

            
            /*
            printf("%d %d %d %d %d %d (%d, %d) => %.4f %.4f %.4f %.4f / %.4f %.4f %.4f %.4f\n",
                (int) i, metric.tex_x, metric.tex_y, metric.tex_z, metric.tex_w, metric.tex_h,
                info.width, info.height,
                u0, v0, u1, v1,
                x0, y0, x1, y1);
                */
            
            // fill a quad (triangles go counter-clockwise)
            
            #define P(x) (*p++) = (x)
            
            // cheat and pack 4 bytes into a float to save space
            #define B(x) memcpy(p++, &(x), 4);
            
            // triangle 1
            P(x0);
            P(y0);
            P(u0);
            P(v0);
            B(mask);
            B(color_top);
            
            P(x0);
            P(y1);
            P(u0);
            P(v1);
            B(mask);
            B(color_bottom);
            
            P(x1);
            P(y1);
            P(u1);
            P(v1);
            B(mask);
            B(color_bottom);
            
            // triangle 2
            
            P(x0);
            P(y0);
            P(u0);
            P(v0);
            B(mask);
            B(color_top);
            
            P(x1);
            P(y1);
            P(u1);
            P(v1);
            B(mask);
            B(color_bottom);
            
            P(x1);
            P(y0);
            P(u1);
            P(v0);
            B(mask);
            B(color_top);
            
            vertexes += 6;
        }
        
        // upload XY coordinates
        glBindBuffer(GL_ARRAY_BUFFER, vbo_xy);
        glEnableVertexAttribArray(shader_attrib_xy);
        glVertexAttribPointer(shader_attrib_xy, 2, GL_FLOAT, false, vertex_bytes, (void *) 0);
        // components (XY), stride in bytes, and offset
        glBufferData(GL_ARRAY_BUFFER, vertexes * vertex_bytes, tmp, GL_DYNAMIC_DRAW);
        
        // upload UV coordinates
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
        glEnableVertexAttribArray(shader_attrib_uv);
        glVertexAttribPointer(shader_attrib_uv, 2, GL_FLOAT, false, vertex_bytes, (char *) (2 * sizeof(GL_FLOAT)));
        // components (UV), stride in bytes, and offset
        glBufferData(GL_ARRAY_BUFFER, vertexes * vertex_bytes, tmp, GL_DYNAMIC_DRAW);
        
        // upload mask information
        glBindBuffer(GL_ARRAY_BUFFER, vbo_mask);
        glEnableVertexAttribArray(shader_attrib_mask);
        glVertexAttribPointer(shader_attrib_mask, 4, GL_UNSIGNED_BYTE, true, vertex_bytes, (char *) (4 * sizeof(GL_FLOAT)));
        // components (UV), stride in bytes, and offset
        glBufferData(GL_ARRAY_BUFFER, vertexes * vertex_bytes, tmp, GL_DYNAMIC_DRAW);
        
        // upload colour information
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
        glEnableVertexAttribArray(shader_attrib_color);
        glVertexAttribPointer(shader_attrib_color, 4, GL_UNSIGNED_BYTE, true, vertex_bytes, (char *) (5 * sizeof(GL_FLOAT)));
        // components (UV), stride in bytes, and offset
        glBufferData(GL_ARRAY_BUFFER, vertexes * vertex_bytes, tmp, GL_DYNAMIC_DRAW);
        
        
        // make the texture active in texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlas_texture);
        
        // finally draw
        glDrawArrays(GL_TRIANGLES, 0, vertexes);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    free(kerning);
    free(metrics);
    free(hdr);

    return 0;
}


