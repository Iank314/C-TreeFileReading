#include "qtree.h"
#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

QTNode *create_quadtree_helper(Image *image, int row, int col, int width, int height, double max_rmse)
{
    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed for QTNode.\n");
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
    return create_quadtree_helper(image, 0, 0, get_image_width(image), get_image_height(image), max_rmse);
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

void delete_quadtree(QTNode *root)
{
    if (root)
    {
        for (int i = 0; i < 4; i++)
        {
            delete_quadtree(root->children[i]);
        }
        free(root);
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename) 
{
    (void)root;
    (void)filename;
}


QTNode *load_preorder_qt(char *filename) 
{
    (void)filename;
    return NULL;
}


void save_preorder_qt(QTNode *root, char *filename) 
{
    (void)root;
    (void)filename;
}








