#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef IMAGE_H
#define IMAGE_H

typedef struct 
{
    unsigned short width;
    unsigned short height;
    unsigned char **pixels;
} 
Image;

Image *load_image(char *filename);
void delete_image(Image *image);
unsigned char get_image_intensity(Image *image, unsigned int row, unsigned int col);
unsigned short get_image_width(Image *image);
unsigned short get_image_height(Image *image);

#endif // IMAGE_H

void skip_comments(FILE *file) 
{
    int ch;
    while ((ch = fgetc(file)) == '#') 
    { 
        while (fgetc(file) != '\n');  
    }
    ungetc(ch, file); 
}

Image *load_image(char *filename) 
{
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    Image *image = malloc(sizeof(Image));
    if (!image) 
    {
        fclose(file);
        return NULL;
    }

    char format[3];
    if (fscanf(file, "%2s", format) != 1 || strcmp(format, "P3") != 0) 
    {
        free(image);
        fclose(file);
        return NULL;
    }

    skip_comments(file);

    if (fscanf(file, "%hu %hu", &image->width, &image->height) != 2) 
    {
        free(image);
        fclose(file);
        return NULL;
    }

    int max_value;
    if (fscanf(file, "%d", &max_value) != 1 || max_value != 255) 
    {
        free(image);
        fclose(file);
        return NULL;
    }

    image->pixels = malloc(image->height * sizeof(unsigned char *));
    if (!image->pixels) 
    {
        free(image);
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < image->height; i++) 
    {
        image->pixels[i] = malloc(image->width * sizeof(unsigned char));
        if (!image->pixels[i]) 
        {
            for (int k = 0; k < i; k++) free(image->pixels[k]);
            free(image->pixels);
            free(image);
            fclose(file);
            return NULL;
        }

        for (int j = 0; j < image->width; j++) 
        {
            int pixel_value;
            if (fscanf(file, "%d", &pixel_value) != 1) 
            {
                for (int k = 0; k <= i; k++) free(image->pixels[k]);
                free(image->pixels);
                free(image);
                fclose(file);
                return NULL;
            }
            image->pixels[i][j] = (unsigned char) pixel_value;
        }
    }

    fclose(file);
    return image;
}

void delete_image(Image *image) 
{
    if (image) 
    {
        for (int i = 0; i < image->height; i++)
        {
            free(image->pixels[i]);
        }
        free(image->pixels);
        free(image);
    }
}

unsigned char get_image_intensity(Image *image, unsigned int row, unsigned int col)
{
    if (row >= image->height || col >= image->width) 
    {
        return 0;
    }
    return image->pixels[row][col];
}

unsigned short get_image_width(Image *image) 
{
    return image->width;
}

unsigned short get_image_height(Image *image) 
{
    return image->height;
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
    return 1; 
}

void reveal_image(char *input_filename, char *output_filename) 
{
    (void)input_filename;
    (void)output_filename;
}