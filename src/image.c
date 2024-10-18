#include "image.h"

Image *load_image(char *filename) {    
    (void)filename;
    return NULL;
}

void delete_image(Image *image) {
    (void)image;
}

unsigned short get_image_width(Image *image) {
    (void)image;
    return 0;
}

unsigned short get_image_height(Image *image) {
    (void)image;
    return 0;
}

unsigned char get_image_intensity(Image *image, unsigned int row, unsigned int col) {
    (void)image;
    (void)row;
    (void)col;
    return 0;
}

unsigned int hide_message(char *message, char *input_filename, char *output_filename) {
    (void)message;
    (void)input_filename;
    (void)output_filename;
    return 0;
}

char *reveal_message(char *input_filename) {
    (void)input_filename;
    return NULL;
}

unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename) {
    (void)secret_image_filename;
    (void)input_filename;
    (void)output_filename;
    return 10;
}

void reveal_image(char *input_filename, char *output_filename) {
    (void)input_filename;
    (void)output_filename;
}
