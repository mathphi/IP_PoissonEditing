#include "targetgraphicsscene.h"
#include "pastedsourceitem.h"

#include <QKeyEvent>
#include <QMessageBox>

TargetGraphicsScene::TargetGraphicsScene(QObject *parent) : QGraphicsScene(parent)
{

}

TargetGraphicsScene::~TargetGraphicsScene() {
    // Remove and delete all pasted items
    foreach (PastedSourceItem *item, m_source_item_list) {
        removeItem(item);
        delete item;
    }

    m_source_item_list.clear();
}

/**
 * @brief TargetGraphicsScene::getSourceItemList
 * @return
 *
 * This function returns the pasted source items list
 */
QList<PastedSourceItem*> TargetGraphicsScene::getSourceItemList() {
    return m_source_item_list;
}

/**
 * @brief TargetGraphicsScene::addSourceItem
 * @param src_item
 *
 * This function adds a source item on the center of the target scene and
 * marks it as selected and gives it the focus.
 */
void TargetGraphicsScene::addSourceItem(PastedSourceItem *src_item) {
    // Check if the size of the pasted source item is larger than the scene rect
    if (!sceneRect().contains(src_item->boundingRect())) {
        return;
    }

    // Add the item to the list
    m_source_item_list.append(src_item);

    // Add the item to the scene
    addItem(src_item);

    // Place the item on the center of the target
    src_item->setPos(QPointF(sceneRect().width()/2 - src_item->boundingRect().width()/2,
                             sceneRect().height()/2 - src_item->boundingRect().height()/2));

    // Set the new item as selected (only this one)
    clearSelection();
    src_item->setSelected(true);
    src_item->setFocus();
}

/**
 * @brief TargetGraphicsScene::isRectangleInsertable
 * @param rect
 * @return
 *
 * This function returns true if the given rectangle can be inserted
 * into the scene rect (= if it is not oversized).
 */
bool TargetGraphicsScene::isRectangleInsertable(QRectF rect) {
    // Create a comparison rectangle with TopLeft corner on (0,0)
    QRectF scene_rect_cmp = sceneRect();
    scene_rect_cmp.moveTo(0,0);

    // Place also its TopLeft corner on (0,0)
    rect.moveTo(0,0);

    // Compare
    return scene_rect_cmp.contains(rect);
}

/**
 * @brief TargetGraphicsScene::removeSelectedSrcItem
 *
 * This function removes the currently selected source item (if one)
 */
void TargetGraphicsScene::removeSelectedSrcItem() {
    QList<QGraphicsItem*> selected_list = selectedItems();

    // Check if one of the selected items is a PastedSourceItem
    foreach (QGraphicsItem *item, selected_list) {
        // Try to cast the item as a PastedSourceItem
        PastedSourceItem *psi = dynamic_cast<PastedSourceItem*>(item);

        // If the cast was successful -> this is a PastedSourceItem
        if (psi) {
            // Remove this item from the scene and the list then delete it
            m_source_item_list.removeAll(psi);
            removeItem(psi);
            delete psi;
        }
    }
}

/**
 * @brief TargetGraphicsScene::removeAllSrcItem
 *
 * This function removes all the pasted source item
 */
void TargetGraphicsScene::removeAllSrcItem() {
    // Remove all items that are in the list
    foreach (PastedSourceItem *item, m_source_item_list) {
        // Remove this item from the scene and the list then delete it
        m_source_item_list.removeAll(item);
        removeItem(item);
        delete item;
    }
}

void TargetGraphicsScene::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Escape: {
        // Escape -> deselect all items
        clearSelection();
        clearFocus();
        break;
    }
    case Qt::Key_Delete: {
        if (event->modifiers() & Qt::ControlModifier) {
            // Delete with control key
            // -> remove all the pasted items
            int ans = QMessageBox::warning(
                        NULL,
                        "Delete confirmation",
                        "This will delete the all the pasted items.\nAre you sure you want to continue ?",
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::No);

            // Remove all if the answer is yes
            if (ans == QMessageBox::Yes) {
                removeAllSrcItem();
            }
        }
        else {
            // Delete without control key
            // -> remove the selected item (if one)
            removeSelectedSrcItem();
        }
        break;
    }
    case Qt::Key_F5: {
        //TODO: recompute selected/all pasted sources (?)
        break;
    }
    default:
        break;
    }
}