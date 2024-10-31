#include "qtree.h"

QTNode *new_node(unsigned char intensity, int is_leaf) 
{
    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node) return NULL;
    node->intensity = intensity;
    node->is_leaf = is_leaf;
    for (int i = 0; i < 4; i++) node->children[i] = NULL;
    return node;
}

QTNode *create_quadtree(Image *image, double max_rmse) 
{
    (void)image;
    (void)max_rmse;
    return NULL;
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