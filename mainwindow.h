#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>

#include "treemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setProgressBarRange(int min, int max);
    void setProgressBarValue(int value);
    void on_actionQuit_triggered();

    void on_actionOpen_folder_triggered();

    void on_convertButton_clicked();

    void on_selectAllButton_clicked();

    void on_deselectAllButton_clicked();

private:
    Ui::MainWindow *ui;

    TreeModel *model = nullptr;
};
#endif // MAINWINDOW_H
