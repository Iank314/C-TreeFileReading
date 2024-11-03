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

    if (rmse <= max_rmse || (width <= 1 && height <= 1)) 
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

static QTNode *load_preorder_qt_helper(FILE *file)
{
    char node_type;
    int intensity, row, height, col, width;

    if (fscanf(file, " %c %d %d %d %d %d", &node_type, &intensity, &row, &height, &col, &width) != 6)
    {
        return NULL;
    }

    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node)
    {
        ERROR("Memory allocation failed for QTNode.");
        return NULL;
    }

    node->intensity = (unsigned char)intensity;
    node->width = width;
    node->height = height;

    if (node_type == 'L')
    {
        node->is_leaf = 1;
        for (int i = 0; i < 4; i++) node->children[i] = NULL;
    }
    else if (node_type == 'N')
    {
        node->is_leaf = 0;

        for (int i = 0; i < 4; i++) node->children[i] = NULL;

        if (width > 1 && height > 1)
        {
            for (int i = 0; i < 4; i++) 
            {
                node->children[i] = load_preorder_qt_helper(file);
                if (!node->children[i]) 
                {
                    for (int j = 0; j < i; j++) free(node->children[j]);
                    free(node);
                    return NULL;
                }
            }
        }
        else if (width > 1)
        {
            for (int i = 0; i < 2; i++) 
            {
                node->children[i] = load_preorder_qt_helper(file);
                if (!node->children[i]) 
                {
                    for (int j = 0; j < i; j++) free(node->children[j]);
                    free(node);
                    return NULL;
                }
            }
        }
        else if (height > 1)
        {
            node->children[0] = load_preorder_qt_helper(file);
            if (!node->children[0]) 
            {
                free(node);
                return NULL;
            }
            node->children[2] = load_preorder_qt_helper(file);
            if (!node->children[2]) 
            {
                free(node->children[0]);
                free(node);
                return NULL;
            }
        }
    }
    return node;
}

QTNode *load_preorder_qt(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        ERROR("Failed to open file for reading.");
        return NULL;
    }
    QTNode *root = load_preorder_qt_helper(file);
    fclose(file);
    if (!root) {
        ERROR("Failed to load quadtree from file.");
    }
    return root;
}




void save_preorder_qt(QTNode *root, char *filename)
{
    (void)*root;
    (void)*filename;
}

void fill_region(unsigned char **image_data, int row, int col, int width, int height, unsigned char intensity) {
    for (int i = row; i < row + height; i++) {
        for (int j = col; j < col + width; j++) {
            image_data[i][j] = intensity;
        }
    }
}

void save_ppm_helper(QTNode *node, unsigned char **image_data, int row, int col) {
    if (!node) return;

    if (node->is_leaf) {
        fill_region(image_data, row, col, node->width, node->height, node->intensity);
    } else {
        int half_width = node->width / 2;
        int half_height = node->height / 2;

        save_ppm_helper(node->children[0], image_data, row, col); // Top-left
        save_ppm_helper(node->children[1], image_data, row, col + half_width); // Top-right
        save_ppm_helper(node->children[2], image_data, row + half_height, col); // Bottom-left
        save_ppm_helper(node->children[3], image_data, row + half_height, col + half_width); // Bottom-right
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename) 
{
    if (!root) 
    {
        ERROR("Root node is NULL.");
        return;
    }

    FILE *file = fopen(filename, "w");
    if (!file) 
    {

        ERROR("Failed to open file for writing.");
        return;
    }

    int width = root->width;
    int height = root->height;

    fprintf(file, "P3\n%d %d\n255\n", width, height);

    unsigned char **image_data = (unsigned char **)malloc(height * sizeof(unsigned char *));
    for (int i = 0; i < height; i++) 
    {
        image_data[i] = (unsigned char *)malloc(width * sizeof(unsigned char));
    }

    save_ppm_helper(root, image_data, 0, 0);

    for (int i = 0; i < height; i++) 
    {
        for (int j = 0; j < width; j++) 
        {
            fprintf(file, "%d %d %d ", image_data[i][j], image_data[i][j], image_data[i][j]);
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < height; i++) 
    {
        free(image_data[i]);
    }
    free(image_data);

    fclose(file);
}


