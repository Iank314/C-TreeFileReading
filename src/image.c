

#include "image.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


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


    int ch;
    do
    {
        ch = fgetc(file);
        if (ch == '#') while (fgetc(file) != '\n');
    }
    while (ch == '#' || ch == '\n');
    ungetc(ch, file);


    unsigned short width, height, max_value;
    if (fscanf(file, "%hu %hu %hu", &width, &height, &max_value) != 3)
    {
        fclose(file);
        return NULL;
    }


    if (max_value != 255)
    {
        fclose(file);
        return NULL;
    }


    Image *image = (Image *)malloc(sizeof(Image));
    if (!image)
    {
        fclose(file);
        return NULL;
    }
    image->width = width;
    image->height = height;
    image->data = (unsigned char *)malloc(3 * width * height);
    if (!image->data)
    {
        free(image);
        fclose(file);
        return NULL;
    }


    unsigned int r, g, b;
    unsigned int pixel_count = 0;


    for (unsigned int i = 0; i < width * height; i++)
    {
        if (fscanf(file, "%u %u %u", &r, &g, &b) != 3)
        {
            free(image->data);
            free(image);
            fclose(file);
            return NULL;
        }
        image->data[pixel_count++] = (unsigned char)r;
        image->data[pixel_count++] = (unsigned char)g;
        image->data[pixel_count++] = (unsigned char)b;
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
    return image->data[(row * image->width + col) * 3];
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
    Image *image = load_image(input_filename);
    if (!image) return 0;

    unsigned int message_length = strlen(message);
    unsigned int pixels_needed = (message_length + 1) * 8; 

    if (pixels_needed > image->width * image->height * 3) 
    {
        delete_image(image);
        return 0; 
    }

    unsigned int char_index = 0;
    unsigned int bit_index = 0;

    for (unsigned int i = 0; i < pixels_needed; i++) 
    {
        unsigned char *pixel = &image->data[i];
        unsigned char bit = (message[char_index] >> (7 - bit_index)) & 1;

        *pixel = (*pixel & ~1) | bit;  

        bit_index++;
        if (bit_index == 8) 
        {
            char_index++;
            bit_index = 0;
        }
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) 
    {
        delete_image(image);
        return 0;
    }

    fprintf(output_file, "P3\n%d %d\n255\n", image->width, image->height);
    for (unsigned int i = 0; i < image->width * image->height * 3; i++) 
    {
        fprintf(output_file, "%d ", image->data[i]);
        if ((i + 1) % (image->width * 3) == 0) fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete_image(image);

    return message_length;
}

char *reveal_message(char *input_filename) 
{
    Image *image = load_image(input_filename);
    if (!image) return NULL;

    unsigned int max_chars = (image->width * image->height * 3) / 8;
    char *message = (char *)malloc(max_chars + 1);
    if (!message) 
    {
        delete_image(image);
        return NULL;
    }

    unsigned int char_index = 0;
    unsigned int bit_index = 0;
    message[0] = 0;

    for (unsigned int i = 0; i < image->width * image->height * 3; i++) 
    {
        message[char_index] |= (image->data[i] & 1) << (7 - bit_index);
        bit_index++;

        if (bit_index == 8) 
        {
            if (message[char_index] == '\0') break;
            char_index++;
            bit_index = 0;
            message[char_index] = 0;
        }
    }

    delete_image(image);
    return message;
}
unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename) 
{
    Image *image = load_image(input_filename);
    Image *secret_image = load_image(secret_image_filename);
    if (!image || !secret_image) {
        if (image) delete_image(image);
        if (secret_image) delete_image(secret_image);
        return 0;
    }

    if (secret_image->width * secret_image->height * 3 > image->width * image->height * 3) 
    {
        delete_image(image);
        delete_image(secret_image);
        return 0;
    }

    for (unsigned int i = 0; i < secret_image->width * secret_image->height * 3; i++) 
    {
        unsigned char bit = (secret_image->data[i] & 1);
        image->data[i] = (image->data[i] & ~1) | bit;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        delete_image(image);
        delete_image(secret_image);
        return 0;
    }

    fprintf(output_file, "P3\n%d %d\n255\n", image->width, image->height);
    for (unsigned int i = 0; i < image->width * image->height * 3; i++) 
    {
        fprintf(output_file, "%d ", image->data[i]);
        if ((i + 1) % (image->width * 3) == 0) fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete_image(image);
    delete_image(secret_image);

    return secret_image->width * secret_image->height;
}

void reveal_image(char *input_filename, char *output_filename) 
{
    Image *image = load_image(input_filename);
    if (!image) return;

    Image *secret_image = (Image *)malloc(sizeof(Image));
    if (!secret_image) {
        delete_image(image);
        return;
    }

    secret_image->width = image->width;
    secret_image->height = image->height;
    secret_image->data = (unsigned char *)malloc(image->width * image->height * 3);
    if (!secret_image->data) {
        delete_image(image);
        free(secret_image);
        return;
    }

    for (unsigned int i = 0; i < image->width * image->height * 3; i++) {
        secret_image->data[i] = (image->data[i] & 1) ? 255 : 0;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        delete_image(image);
        delete_image(secret_image);
        return;
    }

    fprintf(output_file, "P3\n%d %d\n255\n", secret_image->width, secret_image->height);
    for (unsigned int i = 0; i < secret_image->width * secret_image->height * 3; i++) {
        fprintf(output_file, "%d ", secret_image->data[i]);
        if ((i + 1) % (secret_image->width * 3) == 0) fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete_image(image);
    delete_image(secret_image);
}