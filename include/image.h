#ifndef __IMAGE_H
#define __IMAGE_H

#include <stdio.h>
#include <stdlib.h>

#define INFO(...) do {fprintf(stderr, "[          ] [ INFO ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr);} while(0)
#define ERROR(...) do {fprintf(stderr, "[          ] [ ERR  ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr);} while(0) 

typedef struct Image {
    char placeholder[1];  // This is a placeholder. You should not use this struct.
} Image;

Image *load_image(char *filename);
void delete_image(Image *image);
unsigned char get_image_intensity(Image *image, unsigned int row, unsigned int col);
unsigned short get_image_width(Image *image);
unsigned short get_image_height(Image *image);

unsigned int hide_message(char *message, char *input_filename, char *output_filename);
char *reveal_message(char *input_filename);
unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename);
void reveal_image(char *input_filename, char *output_filename);

#endif // __IMAGE_H