#include "metercontroller.h"
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QStringList>

MeterController::MeterController(QSerialPort *serialPort, QTextEdit *debugger, QObject *parent)
    : QObject(parent),
    m_serialPort(serialPort),
    m_debugger(debugger),
    m_sweepTimer(new QTimer(this)),
    m_logFile(nullptr),
    m_logStream(nullptr),
    m_isMeasuring(false),
    m_scenarioIndex(-1),
    m_startFrequency(10),
    m_stopFrequency(100000),
    m_currentFrequency(10),
    m_stepFrequency(1),
    m_startBias(0),
    m_stopBias(1500),
    m_currentBias(0),
    m_stepBias(1),
    m_startVoltage(10),
    m_stopVoltage(2000),
    m_currentVoltage(10),
    m_stepVoltage(1),
    m_R(0.0), m_C(0.0), m_L(0.0), m_Z(0.0), m_DCR(0.0), m_ECAP(0.0),
    m_X(0.0), m_D(0.0), m_Q(0.0), m_THR(0.0), m_ESR(0.0)
{
    // Підключення сигналу готовності до читання до слота читання даних
    connect(m_serialPort, &QSerialPort::readyRead, this, &MeterController::readData);

    // Підключення таймера розгортки до слота обробки вимірювання
    connect(m_sweepTimer, &QTimer::timeout, this, &MeterController::handleTimeout);
}

MeterController::~MeterController()
{
    if (m_logFile && m_logFile->isOpen())
    {
        m_logFile->close();
    }
    delete m_logFile;
    delete m_logStream;
}

void MeterController::sendCommandToMeter(const QString &command)
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->write(command.toUtf8() + "\n");
        emit logMessage(tr("SENT: ") + command);
    }
    else
    {
        emit logMessage(tr("ERROR: Serial port is not opened. Cannot send command."));
    }
}

void MeterController::initializeMeter(const stConfigureField &config)
{
    // Відправляємо команди для налаштування приладу на початок вимірювання
    sendCommandToMeter("SYSTem:BEEPer:STATe 0");
    sendCommandToMeter("FUNCtion:IMPedance:A " + config.impedanceA);
    sendCommandToMeter("FUNCtion:IMPedance:B " + config.impedanceB);
    sendCommandToMeter("EQUivalent " + config.equivalent);
    sendCommandToMeter("APERture " + config.aperture);
    m_delay = config.delay;
}

void MeterController::startMeasurement(const stConfigureField &config)
{
    if (m_isMeasuring)
    {
        emit logMessage(tr("Measurement is already in progress."));
        return;
    }

    initializeMeter(config); // Ініціалізуємо прилад

    m_isMeasuring = true;

    // Ініціалізація внутрішніх змінних класу для початку вимірювання
    m_startVoltage = ( (config.m_startVoltage < 10) || (config.m_startVoltage > 2000) ) ? 10 : config.m_startVoltage;
    m_stopVoltage  = ( (config.m_stopVoltage < 10) || (config.m_stopVoltage > 2000) ) ? 10 : config.m_stopVoltage;
    m_stepVoltage  = (config.m_stepVoltage > 2000) ? 2000 : config.m_stepVoltage;
    m_currentVoltage = m_startVoltage;

    m_startFrequency = ( (config.m_startFrequency < 10) || (config.m_startFrequency > 100000) ) ? 10 : config.m_startFrequency;
    m_stopFrequency  = ( (config.m_stopFrequency < 10) || (config.m_stopFrequency > 100000) ) ? 10 : config.m_stopFrequency;
    m_stepFrequency  = (config.m_stepFrequency > 100000) ? 100000 : config.m_stepFrequency;
    m_currentFrequency = m_startFrequency;

    m_startBias = (config.m_startBias > 1500) ? 1500 : config.m_startBias;
    m_stopBias = (config.m_stopBias > 1500) ? 1500 : config.m_stopBias;
    m_stepBias = (config.m_stepBias > 1500) ? 1500 : config.m_stepBias;
    m_currentBias = m_startBias;

    // Ініціалізація для початку вимірювання
    sendCommandToMeter("VOLTage:LEVel " + QString::number(m_currentVoltage));
    sendCommandToMeter("BIAS:VOLTage " + QString::number(m_currentBias));
    sendCommandToMeter("FREQuency[:CW] " + QString::number(m_currentFrequency));

    emit logMessage(tr("Measurement scenario set to %1").arg(m_scenarioIndex));

    // Створення файлу логу
    QString filename = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + "_measurement_scenario_" + QString::number(m_scenarioIndex) + ".csv";
    m_logFile = new QFile(filename, this);

    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        emit logMessage(tr("ERROR: Could not open log file."));
        delete m_logFile;
        m_logFile = nullptr;
    }
    else
    {
        m_logStream = new QTextStream(m_logFile);

        // Запис заголовка у файл
        QString header = "Сценарій;Тип;Значення A;Значення B;Параметр розгортки\n";
        *m_logStream << header;
    }

    emit logMessage(tr("Starting measurement sweep for scenario %1").arg(m_scenarioIndex));

    // Запускаємо таймер розгортки з затримкою, що прийшла з форми
    m_sweepTimer->start(config.delay);
}

void MeterController::stopMeasurement()
{
    if (!m_isMeasuring)
    {
        emit logMessage(tr("No measurement is currently active."));
        return;
    }

    m_isMeasuring = false;
    m_sweepTimer->stop();

    if (m_logFile && m_logFile->isOpen())
    {
        m_logFile->close();
        emit logMessage(tr("Measurement stopped. Log file saved."));
    }
    else
    {
        emit logMessage(tr("Measurement stopped."));
    }
}

void MeterController::continueMeasurement()
{
    if (m_isMeasuring)
    {
        emit logMessage(tr("Measurement is already in progress."));
        return;
    }

    // Логіка продовження вимірювання
    if (
            (m_scenarioIndex == 1 && m_currentFrequency > m_stopFrequency) ||
            (m_scenarioIndex == 2 && m_currentVoltage > m_stopVoltage) ||
            (m_scenarioIndex == 3 && m_currentBias > m_stopBias)
        )
    {
        emit logMessage(tr("Measurement sweep already completed. Start a new one."));
        return;
    }

    m_isMeasuring = true;

    m_sweepTimer->start(m_delay);

    if (m_scenarioIndex == 1)
    {
        // Impedance
        emit logMessage(tr("Continuing measurement from %1 Hz").arg(m_currentFrequency));
    }

    if (m_scenarioIndex == 2)
    {
        // VAX
        emit logMessage(tr("Continuing measurement from %1 mV").arg(m_currentVoltage));
    }

    if (m_scenarioIndex == 3)
    {
        // CVX
        emit logMessage(tr("Continuing measurement from %1 mV").arg(m_currentBias));
    }
}

void MeterController::readData()
{
    // data is comming from COM Port

    QByteArray data = m_serialPort->readAll();
    QString response = QString(data).trimmed();

    emit logMessage(tr("RECEIVED: ") + response);

    if (m_isMeasuring)
    {
        QStringList values = response.split(',');
        if (values.size() == 2)
        {
            bool ok1, ok2;
            double value1 = values.at(0).toDouble(&ok1);
            double value2 = values.at(1).toDouble(&ok2);

            if (ok1 && ok2)
            {
                emit updateDisplayA(value1);
                emit updateDisplayB(value2);
                emit updateDisplayC(m_currentFrequency);

                logMeasurementData(value1, value2);
            }
            else
            {
                emit logMessage(tr("ERROR: Failed to parse values from response."));
            }
        }
    }
}

void MeterController::logMeasurementData(double valueA, double valueB)
{
    if (m_logStream)
    {
        QString scenarioType;
        uint32_t sweepValue = 0;

        switch(m_scenarioIndex)
        {
        case 1:
            scenarioType = "Frequency Sweep";
            sweepValue = m_currentFrequency;
            break;
        case 2:
            scenarioType = "Voltage Sweep";
            sweepValue = m_currentVoltage;
            break;
        case 3:
            scenarioType = "Bias Sweep";
            sweepValue = m_currentBias;
            break;
        }

        // Форматування рядка для CSV
        QString line = QString("%1;%2;%3;%4;%5\n").arg(m_scenarioIndex).arg(scenarioType).arg(valueA).arg(valueB).arg(sweepValue);
        *m_logStream << line;
    }
}

void MeterController::handleTimeout()
{
    // Перевірка умов зупинки для кожного сценарію
    if (m_scenarioIndex == 1 && m_currentFrequency > m_stopFrequency)
    {
        stopMeasurement();
        emit sweepFinished();
        return;
    }
    else if (m_scenarioIndex == 2 &&  m_currentVoltage > m_stopVoltage)
    {
        stopMeasurement();
        emit sweepFinished();
        return;
    }
    else if (m_scenarioIndex == 3 && m_currentBias > m_stopBias)
    {
        stopMeasurement();
        emit sweepFinished();
        return;
    }

    // Формування команди залежно від сценарію
    QString stepCommand;
    switch (m_scenarioIndex)
    {
        case 1:
            stepCommand = QString("FREQuency[:CW] %1").arg(m_currentFrequency);
            m_currentFrequency += m_stepFrequency;
            break;
        case 2:
            stepCommand = QString("VOLTage[:LEVel] %1").arg(m_currentVoltage);
            m_currentVoltage += m_stepVoltage;
            break;
        case 3:
            stepCommand = QString("BIAS:VOLTage %1").arg(m_currentBias);
            m_currentBias += m_stepBias;
            break;
        default:
            emit logMessage(tr("ERROR: Invalid scenario index in handleTimeout."));
            return;
    }

    if (!stepCommand.isEmpty())
    {
        sendCommandToMeter(stepCommand);
        sendCommandToMeter("FETCh?");
    }
    else
    {
        emit logMessage(tr("ERROR: Command is not formed"));
    }
}
