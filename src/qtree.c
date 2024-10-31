#include "qtree.h"
#include <math.h>

static QTNode *new_node(unsigned char intensity, int is_leaf) 
{
    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node) return NULL;
    node->intensity = intensity;
    node->is_leaf = is_leaf;
    for (int i = 0; i < 4; i++) node->children[i] = NULL;
    return node;
}

static double calculate_rmse(Image *image, int x, int y, int width, int height, unsigned char avg_intensity) 
{
    double sum_squared_diff = 0;
    int count = 0;
    for (int row = y; row < y + height; row++) 
    {
        for (int col = x; col < x + width; col++) 
        {
            unsigned char intensity = get_image_intensity(image, row, col);
            sum_squared_diff += (intensity - avg_intensity) * (intensity - avg_intensity);
            count++;
        }
    }
    return sqrt(sum_squared_diff / count);
}

static unsigned char calculate_average_intensity(Image *image, int x, int y, int width, int height) 
{
    int sum = 0;
    int count = 0;
    for (int row = y; row < y + height; row++) 
    {
        for (int col = x; col < x + width; col++) 
        {
            sum += get_image_intensity(image, row, col);
            count++;
        }
    }
    return (unsigned char)(sum / count);
}

QTNode *create_quadtree_helper(Image *image, int x, int y, int width, int height, double max_rmse) 
{
    if (width <= 0 || height <= 0) return NULL;

    unsigned char avg_intensity = calculate_average_intensity(image, x, y, width, height);
    double rmse = calculate_rmse(image, x, y, width, height, avg_intensity);

    if (rmse <= max_rmse || width == 1 || height == 1) 
    {
        return new_node(avg_intensity, 1);
    }

    QTNode *node = new_node(0, 0);

    int half_width = width / 2;
    int half_height = height / 2;

    node->children[0] = create_quadtree_helper(image, x, y, half_width, half_height, max_rmse);
    node->children[1] = create_quadtree_helper(image, x + half_width, y, width - half_width, half_height, max_rmse);
    node->children[2] = create_quadtree_helper(image, x, y + half_height, half_width, height - half_height, max_rmse);
    node->children[3] = create_quadtree_helper(image, x + half_width, y + half_height, width - half_width, height - half_height, max_rmse);

    return node;
}

QTNode *create_quadtree(Image *image, double max_rmse) 
{
    return create_quadtree_helper(image, 0, 0, image->width, image->height, max_rmse);
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
    if (!root) return;
    for (int i = 0; i < 4; i++) 
    {
        delete_quadtree(root->children[i]);
    }
    free(root);
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