#include "pastedsourceitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QFocusEvent>
#include <QPainter>
#include <QCursor>
#include <QTimer>
#include <QPen>

#include <QDebug>


#define SELECTION_WIDTH 1.5
#define DASH_SIZE       6.0
#define ANIM_INTERVAL   250   // ms


PastedSourceItem::PastedSourceItem(
        SourceImagePack img_pack,
        SparseMatrixXd laplacian_matrix,
        QPainterPath selection_path,
        QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    // Save the source image pack
    m_orig_image = img_pack.image;
    m_orig_matrices = img_pack.matrices;
    m_masks = img_pack.masks;

    // Convert the original image into pixmap (for display)
    m_pixmap = QPixmap::fromImage(m_orig_image);
    m_selection_path = selection_path;

    // Copy the laplacian matrix
    m_laplacian_matrix = laplacian_matrix;

    // Initialize status attributes
    m_is_moving = false;
    m_is_computing = false;

    // This item accepts mouse hover events
    setAcceptHoverEvents(true);

    // This item accepts selection
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    // The item is initially not selected
    setSelected(false);
    updateItemControls();

    // Contour animation timer
    m_anim_timer = new QTimer;
    m_anim_timer->setInterval(ANIM_INTERVAL);
    m_anim_timer->start();

    // Dash offset (will be dynamically updated by timer)
    m_anim_dash_offset = 0.0;

    // Create the wait animation property animator
    m_wait_anim_color = Qt::white;
    m_prop_anim_wait = new QPropertyAnimation(this, "waitAnimColor", this);
    m_prop_anim_wait->setStartValue(QColor(Qt::white));
    m_prop_anim_wait->setKeyValueAt(0.5, QColor(150, 190, 255));
    m_prop_anim_wait->setEndValue(QColor(Qt::white));
    m_prop_anim_wait->setEasingCurve(QEasingCurve::InOutSine);
    m_prop_anim_wait->setDuration(2000);
    m_prop_anim_wait->setLoopCount(-1);     // Infinite repetitions

    connect(m_anim_timer, SIGNAL(timeout()), this, SLOT(animateContour()));
}

PastedSourceItem::~PastedSourceItem() {
    m_anim_timer->stop();
    delete m_anim_timer;
}


/**
 * @brief PastedSourceItem::animateContour
 *
 * This function animates the contours when the items are selected
 */
void PastedSourceItem::animateContour() {
    // Increase by an eighth of a period
    m_anim_dash_offset += DASH_SIZE / 4.0;

    // Increase dash offset with a period of 2*DASH_SIZE
    if (m_anim_dash_offset >= 2.0*DASH_SIZE) {
        m_anim_dash_offset = 0.0;
    }

    update();
}

QRectF PastedSourceItem::boundingRect() const {
    return m_pixmap.rect();
}

QPainterPath PastedSourceItem::shape() const {
    // Take the width of the pen into account
    QPainterPathStroker ps;
    ps.setWidth(SELECTION_WIDTH);

    // Contour path
    QPainterPath p = ps.createStroke(m_selection_path);
    p.addPath(m_selection_path);

    return p;
}

void PastedSourceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Get the scene scale
    qreal scene_scale = painter->deviceTransform().m11() / painter->device()->devicePixelRatioF();

    if (!isComputing()) {
        // Draw the pixmap
        painter->drawPixmap(0, 0, m_pixmap);
    }

    QPen pen;
    pen.setWidthF(SELECTION_WIDTH/scene_scale);

    if (isComputing()) {
        // Translucent-white filling
        pen.setColor(Qt::transparent);
        painter->setPen(pen);
        painter->setBrush(QBrush(m_wait_anim_color));
        painter->drawPolygon(m_selection_path.toFillPolygon());
    }
    else if (isMoving()) {
        // Do nothing
    }
    else if (isSelected()) {
        // Black dashed path
        pen.setColor(QColor(0,0,0,255));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset);
        painter->setPen(pen);
        painter->drawPath(m_selection_path);

        // White dashed path
        pen.setColor(QColor(255,255,255,255));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset + 1.0 * DASH_SIZE);   // + Offset w.r.t. Black dashes
        painter->setPen(pen);
        painter->drawPath(m_selection_path);
    }
    else if (isUnderMouse()) {
        // Translucent-white filling
        pen.setColor(Qt::transparent);
        painter->setPen(pen);
        painter->setBrush(QBrush(QColor(255,255,255,60)));
        painter->drawPolygon(m_selection_path.toFillPolygon());

        // Translucent-black dashed path
        pen.setColor(QColor(0,0,0,125));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset);
        painter->setPen(pen);
        painter->drawPath(m_selection_path);

        // Translucent-white dashed path
        pen.setColor(QColor(255,255,255,125));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset + 1.0 * DASH_SIZE);   // + Offset w.r.t. Black dashes
        painter->setPen(pen);
        painter->drawPath(m_selection_path);
    }
}


/**
 * @brief PastedSourceItem::originalImage
 * @return
 *
 * This function returns the original image as a QImage
 */
QImage PastedSourceItem::originalImage() {
    return m_orig_image;
}

/**
 * @brief PastedSourceItem::originalMatrices
 * @return
 *
 * This function returns the original image in matrices format
 */
ImageMatricesRGB PastedSourceItem::originalMatrices() {
    return m_orig_matrices;
}

/**
 * @brief PastedSourceItem::setOriginalPack
 * @param img_pack
 *
 * This function sets the original image, matrices and masks to the new given one.
 */
void PastedSourceItem::setOriginalPack(SourceImagePack img_pack) {
    m_orig_image = img_pack.image;
    m_orig_matrices = img_pack.matrices;
    m_masks = img_pack.masks;
}

/**
 * @brief PastedSourceItem::blendedImage
 * @return
 *
 * This function returns the blended image as a QImage
 */
QImage PastedSourceItem::blendedImage() {
    return m_blended_image;
}

/**
 * @brief PastedSourceItem::blendedMatrices
 * @return
 *
 * This function returns the blended image in matrices format
 */
ImageMatricesRGB PastedSourceItem::blendedMatrices() {
    return m_blended_matrices;
}

/**
 * @brief PastedSourceItem::setBlendedPack
 * @param img_pack
 *
 * This function sets the blended image and matrices to the given one.
 */
void PastedSourceItem::setBlendedPack(SourceImagePack img_pack) {
    m_blended_image = img_pack.image;
    m_blended_matrices = img_pack.matrices;
}

/**
 * @brief PastedSourceItem::masks
 * @return
 *
 * This function returns the masks used for this pasted source.
 */
SelectMaskMatrices PastedSourceItem::masks() {
    return m_masks;
}

/**
 * @brief PastedSourceItem::getLaplacianMatrix
 * @return
 *
 * This function returns the sparse laplacian matrix
 */
SparseMatrixXd PastedSourceItem::getLaplacianMatrix() {
    return m_laplacian_matrix;
}

/**
 * @brief PastedSourceItem::isMoving
 * @return
 *
 * This function returns true if the item is currently moved by the mouse
 */
bool PastedSourceItem::isMoving() {
    return m_is_moving;
}

/**
 * @brief PastedSourceItem::setSelected
 * @param s
 *
 * This is an overloaded function.
 * It sets the selection states of the item, then updates the item controls.
 */
void PastedSourceItem::setSelected(bool s) {
    // Run the original action
    QGraphicsItem::setSelected(s);

    // Update the item controls
    updateItemControls();
}

/**
 * @brief PastedSourceItem::updateItemControls
 *
 * This function updates the control policy of the item.
 */
void PastedSourceItem::updateItemControls() {
    // If blending computation is running
    if (isComputing()) {
        // This item becomes fixed
        setFlag(QGraphicsItem::ItemIsMovable, false);

        // The cursor is now the loading cursor
        setCursor(Qt::WaitCursor);
    }
    // If it has been selected
    else if (isSelected()) {
        // This item becomes movable
        setFlag(QGraphicsItem::ItemIsMovable, true);

        // The cursor is now the move cursor
        setCursor(Qt::SizeAllCursor);
    }
    // If it has been unselected
    else {
        // This item becomes fixed
        setFlag(QGraphicsItem::ItemIsMovable, false);

        // The cursor is now the hand cursor
        setCursor(Qt::PointingHandCursor);
    }
}

/**
 * @brief PastedSourceItem::isComputing
 * @return
 *
 * This function returns true if this item is currently computing
 */
bool PastedSourceItem::isComputing() {
    return m_is_computing;
}

/**
 * @brief PastedSourceItem::setComputing
 * @param en
 *
 * This function sets the 'computing' state (+ enables waiting animation)
 */
void PastedSourceItem::setComputing(bool en) {
    m_is_computing = en;

    // Start/stop the waiting animation
    if (en) {
        m_prop_anim_wait->start();
    }
    else {
        m_prop_anim_wait->stop();
    }
}

/**
 * @brief PastedSourceItem::waitAnimColor
 * @return
 *
 * This function returns the current color of the wait animation.
 */
QColor PastedSourceItem::waitAnimColor() {
    return m_wait_anim_color;
}

/**
 * @brief PastedSourceItem::setWaitAnimColor
 * @param progress
 *
 * This function updates the wait animation color.
 */
void PastedSourceItem::setWaitAnimColor(QColor color) {
    // Update the animation color
    m_wait_anim_color = color;

    // Update the graphics
    update();
}


void PastedSourceItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // Run the built-in event procedure too
    QGraphicsItem::mousePressEvent(event);
    event->accept();
}

void PastedSourceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // Run the built-in event procedure too
    QGraphicsItem::mouseMoveEvent(event);
    event->accept();

    // Check if we are moving this item
    if (!isMoving() && isSelected() && flags() & QGraphicsItem::ItemIsMovable) {
        m_is_moving = true;

        // Restore the original image on the pixmap
        m_pixmap = QPixmap::fromImage(m_orig_image);
    }
}

void PastedSourceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    // Run the built-in event procedure too
    QGraphicsItem::mouseReleaseEvent(event);
    event->accept();

    if (isMoving()) {
        m_is_moving = false;

        qDebug() << "Moving done";
        //TODO: here, the item has been placed at another position
        //      -> recompute the image
    }

    // Update item controls (enable moving)
    updateItemControls();
}

//FIXME: this function is here to test the 'computing' state
void PastedSourceItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event);

    setComputing(!isComputing());
}

void PastedSourceItem::focusInEvent(QFocusEvent *focusEvent) {
    QGraphicsItem::focusInEvent(focusEvent);
}

void PastedSourceItem::focusOutEvent(QFocusEvent *focusEvent) {
    QGraphicsItem::focusOutEvent(focusEvent);

    // Unselect the item when it looses the focus
    setSelected(false);

    // Update item controls
    updateItemControls();
}

QVariant PastedSourceItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    QVariant new_value = value;

    // Ensure the item is inside the scene rect
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF new_pos = value.toPointF();

        // Check if the item bounding rect is in the scene rect
        QRectF item_rect(new_pos, m_orig_image.size());
        QRectF rect = scene()->sceneRect();

        if (!rect.contains(item_rect)) {
            // Keep the item inside the scene rect.
            new_pos.setX(qMin(rect.right()-m_orig_image.width(), qMax(new_pos.x(), rect.left())));
            new_pos.setY(qMin(rect.bottom()-m_orig_image.height(), qMax(new_pos.y(), rect.top())));
            new_value = QVariant(new_pos);
        }
    }

    return QGraphicsItem::itemChange(change, new_value);
}
