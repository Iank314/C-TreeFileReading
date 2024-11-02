#include <math.h>
#include <stdlib.h>
#include "image.h"
#include "qtree.h"
#include <stdio.h>
#include <string.h>

double calculate_rmse(Image *image, int x, int y, int width, int height, unsigned char avg_intensity) 
{
    double rmse = 0.0;
    int pixel_count = width * height;
    for (int i = y; i < y + height; i++) 
    {
        for (int j = x; j < x + width; j++) 
        {
            unsigned char intensity = get_image_intensity(image, i, j);
            rmse += pow(intensity - avg_intensity, 2);
        }
    }
    return sqrt(rmse / pixel_count);
}
QTNode *create_quadtree_recursive(Image *image, int x, int y, int width, int height, double max_rmse)
{
    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node) 
    {
        ERROR("Memory allocation failed for QTNode");
        return NULL;
    }

    double total_intensity = 0.0;
    int pixel_count = width * height;
    for (int i = y; i < y + height; i++) 
    {
        for (int j = x; j < x + width; j++) 
        {
            total_intensity += get_image_intensity(image, i, j);
        }
    }
    double average_intensity = total_intensity / pixel_count;
    node->intensity = (unsigned char)average_intensity;  

    double rmse = 0.0;
    for (int i = y; i < y + height; i++) 
    {
        for (int j = x; j < x + width; j++) 
        {
            double diff = get_image_intensity(image, i, j) - average_intensity;
            rmse += diff * diff;
        }
    }
    rmse = sqrt(rmse / pixel_count);

    if (rmse <= max_rmse || (width == 1 && height == 1)) 
    {
        node->is_leaf = 1;
        for (int i = 0; i < 4; i++) node->children[i] = NULL;
        return node;
    }

    node->is_leaf = 0;
    int half_width = width / 2;
    int half_height = height / 2;

    if (height == 1) 
    {
        node->children[0] = create_quadtree_recursive(image, x, y, half_width, height, max_rmse);
        node->children[1] = create_quadtree_recursive(image, x + half_width, y, width - half_width, height, max_rmse);
        node->children[2] = NULL;
        node->children[3] = NULL;
    }

    else if (width == 1) 
    {
        node->children[0] = create_quadtree_recursive(image, x, y, width, half_height, max_rmse);
        node->children[2] = create_quadtree_recursive(image, x, y + half_height, width, height - half_height, max_rmse);
        node->children[1] = NULL;
        node->children[3] = NULL;
    }

    else 
    {
        node->children[0] = create_quadtree_recursive(image, x, y, half_width, half_height, max_rmse);
        node->children[1] = create_quadtree_recursive(image, x + half_width, y, width - half_width, half_height, max_rmse);
        node->children[2] = create_quadtree_recursive(image, x, y + half_height, half_width, height - half_height, max_rmse);
        node->children[3] = create_quadtree_recursive(image, x + half_width, y + half_height, width - half_width, height - half_height, max_rmse);
    }

    return node;
}

QTNode *create_quadtree(Image *image, double max_rmse) 
{
    return create_quadtree_recursive(image, 0, 0, image->width, image->height, max_rmse);
}

void delete_quadtree(QTNode *root) 
{
    if (root == NULL) return;
    if (!root->is_leaf) {
        for (int i = 0; i < 4; i++) {
            delete_quadtree(root->children[i]);
        }
    }
    free(root);
}
QTNode *get_child1(QTNode *node) { return node ? node->children[0] : NULL; }
QTNode *get_child2(QTNode *node) { return node ? node->children[1] : NULL; }
QTNode *get_child3(QTNode *node) { return node ? node->children[2] : NULL; }
QTNode *get_child4(QTNode *node) { return node ? node->children[3] : NULL; }

unsigned char get_node_intensity(QTNode *node) { return node ? node->intensity : 0; }





void fill_region(unsigned char *buffer, unsigned char intensity, int row, int col, int width, int height, int image_width)
{
    for (int i = row; i < row + height; i++) 
    {
        for (int j = col; j < col + width; j++) 
        {
            int index = (i * image_width + j) * 3;
            buffer[index] = buffer[index + 1] = buffer[index + 2] = intensity;
        }
    }
}
static void save_ppm_helper(QTNode *node, unsigned char *buffer, int row, int col, int width, int height, int image_width)
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
    if (!root || !filename) {
        ERROR("Invalid root or filename in save_qtree_as_ppm.");
        return;
    }

    int image_width = root->width;
    int image_height = root->height;

    FILE *file = fopen(filename, "w");
    if (!file)
    {
        ERROR("Failed to open file for writing PPM.");
        return;
    }

    fprintf(file, "P3\n%d %d\n255\n", image_width, image_height);
    unsigned char *buffer = (unsigned char *)calloc(image_width * image_height * 3, sizeof(unsigned char));
    if (!buffer)
    {
        ERROR("Memory allocation failed for image buffer.");
        fclose(file);
        return;
    }

    save_ppm_helper(root, buffer, 0, 0, image_width, image_height, image_width);

    for (int i = 0; i < image_height; i++) 
    {
        for (int j = 0; j < image_width; j++) 
        {
            int index = (i * image_width + j) * 3;
            fprintf(file, "%d %d %d ", buffer[index], buffer[index + 1], buffer[index + 2]);
        }
        fprintf(file, "\n");
    }

    free(buffer);
    fclose(file);
}






QTNode *load_preorder_recursive(FILE *file)
{
    char type;
    unsigned char intensity;
    int x, y, width, height;

    if (fscanf(file, " %c %hhu %d %d %d %d", &type, &intensity, &y, &height, &x, &width) != 6)
        return NULL;

    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node) {
        ERROR("Memory allocation failed for QTNode");
        return NULL;
    }

    node->intensity = intensity;
    node->is_leaf = (type == 'L');
    node->width = width;
    node->height = height;

    for (int i = 0; i < 4; i++) node->children[i] = NULL;

    if (!node->is_leaf) 
    {
        node->children[0] = load_preorder_recursive(file);
        node->children[1] = load_preorder_recursive(file);
        node->children[2] = load_preorder_recursive(file);
        node->children[3] = load_preorder_recursive(file);
    }

    return node;
}

QTNode *load_preorder_qt(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        ERROR("Failed to open file for reading preorder");
        return NULL;
    }

    QTNode *root = load_preorder_recursive(file);
    fclose(file);
    return root;
}






void save_preorder_recursive(FILE *file, QTNode *node, int x, int y)
{
    if (!node) return;

    if (node->is_leaf) 
    {
        fprintf(file, "L %d %d %d %d %d\n", node->intensity, y, node->height, x, node->width);
    } 
    else 
    {
        fprintf(file, "N %d %d %d %d %d\n", node->intensity, y, node->height, x, node->width);
        int half_width = node->width / 2;
        int half_height = node->height / 2;
        save_preorder_recursive(file, node->children[0], x, y);
        save_preorder_recursive(file, node->children[1], x + half_width, y);
        save_preorder_recursive(file, node->children[2], x, y + half_height);
        save_preorder_recursive(file, node->children[3], x + half_width, y + half_height);
    }
}

void save_preorder_qt(QTNode *root, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file) 
    {
        ERROR("Failed to open file for writing preorder");
        return;
    }

    save_preorder_recursive(file, root, 0, 0);
    fclose(file);
}