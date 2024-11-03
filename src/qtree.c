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
    if (!file) 
    {
        ERROR("Failed to open file for reading.");
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


        if (width > 1 && height > 1)
        {
            int half_width = width / 2;
            int half_height = height / 2;
            save_preorder_qt_helper(node->children[0], file, row, col, half_width, half_height);
            save_preorder_qt_helper(node->children[1], file, row, col + half_width, width - half_width, half_height);
            save_preorder_qt_helper(node->children[2], file, row + half_height, col, half_width, height - half_height);
            save_preorder_qt_helper(node->children[3], file, row + half_height, col + half_width, width - half_width, height - half_height);
        }
        else if (width > 1)
        {
            int half_width = width / 2;
            save_preorder_qt_helper(node->children[0], file, row, col, half_width, height);
            save_preorder_qt_helper(node->children[1], file, row, col + half_width, width - half_width, height);
        }
        else if (height > 1)
        {
            int half_height = height / 2;
            save_preorder_qt_helper(node->children[0], file, row, col, width, half_height);
            save_preorder_qt_helper(node->children[2], file, row + half_height, col, width, height - half_height);
        }
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
    save_preorder_qt_helper(root, file, 0, 0, root->width, root->height);
    fclose(file);
}


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

void save_qtree_as_ppm_buffered(QTNode *node, unsigned char *buffer, int start_row, int start_col, int image_width)
{
    if (node == NULL) 
    {
        return;
    }

    if (node->is_leaf == 1) 
    {
        fill_region(buffer, node->intensity, start_row, start_col, node->width, node->height, image_width);
    } 
    else 
    {
        int half_width = node->width / 2;
        int half_height = node->height / 2;

        if (node->children[0]) save_qtree_as_ppm_buffered(node->children[0], buffer, start_row, start_col, image_width);
        if (node->children[1]) save_qtree_as_ppm_buffered(node->children[1], buffer, start_row, start_col + half_width, image_width);
        if (node->children[2]) save_qtree_as_ppm_buffered(node->children[2], buffer, start_row + half_height, start_col, image_width);
        if (node->children[3]) save_qtree_as_ppm_buffered(node->children[3], buffer, start_row + half_height, start_col + half_width, image_width);
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL) 
    {
        return;
    }

    fprintf(file, "P3\n%hu %hu\n255\n", root->width, root->height);

    int buffer_size = root->width * root->height * 3;
    unsigned char *buffer = (unsigned char *)malloc(buffer_size);
    if (buffer == NULL) 
    {
        fclose(file);
        return;
    }

    save_qtree_as_ppm_buffered(root, buffer, 0, 0, root->width);

    for (int i = 0; i < buffer_size; i += 3)
    {
        fprintf(file, "%hhu %hhu %hhu ", buffer[i], buffer[i + 1], buffer[i + 2]);
        if ((i / 3 + 1) % root->width == 0) 
        {
            fprintf(file, "\n");
        }
    }

    free(buffer);
    fclose(file);
}