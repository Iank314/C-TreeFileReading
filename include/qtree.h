#ifndef QTREE_H
#define QTREE_H

#include <stdio.h>
#include <stdlib.h>
#include "image.h"

#define INFO(...) do {fprintf(stderr, "[          ] [ INFO ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr);} while(0)
#define ERROR(...) do {fprintf(stderr, "[          ] [ ERR  ] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr);} while(0)

typedef struct QTNode {
    unsigned char intensity;
    struct QTNode *children[4];
    int is_leaf;
} QTNode;

QTNode *create_quadtree(Image *image, double max_rmse);
void save_preorder_qt(QTNode *root, char *filename, int width, int height);
void save_qtree_as_ppm(QTNode *root, char *filename, int width, int height);
void delete_quadtree(QTNode *root);

QTNode *load_preorder_qt(char *filename);
QTNode *get_child1(QTNode *node);
QTNode *get_child2(QTNode *node);
QTNode *get_child3(QTNode *node);
QTNode *get_child4(QTNode *node);
unsigned char get_node_intensity(QTNode *node);

#endif // QTREE_H