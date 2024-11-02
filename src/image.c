
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
    Image *input_image = load_image(input_filename);
    if (!input_image)
    {
        ERROR("Failed to load input image.");
        return 0;
    }

    unsigned int char_count = 0;
    unsigned int message_len = strlen(message);
    unsigned int pixels_needed = (message_len + 1) * 8; // +1 for null character

    if (input_image->width * input_image->height * 3 < pixels_needed)
    {
        ERROR("Image is too small to hide the message.");
        delete_image(input_image);
        return 0;
    }

    unsigned char *data = input_image->data;
    for (unsigned int i = 0; i < message_len + 1; i++)
    {
        unsigned char c = (i < message_len) ? message[i] : '\0';
        for (int bit = 0; bit < 8; bit++)
        {
            data[char_count] = (data[char_count] & ~1) | ((c >> (7 - bit)) & 1);
            char_count++;
        }
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        ERROR("Failed to open output file.");
        delete_image(input_image);
        return 0;
    }

    fprintf(output_file, "P3\n%d %d\n255\n", input_image->width, input_image->height);
    for (unsigned int i = 0; i < input_image->width * input_image->height * 3; i += 3)
    {
        fprintf(output_file, "%d %d %d ", data[i], data[i + 1], data[i + 2]);
        if ((i / 3 + 1) % input_image->width == 0) fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete_image(input_image);
    return message_len;
}

char *reveal_message(char *input_filename)
{
    Image *input_image = load_image(input_filename);
    if (!input_image)
    {
        ERROR("Failed to load input image.");
        return NULL;
    }

    unsigned char *data = input_image->data;
    unsigned int char_count = 0;
    char *message = (char *)malloc(1024);
    if (!message)
    {
        delete_image(input_image);
        return NULL;
    }

    while (1)
    {
        unsigned char c = 0;
        for (int bit = 0; bit < 8; bit++)
        {
            c = (c << 1) | (data[char_count] & 1);
            char_count++;
        }

        if (c == '\0') break;
        message[strlen(message)] = c;
    }

    delete_image(input_image);
    return message;
}

unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename)
{
    Image *input_image = load_image(input_filename);
    Image *secret_image = load_image(secret_image_filename);
    if (!input_image || !secret_image)
    {
        ERROR("Failed to load images.");
        if (input_image) delete_image(input_image);
        if (secret_image) delete_image(secret_image);
        return 0;
    }

    unsigned int total_pixels = input_image->width * input_image->height * 3;
    unsigned int pixels_needed = (2 + secret_image->width * secret_image->height) * 8;

    if (total_pixels < pixels_needed)
    {
        ERROR("Input image is too small to hide the secret image.");
        delete_image(input_image);
        delete_image(secret_image);
        return 0;
    }

    unsigned char *data = input_image->data;
    unsigned int char_count = 0;

    for (int i = 0; i < 8; i++)
    {
        data[char_count] = (data[char_count] & ~1) | ((secret_image->width >> (7 - i)) & 1);
        char_count++;
    }

    for (int i = 0; i < 8; i++)
    {
        data[char_count] = (data[char_count] & ~1) | ((secret_image->height >> (7 - i)) & 1);
        char_count++;
    }

    for (int row = 0; row < secret_image->height; row++)
    {
        for (int col = 0; col < secret_image->width; col++)
        {
            unsigned char intensity = get_image_intensity(secret_image, row, col);
            for (int bit = 0; bit < 8; bit++)
            {
                data[char_count] = (data[char_count] & ~1) | ((intensity >> (7 - bit)) & 1);
                char_count++;
            }
        }
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        ERROR("Failed to open output file.");
        delete_image(input_image);
        delete_image(secret_image);
        return 0;
    }

    fprintf(output_file, "P3\n%d %d\n255\n", input_image->width, input_image->height);
    for (unsigned int i = 0; i < total_pixels; i += 3)
    {
        fprintf(output_file, "%d %d %d ", data[i], data[i + 1], data[i + 2]);
        if ((i / 3 + 1) % input_image->width == 0) fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete_image(input_image);
    delete_image(secret_image);
    return 1;
}

void reveal_image(char *input_filename, char *output_filename)
{
    Image *input_image = load_image(input_filename);
    if (!input_image)
    {
        ERROR("Failed to load input image.");
        return;
    }

    unsigned char *data = input_image->data;
    unsigned int char_count = 0;

    unsigned short width = 0, height = 0;

    for (int i = 0; i < 8; i++)
        width = (width << 1) | (data[char_count++] & 1);

    for (int i = 0; i < 8; i++)
        height = (height << 1) | (data[char_count++] & 1);

    Image *secret_image = (Image *)malloc(sizeof(Image));
    secret_image->width = width;
    secret_image->height = height;
    secret_image->data = (unsigned char *)malloc(width * height * 3);

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            unsigned char intensity = 0;
            for (int bit = 0; bit < 8; bit++)
            {
                intensity = (intensity << 1) | (data[char_count++] & 1);
            }
            secret_image->data[(row * width + col) * 3] = intensity;
            secret_image->data[(row * width + col) * 3 + 1] = intensity;
            secret_image->data[(row * width + col) * 3 + 2] = intensity;
        }
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        ERROR("Failed to open output file.");
        delete_image(input_image);
        delete_image(secret_image);
        return;
    }

    fprintf(output_file, "P3\n%d %d\n255\n", width, height);
    for (unsigned int i = 0; i < width * height * 3; i += 3)
    {
        fprintf(output_file, "%d %d %d ", secret_image->data[i], secret_image->data[i + 1], secret_image->data[i + 2]);
        if ((i / 3 + 1) % width == 0) fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete_image(input_image);
    delete_image(secret_image);
}