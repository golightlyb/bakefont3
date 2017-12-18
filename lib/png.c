// public domain by BSAG2017
// libpng decoder
// based on `libpng/example.c` from https://dev.w3.org/Amaya/libpng/example.c
// and http://www.libpng.org/pub/png/libpng-1.2.5-manual.html

#include <png.h>
#include <stdbool.h>
#include <stdlib.h> // malloc

typedef struct image image;
struct image
{
    int width;
    int height;
    unsigned char *rgba;
};


#define PNG_BYTES_TO_CHECK 4

/* Check to see if a file is a PNG file using png_sig_cmp().  png_sig_cmp()
 * returns zero if the image is a PNG and nonzero if it isn't a PNG.
 *
 * The function check_if_png() shown here, but not used, returns nonzero (true)
 * if the file can be opened and is a PNG, 0 (false) otherwise.
 *
 * If this call is successful, and you are going to keep the file open,
 * you should call png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK); once
 * you have created the png_ptr, so that libpng knows your application
 * has read that many bytes from the start of the file.  Make sure you
 * don't call png_set_sig_bytes() with more than 8 bytes read or give it
 * an incorrect number of bytes read, or you will either have read too
 * many bytes (your fault), or you are telling libpng to read the wrong
 * number of magic bytes (also your fault).
 *
 * Many applications already read the first 2 or 4 bytes from the start
 * of the image to determine the file type, so it would be easiest just
 * to pass the bytes to png_sig_cmp() or even skip that if you know
 * you have a PNG file, and call png_set_sig_bytes().
 */

bool check_if_png(FILE *fp)
{
   char buf[PNG_BYTES_TO_CHECK];

   /* Read in some of the signature bytes */
   if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
      return 0;

   return(!png_sig_cmp((png_bytep) buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}


bool read_png(image *img, FILE *fp)
{
    unsigned int sig_read = PNG_BYTES_TO_CHECK;

   png_structp png_ptr;
   png_infop info_ptr;
   //png_uint_32 width, height;
   //int bit_depth, color_type, interlace_type;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      return false;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return false;
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
      /* If we get here, we had a problem reading the file */
      return false;
   }

   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

    /* If we have already read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

   /*
     * If you have enough memory to read in the entire image at once,
    * and you need to specify only transforms that can be controlled
    * with one of the PNG_TRANSFORM_* bits (this presently excludes
    * dithering, filling, setting background, and doing gamma
    * adjustment), then you can read the entire image (including
    * pixels) into the info structure with this call:
    */
    png_read_png(png_ptr, info_ptr,
    PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, png_voidp_NULL);

   /* At this point you have read the entire image */
    
    img->width  = (int) png_get_image_width(png_ptr, info_ptr);
    img->height = (int) png_get_image_height(png_ptr, info_ptr);
    img->rgba = malloc(img->width * img->height * 4);
    
    // PNG has allocated rows for us.
    // Although this is less efficient, for ease of implementation lets
    // for now just copy this into our own structure.
    png_bytep *row_pointers;
    row_pointers = png_get_rows(png_ptr, info_ptr);
    
    for (int y = 0; y < img->height; y++)
    {
        for (int x = 0; x < img->width; x++)
        {
            for (int i = 0; i < 4; i++) // RGBA
            {
                img->rgba[(y * img->width) + (x * 4)+ i] = row_pointers[y][(x * 4)+ i];
            }
        }
    }
    
   /* clean up after the read, and free any memory allocated - REQUIRED */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    /* that's it */
    return true;
}


