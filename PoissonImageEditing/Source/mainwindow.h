#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGraphicsScene;
class QGraphicsPixmapItem;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void openSourceImage();
    void openTargetImage();

    void updateSourceScene();
    void updateTargetScene();

private:
    Ui::MainWindow *ui;

    QGraphicsScene *m_scene_source;
    QGraphicsScene *m_scene_target;

    QGraphicsPixmapItem *m_pix_item_source;
    QGraphicsPixmapItem *m_pix_item_target;

    QImage m_source_image;
    QImage m_target_image;
};
#endif // MAINWINDOW_H
