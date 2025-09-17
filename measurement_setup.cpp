#include "measurement_setup.h"
#include "ui_measurement_setup.h"
#include "metercontroller.h"
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QLabel>

MeasurementSetup::MeasurementSetup(QWidget *parent, MeterController *controller) :
    QDialog(parent),
    ui(new Ui::MeasurementSetup),
    meterController(controller)
{
    ui->setupUi(this);

    connect(ui->pushButton_start,       &QPushButton::clicked, this, &MeasurementSetup::on_pushButton_start_clicked);
    connect(ui->pushButton_stop,        &QPushButton::clicked, this, &MeasurementSetup::on_pushButton_stop_clicked);
    connect(ui->pushButton_continue,    &QPushButton::clicked, this, &MeasurementSetup::on_pushButton_continue_clicked);
    connect(ui->pushButton_exit,        &QPushButton::clicked, this, &MeasurementSetup::on_pushButton_exit_clicked);

    if (meterController)
    {
        connect(meterController, &MeterController::updateDisplayA,  this, &MeasurementSetup::onUpdateDisplayA);
        connect(meterController, &MeterController::updateDisplayB,  this, &MeasurementSetup::onUpdateDisplayB);
        connect(meterController, &MeterController::sweepFinished,   this, &MeasurementSetup::onSweepFinished);
    }
}

MeasurementSetup::~MeasurementSetup()
{
    delete ui;
}

void MeasurementSetup::on_pushButton_start_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;

    if (meterController)
    {
        if(this->configureIndex)
        {
            this->currentConfigure.impedanceA = ui->comboBoxImpedanceA->currentText();  // first parameter
            this->currentConfigure.impedanceB = ui->comboBoxImpedanceB->currentText();  // second parameter
            this->currentConfigure.equivalent = ui->comboBoxEQUivalent->currentText();  // model
            this->currentConfigure.aperture   = ui->comboBoxAPERture->currentText();    // fast, med, slow

            switch(this->configureIndex)
            {
                case 1:
                    this->currentConfigure.scenario = 1;

                    this->currentConfigure.m_startBias      = ui->StartSpinBoxFixedBImpedance->value();
                    this->currentConfigure.m_startVoltage   = ui->SpinBoxFixedVImpedance->value();
                    this->currentConfigure.m_startFrequency = ui->SpinBoxStartFImpedance->value();
                    this->currentConfigure.m_stopFrequency  = ui->SpinBoxStopFImpedance->value();
                    this->currentConfigure.m_stepFrequency  = ui->SpinBoxStepFImpedance->value();

                    this->configureIndex = -1;
                break;

                case 2:
                    // Scenario 2
                    // toDo
                    this->configureIndex = -1;

                break;

                case 3:
                    // Scenario 3
                    // toDo
                    this->configureIndex = -1;

                break;

                default:
                break;

                meterController->startMeasurement(currentConfigure);
            }
        }
        else
        {
            QMessageBox::information(this, "Configure Scenario is failed.", "Configure first");
        }
    }
    else
    {
        QMessageBox::information(this, "MeterController module is not set.", "Check it");
    }
}

void MeasurementSetup::on_pushButton_stop_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    if (meterController)
    {
        meterController->stopMeasurement();
    }
}

void MeasurementSetup::on_pushButton_continue_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    if (meterController)
    {
        meterController->continueMeasurement();
    }
}

void MeasurementSetup::on_pushButton_exit_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    this->configureIndex = -1;
    close();
}

void MeasurementSetup::onUpdateDisplayA(double value)
{
    ui->display_A->setText(QString("LCD A: %1").arg(value));
}

void MeasurementSetup::onUpdateDisplayB(double value)
{
    ui->display_B->setText(QString("LCD B: %1").arg(value));
}

void MeasurementSetup::onSweepFinished()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    QMessageBox::information(this, "Вимірювання завершено", "Розгортка вимірювання успішно завершена.");
}

void MeasurementSetup::on_pushButtonImpedance_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    this->configureIndex = 1;
}

void MeasurementSetup::on_pushButtonVAX_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    this->configureIndex = 2;
}

void MeasurementSetup::on_pushButtonCV_clicked()
{
    qDebug() << "Function:" << Q_FUNC_INFO;
    this->configureIndex = 3;
}
