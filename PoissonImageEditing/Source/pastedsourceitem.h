#ifndef PASTEDSOURCEITEM_H
#define PASTEDSOURCEITEM_H

#include <QImage>
#include <QPainterPath>
#include <QGraphicsObject>

#include "computationhandler.h"

class QPropertyAnimation;

class PastedSourceItem : public QGraphicsObject
{
    Q_OBJECT

public:
    PastedSourceItem(QImage src_img,
                     QPainterPath selection_path,
                     ComputationHandler *ch_ptr,
                     QGraphicsItem *parent = nullptr);
    ~PastedSourceItem();

    // Painting functions
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    QPainterPath getSelectionPath();

    // Image data manipulation function
    QImage originalImage();
    QImage originalImageMasked();
    QImage blendedImage();

    ImageMatricesRGB originalMatrices();
    SelectMaskMatrices masks();
    SparseMatrixXd laplacianMatrix();
    ImageVectorRGB gradientVectors();

    // Item control functions
    bool isMoving();
    void setSelected(bool s);
    void updateItemControls();

    bool isComputing();
    void setComputing(bool en);

public slots:
    void transferFinished();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void focusInEvent(QFocusEvent *focusEvent) override;
    virtual void focusOutEvent(QFocusEvent *focusEvent) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private slots:
    void animateContour();

private:
    Q_PROPERTY(QColor waitAnimColor READ waitAnimColor WRITE setWaitAnimColor)
    QColor waitAnimColor();
    void setWaitAnimColor(QColor color);

    // Original/blended image data
    QImage m_orig_image;
    QImage m_orig_image_masked;
    ImageMatricesRGB m_orig_matrices;

    QImage m_blended_image;

    SelectMaskMatrices m_masks;
    SparseMatrixXd m_laplacian_matrix;

    ImageVectorRGB m_gradient_vectors;

    // Graphics attributes
    QPixmap m_pixmap;
    QPainterPath m_selection_path;
    QPainterPath m_normalized_path;

    // Contour animation
    QTimer *m_anim_timer;
    qreal m_anim_dash_offset;

    // Wait computing animation
    QPropertyAnimation *m_prop_anim_wait;
    QColor m_wait_anim_color;

    // Status attributes
    bool m_is_moving;
    bool m_is_computing;

    // Pointer to the computation handler
    ComputationHandler *m_computation_hander;
    TransferComputationUnit *m_transfer_job;
};

#endif // PASTEDSOURCEITEM_H
