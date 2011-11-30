#include <stdlib.h>
#include <png.h>
#include <GL/gl.h>
#include <GL/glu.h>

/* Ripped from the libpng manual */
GLenum readpng(const char *filename) {
   FILE *fp = fopen(filename, "rb");
   png_byte header[8];
   png_structp png_ptr;
   png_infop info_ptr, end_info;

   if (!fp) {
      fprintf(stderr, "%s ", filename);
      perror("fopen");
      return 0;
   }
   fread(header, 1, 8, fp);
   if(png_sig_cmp(header, 0, 8))
   {
      fprintf(stderr, "%s: Not a PNG image!\n", filename);
      return 0;
   }

   png_ptr = png_create_read_struct(
      PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);
   if(!png_ptr)
      return 0;

   info_ptr = png_create_info_struct(png_ptr);
   if(!info_ptr) {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return 0;
   }

   end_info = png_create_info_struct(png_ptr);
   if(!end_info) {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return 0;
   }

   /* Set up jump target for libpng errors */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      fprintf(stderr, "libpng error!\n");
      fclose(fp);
      return 0;
   }

   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);
   png_read_png(png_ptr, info_ptr, 0, NULL);

   /* Make sure the image is in the format we want */
   int width = png_get_image_width(png_ptr, info_ptr);
   int height = png_get_image_height(png_ptr, info_ptr);
   if(png_get_bit_depth(png_ptr, info_ptr) != 8)
      fprintf(stderr, "Need an 8 bit/color image!\n");
   int type = png_get_color_type(png_ptr, info_ptr);
   if(type != PNG_COLOR_TYPE_RGB &&
      type != PNG_COLOR_TYPE_RGBA)
      fprintf(stderr, "Need an RGB or RGBA image!\n");
   int Bpp = (type==PNG_COLOR_TYPE_RGB)?3:4;

   png_bytepp rows = png_get_rows(png_ptr, info_ptr);
   char *tmp = (char*)malloc(width*height*Bpp);
   for(int y=0; y < height; y++)
      memcpy(tmp+width*y*Bpp, rows[height-y-1], width*Bpp);

   GLenum texnum;
   glGenTextures(1, &texnum);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, texnum);
   glTexImage2D(GL_TEXTURE_2D, 0, Bpp, width, height, 0,
      (type==PNG_COLOR_TYPE_RGB)?GL_RGB:GL_RGBA, GL_UNSIGNED_BYTE, tmp);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   //png_read_end(png_ptr, NULL);
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

   return texnum;
}
