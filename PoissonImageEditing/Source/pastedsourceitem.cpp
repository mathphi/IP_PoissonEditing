#include "pastedsourceitem.h"
#include "transfercomputationunit.h"
#include "blendingcomputationunit.h"

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
        QImage src_img,
        QPainterPath selection_path,
        QImage target_image,
        bool compute_transfer_data,
        QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    // Blending settings
    m_is_real_time = true;
    m_is_mixed_blending = true;

    // Initialize the transfer job to nullptr
    m_transfer_job = nullptr;

    // Save the link to target image
    m_target_image = target_image;

    // Save the source image
    m_orig_image = src_img;

    // Set the drawn pixmap
    m_pixmap = QPixmap::fromImage(m_orig_image);

    // Save the selection path
    m_selection_path = selection_path;

    ///////////////
    // Save a normalized version of the selection path
    m_normalized_path = selection_path;

    // Place the path origin to (0,0)
    m_normalized_path.translate(-selection_path.boundingRect().topLeft());

    // Add the 1px margin to the path
    QSizeF path_size = selection_path.boundingRect().size();
    m_normalized_path.translate(QPointF((src_img.width()-path_size.width())/2.0, (src_img.height()-path_size.height())/2.0));

    // Add 0.5 to align the selection path on the center of the pixel
    m_normalized_path.translate(0.5,0.5);
    ///////////////

    // Initialize status attributes
    m_is_moving = false;
    m_is_computing = false;
    m_is_invalid = true;

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
    m_prop_anim_wait->setStartValue(QColor(150, 190, 255));
    m_prop_anim_wait->setKeyValueAt(0.25, QColor(Qt::white));
    m_prop_anim_wait->setKeyValueAt(0.50, QColor(Qt::transparent));
    m_prop_anim_wait->setKeyValueAt(0.75, QColor(Qt::white));
    m_prop_anim_wait->setEndValue(QColor(150, 190, 255));
    m_prop_anim_wait->setEasingCurve(QEasingCurve::InOutSine);
    m_prop_anim_wait->setDuration(2000);
    m_prop_anim_wait->setLoopCount(-1);     // Infinite repetitions

    connect(m_anim_timer, SIGNAL(timeout()), this, SLOT(animateContour()));

    // Compute transfer data if needed
    if (compute_transfer_data) {
        // Switch in computing mode
        setComputing(true);

        // Create and configure the transfer computation unit
        m_transfer_job = new TransferComputationUnit(m_orig_image, m_selection_path);

        // Connect the transfer job signal
        connect(m_transfer_job, SIGNAL(computationFinished()), this, SLOT(transferFinished()));

        // Send the transfer job to the computation handler
        ComputationHandler::startComputationJob(m_transfer_job);
    }
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
    return m_orig_image.rect();
}

QPainterPath PastedSourceItem::shape() const {
    // Take the width of the pen into account
    QPainterPathStroker ps;
    ps.setWidth(SELECTION_WIDTH);

    // Contour path
    QPainterPath p = ps.createStroke(m_normalized_path);
    p.addPath(m_normalized_path);

    return p;
}

void PastedSourceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    // Get the scene scale
    qreal scene_scale = painter->deviceTransform().m11() / painter->device()->devicePixelRatioF();

    // Smooth pixmap transformations
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

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
        painter->drawPolygon(m_normalized_path.toFillPolygon());
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
        painter->drawPath(m_normalized_path);

        // White dashed path
        pen.setColor(QColor(255,255,255,255));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset + 1.0 * DASH_SIZE);   // + Offset w.r.t. Black dashes
        painter->setPen(pen);
        painter->drawPath(m_normalized_path);
    }
    else if (isUnderMouse()) {
        // Translucent-white filling
        pen.setColor(Qt::transparent);
        painter->setPen(pen);
        painter->setBrush(QBrush(QColor(255,255,255,60)));
        painter->drawPolygon(m_normalized_path.toFillPolygon());

        // Translucent-black dashed path
        pen.setColor(QColor(0,0,0,125));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset);
        painter->setPen(pen);
        painter->drawPath(m_normalized_path);

        // Translucent-white dashed path
        pen.setColor(QColor(255,255,255,125));
        pen.setDashPattern({1.0 * DASH_SIZE, 1.0 * DASH_SIZE});
        pen.setDashOffset(-m_anim_dash_offset + 1.0 * DASH_SIZE);   // + Offset w.r.t. Black dashes
        painter->setPen(pen);
        painter->drawPath(m_normalized_path);
    }
}


QPainterPath PastedSourceItem::getSelectionPath() {
    return m_selection_path;
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
 * @brief PastedSourceItem::blendedImage
 * @return
 *
 * This function returns the blended image as a QImage
 */
QImage PastedSourceItem::blendedImage() {
    return m_blended_image;
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
 * @brief PastedSourceItem::laplacianMatrix
 * @return
 *
 * This function returns the sparse laplacian matrix
 */
SparseMatrixXd PastedSourceItem::laplacianMatrix() {
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
 * @brief PastedSourceItem::isInvalid
 *
 * This function returns true if the current blending is invalid
 */
bool PastedSourceItem::isInvalid() {
    return m_is_invalid;
}

/**
 * @brief PastedSourceItem::invalidateBlending
 *
 * This function invalidates the blended image.
 * This will switch the shown image to the original masked one.
 */
void PastedSourceItem::invalidateBlending() {
    // Mark this item as invalid
    m_is_invalid = true;

    // Restore the original image on the pixmap
    m_pixmap = QPixmap::fromImage(m_orig_image_masked);
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

        // Mark this item as invalid
        m_is_invalid = true;
    }
    else {
        m_prop_anim_wait->stop();
    }

    updateItemControls();
    update();
}

/**
 * @brief PastedSourceItem::isRealTime
 * @return
 *
 * This funciton returns true if this item computes the blending in real time
 */
bool PastedSourceItem::isRealTime() {
    return m_is_real_time;
}

/**
 * @brief PastedSourceItem::setRealTime
 * @param en
 *
 * This function enables/disables the real time blending
 */
void PastedSourceItem::setRealTime(bool en) {
    m_is_real_time = en;
}

/**
 * @brief PastedSourceItem::isMixedBlending
 * @return
 *
 * This function returns true if mixed blending is enabled
 */
bool PastedSourceItem::isMixedBlending() {
    return m_is_mixed_blending;
}

/**
 * @brief PastedSourceItem::setMixedBlending
 * @param en
 *
 * This function enables/disables the mixed blending
 */
void PastedSourceItem::setMixedBlending(bool en) {
    m_is_mixed_blending = en;
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

/**
 * @brief PastedSourceItem::startBlendingComputation
 *
 * This function starts a new blending job (threaded)
 */
void PastedSourceItem::startBlendingComputation() {
    // Check if a blending job is already running
    if (m_blending_unit_list.size() > 0)
        return;

    // Enable computing state
    setComputing(true);

    // Get the interesting part of the target image
    QRect copy_rect(pos().toPoint(), boundingRect().size().toSize());
    QImage target_image_part = m_target_image.copy(copy_rect);

    // For each color channel
    for (int i = 0 ; i < 3 ; i++) {
        // Create the computation unit
        BlendingComputationUnit *bcu = new BlendingComputationUnit(
                    i,
                    target_image_part,
                    m_orig_matrices[i],
                    m_masks,
                    m_laplacian_matrix,
                    m_is_mixed_blending);

        // Connect the computation unit to the slot
        connect(bcu, SIGNAL(computationFinished()), this, SLOT(blendingFinished()));

        // Lock the blending unit list
        m_blending_mutex.lock();

        // Add this computation unit to the control list
        m_blending_unit_list.append(bcu);

        // Unlock the blending unit list
        m_blending_mutex.unlock();

        //Add the computation unit to the thread pool queue
        ComputationHandler::startComputationJob(bcu);
    }
}


/**
 * @brief PastedSourceItem::transferFinished
 *
 * This slot is called when the Transfer job thread finished computing
 * the transfer parameters
 */
void PastedSourceItem::transferFinished() {
    // Retreive the computation results
    m_orig_matrices     = m_transfer_job->getOriginalMatrices();
    m_masks             = m_transfer_job->getMasks();
    m_orig_image_masked = m_transfer_job->getOriginalImageMasked();
    m_laplacian_matrix  = m_transfer_job->getLaplacian();

    // Update the current pixmap with the original masked image
    m_pixmap = QPixmap::fromImage(m_orig_image_masked);

    // Delete the computation unit
    delete m_transfer_job;
    m_transfer_job = nullptr;

    // Set computing as finished
    setComputing(false);
}


/**
 * @brief PastedSourceItem::blendingFinished
 *
 * This slot is called by the blending computation units when
 * the computation is finished
 */
void PastedSourceItem::blendingFinished() {
    // Retrive the sender of the computationFinished signal
    BlendingComputationUnit *bcu = qobject_cast<BlendingComputationUnit*> (sender());

    // If the sender was not found -> abort
    if (!bcu)
        return;

    // Thead-lock this section
    m_blending_mutex.lock();

    // Retreive the computation unit's channel number
    int channel = bcu->getChannelNumber();

    // Save the blended matrix for this channel
    m_blended_matrices[channel] = bcu->getBlendedChannel();

    // Remove this computation unit from the list
    m_blending_unit_list.removeAll(bcu);

    // Check if the blending if finished for all channels
    if (m_blending_unit_list.size() == 0) {
        // BLENDING FINISHED ! //

        // Convert the blended matrices to a QImage
        m_blended_image = ComputationHandler::matricesToImage(m_blended_matrices, m_masks.positive_mask);

        // Update the graphics
        m_pixmap = QPixmap::fromImage(m_blended_image);

        // Exit the computing state
        setComputing(false);

        // The result is now valid
        m_is_invalid = false;
    }

    // Delete the computation unit
    delete bcu;

    // Unlock this section
    m_blending_mutex.unlock();
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

        // Mark the computed blending as invalid
        invalidateBlending();
    }
}

void PastedSourceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    // Run the built-in event procedure too
    QGraphicsItem::mouseReleaseEvent(event);
    event->accept();

    if (isMoving()) {
        m_is_moving = false;

        // Align the position to the pixel grid
        setPos(pos().toPoint());

        // If real time is enabled -> start blending when item released
        if (m_is_real_time) {
            startBlendingComputation();
        }
    }

    // Update item controls (enable moving)
    updateItemControls();
}

void PastedSourceItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem::mouseDoubleClickEvent(event);

    // Start blending is double clicked
    startBlendingComputation();
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


/*
 * Class serialization function
 */

QDataStream &operator>>(QDataStream &in, PastedSourceItem *&o) {
    QPointF pos;
    QImage src_img, tgt_img;
    QPainterPath sel_path;
    bool is_invalid, is_selected;

    in >> pos;
    in >> src_img;
    in >> tgt_img;
    in >> sel_path;

    // Initialize the object (don't compute transfer data)
    o = new PastedSourceItem(src_img, sel_path, tgt_img, false);
    o->setPos(pos);

    in >> o->m_orig_image_masked;
    in >> o->m_orig_matrices;
    in >> o->m_blended_image;
    in >> o->m_masks;
    in >> o->m_laplacian_matrix;

    in >> o->m_is_real_time;
    in >> o->m_is_mixed_blending;

    in >> is_invalid;
    if (is_invalid) {
        o->invalidateBlending();
    }
    else {
        o->m_is_invalid = false;
        o->m_pixmap = QPixmap::fromImage(o->m_blended_image);
    }

    o->updateItemControls();

    in >> is_selected;
    o->setSelected(is_selected);

    return in;
}

QDataStream &operator<<(QDataStream &out, PastedSourceItem *o) {
    out << o->pos();
    out << o->m_orig_image;
    out << o->m_target_image;
    out << o->m_selection_path;

    out << o->m_orig_image_masked;
    out << o->m_orig_matrices;
    out << o->m_blended_image;
    out << o->m_masks;
    out << o->m_laplacian_matrix;

    out << o->m_is_real_time;
    out << o->m_is_mixed_blending;

    out << o->m_is_invalid;
    out << o->isSelected();

    return out;
}
