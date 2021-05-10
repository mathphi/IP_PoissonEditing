#include "blendingcomputationunit.h"

BlendingComputationUnit::BlendingComputationUnit(
        int channel_num,
        QImage target_img,
        VectorXd src_grad,
        SelectMaskMatrices masks,
        SparseMatrixXd laplacian,
        bool mixed_blending)
    : QObject(), QRunnable()
{
    m_channel_num = channel_num;
    m_target_img = target_img;
    m_src_grad = src_grad;
    m_masks = masks;
    m_laplacian = laplacian;
    m_mixed_blending = mixed_blending;

    setAutoDelete(false);
}

void BlendingComputationUnit::run() {
    // Emit started signal
    emit computationStarted();

    // Compute...
    computeBlendingData();

    // Emit finished signal
    emit computationFinished();
}

void BlendingComputationUnit::computeBlendingData() {
    // Convert the target image into matrices
    MatrixXd tgt_matrix_ch = ComputationHandler::imageToChannelMatrix(m_target_img, m_channel_num);

    // Compute the boundary conditions with the target image
    VectorXd bound = ComputationHandler::computeBoundaryNeighbors(tgt_matrix_ch, m_masks);

    // Gradient vector
    VectorXd grad;

    // If mixed blending -> also compute the target gradient then mix them
    if (m_mixed_blending) {
        grad = ComputationHandler::computeImageGradientMixed(tgt_matrix_ch, m_src_grad, m_masks);
    }
    else {
        grad = m_src_grad;
    }

    // Compute the independent terms vector (b vector in linear problem Ax=b)
    VectorXd b = grad + bound;

    // Initialise the solver and factorize the laplacian (A matrix)
    Eigen::ConjugateGradient<Eigen::SparseMatrix<float>> solver;
    solver.analyzePattern(m_laplacian);
    solver.factorize(m_laplacian);

    // The the linear algebra equation
    VectorXd x = solver.solve(b);

    // Reshape the vector to a image matrix
    // The image matrix size is given without the 1px margin (-QSize(2,2))
    MatrixXd x_mat = ComputationHandler::vectorToMatrixImage(x, m_target_img.size() - QSize(2,2));

    // Place the x_mat at the center of a matrix WITH 1px margin (original image dimension)
    MatrixXd x_mat_outer(m_target_img.height(), m_target_img.width());
    x_mat_outer.block(1, 1, x_mat.rows(), x_mat.cols()) = x_mat;

    // Store the result
    m_blended_channel = x_mat_outer;
}

int BlendingComputationUnit::getChannelNumber() {
    return m_channel_num;
}

MatrixXd BlendingComputationUnit::getBlendedChannel() {
    return m_blended_channel;
}
