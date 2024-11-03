#include <math.h>
#include <stdlib.h>
#include "image.h"
#include "qtree.h"
#include <stdio.h>


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

    double rmse = calculate_rmse(image, x, y, width, height, node->intensity);

    if (rmse <= max_rmse || (width <= 1 && height <= 1))
    {
        node->is_leaf = 1;
        node->child1 = node->child2 = node->child3 = node->child4 = NULL;
        return node;
    }

    node->is_leaf = 0;
    int half_width = width / 2;
    int half_height = height / 2;

    if (height == 1)
    {
        node->child1 = create_quadtree_recursive(image, x, y, half_width, height, max_rmse);
        node->child2 = create_quadtree_recursive(image, x + half_width, y, width - half_width, height, max_rmse);
        node->child3 = NULL;
        node->child4 = NULL;
    }
    else if (width == 1)
    {
        node->child1 = create_quadtree_recursive(image, x, y, width, half_height, max_rmse);
        node->child3 = create_quadtree_recursive(image, x, y + half_height, width, height - half_height, max_rmse);
        node->child2 = NULL;
        node->child4 = NULL;
    }
    else
    {
        node->child1 = create_quadtree_recursive(image, x, y, half_width, half_height, max_rmse);
        node->child2 = create_quadtree_recursive(image, x + half_width, y, width - half_width, half_height, max_rmse);
        node->child3 = create_quadtree_recursive(image, x, y + half_height, half_width, height - half_height, max_rmse);
        node->child4 = create_quadtree_recursive(image, x + half_width, y + half_height, width - half_width, height - half_height, max_rmse);
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
        delete_quadtree(root->child1);
        delete_quadtree(root->child2);
        delete_quadtree(root->child3);
        delete_quadtree(root->child4);
    }
    free(root);
}

QTNode *get_child1(QTNode *node) { return node ? node->child1 : NULL; }
QTNode *get_child2(QTNode *node) { return node ? node->child2 : NULL; }
QTNode *get_child3(QTNode *node) { return node ? node->child3 : NULL; }
QTNode *get_child4(QTNode *node) { return node ? node->child4 : NULL; }

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
        node->child1 = node->child2 = node->child3 = node->child4 = NULL;
    }
    else if (node_type == 'N')
    {
        node->is_leaf = 0;
        node->child1 = node->child2 = node->child3 = node->child4 = NULL;
        if (width > 1 && height > 1)
        {
            node->child1 = load_preorder_qt_helper(file);
            node->child2 = load_preorder_qt_helper(file);
            node->child3 = load_preorder_qt_helper(file);
            node->child4 = load_preorder_qt_helper(file);
        }
        else if (width > 1)
        {
            node->child1 = load_preorder_qt_helper(file);
            node->child2 = load_preorder_qt_helper(file);
        }
        else if (height > 1)
        {
            node->child1 = load_preorder_qt_helper(file);
            node->child3 = load_preorder_qt_helper(file);
        }
    }
    return node;
}

QTNode *load_preorder_qt(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) 
    {
        return NULL;
    }
    QTNode *root = load_preorder_qt_helper(file);
    fclose(file);
    if (!root) 
    {
        ERROR("Failed to load quadtree from file.");
    }
    return root;
}

static void save_preorder_qt_helper(QTNode *node, FILE *file, int row, int col, int width, int height)
{
    if (!node) return;

    if (node->is_leaf)
    {
        fprintf(file, "L %d %d %d %d %d\n", node->intensity, row, height, col, width);
    }
    else
    {
        fprintf(file, "N %d %d %d %d %d\n", node->intensity, row, height, col, width);

        int half_width = width / 2;
        int half_height = height / 2;

        if (width > 1 && height > 1)
        {
            save_preorder_qt_helper(node->child1, file, row, col, half_width, half_height);
            save_preorder_qt_helper(node->child2, file, row, col + half_width, width - half_width, half_height);
            save_preorder_qt_helper(node->child3, file, row + half_height, col, half_width, height - half_height);
            save_preorder_qt_helper(node->child4, file, row + half_height, col + half_width, width - half_width, height - half_height);
        }
        else if (width > 1)
        {
            save_preorder_qt_helper(node->child1, file, row, col, half_width, height);
            save_preorder_qt_helper(node->child2, file, row, col + half_width, width - half_width, height);
        }
        else if (height > 1)
        {
            save_preorder_qt_helper(node->child1, file, row, col, width, half_height);
            save_preorder_qt_helper(node->child3, file, row + half_height, col, width, height - half_height);
        }
    }
}

void save_preorder_qt(QTNode *root, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        return;
    }
    save_preorder_qt_helper(root, file, 0, 0, root->width, root->height);
    fclose(file);
}


void write_leaf_pixels(QTNode *node, FILE *file) 
{
    for (int i = 0; i < node->width * node->height; i++) 
    {
        fprintf(file, "%hhu %hhu %hhu ", node->intensity, node->intensity, node->intensity);
    }
}

void save_qtree_as_ppm_helper(QTNode *node, FILE *file) 
{
    if (node == NULL) 
    {
        return;
    }

    if (node->is_leaf) 
    {
        write_leaf_pixels(node, file);
    } 
    else 
    {
        save_qtree_as_ppm_helper(node->child1, file);
        save_qtree_as_ppm_helper(node->child2, file);
        save_qtree_as_ppm_helper(node->child3, file);
        save_qtree_as_ppm_helper(node->child4, file);
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename) 
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) 
    {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(file, "P3\n%d %d\n255\n", root->width, root->height);

    save_qtree_as_ppm_helper(root, file);

    fclose(file);
}