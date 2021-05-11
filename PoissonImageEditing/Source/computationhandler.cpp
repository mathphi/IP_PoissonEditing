#include "computationhandler.h"
#include "transfercomputationunit.h"
#include "pastedsourceitem.h"

#include <QImage>
#include <QThreadPool>


// Static thread pool used by the computation handler
static QThreadPool *g_thread_pool = nullptr;


/**
 * @brief ComputationHandler::initializeComputationHandler
 * @param parent
 *
 * This function initializes the computation handler's thread pool.
 * If a parent argument is defined, it is used as parent for the QThreadPool.
 */
void ComputationHandler::initializeComputationHandler(QObject *parent) {
    g_thread_pool = new QThreadPool(parent);
}

/**
 * @brief ComputationHandler::startComputationJob
 * @param cu
 * @return
 *
 * This function adds the runnable object to the shared thread pool queue.
 * It returns false if the operation fails (thread pool not initialized).
 */
bool ComputationHandler::startComputationJob(QRunnable *cu) {
    // If the thread pool is not initialized
    if (!g_thread_pool)
        return false;

    // Add this computation unit to the thread pool queue
    g_thread_pool->start(cu);

    return true;
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
 * @brief ComputationHandler::imageToChannelMatrix
 * @param img
 * @param channel
 * @return
 *
 * This function converts an image's color channel to a matrix
 */
MatrixXd ComputationHandler::imageToChannelMatrix(QImage img, int channel) {
    QColor color;

    // Initialize the 3 channels matrices
    MatrixXd img_rgb_ch(img.height(), img.width());

    // Loop across the image rows
    for (int y = 0 ; y < img.height() ; y++) {
        // Loop across the pixels in the row
        for (int x = 0 ; x < img.width() ; x++) {
            color = img.pixel(x,y);

            // Put the needed color channel
            if (channel == 0) {
                img_rgb_ch(y,x) = color.redF();
            }
            else if (channel == 1) {
                img_rgb_ch(y,x) = color.greenF();
            }
            else {
                img_rgb_ch(y,x) = color.blueF();
            }
        }
    }

    return img_rgb_ch;
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
                    qBound(0.0, im_rgb[0](y,x) * 255.0, 255.0),
                    qBound(0.0, im_rgb[1](y,x) * 255.0, 255.0),
                    qBound(0.0, im_rgb[2](y,x) * 255.0, 255.0)
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
                    qBound(0.0, im_rgb[0](y,x) * 255.0, 255.0),
                    qBound(0.0, im_rgb[1](y,x) * 255.0, 255.0),
                    qBound(0.0, im_rgb[2](y,x) * 255.0, 255.0),
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

    // Dimension of the selection bounding rect (with 1px margin)
    QRect b_rect = selection_path.boundingRect().toAlignedRect().adjusted(-1, -1, 1, 1);

    // Allocating matrices
    smm.positive_mask = MatrixXd(b_rect.height(), b_rect.width());
    smm.negative_mask = MatrixXd(b_rect.height(), b_rect.width());

    // Mask = 1 inside the selection path, 0 outside (and vice-versa)
    for (int x = 0 ; x < b_rect.width() ; x++) {
        for (int y = 0 ; y < b_rect.height() ; y++) {
            if (selection_path.contains(QPoint(x,y) + b_rect.topLeft())) {
                smm.positive_mask(y,x) = 1.0;
                smm.negative_mask(y,x) = 0.0;
            }
            else {
                smm.positive_mask(y,x) = 0.0;
                smm.negative_mask(y,x) = 1.0;
            }
        }
    }

    return smm;
}

/**
 * @brief ComputationHandler::laplacianMatrix
 * @param img_size
 * @return
 *
 * Compute the laplacian for an image of size 'img_size'
 */
SparseMatrixXd ComputationHandler::laplacianMatrix(const QSize img_size, SelectMaskMatrices masks) {
    // Compute the fixed dimensions
    const uint32_t width = img_size.width();
    const uint32_t height = img_size.height();
    const uint32_t total_size = width*height;

    // Allocate the sparse matrix
    SparseMatrixXd lapl_mat(total_size, total_size);

    // Laplacian index
    uint32_t idx;

    // Allocate the non-zero elements in each row
    lapl_mat.reserve(Eigen::VectorXi::Constant(total_size, 5));
    for (uint32_t y = 0 ; y < height ; y++) {
        for (uint32_t x = 0 ; x < width ; x++) {
            // NOTE: the masks have a 1px margin
            // Don't care if outside the mask
            if (masks.positive_mask(y+1,x+1) == 0.0)
                continue;

            // Compute laplacian index
            idx = y*width + x;

            // Diagonal
            lapl_mat.insert(idx,idx) = 4.0;

            // Neighbour left
            if (masks.positive_mask(y+1,x) == 1.0) {
                lapl_mat.insert(idx,idx-1) = -1.0;
            }
            // Neighbour right
            if (masks.positive_mask(y+1,x+2) == 1.0) {
                lapl_mat.insert(idx,idx+1) = -1.0;
            }
            // Neighbour top
            if (masks.positive_mask(y,x+1) == 1.0) {
                lapl_mat.insert(idx,idx-width) = -1.0;
            }
            // Neighbour bottom
            if (masks.positive_mask(y+2,x+1) == 1.0) {
                lapl_mat.insert(idx,idx+width) = -1.0;
            }
        }
    }

    return lapl_mat;
}

/**
 * @brief ComputationHandler::computeImageGradient
 * @param img_ch
 * @param masks
 * @return
 *
 * This function computes the gradient vector from the image (sum v_{pq} in reference paper).
 */
VectorXd ComputationHandler::computeImageGradient(MatrixXd img_ch, SelectMaskMatrices masks) {
    // Size of the image (remove the 1px margin)
    const uint32_t inner_width = img_ch.cols() - 2;
    const uint32_t inner_height = img_ch.rows() - 2;

    // Column vector length
    const uint32_t N = inner_width*inner_height;
    VectorXd grad_vect(N);

    // Initialize the vector to 0
    grad_vect.setZero();

    // For each pixel p∈Ω -> compute the numerical gradient
    for (uint32_t y = 1 ; y < inner_height+1 ; y++) {
        for (uint32_t x = 1 ; x < inner_width+1 ; x++) {
            // Check if this pixel is in the mask
            if (masks.positive_mask(y,x) == 0)
                continue;

            // Compute gradient: v_{pq} = 4*p - sum(N_p)
            grad_vect((y-1)*inner_width + (x-1)) =
                    4.0 * img_ch(y,x)
                    - img_ch(y,x+1) - img_ch(y,x-1)     // Vertical neighbors
                    - img_ch(y+1,x) - img_ch(y-1,x);    // Horizontal neighbors
        }
    }

    return grad_vect;
}

/**
 * @brief ComputationHandler::computeImageGradientMixed
 * @param img_ch
 * @param other_grad
 * @param masks
 * @return
 *
 * This function computes the gradient of img_ch and returns a vector containing the largest gradient between
 * the two gradient vectors in absolute value.
 */
VectorXd ComputationHandler::computeImageGradientMixed(MatrixXd img_ch, VectorXd other_grad, SelectMaskMatrices masks) {
    // Size of the image (remove the 1px margin)
    const uint32_t inner_width = img_ch.cols() - 2;
    const uint32_t inner_height = img_ch.rows() - 2;

    // Column vector length
    const uint32_t N = inner_width*inner_height;
    VectorXd grad_vect(N);

    // Initialize the vector to 0
    grad_vect.setZero();

    // Temporary variable
    uint32_t idx = 0;
    float cmp_grad;

    // For each pixel p∈Ω -> compute the numerical gradient
    for (uint32_t y = 1 ; y < inner_height+1 ; y++) {
        for (uint32_t x = 1 ; x < inner_width+1 ; x++) {
            // Check if this pixel is in the mask
            if (masks.positive_mask(y,x) == 0)
                continue;

            // Compute the vector index
            idx = (y-1)*inner_width + (x-1);

            // Compute gradient: v_{pq} = 4*p - sum(N_p)
            cmp_grad =
                    4.0 * img_ch(y,x)
                    - img_ch(y,x+1) - img_ch(y,x-1)     // Vertical neighbors
                    - img_ch(y+1,x) - img_ch(y-1,x);    // Horizontal neighbors

            // Compare gradients in absolute value
            if (qAbs(cmp_grad) > qAbs(other_grad(idx))) {
                grad_vect(idx) = cmp_grad;
            }
            else {
                grad_vect(idx) = other_grad(idx);
            }

        }
    }

    return grad_vect;
}

/**
 * @brief ComputationHandler::computeBoundaryNeighbors
 * @param tgt_img_ch
 * @param masks
 * @return
 *
 * This function computes the sum of the neighbors of each pixel in the mask boundary.
 */
VectorXd ComputationHandler::computeBoundaryNeighbors(MatrixXd tgt_img_ch, SelectMaskMatrices masks) {
    // Size of the image (remove the 1px margin)
    const uint32_t inner_width = tgt_img_ch.cols() - 2;
    const uint32_t inner_height = tgt_img_ch.rows() - 2;

    // Column vector length
    const uint32_t N = inner_width*inner_height;
    VectorXd bound_vect(N);

    // Initialize the vector to 0
    bound_vect.setZero();

    // Compute the image masked using the negative mask
    MatrixXd neg_img_ch = tgt_img_ch.cwiseProduct(masks.negative_mask);

    // For each pixel p∈Ω -> compute the numerical gradient
    for (uint32_t y = 1 ; y < inner_height+1 ; y++) {
        for (uint32_t x = 1 ; x < inner_width+1 ; x++) {
            // Check if this pixel is in the mask
            if (masks.positive_mask(y,x) == 0)
                continue;

            // Compute the sum of the neighbors
            bound_vect((y-1)*inner_width + (x-1)) =
                    neg_img_ch(y,x+1) + neg_img_ch(y,x-1) +
                    neg_img_ch(y+1,x) + neg_img_ch(y-1,x);
        }
    }

    return bound_vect;
}

/**
 * @brief ComputationHandler::vectorToMatrixImage
 * @param img_vect
 * @param img_size
 * @return
 *
 * This function reshapes the image vector into an image matrix
 */
MatrixXd ComputationHandler::vectorToMatrixImage(VectorXd img_vect, QSize img_size) {
    // Allocate the image matrix
    MatrixXd img_mat(img_size.height(), img_size.width());

    // Loop over each pixel and copy it into the matrix
    for (int32_t y = 0 ; y < img_size.height() ; y++) {
        for (int32_t x = 0 ; x < img_size.width() ; x++) {
            img_mat(y,x) = img_vect(y*img_size.width() + x);
        }
    }

    return img_mat;
}



/*
 * Types serialization functions declaration
 */

// Eigen matrix serialization
QDataStream &operator>>(QDataStream &in, MatrixXd &p) {
    Eigen::Index rows;
    Eigen::Index cols;

    in >> rows;
    in >> cols;

    p.resize(rows, cols);
    in.readRawData((char*)p.data(), rows*cols*sizeof(float));

    return in;
}

QDataStream &operator<<(QDataStream &out, MatrixXd &p) {
    out << (Eigen::Index)p.rows();
    out << (Eigen::Index)p.cols();

    out.writeRawData((char*)p.data(), p.rows()*p.cols()*sizeof(float));

    return out;
}

// Eigen vector serialization
QDataStream &operator>>(QDataStream &in, VectorXd &p) {
    Eigen::Index rows;
    Eigen::Index cols;

    in >> rows;
    in >> cols;

    p.resize(rows, cols);
    in.readRawData((char*)p.data(), rows*cols*sizeof(float));

    return in;
}

QDataStream &operator<<(QDataStream &out, VectorXd &p) {
    out << (Eigen::Index)p.rows();
    out << (Eigen::Index)p.cols();

    out.writeRawData((char*)p.data(), p.rows()*p.cols()*sizeof(float));

    return out;
}

// Eigen sparse matrix serialization
QDataStream &operator>>(QDataStream &in, SparseMatrixXd &p) {
    Eigen::Index rows, cols, nnz, inSz, outSz;
    in >> rows;
    in >> cols;
    in >> nnz;
    in >> outSz;
    in >> inSz;

    p.resize(rows, cols);
    p.makeCompressed();
    p.resizeNonZeros(nnz);

    in.readRawData((char*)(p.valuePtr()), nnz * sizeof(float));
    in.readRawData((char*)(p.outerIndexPtr()), outSz * sizeof(SparseMatrixXd::StorageIndex));
    in.readRawData((char*)(p.innerIndexPtr()), nnz * sizeof(SparseMatrixXd::StorageIndex));

    p.finalize();

    return in;
}

QDataStream &operator<<(QDataStream &out, SparseMatrixXd &p) {
    // Compress sparse matrix
    p.makeCompressed();

    // Size of the vectors composing the sparse matrix
    out << (Eigen::Index) p.rows();
    out << (Eigen::Index) p.cols();
    out << (Eigen::Index) p.nonZeros();
    out << (Eigen::Index) p.outerSize();
    out << (Eigen::Index) p.innerSize();

    // Sparse matrix data
    out.writeRawData((char*)(p.valuePtr()), p.nonZeros() * sizeof(float));
    out.writeRawData((char*)(p.outerIndexPtr()), p.outerSize() * sizeof(SparseMatrixXd::StorageIndex));
    out.writeRawData((char*)(p.innerIndexPtr()), p.nonZeros() * sizeof(SparseMatrixXd::StorageIndex));

    return out;
}

// SelectMaskMatrices serialization
QDataStream &operator>>(QDataStream &in, SelectMaskMatrices &p) {
    in >> p.positive_mask;
    in >> p.negative_mask;

    return in;
}

QDataStream &operator<<(QDataStream &out, SelectMaskMatrices &p) {
    out << p.positive_mask;
    out << p.negative_mask;

    return out;
}

