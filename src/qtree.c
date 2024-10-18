#include "qtree.h"

QTNode *create_quadtree(Image *image, double max_rmse) {
    (void)image;
    (void)max_rmse;
    return NULL;
}

QTNode *get_child1(QTNode *node) {
    (void)node;
    return NULL;
}

QTNode *get_child2(QTNode *node) {
    (void)node;
    return NULL;
}

QTNode *get_child3(QTNode *node) {
    (void)node;
    return NULL;
}

QTNode *get_child4(QTNode *node) {
    (void)node;
    return NULL;
}

unsigned char get_node_intensity(QTNode *node) {
    (void)node;
    return 0;
}

void delete_quadtree(QTNode *root) {
    (void)root;
}

void save_qtree_as_ppm(QTNode *root, char *filename) {
    (void)root;
    (void)filename;
}

QTNode *load_preorder_qt(char *filename) {
    (void)filename;
    return NULL;
}

void save_preorder_qt(QTNode *root, char *filename) {
    (void)root;
    (void)filename;
}

