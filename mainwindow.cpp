#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setProgressBarRange(int min, int max)
{
    ui->progressBar->setRange(min, max);
}

void MainWindow::setProgressBarValue(int value)
{
    ui->progressBar->setValue(value);
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

    connect(model, &TreeModel::setProgressBarRange, this, &MainWindow::setProgressBarRange);
    connect(model, &TreeModel::setProgressBarValue, this, &MainWindow::setProgressBarValue);
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
    if (model == nullptr) return;

    QString homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    QString filePath = QFileDialog::getSaveFileName(this, "Save to file", homePath, "Text (*.txt)");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    setProgressBarRange(0, model->checklist.size());

    int counter = 1;
    QString totalText = "";
    for (auto [key, value] : model->checklist.asKeyValueRange()) {
        setProgressBarValue(counter);
        if (value == Qt::Checked) {
            QString filePath = model->filePath(key);
            QFileInfo check_file(filePath);
            if (check_file.exists() && check_file.isFile()) {
                totalText += readFileWithParameters(filePath);
            }
        }
        counter++;
    }

    out << totalText << "\n";

    file.close();
}

void MainWindow::on_selectAllButton_clicked()
{
    if (model == nullptr) return;

    model->selectAll(ui->treeView->rootIndex());
}

void MainWindow::on_deselectAllButton_clicked()
{
    if (model == nullptr) return;

    model->deselectAll(ui->treeView->rootIndex());
}

void MainWindow::on_treeView_activated(const QModelIndex &index)
{
    qDebug() << "Activated:" << model->filePath(index);
}


void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    ui->fileContentViewer->clear();

    // QFile file();
    // if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //     QTextStream in(&file);
    //     while (!in.atEnd()) {
    //         ui->fileContentViewer->appendPlainText(in.readLine());
    //     }
    // }

    ui->fileContentViewer->setPlainText(readFileWithParameters(model->filePath(index)));
}

QString MainWindow::readFileWithParameters(QString filePath)
{
    if (model == nullptr) return "";

    QFile file(filePath);
    QString textFromFile = "";
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (ui->fileAnnotationCheck->isChecked()) {
            QString rootDir = model->rootPath();
            QString fileName;
            if (file.fileName().contains(rootDir)) {
                fileName = file.fileName().right(file.fileName().length() - rootDir.length() - 1);
            } else {
                fileName = file.fileName();
            }
            QStringList annotation = ui->fileAnnotationText->text().split("<filepath>");
            if (annotation.size() == 2) {
                textFromFile += annotation[0] + fileName + annotation[1] + "\n";
            }
        }
        if (ui->useThreeBackticksCheck->isChecked()) {
            textFromFile += "```\n";
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();

            if (line.isEmpty() && ui->removeEmptyLinesCheck->isChecked())
                continue;

            if (ui->removeSpacesCheck->isChecked()) line = line.trimmed();

            if (ui->excludeLinesWithCheck->isChecked()) {
                QString excludeText = ui->excludeText->text();
                if (line.left(excludeText.length()) == ui->excludeText->text())
                    continue;
            }

            line += "\n";

            textFromFile += line;
        }

        if (ui->useThreeBackticksCheck->isChecked()) {
            textFromFile += "```";
        }

        file.close();
    }

    return textFromFile;
}

