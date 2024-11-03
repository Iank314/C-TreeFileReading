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

    double rmse = calculate_rmse(image, x, y, width, height, average_intensity);

    if (rmse <= max_rmse || (width <= 1 && height <= 1)) 
    {
        node->is_leaf = 1;
        for (int i = 0; i < 4; i++) node->children[i] = NULL;
        node->width = width;
        node->height = height;
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
    } else if (width == 1) 
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
    node->width = width;
    node->height = height;
    return node;
}

QTNode *create_quadtree(Image *image, double max_rmse) 
{
    return create_quadtree_recursive(image, 0, 0, image->width, image->height, max_rmse);
}

void delete_quadtree(QTNode *root) 
{
    if (root == NULL) return;
    if (!root->is_leaf) 
    {
        for (int i = 0; i < 4; i++) 
        {
            delete_quadtree(root->children[i]);
        }
    }
    free(root);
}

QTNode *get_child1(QTNode *node) 
{ 
    return node ? node->children[0] : NULL; 
}
QTNode *get_child2(QTNode *node) 
{ 
    return node ? node->children[1] : NULL; 
}
QTNode *get_child3(QTNode *node) 
{ 
    return node ? node->children[2] : NULL; 
}
QTNode *get_child4(QTNode *node) 
{ 
    return node ? node->children[3] : NULL; 
}

unsigned char get_node_intensity(QTNode *node) 
{ 
    return node ? node->intensity : 0; 
}

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
    else 
    {
        node->is_leaf = 0;
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
            node->children[0] = load_preorder_qt_helper(file);
            node->children[1] = load_preorder_qt_helper(file);
            node->children[2] = NULL;
            node->children[3] = NULL;
        } 
        else 
        {
            node->children[0] = load_preorder_qt_helper(file);
            node->children[2] = load_preorder_qt_helper(file);
            node->children[1] = NULL;
            node->children[3] = NULL;
        }
    }
    return node;
}

QTNode *load_preorder_qt(char *filename) 
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    QTNode *root = load_preorder_qt_helper(file);
    fclose(file);
    if (!root) {
        ERROR("Failed to load quadtree from file.");
    }
    return root;
}

void save_preorder_qt_helper(QTNode *node, FILE *file, int depth) 
{
    if (node == NULL) 
    {
        return;
    }

    for (int i = 0; i < depth; i++) 
    {
        fprintf(file, "   ");
    }

    if (node->is_leaf) 
    {
        fprintf(file, "%c %hhu %d %d\n", 'L', node->intensity, node->height, node->width);
    }
     else 
     {
        fprintf(file, "%c %hhu %d %d\n", 'N', node->intensity, node->height, node->width);
        save_preorder_qt_helper(node->children[0], file, depth + 1);
        save_preorder_qt_helper(node->children[1], file, depth + 1);
        save_preorder_qt_helper(node->children[2], file, depth + 1);
        save_preorder_qt_helper(node->children[3], file, depth + 1);
    }
}

void save_preorder_qt(QTNode *root, char *filename) 
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) 
    {
        ERROR("Failed to open file %s for writing", filename);
        return;
    }

    save_preorder_qt_helper(root, file, 0);
    fclose(file);
}

void save_qtree_as_ppm_helper(QTNode *node, FILE *file) 
{
    if (node == NULL) 
    {
        return;
    }

    if (node->is_leaf)
    {
        for (int i = 0; i < node->width * node->height; i++) 
        {
            fprintf(file, "%hhu %hhu %hhu ", node->intensity, node->intensity, node->intensity);
        }
    } 
    else 
    {
        save_qtree_as_ppm_helper(node->children[0], file);
        save_qtree_as_ppm_helper(node->children[1], file);
        save_qtree_as_ppm_helper(node->children[2], file);
        save_qtree_as_ppm_helper(node->children[3], file);
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename) 
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) 
    {
        ERROR("Failed to open file %s for writing", filename);
        return;
    }

    fprintf(file, "P3\n%d %d\n255\n", root->width, root->height);
    save_qtree_as_ppm_helper(root, file);
    fclose(file);
}