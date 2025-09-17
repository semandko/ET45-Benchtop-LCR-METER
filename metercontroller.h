#ifndef METERCONTROLLER_H
#define METERCONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>

// Константи SCPI-команд для імпедансу (частина A)
namespace ScpiImpedanceA
{
const QString AUTO = "AUTO";
const QString R = "R";
const QString C = "C";
const QString L = "L";
const QString Z = "Z";
const QString DCR = "DCR";
const QString ECAP = "ECAP";
}

// Константи SCPI-команд для імпедансу (частина B)
namespace ScpiImpedanceB
{
const QString X = "X";
const QString D = "D";
const QString Q = "Q";
const QString THETA = "THR";
const QString ESR = "ESR";
}

// Константи SCPI-команд для еквівалентної схеми
namespace ScpiEquivalent
{
const QString SERIAL = "SERial";
const QString PARALLEL = "PALlel";
}

// Константи SCPI-команд для швидкості
namespace ScpiAPERture
{
const QString SLOW = "SLOW";
const QString FAST = "FAST";
const QString MEDIUM = "MEDium";
}

// Константи SCPI-команд для затримки
namespace ScpiDelay
{
const QString SLOW = "SLOW";
const QString FAST = "FAST";
const QString MEDIUM = "MEDium";
}


// Структура для зберігання початкових параметрів конфігурації вимірювання
struct stConfigureField
{
    // Загальні параметри
    uint32_t delay;
    int scenario = -1;

    // Параметри для сценарію "Імпеданс" (частота)
    // FREQuency[:CW] 10
    // FREQuency[:CW]?
    // <NR3><NL>
    uint32_t m_startFrequency; // 10 Hz
    uint32_t m_stopFrequency;  // 100000 Hz
    uint32_t m_stepFrequency;

    // Параметри для сценарію "VAX"
    // VOLTage[:LEVel]
    // VOLTage[:LEVel]?
    // <NR3><NL>
    uint32_t m_startVoltage; // 10 mV
    uint32_t m_stopVoltage;  // 2000 mV
    uint32_t m_stepVoltage;

    // Параметри для сценарію "CV"
    // BIAS:VOLTage 1500
    // BIAS:VOLTage:LEVel?
    // Returns： 1500
    uint32_t m_startBias; // 0 mV
    uint32_t m_stopBias; // 1500 mV
    uint32_t m_stepBias;

    // Параметри приладу
    QString impedanceA; // {AUTO|R|C|L|Z|DCR|ECAP}<NL>
    QString impedanceB; // {X | D | | Q THR | ESR}
    QString equivalent; // {SERial|PALlel}
    QString aperture;   // {FAST|MEDium|SLOW}
};

class MeterController : public QObject
{
    Q_OBJECT

public:
    explicit MeterController(QSerialPort *serialPort, QTextEdit *debugger, QObject *parent = nullptr);
    ~MeterController();

public slots:
    void sendCommandToMeter(const QString &command);
    void startMeasurement(const stConfigureField &config);
    void stopMeasurement();
    void readData();
    void continueMeasurement();

signals:
    void logMessage(const QString &message);
    void updateDisplayA(double value);
    void updateDisplayB(double value);
    void updateDisplayC(double value);
    void sweepFinished();

private slots:
    void handleTimeout();

private:
    void initializeMeter(const stConfigureField &config);
    void logMeasurementData(double valueA, double valueB);

    QSerialPort *m_serialPort;
    QTextEdit   *m_debugger;
    QTimer      *m_sweepTimer;
    QFile       *m_logFile;
    QTextStream *m_logStream;

    bool m_isMeasuring;
    int m_scenarioIndex;

    uint32_t m_delay;

    uint32_t m_startFrequency;
    uint32_t m_stopFrequency;
    uint32_t m_currentFrequency;
    uint32_t m_stepFrequency;

    uint32_t m_startBias;
    uint32_t m_stopBias;
    uint32_t m_currentBias;
    uint32_t m_stepBias;

    uint32_t m_startVoltage;
    uint32_t m_stopVoltage;
    uint32_t m_currentVoltage;
    uint32_t m_stepVoltage;

    // ImpedanceA {AUTO|R|C|L|Z|DCR|ECAP}<NL>
    double m_R;    //
    double m_C;    //
    double m_L;    //
    double m_Z;    //
    double m_DCR;  //
    double m_ECAP; //
    // ImpedanceB {X | D | | Q THR | ESR}
    double m_X;    //
    double m_D;    //
    double m_Q;    //
    double m_THR;  //
    double m_ESR;  //
};

#endif // METERCONTROLLER_H
