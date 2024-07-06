#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QVector>
#include <QVBoxLayout>
#include "qcustomplot.h"

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
    void on_pB_connect_clicked();
    void on_pB_refresh_clicked();
    void readData();
    void updatePlot();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *timer;
    QVBoxLayout *mainLayout;
    QCustomPlot *customPlot;
    QByteArray buffer;

    const double startFrequency = 500e6;  // Начальная частота
    const double stopFrequency = 1600e6;  // Конечная частота

    void sendCommand(const QString &command);
    void decodeAndPlot(const QByteArray &data);
};

#endif // MAINWINDOW_H
