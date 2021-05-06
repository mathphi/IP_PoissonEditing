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
ImageMatricesRGB ComputationHandler::imageToMatrices(QImage img) {
    QColor color;

    // Initialize the 3 channels matrices
    MatrixXd img_rgb_r(img.height(), img.width());
    MatrixXd img_rgb_g(img.height(), img.width());
    MatrixXd img_rgb_b(img.height(), img.width());

    // Loop across the image rows
    for (int y = 0 ; y < img.height() ; y++) {
        // Loop across the pixels in the row
        for (int x = 0 ; x < img.width() ; x++) {
            color = img.pixel(x,y);

            // Split channels into matrices
            img_rgb_r(y,x) = color.redF();
            img_rgb_g(y,x) = color.greenF();
            img_rgb_b(y,x) = color.blueF();
        }
    }

    return {img_rgb_r, img_rgb_g, img_rgb_b};
}

/**
 * @brief ComputationHandler::matricesToImage
 * @param im_rgb
 * @return
 *
 * This function performs a conversion from 3-matrix format to QImage
 */
QImage ComputationHandler::matricesToImage(ImageMatricesRGB im_rgb) {
    // Allocate the QImage
    QImage img(im_rgb[0].cols(), im_rgb[0].rows(), QImage::Format_RGB32);

    for (int y = 0 ; y < img.height() ; y++) {
        // Get a RGB pointer to the destination image row
        QRgb *row = (QRgb*) img.scanLine(y);

        // Set each pixel in the row
        for (int x = 0 ; x < img.width() ; x++) {
            row[x] = qRgb(
                    im_rgb[0](y,x) * 255,
                    im_rgb[1](y,x) * 255,
                    im_rgb[2](y,x) * 255
                );
        }
    }

    return img;
}

/**
 * @brief ComputationHandler::matricesToImage
 * @param im_rgb
 * @return
 *
 * This function performs a conversion from 3-matrix format to QImage
 */
QImage ComputationHandler::matricesToImage(ImageMatricesRGB im_rgb, MatrixXd alpha_mask) {
    // Allocate the QImage
    QImage img(im_rgb[0].cols(), im_rgb[0].rows(), QImage::Format_ARGB32);

    for (int y = 0 ; y < img.height() ; y++) {
        // Get a RGB pointer to the destination image row
        QRgb *row = (QRgb*) img.scanLine(y);

        // Set each pixel in the row
        for (int x = 0 ; x < img.width() ; x++) {
            row[x] = qRgba(
                    im_rgb[0](y,x) * 255,
                    im_rgb[1](y,x) * 255,
                    im_rgb[2](y,x) * 255,
                    alpha_mask(y,x) * 255
                );
        }
    }

    return img;
}

/**
 * @brief ComputationHandler::selectionToMask
 * @param selection_path
 * @return
 *
 * This function computes a mask and its invert inside the selection bounding rect
 */
SelectMaskMatrices ComputationHandler::selectionToMask(QPainterPath selection_path) {
    SelectMaskMatrices smm;

    // Dimension of the selection bounding rect
    QRect b_rect = selection_path.boundingRect().toRect();

    // Allocating matrices
    smm.mask = MatrixXd(b_rect.height(), b_rect.width());
    smm.inverted_mask = MatrixXd(b_rect.height(), b_rect.width());

    // Mask = 1 inside the selection path, 0 outside (and vice-versa)
    for (int x = 0 ; x < b_rect.width() ; x++) {
        for (int y = 0 ; y < b_rect.height() ; y++) {
            if (selection_path.contains(QPoint(x,y) + b_rect.topLeft())) {
                smm.mask(y,x) = 1.0;
                smm.inverted_mask(y,x) = 0.0;
            }
            else {
                smm.mask(y,x) = 0.0;
                smm.inverted_mask(y,x) = 1.0;
            }
        }
    }

    return smm;
}


SparseMatrixXd ComputationHandler::laplacianMatrix(const QSize img_size) {
    // Compute the fixed dimensions
    const uint32_t row_size = img_size.width();
    const uint32_t total_size = img_size.width()*img_size.height();

    // Allocate the sparse matrix
    SparseMatrixXd lapl_mat(total_size, total_size);

    // Allocate the non-zero elements in each row
    lapl_mat.reserve(Eigen::VectorXi::Constant(total_size, 5));
    for (uint32_t i = 0 ; i < total_size ; i++) {
        // Diagonal
        lapl_mat.insert(i,i) = 4.0;

        // Neighbour left
        if (i > 0) {
            lapl_mat.insert(i,i-1) = -1.0;
        }
        // Neighbour right
        if (i < total_size-1) {
            lapl_mat.insert(i,i+1) = -1.0;
        }
        // Neighbour top
        if (i > row_size-1) {
            lapl_mat.insert(i,i-row_size) = -1.0;
        }
        // Neighbour bottom
        if (i < total_size-row_size-1) {
            lapl_mat.insert(i,i+row_size) = -1.0;
        }
    }

    return lapl_mat;
}
