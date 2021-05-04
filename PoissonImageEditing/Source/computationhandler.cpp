#include "computationhandler.h"

#include <QImage>


ComputationHandler::ComputationHandler(QObject *parent) : QObject(parent)
{

}

/**
 * @brief ComputationHandler::imageToMatrices
 * @param img
 * @return
 *
 * This function converts an image into 3 matrices (for the 3 channels: red, green, blue).
 * The image format in the matrices is float (pixel values from 0 to 1).
 */
std::array<MatrixXd,3> ComputationHandler::imageToMatrices(QImage img) {
    // Initialize the 3 channels matrices
    MatrixXd img_mat_r(img.height(), img.width());
    MatrixXd img_mat_g(img.height(), img.width());
    MatrixXd img_mat_b(img.height(), img.width());

    // Loop across the image rows
    for (int y = 0 ; y < img.height() ; y++) {
        // Get a pointer to the yth row of the image
        const uint8_t *img_row = img.constScanLine(y);

        // Loop across the pixels in the row
        for (int x = 0 ; x < img.width() ; x++) {
            img_mat_b(y,x) = *img_row++ / 255.0;
            img_mat_g(y,x) = *img_row++ / 255.0;
            img_mat_r(y,x) = *img_row++ / 255.0;

            // Skip alpha channel
            img_row++;
        }
    }

    return {img_mat_r, img_mat_g, img_mat_b};
}
