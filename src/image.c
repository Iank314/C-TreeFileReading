#include "image.h"
#include <string.h>

Image *load_image(char *filename) 
{
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    char format[3];
    fscanf(file, "%2s", format);
    if (strcmp(format, "P3") != 0) 
    {
        fclose(file);
        return NULL;
    }

    unsigned short width, height, max_value;
    fscanf(file, "%hu %hu %hu", &width, &height, &max_value);

    Image *image = (Image *)malloc(sizeof(Image));
    if (!image) 
    {
        fclose(file);
        return NULL;
    }
    image->width = width;
    image->height = height;
    image->data = (unsigned char *)malloc(width * height * 3);
    if (!image->data) 
    {
        free(image);
        fclose(file);
        return NULL;
    }

    unsigned int r, g, b;
    unsigned int pixel_count = width * height;
    for (unsigned int i = 0; i < pixel_count; i++) 
    {
        while (fscanf(file, "%u", &r) != 1) 
        {
            while (fgetc(file) != '\n'); //
        }
        while (fscanf(file, "%u", &g) != 1) 
        {
            while (fgetc(file) != '\n');
        }
        while (fscanf(file, "%u", &b) != 1) 
        {
            while (fgetc(file) != '\n');
        }
        image->data[i * 3] = (unsigned char)r;
        image->data[i * 3 + 1] = (unsigned char)g;
        image->data[i * 3 + 2] = (unsigned char)b;
    }

    fclose(file);
    return image;
}
void delete_image(Image *image)
{
    if (image) 
    {
        free(image->data);
        free(image);
    }
}

unsigned char get_image_intensity(Image *image, unsigned int row, unsigned int col)
{
    if (!image || row >= image->height || col >= image->width) return 0;
    return image->data[row * image->width + col];
}

unsigned short get_image_width(Image *image)
{
    return image ? image->width : 0;
}

unsigned short get_image_height(Image *image)
{
    return image ? image->height : 0;
}

unsigned int hide_message(char *message, char *input_filename, char *output_filename)
{
    (void)message;
    (void)input_filename;
    (void)output_filename;
    return 0;
}

char *reveal_message(char *input_filename)
{
    (void)input_filename;
    return NULL;
}

unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename)
{
    (void)secret_image_filename;
    (void)input_filename;
    (void)output_filename;
    return 0;
}

void reveal_image(char *input_filename, char *output_filename)
{
    (void)input_filename;
    (void)output_filename;
}