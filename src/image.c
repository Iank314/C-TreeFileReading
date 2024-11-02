

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
    FILE *input = fopen(input_filename, "r");
    FILE *output = fopen(output_filename, "w");
    char img_format[3];
    unsigned int max_color_value;
    unsigned short img_width;
    unsigned short img_height;

    fscanf(input, "%2s", img_format);
    char check;
    while (fscanf(input, " %c", &check) == 1 && check == '#') 
    {
        while (fgetc(input) != '\n');
    }
    ungetc(check, input);
    fscanf(input, "%hu %hu %u", &img_width, &img_height, &max_color_value);
    fprintf(output, "%c%c\n%hu %hu\n%u\n", img_format[0], img_format[1], img_width, img_height, max_color_value);

    long unsigned int msg_len = strlen(message);
    long unsigned int available_space = img_width * img_height;
    long unsigned int msg_idx = 0;
    int encoded_length = 0;

    while (available_space >= 8 && msg_idx <= msg_len) 
    {
        encoded_length++;
        char current_char = (available_space > 8) ? message[msg_idx] : '\0';
        if (current_char == '\0') 
        {
            encoded_length--;
        }

        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) 
        {
            unsigned int pixel_color;
            fscanf(input, "%u %*u %*u", &pixel_color);
            int bit = ((current_char >> bit_pos) & 1);
            pixel_color = (pixel_color & ~1) | bit;
            fprintf(output, "%u %u %u ", pixel_color, pixel_color, pixel_color);
        }
        available_space -= 8;
        msg_idx++;
    }

    unsigned int remaining_pixel;
    while (fscanf(input, "%u %*u %*u", &remaining_pixel) == 1) 
    {
        fprintf(output, "%u %u %u ", remaining_pixel, remaining_pixel, remaining_pixel);
    }

    fclose(input);
    fclose(output);
    return encoded_length;
}

char *reveal_message(char *input_filename) 
{
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) 
    {
        return NULL; 
    }

    char img_format[3];
    unsigned int max_color_value;
    unsigned short img_width, img_height;

    fscanf(input_file, "%2s", img_format);
    char check;
    while (fscanf(input_file, " %c", &check) == 1 && check == '#') 
    {
        while (fgetc(input_file) != '\n'); 
    }
    ungetc(check, input_file);
    fscanf(input_file, "%hu %hu %u", &img_width, &img_height, &max_color_value);

    char *message = (char *)malloc(10000000);
    if (!message) 
    {
        fclose(input_file);
        return NULL; 
    }

    unsigned int msg_index = 0;
    unsigned char character = 0;
    int pixel_count = img_width * img_height;
    int total_count = pixel_count / 8;
    int count = 0;

    while (count < total_count) 
    {
        character = 0;
        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) 
        {
            unsigned int pixel_color;
            fscanf(input_file, "%u %*u %*u", &pixel_color);
            character |= (pixel_color & 1) << bit_pos;
        }

        if (character == '\0') 
        {
            break;
        }
        
        message[msg_index++] = character;
        count++;
    }

    message[msg_index] = '\0'; 

    fclose(input_file);
    return message;
}




int read_header(FILE *file, char *format, unsigned short *width, unsigned short *height, unsigned int *max_color)
{
    if (fscanf(file, "%2s", format) != 1) return 0;
    char check;
    while (fscanf(file, " %c", &check) == 1 && check == '#') 
    {
        while (fgetc(file) != '\n');
    }
    ungetc(check, file);
    return fscanf(file, "%hu %hu %u", width, height, max_color) == 3;
}

void encode_dimension_bits(FILE *input_file, FILE *output_file, unsigned short dimension)
{
    for (int bit_pos = 7; bit_pos >= 0; bit_pos--) 
    {
        unsigned int bit = (dimension >> bit_pos) & 1;
        unsigned int pixel_color;
        fscanf(input_file, "%u %*u %*u", &pixel_color);
        pixel_color = (pixel_color & ~1) | bit;
        fprintf(output_file, "%u %u %u ", pixel_color, pixel_color, pixel_color);
    }
}

void embed_image(FILE *secret_file, FILE *input_file, FILE *output_file)
{
    unsigned int secret_pixel;
    while (fscanf(secret_file, "%u %*u %*u", &secret_pixel) == 1) 
    {
        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) 
        {
            unsigned int bit = (secret_pixel >> bit_pos) & 1;
            unsigned int input_pixel;
            fscanf(input_file, "%u %*u %*u", &input_pixel);
            input_pixel = (input_pixel & ~1) | bit;
            fprintf(output_file, "%u %u %u ", input_pixel, input_pixel, input_pixel);
        }
    }
}

void copy_remaining_pixels(FILE *input_file, FILE *output_file)
{
    unsigned int remaining_pixel;
    while (fscanf(input_file, "%u %*u %*u", &remaining_pixel) == 1) 
    {
        fprintf(output_file, "%u %u %u ", remaining_pixel, remaining_pixel, remaining_pixel);
    }
}




unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename) 
{
    FILE *secret_file = fopen(secret_image_filename, "r");
    FILE *input_file = fopen(input_filename, "r");
    FILE *output_file = fopen(output_filename, "w");

    if (!secret_file || !input_file || !output_file) 
    {
        if (secret_file) fclose(secret_file);
        if (input_file) fclose(input_file);
        if (output_file) fclose(output_file);
        return 0; 
    }

    char secret_format[3], input_format[3];
    unsigned short secret_width, secret_height, input_width, input_height;
    unsigned int secret_max_color, input_max_color;

    if (!read_header(secret_file, secret_format, &secret_width, &secret_height, &secret_max_color) ||
        !read_header(input_file, input_format, &input_width, &input_height, &input_max_color))
    {
        fclose(secret_file);
        fclose(input_file);
        fclose(output_file);
        return 0;
    }

    unsigned long required_space = (secret_width * secret_height * 8) + 16;
    unsigned long available_space = input_width * input_height;
    if (required_space > available_space) 
    {
        fclose(secret_file);
        fclose(input_file);
        fclose(output_file);
        return 0; 
    }

    fprintf(output_file, "%s\n%hu %hu\n%u\n", "P3", input_width, input_height, 255);

    encode_dimension_bits(input_file, output_file, secret_height);
    encode_dimension_bits(input_file, output_file, secret_width);

    embed_image(secret_file, input_file, output_file);

    copy_remaining_pixels(input_file, output_file);

    fclose(secret_file);
    fclose(input_file);
    fclose(output_file);

    return 1;
}

int read_image_header(FILE *file, char *format, unsigned short *width, unsigned short *height, unsigned int *max_color)
{
    if (fscanf(file, "%2s", format) != 1 || format[0] != 'P' || format[1] != '3') return 0;
    char check;
    while (fscanf(file, " %c", &check) == 1 && check == '#') 
    {
        while (fgetc(file) != '\n');
    }
    ungetc(check, file);
    return fscanf(file, "%hu %hu %u", width, height, max_color) == 3;
}

unsigned short extract_dimension(FILE *input)
{
    unsigned short dimension = 0;
    for (int bit_pos = 7; bit_pos >= 0; bit_pos--) 
    {
        unsigned int pixel_val;
        fscanf(input, "%u %*u %*u", &pixel_val);
        dimension |= ((pixel_val & 1) << bit_pos);
    }
    return dimension;
}

void extract_hidden_image(FILE *input, FILE *output, int total_pixels)
{
    for (int i = 0; i < total_pixels; i++) 
    {
        unsigned int hidden_pixel = 0;
        for (int bit_pos = 7; bit_pos >= 0; bit_pos--) 
        {
            unsigned int pixel_val;
            fscanf(input, "%u %*u %*u", &pixel_val);
            hidden_pixel |= ((pixel_val & 1) << bit_pos);
        }
        fprintf(output, "%u %u %u ", hidden_pixel, hidden_pixel, hidden_pixel);
    }
}

void reveal_image(char *input_filename, char *output_filename) 
{
    FILE *input = fopen(input_filename, "r");
    FILE *output = fopen(output_filename, "w");

    if (!input || !output) 
    {
        fclose(input);
        fclose(output);
        return;
    }

    char img_format[3];
    unsigned short img_width, img_height;
    unsigned int img_max_color;

    if (!read_image_header(input, img_format, &img_width, &img_height, &img_max_color)) 
    {
        fclose(input);
        fclose(output);
        return;
    }

    unsigned short hidden_width = extract_dimension(input);
    unsigned short hidden_height = extract_dimension(input);

    fprintf(output, "%s\n%hu %hu\n%u\n", "P3", hidden_width, hidden_height, 255);
    int total_pixels = hidden_width * hidden_height;

    extract_hidden_image(input, output, total_pixels);

    fclose(input);
    fclose(output);
}