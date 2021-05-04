#include "sourcegraphicsscene.h"
#include "graphicslassoitem.h"

#include <QPen>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

#include <QDebug>


SourceGraphicsScene::SourceGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
    m_current_lasso = nullptr;
    m_lasso_state = SourceLassoState::None;
}

SourceGraphicsScene::~SourceGraphicsScene() {
    // Remove and delete lasso (if one)
    if (m_current_lasso) {
        removeItem(m_current_lasso);
        delete m_current_lasso;
    }
}

void SourceGraphicsScene::removeLasso() {
    if (!m_current_lasso)
        return;

    // Remove and delete lasso
    removeItem(m_current_lasso);
    delete m_current_lasso;

    // Set the pointer to nullptr to indicate there is not lasso
    m_current_lasso = nullptr;

    // Lasso status -> None
    m_lasso_state = SourceLassoState::None;
}


void SourceGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // If there is no lasso currently, start drawing a new one
    if (m_lasso_state == SourceLassoState::None) {
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
        m_current_lasso->addPathPoint(event->scenePos());
        m_current_lasso->terminateLasso();
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
