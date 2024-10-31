
#include "image.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Image *load_image(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    char format[3];
    if (fscanf(file, "%2s", format) != 1 || strcmp(format, "P3") != 0)
    {
        fclose(file);
        return NULL;
    }

    unsigned short width, height;
    unsigned short max_value; // Declare max_value but do not use it
    if (fscanf(file, "%hu %hu %hu", &width, &height, &max_value) != 3)
    {
        fclose(file);
        return NULL; // Handle potential read failure
    }

    Image *image = (Image *)malloc(sizeof(Image));
    if (!image)
    {
        fclose(file);
        return NULL;
    }
    image->width = width;
    image->height = height;
    image->data = (unsigned char *)malloc(width * height);
    if (!image->data)
    {
        free(image);
        fclose(file);
        return NULL;
    }

    unsigned int intensity;
    unsigned int pixel_count = 0;

    while (pixel_count < width * height)
    {
        if (fscanf(file, "%u", &intensity) == 1)
        {
            image->data[pixel_count++] = (unsigned char)intensity;
        }
        else
        {
            char c;
            while ((c = fgetc(file)) != '\n' && c != EOF)
            {
                if (c == '#')
                {
                    while (fgetc(file) != '\n' && !feof(file));
                    break;
                }
            }
        }
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
