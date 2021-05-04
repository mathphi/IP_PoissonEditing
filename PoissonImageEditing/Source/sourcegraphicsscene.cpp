#include "sourcegraphicsscene.h"
#include "graphicslassoitem.h"

#include <QPen>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>


SourceGraphicsScene::SourceGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
    m_current_lasso = nullptr;
    m_lasso_state = SourceLassoState::Disabled;
}

SourceGraphicsScene::~SourceGraphicsScene() {
    // Remove and delete lasso (if one)
    if (m_current_lasso) {
        removeItem(m_current_lasso);
        delete m_current_lasso;
    }
}

/**
 * @brief SourceGraphicsScene::getSelectionPath
 * @return
 *
 * This function returns a copy of the current selection path.
 * If there is no valid selection path, it returns an empty path.
 */
QPainterPath SourceGraphicsScene::getSelectionPath() const {
    if (!m_current_lasso->isLassoValid())
        return QPainterPath();

    return m_current_lasso->path();
}

/**
 * @brief SourceGraphicsScene::isSelectionValid
 * @return
 *
 * This function returns true if the lasso has been drawn and is valid
 */
bool SourceGraphicsScene::isSelectionValid() const {
    if (!m_current_lasso)
        return false;

    return m_current_lasso->isLassoValid();
}

/**
 * @brief SourceGraphicsScene::enableLasso
 * @param en
 *
 * This slot enables or disable the lasso (any existing lasso is removed if disable)
 */
void SourceGraphicsScene::enableLasso(bool en) {
    // If it was disabled and now enabled -> Lasso state to NoLasso
    if (en && m_lasso_state == SourceLassoState::Disabled) {
        m_lasso_state = SourceLassoState::NoLasso;
    }
    // If it was enabled and now disabled -> Remove lasso and state to Disabled
    else if (!en && m_lasso_state != SourceLassoState::Disabled) {
        // Remove the current lasso (if one)
        removeLasso();

        // Note: set lasso state after calling removeLasso() since it changes the state too
        m_lasso_state = SourceLassoState::Disabled;
    }
}

/**
 * @brief SourceGraphicsScene::removeLasso
 *
 * This slot removes the lasso (if one)
 */
void SourceGraphicsScene::removeLasso() {
    if (!m_current_lasso)
        return;

    // Remove and delete lasso
    removeItem(m_current_lasso);
    delete m_current_lasso;

    // Set the pointer to nullptr to indicate there is not lasso
    m_current_lasso = nullptr;

    // Don't change the lasso state if currently disabled
    if (m_lasso_state != SourceLassoState::Disabled) {
        // Lasso status -> None
        m_lasso_state = SourceLassoState::NoLasso;
    }
}


void SourceGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // If there is no lasso currently, start drawing a new one
    if (m_lasso_state == SourceLassoState::NoLasso) {
        // If there is already a lasso -> remove and delete (should not happen)
        removeLasso();

        // Create a new lasso item
        m_current_lasso = new GraphicsLassoItem(event->scenePos());
        addItem(m_current_lasso);

        // Indicate that we are drawing the lasso
        m_lasso_state = SourceLassoState::Drawing;
    }
    // If the scene is clicked and a lasso was previously drawn -> remove the lasso and delete it
    else if (m_lasso_state == SourceLassoState::Drawn) {
        removeLasso();
    }
}

void SourceGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    // If we are currently drawing a lasso, terminate drawing
    if (m_lasso_state == SourceLassoState::Drawing) {
        // Add a last point to the lasso
        m_current_lasso->addPathPoint(event->scenePos());

        // Get the selection bounding rect
        // We add a margin of 1px to ensure that all selected pixels have 4 neighbors
        QRectF bounding_rect = sceneRect().adjusted(1,1,-1,-1);

        // Terminate the lasso (simplify its geometry)
        m_current_lasso->terminateLasso(bounding_rect);

        // Mark lasso as drawn
        m_lasso_state = SourceLassoState::Drawn;
    }
}

void SourceGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    // If we are currently drawing a lasso, continue drawing
    if (m_lasso_state == SourceLassoState::Drawing) {
        // Add the current position to the lasso
        m_current_lasso->addPathPoint(event->scenePos());
    }
}

void SourceGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event);
}


void SourceGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *event) {
    Q_UNUSED(event);
}


void SourceGraphicsScene::keyPressEvent(QKeyEvent *event) {
    // If escape has been pressed, remove lasso (if one)
    if (event->key() == Qt::Key_Escape) {
        if (m_lasso_state == SourceLassoState::Drawing) {
            removeLasso();
        }
        else if (m_lasso_state == SourceLassoState::Drawn) {
            removeLasso();
        }
    }
}
