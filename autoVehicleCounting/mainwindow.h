#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "videoprocessingthread.h"
#include "qcustomplot.h"
#include "aboutdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openPushButton_clicked();
    void on_showResults(Mat);
    void on_showAerial(Mat);
    void on_runPushButton_clicked();
    void on_stopPushButton_clicked();

    void on_pushButton_clicked();
    void on_maneuverCount(int);
    void on_actionExit_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    videoProcessingThread *vpt;
    void init();
    QVector<double> ticks;
    QVector<double> counts;
    QCPBars *turns;

};

#endif // MAINWINDOW_H
