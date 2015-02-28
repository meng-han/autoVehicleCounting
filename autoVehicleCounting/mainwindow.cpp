#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->init();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    this->vpt = new videoProcessingThread(this);
    qRegisterMetaType< Mat >("Mat");
    connect(this->vpt,SIGNAL(showResults(Mat)),this,SLOT(on_showResults(Mat)));
    this->vpt->stop = false;
}


void MainWindow::on_showResults(Mat frame)
{
    imshow("Current Frame", frame);
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
    this->vpt->stop = false;
    this->vpt->start();
}

void MainWindow::on_stopPushButton_clicked()
{
    this->vpt->stop = true;

}
