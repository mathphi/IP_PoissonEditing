#include "transfercomputationunit.h"
#include "computationhandler.h"
#include "pastedsourceitem.h"

TransferComputationUnit::TransferComputationUnit(PastedSourceItem *origin)
{
    m_origin = origin;
}

void TransferComputationUnit::run() {
    computeTransferData();
}

void TransferComputationUnit::computeTransferData() {
    // Inform the origin that computation started
    m_origin->setComputing(true);

    QImage source_image = m_origin->originalImage();
    QPainterPath select_path = m_origin->getSelectionPath();

    // Convert the image into RGB matrices
    ImageMatricesRGB img_mat = ComputationHandler::imageToMatrices(source_image);

    // Compute the selection masks
    SelectMaskMatrices smm = ComputationHandler::selectionToMask(select_path);

    // Compute the masked original image
    ImageMatricesRGB masked_src_img;
    masked_src_img[0] = img_mat[0].cwiseProduct(smm.positive_mask);
    masked_src_img[1] = img_mat[1].cwiseProduct(smm.positive_mask);
    masked_src_img[2] = img_mat[2].cwiseProduct(smm.positive_mask);

    // Convert the masked image matrices to QImage
    QImage masked_img = ComputationHandler::matricesToImage(masked_src_img, smm.positive_mask);


    // Compute the Laplacian matrix
    // Remove 2px (1px margin top/bottom; right/left)
    SparseMatrixXd laplacian_mat = ComputationHandler::laplacianMatrix(source_image.size() - QSize(2,2), smm);

    // Compute the source image gradient vectors
    ImageVectorRGB grad_vectors;
    grad_vectors[0] = ComputationHandler::computeImageGradient(img_mat[0], smm);
    grad_vectors[1] = ComputationHandler::computeImageGradient(img_mat[1], smm);
    grad_vectors[2] = ComputationHandler::computeImageGradient(img_mat[2], smm);


    // Send computed results to the origin
    m_origin->setOriginalMatrices(img_mat);
    m_origin->setMasks(smm);
    m_origin->setOriginalImageMasked(masked_img);
    m_origin->setLaplacianMatrix(laplacian_mat);
    m_origin->setGradientVectors(grad_vectors);


    // Inform the origin that computation is done
    m_origin->transferFinished();
}
