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
    connect(this->vpt,SIGNAL(showAerial(Mat)),this,SLOT(on_showAerial(Mat)));
    connect(this->vpt,SIGNAL(maneuverCount(int)),this,SLOT(on_maneuverCount(int)));
    this->vpt->stop = false;

    // create graph and assign data to it:
    ui->turnsWidget->addGraph(0);
    //x.resize(0);
    //y.resize(0);
    //ui->turnsWidget->graph(0)->setData(this->x, this->y);
    // give the axes some labels:
    //ui->turnsWidget->xAxis->setLabel("Elapsed Time (s)");
    //ui->turnsWidget->yAxis->setLabel("Flow Rate (GPM)");
    // set axes ranges, so we see all data:
    //ui->turnsWidget->xAxis->setRange(0, this->xRng);
    //ui->turnsWidget->yAxis->setRange(0, this->yRng);
    //ui->turnsWidget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    //ui->turnsWidget->replot();

    this->turns = new QCPBars(ui->turnsWidget->xAxis, ui->turnsWidget->yAxis);
    ui->turnsWidget->addPlottable(turns);

    // set names and colors:
    QPen pen;
    pen.setWidthF(1.2);
    turns->setName("Maneuvers Count");
    pen.setColor(QColor(255, 131, 0));
    turns->setPen(pen);
    turns->setBrush(QColor(255, 131, 0, 50));

    // prepare x axis with country labels:
    QVector<QString> labels;
    this->ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 << 12;
    this->counts << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    labels << "West - Left" << "West - Straight" << "West - Right" << "South - Left" << "South - Straight"
           << "South - Right" << "East - Left" << "East - Straight" << "East - Right" << "North - Left" <<
              "North - Straight" << "North - Right";
    ui->turnsWidget->xAxis->setAutoTicks(false);
    ui->turnsWidget->xAxis->setAutoTickLabels(false);
    ui->turnsWidget->xAxis->setTickVector(ticks);
    ui->turnsWidget->xAxis->setTickVectorLabels(labels);
    ui->turnsWidget->xAxis->setTickLabelRotation(60);
    ui->turnsWidget->xAxis->setSubTickCount(0);
    ui->turnsWidget->xAxis->setTickLength(0, 4);
    ui->turnsWidget->xAxis->grid()->setVisible(true);
    ui->turnsWidget->xAxis->setRange(0, 13);

    // prepare y axis:
    ui->turnsWidget->yAxis->setRange(0, 40);
    ui->turnsWidget->yAxis->setPadding(5); // a bit more space to the left border
    ui->turnsWidget->yAxis->setLabel("Maneuvers Count");
    ui->turnsWidget->yAxis->grid()->setSubGridVisible(true);
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 25));
    ui->turnsWidget->yAxis->grid()->setPen(gridPen);
    gridPen.setStyle(Qt::DotLine);
    ui->turnsWidget->yAxis->grid()->setSubGridPen(gridPen);
    this->turns->setData(this->ticks,this->counts);
    //turns->setData(this->ticks,this->counts);
    ui->turnsWidget->replot();






}

void MainWindow::on_showAerial(Mat aerial)
{

    namedWindow("Aerial Image",CV_NORMAL);
    //moveWindow("Aerial Image", 300, 700);
    imshow("Aerial Image", aerial);
}

void MainWindow::on_showResults(Mat frame)
{
    namedWindow("Current Frame",CV_NORMAL);
    //moveWindow("Current Frame", 1000, 700);
    imshow("Current Frame", frame);
}

void MainWindow::on_maneuverCount(int maneuver)
{
    this->counts[maneuver-1]++;// = this->counts.at(maneuver-1)  + 1;
    qDebug() << this->counts;
    this->turns->setData(this->ticks,this->counts);
    ui->turnsWidget->replot();

}

void MainWindow::on_openPushButton_clicked()
{
    QString selectedVideoString =  QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "C:\\Users\\Meng\\Documents\\Qt Projects\\AutoCarCounting",
                "Video Files (*.avi && *.mp4 && *.mov && *.mkv && *.wmv)"
                );
    if (selectedVideoString.size()!=0)
    {
        if(ui->videoTableWidget->rowCount()==1)ui->videoTableWidget->removeRow(0);
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

void MainWindow::on_pushButton_clicked()
{
    if(ui->videoTableWidget->rowCount()==1)
        ui->videoTableWidget->removeRow(0);
}

void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionAbout_triggered()
{
    aboutDialog *ad = new aboutDialog();
    ad->show();
}
