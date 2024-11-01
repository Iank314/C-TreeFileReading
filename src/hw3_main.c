#include "qtree.h"
#include "image.h"
#include "tests_utils.h"

int main() {
    struct stat st;
    if (stat("tests/output", &st) == -1)
        mkdir("tests/output", 0700);
    prepare_input_image_file("building1.ppm"); // copies the image to the images/ directory

    /******************************* create_quadtree *******************************/
    double max_rmse = 25;
    Image *image = load_image("images/building1.ppm");
    QTNode *root = create_quadtree(image, max_rmse);
    int width = get_image_width(image);
    int height = get_image_height(image);

    delete_image(image);

    /******************************* save_preorder_qt *******************************/
    save_preorder_qt(root, "tests/output/save_preorder_qt1_qtree.txt", width, height);

    /******************************* save_qtree_as_ppm *******************************/
    save_qtree_as_ppm(root, "tests/output/save_qtree_as_ppm1.ppm", width, height);

    delete_quadtree(root);

    return 0;
}