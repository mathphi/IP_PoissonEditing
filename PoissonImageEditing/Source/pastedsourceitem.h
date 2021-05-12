#ifndef PASTEDSOURCEITEM_H
#define PASTEDSOURCEITEM_H

#include <QMutex>
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
                     QImage target_image,
                     bool compute_transfer_data = true,
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

    // Item control functions
    bool isMoving();
    void setSelected(bool s);
    void updateItemControls();

    bool isInvalid();
    void invalidateBlending();

    bool isComputing();
    void setComputing(bool en);

    bool isRealTime();
    void setRealTime(bool en);

    bool isMixedBlending();
    void setMixedBlending(bool en);

    void startBlendingComputation();

public slots:
    void transferFinished();
    void blendingFinished();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private slots:
    void animateContour();

private:
    Q_PROPERTY(QColor waitAnimColor READ waitAnimColor WRITE setWaitAnimColor)
    QColor waitAnimColor();
    void setWaitAnimColor(QColor color);

    // Link to the whole target image
    QImage m_target_image;

    // Original/blended image data
    QImage m_orig_image;
    QImage m_orig_image_masked;
    ImageMatricesRGB m_orig_matrices;

    QImage m_blended_image;
    ImageMatricesRGB m_blended_matrices;

    SelectMaskMatrices m_masks;
    SparseMatrixXd m_laplacian_matrix;

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
    bool m_is_invalid;

    // Blending attributes
    bool m_is_real_time;
    bool m_is_mixed_blending;

    // Transfer computation attributes
    TransferComputationUnit *m_transfer_job;

    // Blending management attributes
    QList<BlendingComputationUnit*> m_blending_unit_list;
    QMutex m_blending_mutex;


    // Operator overloaded to write objects from this class into a files
    friend QDataStream &operator>>(QDataStream &in, PastedSourceItem *&o);
    friend QDataStream &operator<<(QDataStream &out, PastedSourceItem *o);
};


#endif // PASTEDSOURCEITEM_H
