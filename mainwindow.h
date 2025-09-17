#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QMap>
#include "metercontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setupUi();
    void setupConnections();
    void fillPortsInfo();
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    void onUpdateDisplayC(double value);
    void onSweepFinished();
    void on_pushButton_setup_measurement_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serialPort;
    MeterController *m_meterController;
    QMap<QString, QString> m_scpiCommands;
};

#endif // MAINWINDOW_H
