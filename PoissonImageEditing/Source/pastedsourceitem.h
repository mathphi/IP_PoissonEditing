#ifndef PASTEDSOURCEITEM_H
#define PASTEDSOURCEITEM_H

#include <QImage>
#include <QPainterPath>
#include <QGraphicsObject>

#include "computationhandler.h"

class PastedSourceItem : public QGraphicsObject
{
    Q_OBJECT

public:
    PastedSourceItem(SourceImagePack img_pack, QPainterPath selection_path, QGraphicsItem *parent = nullptr);
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

    // Item control functions
    bool isMoving();
    void setSelected(bool s);
    void updateItemControls();

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
    // Original/blended image data
    QImage m_orig_image;
    ImageMatricesRGB m_orig_matrices;

    QImage m_blended_image;
    ImageMatricesRGB m_blended_matrices;

    SelectMaskMatrices m_masks;

    // Graphics attributes
    QPixmap m_pixmap;
    QPainterPath m_selection_path;

    // Contour animation
    QTimer *m_anim_timer;
    qreal m_anim_dash_offset;

    // Moving status attribute
    bool m_is_moving;
};

#endif // PASTEDSOURCEITEM_H
