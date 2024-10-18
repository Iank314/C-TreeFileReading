#include "tests_utils.h"

void copy_file(char *src_filename, char *dest_filename) {
    FILE *src_fp = fopen(src_filename, "r");
    if (!src_fp) {
        perror("copy_file(): Failed to open source file");
        return;
    }

    FILE *dest_fp = fopen(dest_filename, "w");
    if (!dest_fp) {
        perror("copy_file(): Failed to open destination file");
        fclose(src_fp);
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        if (fwrite(buffer, 1, n, dest_fp) != n) {
            perror("copy_file(): Failed to write to destination file");
            fclose(src_fp);
            fclose(dest_fp);
            return;
        }
    }

    if (ferror(src_fp)) {
        perror("copy_file(): Failed to read from source file");
    }

    fclose(src_fp);
    fclose(dest_fp);
}

void prepare_input_image_file(char *image_filename) {
    char source_file[256], dest_file[256];
    sprintf(source_file, "images/originals/%s", image_filename);
    sprintf(dest_file, "images/%s", image_filename);
    chmod(source_file, 0444); // protect original file
    chmod(dest_file, 0666); // make destination writeable
    copy_file(source_file, dest_file);
}
