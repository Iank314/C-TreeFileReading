
#include "image.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Image *load_image(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) 
    {
        ERROR("Failed to open file: %s", filename);
        return NULL;
    }

    char format[3];
    if (fscanf(file, "%2s", format) != 1 || strcmp(format, "P3") != 0)
    {
        ERROR("Incorrect or unsupported format: %s", format);
        fclose(file);
        return NULL;
    }

    unsigned short width, height, max_value;
    if (fscanf(file, "%hu %hu %hu", &width, &height, &max_value) != 3)
    {
        ERROR("Failed to read width, height, or max_value.");
        fclose(file);
        return NULL;
    }

    if (max_value != 255)
    {
        ERROR("Unsupported max color value: %hu", max_value);
        fclose(file);
        return NULL;
    }

    Image *image = (Image *)malloc(sizeof(Image));
    if (!image)
    {
        ERROR("Memory allocation for image structure failed.");
        fclose(file);
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->data = (unsigned char *)malloc(3 * width * height);  
    if (!image->data)
    {
        ERROR("Memory allocation for image data failed.");
        free(image);
        fclose(file);
        return NULL;
    }

    unsigned int r, g, b;
    unsigned int pixel_count = 0;

    while (pixel_count < width * height)
    {
        int read = fscanf(file, "%u %u %u", &r, &g, &b);
        if (read == 3)
        {
            image->data[pixel_count * 3] = (unsigned char)r;
            image->data[pixel_count * 3 + 1] = (unsigned char)g;
            image->data[pixel_count * 3 + 2] = (unsigned char)b;
            pixel_count++;
        }
        else
        {
            char c;
            do {
                c = fgetc(file);
                if (c == '#') 
                    while (fgetc(file) != '\n' && !feof(file));
            } while (c != '\n' && c != EOF);
        }
    }

    if (pixel_count != width * height)
    {
        ERROR("Mismatch in pixel data count. Expected %d, got %d", width * height, pixel_count);
        free(image->data);
        free(image);
        fclose(file);
        return NULL;
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
