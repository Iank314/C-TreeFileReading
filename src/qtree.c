#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "image.h"

typedef struct QTNode {
    unsigned char intensity;
    int row, col;
    int width, height;
    struct QTNode *child1, *child2, *child3, *child4;
} QTNode;

static double calculate_rmse_and_avg(Image *image, int row, int col, int width, int height, unsigned char *avg_intensity);
static QTNode *create_quadtree_helper(Image *image, int row, int col, int width, int height, double max_rmse);
static void save_qtree_as_ppm_helper(QTNode *node, FILE *file);
static QTNode *load_preorder_qt_helper(FILE *file);
static void save_preorder_qt_helper(QTNode *node, FILE *file);

static double calculate_rmse_and_avg(Image *image, int row, int col, int width, int height, unsigned char *avg_intensity) {
    int total_pixels = width * height;
    double sum = 0.0, sum_squares = 0.0;
    for (int i = row; i < row + height; i++) {
        for (int j = col; j < col + width; j++) {
            unsigned char intensity = get_image_intensity(image, i, j);
            sum += intensity;
            sum_squares += intensity * intensity;
        }
    }
    *avg_intensity = (unsigned char)(sum / total_pixels);
    double mean = sum / total_pixels;
    double mean_square = sum_squares / total_pixels;
    return sqrt(mean_square - mean * mean);
}

static QTNode *create_quadtree_helper(Image *image, int row, int col, int width, int height, double max_rmse) {
    unsigned char avg_intensity;
    double rmse = calculate_rmse_and_avg(image, row, col, width, height, &avg_intensity);

    QTNode *node = (QTNode *)malloc(sizeof(QTNode));
    if (!node) return NULL;

    node->intensity = avg_intensity;
    node->row = row;
    node->col = col;
    node->width = width;
    node->height = height;

    if (rmse <= max_rmse || width <= 1 || height <= 1) {
        node->child1 = node->child2 = node->child3 = node->child4 = NULL;
        return node;
    }

    int half_width = width / 2;
    int half_height = height / 2;

    node->child1 = create_quadtree_helper(image, row, col, half_width, half_height, max_rmse);
    node->child2 = create_quadtree_helper(image, row, col + half_width, half_width, half_height, max_rmse);
    node->child3 = create_quadtree_helper(image, row + half_height, col, half_width, half_height, max_rmse);
    node->child4 = create_quadtree_helper(image, row + half_height, col + half_width, half_width, half_height, max_rmse);

    return node;
}

QTNode *create_quadtree(Image *image, double max_rmse) {
    return create_quadtree_helper(image, 0, 0, get_image_width(image), get_image_height(image), max_rmse);
}

void delete_quadtree(QTNode *root) {
    if (!root) return;
    delete_quadtree(root->child1);
    delete_quadtree(root->child2);
    delete_quadtree(root->child3);
    delete_quadtree(root->child4);
    free(root);
}

static void save_qtree_as_ppm_helper(QTNode *node, FILE *file) {
    if (!node) return;
    if (!node->child1 && !node->child2 && !node->child3 && !node->child4) {
        for (int i = 0; i < node->height; i++) {
            for (int j = 0; j < node->width; j++) {
                fprintf(file, "%d %d %d\n", node->intensity, node->intensity, node->intensity);
            }
        }
    } else {
        save_qtree_as_ppm_helper(node->child1, file);
        save_qtree_as_ppm_helper(node->child2, file);
        save_qtree_as_ppm_helper(node->child3, file);
        save_qtree_as_ppm_helper(node->child4, file);
    }
}

void save_qtree_as_ppm(QTNode *root, char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;
    fprintf(file, "P3\n%d %d\n255\n", root->width, root->height);
    save_qtree_as_ppm_helper(root, file);
    fclose(file);
}

QTNode *load_preorder_qt(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return NULL;
    QTNode *root = load_preorder_qt_helper(file);
    fclose(file);
    return root;
}

static QTNode *load_preorder_qt_helper(FILE *file) {
    char type;
    if (fscanf(file, " %c", &type) != 1) return NULL;

    QTNode *node = malloc(sizeof(QTNode));
    if (!node) return NULL;

    fscanf(file, "%hhu %d %d %d %d", &node->intensity, &node->row, &node->col, &node->width, &node->height);

    if (type == 'L') {
        node->child1 = node->child2 = node->child3 = node->child4 = NULL;
    } else {
        node->child1 = load_preorder_qt_helper(file);
        node->child2 = load_preorder_qt_helper(file);
        node->child3 = load_preorder_qt_helper(file);
        node->child4 = load_preorder_qt_helper(file);
    }
    return node;
}

void save_preorder_qt(QTNode *root, char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;
    save_preorder_qt_helper(root, file);
    fclose(file);
}

static void save_preorder_qt_helper(QTNode *node, FILE *file) {
    if (!node) return;
    char type = (node->child1 || node->child2 || node->child3 || node->child4) ? 'N' : 'L';
    fprintf(file, "%c %hhu %d %d %d %d\n", type, node->intensity, node->row, node->col, node->width, node->height);

    save_preorder_qt_helper(node->child1, file);
    save_preorder_qt_helper(node->child2, file);
    save_preorder_qt_helper(node->child3, file);
    save_preorder_qt_helper(node->child4, file);
}