#ifndef MEASUREMENT_SETUP_H
#define MEASUREMENT_SETUP_H

#include <QDialog>
#include "metercontroller.h" // Тепер включаємо правильний заголовок

// Передова декларація для класу QTimer (більше не потрібна тут, оскільки він переїжджає)

namespace Ui {
class MeasurementSetup;
}

class MeasurementSetup : public QDialog
{
    Q_OBJECT

public:
    // Конструктор приймає вказівник на головний контролер
    explicit MeasurementSetup(QWidget *parent = nullptr, MeterController *controller = nullptr);
    ~MeasurementSetup();

private slots:
    // Слоти для кнопок
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_continue_clicked();
    void on_pushButton_exit_clicked();

    // Слот для оновлення дисплеїв
    void onUpdateDisplayA(double value);
    void onUpdateDisplayB(double value);
    void onSweepFinished();

    void on_pushButtonImpedance_clicked();
    void on_pushButtonVAX_clicked();
    void on_pushButtonCV_clicked();

private:
    Ui::MeasurementSetup *ui;
    MeterController *meterController;
    stConfigureField currentConfigure;
    qint8 configureIndex = -1;
};

#endif // MEASUREMENT_SETUP_H
