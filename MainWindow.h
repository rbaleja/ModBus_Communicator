#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialBus>
#include <QSerialPort>
//#include <QModbusRtuSerialMaster>
//#include <QModbusTcpServer>
#include <QModbusClient>
#include <QSerialPortInfo>
#include <QDateTime>
#include <QDebug>
#include <QVector>





QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
     int adresNumber;
     int *adresNumber_ptr = &adresNumber;

     int registerValue;
     int *registerValue_ptr = &registerValue;

     QString stringValue;
     QString *stringValue_ptr = &stringValue;

     QString stringStartAdress;
     QString *stringStartAdress_prt = &stringStartAdress;

private slots:
    void on_pushButtonSearchDevice_clicked();

    void on_pushButtonConnectDevice_clicked();

    void on_pushButtonDisconnectDevice_clicked();

    void on_pushButton_Write_Register_clicked();

    void on_pushButton_Read_Register_clicked();

    void on_pushButton_Clouse_clicked();

    void on_pushButton_ConnectTCP_clicked();



    void on_checkBoxModBusRTU_clicked();

    void on_checkBoxModBusTCP_clicked();

private:
    Ui::MainWindow *ui;

    void addToLogs(QString);

    QSerialPort *device;

    //QModbusRtuSerialMaster *connection = new QModbusRtuSerialMaster(this);
    QModbusTcpClient *connection = new QModbusTcpClient(this);


    QModbusDataUnit *reply;

    QModbusDataUnit unit;
    QModbusDataUnit *unit_ptr = &unit;


    QString portName;

    void readReady();

    float conversion_to_float(quint32);

    float read_32_register(QModbusDataUnit *, int);




};
#endif // MAINWINDOW_H
