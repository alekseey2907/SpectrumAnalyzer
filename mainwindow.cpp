#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), serial(new QSerialPort(this)), timer(new QTimer(this))
    , ui(new Ui::MainWindow), mainLayout(new QVBoxLayout), customPlot(new QCustomPlot)
{
    ui->setupUi(this);
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBox_ports->addItem(info.portName());
    }

    connect(timer, &QTimer::timeout, this, &MainWindow::updatePlot);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);

    mainLayout->addWidget(ui->comboBox_ports);
    mainLayout->addWidget(ui->pB_connect);
    mainLayout->addWidget(ui->pB_refresh);
    mainLayout->addWidget(customPlot);

    setCentralWidget(centralWidget);

    // Настройка графика
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Частота (Гц)");
    customPlot->yAxis->setLabel("Амплитуда (дБм)");
    customPlot->yAxis->setRange(-130, 60);
    customPlot->xAxis->setRange(startFrequency, stopFrequency);

    // Первоначальное обновление списка портов
    on_pB_refresh_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pB_connect_clicked()
{
    if (serial->isOpen())
        serial->close();

    QString selectedPortName = ui->comboBox_ports->currentText();
    if (selectedPortName.isEmpty()) {
        qDebug() << "Порт не выбран";
        return;
    }

    qDebug() << "Попытка подключения к порту:" << selectedPortName;

    serial->setPortName(selectedPortName);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "COM Port успешно открыт: " << selectedPortName;
        sendCommand("scn20 500000000 1500000000 5000000 200 20 10700000 8500 8\r\n");  // Настройка диапазона частот 500-1500 МГц
        timer->start(500);
    } else {
        qDebug() << "Не удалось открыть последовательный порт";
    }
}

void MainWindow::on_pB_refresh_clicked()
{
    ui->comboBox_ports->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->comboBox_ports->addItem(info.portName());
    }
    qDebug() << "Список портов обновлен";
}

void MainWindow::sendCommand(const QString &command)
{
    if (serial->isOpen() && serial->isWritable()) {
        serial->write(command.toUtf8());
    } else {
        qDebug() << "COM Port недоступен";
    }
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    buffer.append(data);

    decodeAndPlot(buffer);
    buffer.clear();


}

void MainWindow::decodeAndPlot(const QByteArray &data)
{
    QVector<double> amplitudes;
    for (int i = 0; i < data.size() - 2; i += 2) {
        uint16_t val = (static_cast<uint8_t>(data[i]) << 8) | static_cast<uint8_t>(data[i + 1]);
        uint16_t amplitudeData = val & 0b0000011111111111;
        double amplitude = (800.0 - amplitudeData) / 10.0;
        amplitudes.append(amplitude);
    }

    QVector<double> xValues;


    double stepFrequency = 2400000000.0 / data.size();  // Шаг частоты
    for (int i = 0; i < amplitudes.size(); ++i) {
        xValues.append(startFrequency + i * stepFrequency);
    }

    customPlot->graph(0)->setData(xValues, amplitudes);
    customPlot->replot();
}

void MainWindow::updatePlot()
{
    if (serial->isOpen() && serial->isWritable()) {
        sendCommand("scn20 500000000 1500000000 5000000 200 20 10700000 8500 8\r\n");  // Переотправка команды для обновления данных
    } else {
        qDebug() << "Последовательный порт закрыт или не доступен для записи";
    }
}
