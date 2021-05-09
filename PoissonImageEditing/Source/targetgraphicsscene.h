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
    void addSourceItem(PastedSourceItem *src_item);

    bool isRectangleInsertable(QRectF rect);

public slots:
    void removeSelectedSrcItem();
    void removeAllSrcItem();

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

signals:
    void keyPressed(QKeyEvent*);

private:
    QList<PastedSourceItem*> m_source_item_list;
};

#endif // TARGETGRAPHICSSCENE_H
