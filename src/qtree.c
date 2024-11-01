#include "qtree.h"
#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static int global_image_width = 0;
static int global_image_height = 0;
void fill_region(unsigned char *buffer, unsigned char intensity, int start_row, int start_col, int width, int height, int image_width)
{
    for (int i = start_row; i < start_row + height; i++)
    {
        for (int j = start_col; j < start_col + width; j++)
        {
            int index = (i * image_width + j) * 3; 
            buffer[index] = intensity;
            buffer[index + 1] = intensity;
            buffer[index + 2] = intensity;
        }
    }
}
QTNode *create_quadtree_helper(Image *image, int row, int col, int width, int height, double max_rmse)
{
    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node)
    {
        ERROR("Memory allocation failed for QTNode.");
        return NULL;
    }

    double total_intensity = 0;
    for (int i = row; i < row + height; i++)
    {
        for (int j = col; j < col + width; j++)
        {
            total_intensity += get_image_intensity(image, i, j);
        }
    }
    int num_pixels = width * height;
    double avg_intensity = total_intensity / num_pixels;
    node->intensity = (unsigned char)(avg_intensity + 0.5);

    double rmse = 0;
    for (int i = row; i < row + height; i++)
    {
        for (int j = col; j < col + width; j++)
        {
            double diff = get_image_intensity(image, i, j) - avg_intensity;
            rmse += diff * diff;
        }
    }
    rmse = sqrt(rmse / num_pixels);

    if (rmse > max_rmse && width > 1 && height > 1)
    {
        int half_width = (width + 1) / 2;
        int half_height = (height + 1) / 2;

        node->children[0] = create_quadtree_helper(image, row, col, half_width, half_height, max_rmse);
        node->children[1] = create_quadtree_helper(image, row, col + half_width, width - half_width, half_height, max_rmse);
        node->children[2] = create_quadtree_helper(image, row + half_height, col, half_width, height - half_height, max_rmse);
        node->children[3] = create_quadtree_helper(image, row + half_height, col + half_width, width - half_width, height - half_height, max_rmse);

        node->is_leaf = 0;
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            node->children[i] = NULL;
        }
        node->is_leaf = 1;
    }

    return node;
}

QTNode *create_quadtree(Image *image, double max_rmse)
{
    global_image_width = get_image_width(image);
    global_image_height = get_image_height(image);

    return create_quadtree_helper(image, 0, 0, global_image_width, global_image_height, max_rmse);
}
void delete_quadtree(QTNode *root)
{
    if (root == NULL)
        return;

    for (int i = 0; i < 4; i++)
    {
        delete_quadtree(root->children[i]);
    }
    free(root);
}
void save_preorder_qt_helper(QTNode *node, FILE *file, int row, int col, int width, int height)
{
    if (!node)
        return;

    if (node->is_leaf)
    {
        fprintf(file, "L %d %d %d %d %d\n", node->intensity, row, height, col, width);
    }
    else
    {
        fprintf(file, "N %d %d %d %d %d\n", node->intensity, row, height, col, width);
        int half_width = (width + 1) / 2;
        int half_height = (height + 1) / 2;
        save_preorder_qt_helper(node->children[0], file, row, col, half_width, half_height);
        save_preorder_qt_helper(node->children[1], file, row, col + half_width, width - half_width, half_height);
        save_preorder_qt_helper(node->children[2], file, row + half_height, col, half_width, height - half_height);
        save_preorder_qt_helper(node->children[3], file, row + half_height, col + half_width, width - half_width, height - half_height);
    }
}

void save_preorder_qt(QTNode *root, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        ERROR("Failed to open file for writing.");
        return;
    }

    save_preorder_qt_helper(root, file, 0, 0, global_image_width, global_image_height);
    fclose(file);
}

void save_ppm_helper(QTNode *node, unsigned char *buffer, int row, int col, int width, int height, int image_width)
{
    if (!node)
        return;

    if (node->is_leaf)
    {
        fill_region(buffer, node->intensity, row, col, width, height, image_width);
    }
    else
    {
        int half_width = (width + 1) / 2;
        int half_height = (height + 1) / 2;

        save_ppm_helper(node->children[0], buffer, row, col, half_width, half_height, image_width);
        save_ppm_helper(node->children[1], buffer, row, col + half_width, width - half_width, half_height, image_width);
        save_ppm_helper(node->children[2], buffer, row + half_height, col, half_width, height - half_height, image_width);
        save_ppm_helper(node->children[3], buffer, row + half_height, col + half_width, width - half_width, height - half_height, image_width);
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        ERROR("Failed to open file for writing PPM.");
        return;
    }

    fprintf(file, "P3\n%d %d\n255\n", global_image_width, global_image_height);
    unsigned char *buffer = (unsigned char *)malloc(global_image_width * global_image_height * 3);
    if (!buffer)
    {
        ERROR("Memory allocation failed for image buffer.");
        fclose(file);
        return;
    }

    save_ppm_helper(root, buffer, 0, 0, global_image_width, global_image_height, global_image_width);

    for (int i = 0; i < global_image_width * global_image_height * 3; i += 3)
    {
        fprintf(file, "%d %d %d ", buffer[i], buffer[i + 1], buffer[i + 2]);
        if ((i / 3 + 1) % global_image_width == 0)
            fprintf(file, "\n");
    }

    free(buffer);
    fclose(file);
}

QTNode *load_preorder_qt_helper(FILE *file)
{
    char type;
    int intensity, row, height, col, width;
    if (fscanf(file, " %c %d %d %d %d %d", &type, &intensity, &row, &height, &col, &width) != 6)
        return NULL;

    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node)
    {
        ERROR("Memory allocation failed for QTNode.");
        return NULL;
    }
    node->intensity = (unsigned char)intensity;

    if (type == 'L')
    {
        node->is_leaf = 1;
        for (int i = 0; i < 4; i++)
            node->children[i] = NULL;
    }
    else if (type == 'N')
    {
        node->is_leaf = 0;
        node->children[0] = load_preorder_qt_helper(file);
        node->children[1] = load_preorder_qt_helper(file);
        node->children[2] = load_preorder_qt_helper(file);
        node->children[3] = load_preorder_qt_helper(file);
    }

    return node;
}

QTNode *load_preorder_qt(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        ERROR("Failed to open file for reading.");
        return NULL;
    }
    QTNode *root = load_preorder_qt_helper(file);
    fclose(file);
    return root;
}




