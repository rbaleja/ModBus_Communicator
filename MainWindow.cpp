#include "MainWindow.h"
#include "ui_MainWindow.h"




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    device = new QSerialPort;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete device;
    delete connection;
    delete reply;
}

void MainWindow::addToLogs(QString message)// Metoda dodaje (QString) do okna Logs wiadomości z dodanym czasem
{
    QString curreentDateTime = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    ui->textEditLogs->append(curreentDateTime + "\t" + message);
}

void MainWindow::on_pushButtonSearchDevice_clicked()// Metoda szuka urzadzeń podłaczonych do portu COM
{
    qDebug() << "Szukam urządzeń";

    QList<QSerialPortInfo> devices;
    devices = QSerialPortInfo::availablePorts();
    for(int i = 0; i < devices.count(); i++)
    {
        qDebug() << devices.at(i).portName() << devices.at(i).description();

        this->addToLogs("Znalazłem urządzenie: " + devices.at(i).portName() + " " + devices.at(i).description());
        ui->comboBoxDevicesList->addItem(devices.at(i).portName() + "\t" + devices.at(i).description());
    }
}

void MainWindow::on_pushButtonConnectDevice_clicked() // Metoda łączy się z urządzeniem MODBUS przez COM
{
    if(ui->comboBoxDevicesList->count() ==0 )
        {
            this->addToLogs("Nie wykryto urządzeń!");
            return;
        }

    portName = ui->comboBoxDevicesList->currentText().split("\t").first();
    device->setPortName(portName);


    connection->setConnectionParameter(QModbusDevice::SerialPortNameParameter, portName);
    connection->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,QSerialPort::Baud9600);
    connection->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,QSerialPort::Data8);
    connection->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    connection->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
    connection->setTimeout(100);
    connection->setNumberOfRetries(5);
    connection->connectDevice();
    this->addToLogs("Połączono z urządzeniem MODBUS do portu " + portName);

    qDebug() << connection->state();
    qDebug() << connection->errorString();
}

void MainWindow::on_pushButtonDisconnectDevice_clicked()// Metoda rozłącza się z urządzeniem MODBUS
{
    connection->disconnectDevice();
    this->addToLogs("Rozłączono urządznie od portu " + portName);

    qDebug() << connection->state();
}

void MainWindow::on_pushButton_Write_Register_clicked()// Metoda zapisuje wartość(zmienna registerValue) w rejestrze(zmienna adress number)
{
    *adresNumber_ptr = ui->lineEdit_SetRegisterAdress->text().toInt();
    *registerValue_ptr = ui->lineEdit_SetRegisterValue->text().toInt();

    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, *adresNumber_ptr, 1); // tu działa zapis rejestru 4006 miernika N13
    writeUnit.setValue(0, *registerValue_ptr);

    qDebug() << *registerValue_ptr;
    qDebug() << registerValue;
    qDebug() << *adresNumber_ptr;
    qDebug() << adresNumber;

    *stringValue_ptr = QString::number(*registerValue_ptr);
    *stringStartAdress_prt = QString::number(*adresNumber_ptr);

    this->addToLogs("Zapisano adres: " + *stringStartAdress_prt + " " + "Wartością: " + *stringValue_ptr);

    if (auto *reply = connection->sendWriteRequest(writeUnit, 1))
    {

        if (!reply->isFinished())


        {
            connect(reply, &QModbusReply::finished, this, [reply]()
            {
                if (reply->error() != QModbusDevice::NoError)

                    reply->deleteLater();
            });

        }
        else
        {
            if (reply->error() != QModbusDevice::NoError)
                reply->deleteLater();
        }
    }


}

void MainWindow::on_pushButton_Read_Register_clicked()
{
    //*adresNumber_ptr = ui->lineEdit_SetRegisterAdress->text().toInt();
    *adresNumber_ptr = 7500;
    ui->lineEdit_SetRegisterAdress->setText(QString::number(*adresNumber_ptr));
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, *adresNumber_ptr, 33);

    if (auto *reply = connection->sendReadRequest(readUnit, 1))
    {
        if (!reply->isFinished())
        {
            // connect the finished signal of the request to your read slot
            connect(reply, &QModbusReply::finished, this, &MainWindow::readReady);

        }
        else
        {
            delete reply; // broadcast replies return immediately
        }
    }
    else
    {
        // request error
    }

    qDebug() << reply;
    qDebug() << connection->state();

}

void MainWindow::readReady()
{
    QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
        if (!reply)
            return;

        if (reply->error() == QModbusDevice::NoError)
        {
            QModbusDataUnit unit = reply->result();
            //int startAddress = &unit->startAddress(); // odczyt rejestru startowego


            //int value = unit.value(0); // wartość w rejestrze 16 bitów start + 0


            ui->lcdNumber_L1->display(read_32_register(&unit, 0));


            ui->lcdNumber_L2->display(read_32_register(&unit, 14));


            ui->lcdnumber_L3->display(read_32_register(&unit, 28));


            ui->lcdNumber_Freq->display(read_32_register(&unit, 56));






            //void *v_ptr_L1 = &value32_L1; //konwersja uint32 na float
            //float float_value_L1 = *(float*)v_ptr_L1;


            //*stringValue_ptr = QString::number(float_value_L1);
            //*stringStartAdress_prt = QString::number(startAddress);

            this->addToLogs("Odczytano adres: " + *stringStartAdress_prt +" " + "o wartości: " + *stringValue_ptr);



        }
        else
        {
            this->addToLogs("Urzadznie zajete komunikacją. Poczekaj na zakonćzenie komunikacji");
        }

        reply->deleteLater();
}

float MainWindow::conversion_to_float(quint32 value32)
{
    union u32_to_float //konwersja uint32 do float za pomocą Unii
    {
        uint32_t value32;
        float f_value;

    };

    union u32_to_float tmp;
    tmp.value32 = value32;
    float converted = tmp.f_value;

    return converted;

}

float MainWindow::read_32_register(QModbusDataUnit *unit, int index)
{
    quint32 value32 = unit->value(index);//odczyt pierwszych 16 bitów rejestru (adresy rejestrów 32 mnożymy x2 i to jest jego pierwsze 16 bitów)
    value32 = (value32 << 16) | unit->value(index + 1);//odczyt drugich 16 bitów rejestru
    float float_value = conversion_to_float(value32);
    return float_value;

}

void MainWindow::on_pushButton_Clouse_clicked()
{
    connection->disconnectDevice();
    MainWindow::close();
}


void MainWindow::on_pushButton_ConnectTCP_clicked()
{
    QString Adress_IP = "192.168.1.129";
    QString Port_TCP = "502";
    connection->setConnectionParameter(QModbusDevice::NetworkAddressParameter, Adress_IP);
    connection->setConnectionParameter(QModbusDevice::NetworkPortParameter, Port_TCP);
    connection->setTimeout(100);
    connection->connectDevice();
    this->addToLogs("Połączono z urządzeniem MODBUS o adresie " + Adress_IP + " port " + Port_TCP);

    qDebug() << connection->state();
    qDebug() << connection->errorString();
}





void MainWindow::on_checkBoxModBusRTU_clicked()
{


}

void MainWindow::on_checkBoxModBusTCP_clicked()
{


}



