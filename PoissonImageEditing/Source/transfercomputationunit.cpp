#include "transfercomputationunit.h"
#include "computationhandler.h"
#include "pastedsourceitem.h"

TransferComputationUnit::TransferComputationUnit(QImage source_image, QPainterPath selection_path)
    : QObject(), QRunnable()
{
    m_source_image = source_image;
    m_selection_path = selection_path;

    setAutoDelete(false);
}

void TransferComputationUnit::run() {
    // Emit started signal
    emit computationStarted();

    // Compute...
    computeTransferData();

    // Emit finished signal
    emit computationFinished();
}

void TransferComputationUnit::computeTransferData() {
    // Convert the image into RGB matrices
    ImageMatricesRGB img_mat = ComputationHandler::imageToMatrices(m_source_image);

    // Compute the selection masks
    SelectMaskMatrices smm = ComputationHandler::selectionToMask(m_selection_path);

    // Compute the masked original image
    ImageMatricesRGB masked_src_img;
    masked_src_img[0] = img_mat[0].cwiseProduct(smm.positive_mask);
    masked_src_img[1] = img_mat[1].cwiseProduct(smm.positive_mask);
    masked_src_img[2] = img_mat[2].cwiseProduct(smm.positive_mask);

    // Convert the masked image matrices to QImage
    QImage masked_img = ComputationHandler::matricesToImage(masked_src_img, smm.positive_mask);


    // Compute the Laplacian matrix
    // Remove 2px (1px margin top/bottom; right/left)
    SparseMatrixXd laplacian_mat = ComputationHandler::laplacianMatrix(m_source_image.size() - QSize(2,2), smm);

    // Save computed results
    m_original_matrices     = img_mat;
    m_masks                 = smm;
    m_original_image_masked = masked_img;
    m_laplacian             = laplacian_mat;
}


ImageMatricesRGB TransferComputationUnit::getOriginalMatrices() {
    return m_original_matrices;
}

SelectMaskMatrices TransferComputationUnit::getMasks() {
    return m_masks;
}

QImage TransferComputationUnit::getOriginalImageMasked() {
    return m_original_image_masked;
}

SparseMatrixXd TransferComputationUnit::getLaplacian() {
    return m_laplacian;
}
