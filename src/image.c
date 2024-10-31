#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned char **pixels;
} Image;

void skip_comments(FILE *file) {
    int ch;
    while ((ch = fgetc(file)) == '#') {
        while (fgetc(file) != '\n');
    }
    ungetc(ch, file);
}

Image *load_image(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    Image *image = malloc(sizeof(Image));
    if (!image) {
        fclose(file);
        return NULL;
    }

    char format[3];
    fscanf(file, "%2s", format);
    skip_comments(file);
    fscanf(file, "%hu %hu", &image->width, &image->height);
    int max_value;
    fscanf(file, "%d", &max_value);

    image->pixels = malloc(image->height * sizeof(unsigned char *));
    for (int i = 0; i < image->height; i++) {
        image->pixels[i] = malloc(image->width * sizeof(unsigned char));
        for (int j = 0; j < image->width; j++) {
            int r, g, b;
            fscanf(file, "%d %d %d", &r, &g, &b);
            image->pixels[i][j] = (unsigned char)((r + g + b) / 3);
        }
    }
    fclose(file);
    return image;
}

void delete_image(Image *image) {
    if (image) {
        for (int i = 0; i < image->height; i++) {
            free(image->pixels[i]);
        }
        free(image->pixels);
        free(image);
    }
}

unsigned char get_image_intensity(Image *image, unsigned int row, unsigned int col) {
    if (row >= image->height || col >= image->width) return 0;
    return image->pixels[row][col];
}

unsigned short get_image_width(Image *image) {
    return image->width;
}

unsigned short get_image_height(Image *image) {
    return image->height;
}

unsigned int hide_message(char *message, char *input_filename, char *output_filename) {
    FILE *input = fopen(input_filename, "r");
    if (!input) return 0;

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        fclose(input);
        return 0;
    }

    char format[3];
    unsigned short width, height, max_value;
    fscanf(input, "%2s", format);
    fprintf(output, "%s\n", format);
    fscanf(input, "%hu %hu %hu", &width, &height, &max_value);
    fprintf(output, "%hu %hu\n%hu\n", width, height, max_value);

    unsigned int max_chars = (width * height) / 8;
    unsigned int chars_to_encode = (unsigned int)strlen(message) + 1;
    if (chars_to_encode > max_chars) chars_to_encode = max_chars;

    unsigned char ch;
    unsigned int char_index = 0, bit_index = 0;
    for (unsigned int i = 0; i < width * height; i++) {
        int pixel;
        fscanf(input, "%d", &pixel);

        if (char_index < chars_to_encode) {
            ch = message[char_index];
            pixel = (pixel & 0xFE) | ((ch >> (7 - bit_index)) & 1);
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                char_index++;
            }
        }

        fprintf(output, "%d ", pixel);
    }

    fclose(input);
    fclose(output);
    return chars_to_encode - 1;
}

char *reveal_message(char *input_filename) {
    FILE *input = fopen(input_filename, "r");
    if (!input) return NULL;

    char format[3];
    unsigned short width, height, max_value;
    fscanf(input, "%2s", format);
    fscanf(input, "%hu %hu %hu", &width, &height, &max_value);

    unsigned int max_chars = (width * height) / 8;
    char *message = malloc(max_chars);
    if (!message) {
        fclose(input);
        return NULL;
    }

    unsigned char ch = 0;
    int char_index = 0, bit_index = 0;
    for (unsigned int i = 0; i < width * height; i++) {
        int pixel;
        fscanf(input, "%d", &pixel);
        ch = (ch << 1) | (pixel & 1);
        bit_index++;
        if (bit_index == 8) {
            if (ch == '\0') break;
            message[char_index++] = ch;
            ch = 0;
            bit_index = 0;
        }
    }
    message[char_index] = '\0';

    fclose(input);
    return message;
}

unsigned int hide_image(char *secret_image_filename, char *input_filename, char *output_filename) {
    FILE *input = fopen(input_filename, "r");
    FILE *secret = fopen(secret_image_filename, "r");
    if (!input || !secret) return 0;

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        fclose(input);
        fclose(secret);
        return 0;
    }

    char format[3];
    unsigned short width, height, max_value;
    fscanf(input, "%2s", format);
    fprintf(output, "%s\n", format);
    fscanf(input, "%hu %hu %hu", &width, &height, &max_value);
    fprintf(output, "%hu %hu\n%hu\n", width, height, max_value);

    unsigned short secret_width, secret_height;
    fscanf(secret, "%2s", format);
    fscanf(secret, "%hu %hu %hu", &secret_width, &secret_height, &max_value);

    if ((width * height) < (16 + 8 * secret_width * secret_height)) {
        fclose(input);
        fclose(secret);
        fclose(output);
        return 0;
    }

    for (int i = 0; i < 8; i++) {
        int pixel;
        fscanf(input, "%d", &pixel);
        pixel = (pixel & 0xFE) | ((secret_width >> (7 - i)) & 1);
        fprintf(output, "%d ", pixel);
    }
    for (int i = 0; i < 8; i++) {
        int pixel;
        fscanf(input, "%d", &pixel);
        pixel = (pixel & 0xFE) | ((secret_height >> (7 - i)) & 1);
        fprintf(output, "%d ", pixel);
    }

    for (unsigned int i = 0; i < secret_width * secret_height; i++) {
        int secret_pixel;
        fscanf(secret, "%d", &secret_pixel);
        for (int j = 0; j < 8; j++) {
            int pixel;
            fscanf(input, "%d", &pixel);
            pixel = (pixel & 0xFE) | ((secret_pixel >> (7 - j)) & 1);
            fprintf(output, "%d ", pixel);
        }
    }

    int pixel;
    while (fscanf(input, "%d", &pixel) == 1) {
        fprintf(output, "%d ", pixel);
    }

    fclose(input);
    fclose(secret);
    fclose(output);
    return 1;
}

void reveal_image(char *input_filename, char *output_filename) {
    FILE *input = fopen(input_filename, "r");
    FILE *output = fopen(output_filename, "w");
    if (!input || !output) return;

    char format[3];
    unsigned short width, height, max_value;
    fscanf(input, "%2s", format);
    fscanf(input, "%hu %hu %hu", &width, &height, &max_value);

    unsigned short secret_width = 0, secret_height = 0;
    for (int i = 0; i < 8; i++) {
        int pixel;
        fscanf(input, "%d", &pixel);
        secret_width = (secret_width << 1) | (pixel & 1);
    }
    for (int i = 0; i < 8; i++) {
        int pixel;
        fscanf(input, "%d", &pixel);
        secret_height = (secret_height << 1) | (pixel & 1);
    }

    fprintf(output, "P3\n%hu %hu\n255\n", secret_width, secret_height);

    for (unsigned int i = 0; i < secret_width * secret_height; i++) {
        unsigned char secret_pixel = 0;
        for (int j = 0; j < 8; j++) {
            int pixel;
            fscanf(input, "%d", &pixel);
            secret_pixel = (secret_pixel << 1) | (pixel & 1);
        }
        fprintf(output, "%d %d %d ", secret_pixel, secret_pixel, secret_pixel);
    }

    fclose(input);
    fclose(output);
}