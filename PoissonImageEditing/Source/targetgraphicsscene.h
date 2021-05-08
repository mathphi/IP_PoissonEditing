#ifndef TARGETGRAPHICSSCENE_H
#define TARGETGRAPHICSSCENE_H

#include <QGraphicsScene>

class PastedSourceItem;

class TargetGraphicsScene : public QGraphicsScene
{
public:
    TargetGraphicsScene(QObject *parent = nullptr);
    ~TargetGraphicsScene();

    QList<PastedSourceItem*> getSourceItemList();
    void addSourceItem(PastedSourceItem *src_item);

    bool isRectangleInsertable(QRectF rect);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    void removeSelectedSrcItem();
    void removeAllSrcItem();

    QList<PastedSourceItem*> m_source_item_list;
};

#endif // TARGETGRAPHICSSCENE_H
