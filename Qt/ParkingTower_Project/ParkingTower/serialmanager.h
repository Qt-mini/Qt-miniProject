#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <QSerialPort>
#include <QString>

class SerialManager : public QObject
{
    Q_OBJECT

private:
    QSerialPort uartSerial;
    QByteArray rxBuffer;

    void parseReceivedData(const QByteArray &data);

public:
    explicit SerialManager(QObject *parent = nullptr);
    bool open(const QString &portName);
    void close();

    bool isOpen() const;

    void sendData(const QString &data);

private slots:
    void receiveData();

signals:
    // void dataReceived(const QString &data);

    void readyReceived();
    void slotStatusReceived(quint8 slotStatus);
    void parkedReceived(int floor);
    void exitedReceived();
    void detectedReceived();
};

#endif // SERIALMANAGER_H
