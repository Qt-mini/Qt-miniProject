#include "serialmanager.h"

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
{
    connect(&uartSerial,
            &QSerialPort::readyRead,
            this,
            &SerialManager::receiveData);
}

bool SerialManager::open(const QString &portName)
{
    uartSerial.setPortName(portName);

    uartSerial.setBaudRate(QSerialPort::Baud115200);
    uartSerial.setDataBits(QSerialPort::Data8);
    uartSerial.setParity(QSerialPort::NoParity);
    uartSerial.setStopBits(QSerialPort::OneStop);
    uartSerial.setFlowControl(QSerialPort::NoFlowControl);

    if (!uartSerial.open(QIODevice::ReadWrite))
    {
        qDebug() << "Failed to open serial port:"
                 << uartSerial.errorString();
        return false;
    }

    qDebug() << "Serial port opened successfully:"
             << portName;
    return true;
}

void SerialManager::close()
{
    if (uartSerial.isOpen())
    {
        uartSerial.close();
        qDebug() << "Serial port closed.";
    }
}

bool SerialManager::isOpen() const
{
    return uartSerial.isOpen();
}

void SerialManager::sendData(const QString &data)
{
    if (!uartSerial.isOpen())
    {
        qDebug() << "Serial port is not open. Cannot send data.";
        return;
    }
    QByteArray dataToSend = data.toUtf8();
    dataToSend.append("\r\n");
    uartSerial.write(dataToSend);
    qDebug() << "Sent data:" << dataToSend;
}
void SerialManager::receiveData()
{
    rxBuffer.append(uartSerial.readAll());
    while (rxBuffer.contains("\r\n"))
    {
        int endIndex = rxBuffer.indexOf("\r\n");
        QByteArray receivedData = rxBuffer.left(endIndex);
        rxBuffer.remove(0, endIndex + 2);
        if (!receivedData.isEmpty())
        {
            qDebug() << "Received data:" << receivedData;
            // emit dataReceived(receivedData);
            parseReceivedData(receivedData);
        }
    }
}

void SerialManager::parseReceivedData(const QByteArray &data)
{
    if (data == "R")
    {
        qDebug() << "STM32 is READY.";
        emit readyReceived();
    }

    // for Tera Term
    else if (data.size() == 10 &&
             data[0] == 'S' &&
             data[1] == ':')
    {
        bool ok = false;

        quint8 slotStatus =
            static_cast<quint8>(data.mid(2).toUInt(&ok, 2));

        if (ok)
        {
            qDebug() << "Received SLOT STATUS:"
                     << QByteArray::number(slotStatus, 2).rightJustified(8, '0');

            emit slotStatusReceived(slotStatus);
        }
        else
        {
            qDebug() << "Invalid SLOT STATUS:" << data;
        }
    }

    /* for STM32
    else if (data.size() == 3 &&
             data[0] == 'S' &&
             data[1] == ':')
    {
        quint8 slotStatus = static_cast<quint8>(
            static_cast<unsigned char>(data[2]));
        qDebug() << "Received SLOT STATUS:" << QByteArray::number(slotStatus, 2).rightJustified(8, '0');
        emit slotStatusReceived(slotStatus);
    }
    */
    else if (data.startsWith("P:"))
    {
        bool ok = false;
        int floor = data.mid(2).toInt(&ok);
        if (ok && floor >= 1 && floor <= 8)
        {
            qDebug() << "Parked completed. Floor:" << floor;
            emit parkedReceived(floor);
        }
        else
        {
            qDebug() << "Invalid PARKED FLOOR:" << data;
        }
    }
    else if (data == "E")
    {
        qDebug() << "Exit completed.";
        emit exitedReceived();
    }
    else if (data == "D")
    {
        qDebug() << "Car Detected.";
        emit detectedReceived();
    }
    else
    {
        qDebug() << "Unknown data received:" << data;
    }
}