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
    PastedSourceItem(SourceImagePack img_pack,
                     SparseMatrixXd laplacian_matrix,
                     QPainterPath selection_path,
                     QGraphicsItem *parent = nullptr);
    ~PastedSourceItem();

    // Painting functions
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    // Image data manipulation function
    QImage originalImage();
    ImageMatricesRGB originalMatrices();
    void setOriginalPack(SourceImagePack img_pack);

    QImage blendedImage();
    ImageMatricesRGB blendedMatrices();
    void setBlendedPack(SourceImagePack img_pack);

    SelectMaskMatrices masks();
    SparseMatrixXd getLaplacianMatrix();

    // Item control functions
    bool isMoving();
    void setSelected(bool s);
    void updateItemControls();

    bool isComputing();
    void setComputing(bool en);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

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
    ImageMatricesRGB m_orig_matrices;

    QImage m_blended_image;
    ImageMatricesRGB m_blended_matrices;

    SelectMaskMatrices m_masks;
    SparseMatrixXd m_laplacian_matrix;

    // Graphics attributes
    QPixmap m_pixmap;
    QPainterPath m_selection_path;

    // Contour animation
    QTimer *m_anim_timer;
    qreal m_anim_dash_offset;

    // Wait computing animation
    QPropertyAnimation *m_prop_anim_wait;
    QColor m_wait_anim_color;

    // Status attributes
    bool m_is_moving;
    bool m_is_computing;
};

#endif // PASTEDSOURCEITEM_H
