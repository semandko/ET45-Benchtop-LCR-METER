#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include "measurement_setup.h"

// --- MainWindow Implementation ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serialPort(new QSerialPort(this))
    // Corrected constructor call: added ui->textEdit_debugger for logging
    , m_meterController(new MeterController(m_serialPort, ui->textEdit_debugger, this))
{
    ui->setupUi(this);
    setupUi();
    setupConnections();
    fillPortsInfo();
}

MainWindow::~MainWindow()
{
    // The objects ui, m_serialPort, m_meterController will be automatically
    // deleted due to the QObject hierarchy, as they are child objects.
}

void MainWindow::setupUi()
{
    // Configure the user interface
    ui->comboBox_baudrate->addItem(tr("9600"), QSerialPort::Baud9600);
    ui->comboBox_baudrate->addItem(tr("115200"), QSerialPort::Baud115200);
    // Add other standard baud rates
    ui->pushButton_connect->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);

    // Initialize the SCPI commands map
    m_scpiCommands = {
        {"*IDN?", "Query device identification"},
        {"SYSTem:VERSion?", "Query SCPI version"},
        {"APERture?", "Query measurement speed (FAST|MEDium|SLOW)"},
        {"BIAS:VOLTage", "Set bias voltage (mV)"},
        {"BIAS:VOLTage:LEVel?", "Query bias voltage (mV)"},
        {"COMParator:STATe?", "Query comparator state"},
        {"COMParator:TOLerance:NOMinal?", "Query comparator nominal"},
        {"COMParator:TOLerance:RANGe:CH?", "Query comparator tolerance range"},
        {"FETCh?", "Fetch measurement result"},
        {"VOLTage:LEVel?", "Query voltage level"},
        {"FREQuency[:CW]", "Set measurement frequency (Hz)"},
        {"FREQuency[:CW]?", "Query measurement frequency"},
        {"FUNCtion:DEV:MODE", "Enable/Disable development mode"},
        {"FUNCtion:DEV:MODE?", "Query development mode state"},
        {"FUNCtion:IMPedance:A?", "Query main measurement parameter"},
        {"FUNCtion:IMPedance:B?", "Query secondary measurement parameter"},
        {"FUNCtion:IMPedance:EQUivalent?", "Query equivalent circuit mode"},
        {"FUNCtion:IMPedance:EQUivalent", "Set equivalent circuit mode (SERial|PALlel)"},
        {"FUNCtion:IMPedance:RANGe:AUTO", "Enable/disable auto range"},
        {"FUNCtion:IMPedance:RANGe:AUTO?", "Query auto range state"},
        {"FUNCtion:IMPedance:RANGe[:VALue]", "Set manual range"},
        {"FUNCtion:IMPedance:RANGe[:VALue]?", "Query manual range"},
        {"SYSTem:BEEPer:STATe", "Enable/disable beeper"},
        {"SYSTem:BEEPer:STATe?", "Query beeper state"},
        {"LOCal", "Set instrument to local mode"}
    };
}

void MainWindow::setupConnections()
{
    connect(ui->pushButton_connect, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->pushButton_disconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectButtonClicked);
    connect(ui->pushButton_setup_measurement, &QPushButton::clicked, this, &MainWindow::on_pushButton_setup_measurement_clicked);
    connect(ui->pushButton_exit, &QPushButton::clicked, this, &MainWindow::close);
    connect(ui->pushButton_send_command, &QPushButton::clicked, this, [this]() {
        QString command = ui->lineEdit_command->text();
        m_meterController->sendCommandToMeter(command);
    });

    // Connect SCPI command buttons. Now reading values from text fields.

    connect(ui->pushButton_idn, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query device identification"));
    });
    connect(ui->pushButton_system_version, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query SCPI version"));
    });
    connect(ui->pushButton_aperture, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query measurement speed (FAST|MEDium|SLOW)"));
    });
    connect(ui->pushButton_bias_voltage, &QPushButton::clicked, this, [this]() {
        QString value = ui->lineEdit_biasVoltage->text();
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Set bias voltage (mV)") + " " + value);
    });
    connect(ui->pushButton_bias_voltage_level, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query bias voltage (mV)"));
    });
    connect(ui->pushButton_comparator_state, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query comparator state"));
    });
    connect(ui->pushButton_comparator_nominal, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query comparator nominal"));
    });
    connect(ui->pushButton_comparator_range, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query comparator tolerance range"));
    });
    connect(ui->pushButton_fetch, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Fetch measurement result"));
    });
    connect(ui->pushButton_voltage_level, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query voltage level"));
    });
    connect(ui->pushButton_frequency_cw, &QPushButton::clicked, this, [this]() {
        QString value = ui->lineEdit_frequency->text();
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Set measurement frequency (Hz)") + " " + value);
    });
    connect(ui->pushButton_frequency_cw_query, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query measurement frequency"));
    });
    connect(ui->pushButton_function_dev_mode, &QPushButton::clicked, this, [this]() {
        QString value = ui->lineEdit_devMode->text();
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Enable/Disable development mode") + " " + value);
    });
    connect(ui->pushButton_function_dev_mode_query, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query development mode state"));
    });
    connect(ui->pushButton_function_impedance_a, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query main measurement parameter"));
    });
    connect(ui->pushButton_function_impedance_b, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query secondary measurement parameter"));
    });
    connect(ui->pushButton_function_impedance_equivalent_query, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query equivalent circuit mode"));
    });
    connect(ui->pushButton_function_impedance_equivalent, &QPushButton::clicked, this, [this]() {
        QString value = ui->lineEdit_equivalentMode->text();
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Set equivalent circuit mode (SERial|PALlel)") + " " + value);
    });
    connect(ui->pushButton_function_impedance_range_auto, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Enable/disable auto range"));
    });
    connect(ui->pushButton_function_impedance_range_auto_query, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query auto range state"));
    });
    connect(ui->pushButton_function_impedance_range_value, &QPushButton::clicked, this, [this]() {
        QString value = ui->lineEdit_manualRange->text();
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Set manual range") + " " + value);
    });
    connect(ui->pushButton_function_impedance_range_value_query, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query manual range"));
    });
    connect(ui->pushButton_system_beeper_state, &QPushButton::clicked, this, [this]() {
        QString value = ui->lineEdit_beeperState->text();
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Enable/disable beeper") + " " + value);
    });
    connect(ui->pushButton_system_beeper_state_query, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Query beeper state"));
    });
    connect(ui->pushButton_local, &QPushButton::clicked, m_meterController, [this]() {
        m_meterController->sendCommandToMeter(m_scpiCommands.key("Set instrument to local mode"));
    });

    // Connect signals from MeterController to MainWindow slots
    connect(m_meterController, &MeterController::logMessage,        ui->textEdit_debugger,  &QTextEdit::append);
    connect(m_meterController, &MeterController::updateDisplayC,    this,                   &MainWindow::onUpdateDisplayC);
    connect(m_meterController, &MeterController::sweepFinished,     this,                   &MainWindow::onSweepFinished);
}

void MainWindow::fillPortsInfo()
{
    ui->comboBox_comport->clear();

    for (const auto &portInfo : QSerialPortInfo::availablePorts())
    {
        ui->comboBox_comport->addItem(portInfo.portName());
    }
}

void MainWindow::onConnectButtonClicked()
{
    QString portName = ui->comboBox_comport->currentText();
    int baudRate = ui->comboBox_baudrate->currentData().toInt();

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(static_cast<QSerialPort::BaudRate>(baudRate));

    if (m_serialPort->open(QIODevice::ReadWrite))
    {
        ui->textEdit_debugger->append("Connected to port " + portName);
        ui->pushButton_connect->setEnabled(false);
        ui->pushButton_disconnect->setEnabled(true);
        // Activate "Setup Measurement" button
        ui->pushButton_setup_measurement->setVisible(true);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open port: ") + m_serialPort->errorString());
        ui->textEdit_debugger->append("Connection failed: " + m_serialPort->errorString());
    }
}

void MainWindow::onDisconnectButtonClicked()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->close();
        ui->textEdit_debugger->append("Disconnected.");
        ui->pushButton_connect->setEnabled(true);
        ui->pushButton_disconnect->setEnabled(false);
        ui->pushButton_setup_measurement->setVisible(false);
    }
}

void MainWindow::onUpdateDisplayC(double value)
{
    ui->lcdNumber_displayC->display(value);
}

void MainWindow::onSweepFinished()
{
    ui->textEdit_debugger->append("Sweep finished.");
    ui->pushButton_setup_measurement->setVisible(true);
}

// Slot for opening the measurement setup window
void MainWindow::on_pushButton_setup_measurement_clicked()
{
    // Create a new measurement setup dialog
    MeasurementSetup setupDialog(this, m_meterController);
    setupDialog.exec();
}
