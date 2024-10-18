#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>

#define BUFFER_SIZE 8192

#include "qtree.h"

void copy_file(char *src_filename, char *dest_filename);
void prepare_input_image_file(char *image_filename);
