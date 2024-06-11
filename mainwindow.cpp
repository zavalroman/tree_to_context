#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QStandardPaths>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_folder_triggered()
{
    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QString path = QFileDialog::getExistingDirectory(this, "Choose project directory", homePath,
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    qDebug() << "Path:" << path;

    if (path.isEmpty()) return;

    if (model != nullptr) delete model;

    model = new TreeModel;
    model->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    model->setResolveSymlinks(true);
    model->setRootPath(path);

    QModelIndex rootIndex = model->index(path);

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(rootIndex);

    for (int i = 1; i < model->columnCount(); ++i)
        ui->treeView->hideColumn(i);
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_convertButton_clicked()
{
    qDebug() << "Convert clicked";
    for (auto [key, value] : model->checklist.asKeyValueRange()) {
        //qDebug() << key << ":" << value;
        if (value == Qt::Checked) {
            qDebug() << model->filePath(key);
        }
    }
}

