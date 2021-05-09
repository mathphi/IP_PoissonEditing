#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainterPath>

class QGraphicsScene;
class QGraphicsPixmapItem;

class SourceGraphicsScene;
class TargetGraphicsScene;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSourceImage(QString filename = QString());
    void openTargetImage(QString filename = QString());

    void tempTestAction();


    // Help action slots
    void aboutQtDialog();
    void aboutProgramDialog();

    // Transfer from source to target
    void transferLassoSelection();

    // Source scene related slots
    void updateSourceScene();
    void sourceLassoDrawn(QPainterPath path);
    void sourceLassoRemoved();

    // Target scene related slots
    void updateTargetScene();
    void targetSceneKeyPressed(QKeyEvent *event);
    void targetSceneSelectionChanged();
    void askRemoveAllLayers();

private:
    Ui::MainWindow *ui;

    SourceGraphicsScene *m_scene_source;
    TargetGraphicsScene *m_scene_target;

    QGraphicsPixmapItem *m_pix_item_source;
    QGraphicsPixmapItem *m_pix_item_target;

    QImage m_source_image;
    QImage m_target_image;
};
#endif // MAINWINDOW_H
