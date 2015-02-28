#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->vpt = new videoProcessingThread(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openPushButton_clicked()
{
    QString selectedVideoString =  QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "D://",
                "Video Files (*.avi && *.mp4 && *.mov && *.mkv)"
                );
    if (selectedVideoString.size()!=0)
    {
        this->vpt->videoName = selectedVideoString.toStdString();
        //show filenames in table widget
        ui->videoTableWidget->insertRow(0);
        QTableWidgetItem *item = new QTableWidgetItem(selectedVideoString);
        ui->videoTableWidget->setItem(0, 0, item);
    }
}

void MainWindow::on_runPushButton_clicked()
{
    this->vpt->start();
}
