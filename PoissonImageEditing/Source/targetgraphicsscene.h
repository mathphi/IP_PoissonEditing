#ifndef TARGETGRAPHICSSCENE_H
#define TARGETGRAPHICSSCENE_H

#include <QGraphicsScene>

class PastedSourceItem;

class TargetGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    TargetGraphicsScene(QObject *parent = nullptr);
    ~TargetGraphicsScene();

    QList<PastedSourceItem*> getSourceItemList();
    void addSourceItem(PastedSourceItem *src_item, bool place_center = true, bool select = true);

    bool isRectangleInsertable(QRectF rect);

public slots:
    void removeSelectedSrcItem();
    void removeAllSrcItem();

    void recomputeBlendingSelected();
    void recomputeBlendingAll();

    void changeRealTimeBlending(bool en);
    void changeMixedBlending(bool en);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

signals:
    void keyPressed(QKeyEvent*);
    void sourceItemListChanged();

private:
    QList<PastedSourceItem*> m_source_item_list;
};

#endif // TARGETGRAPHICSSCENE_H
