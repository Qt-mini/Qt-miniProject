#ifndef PARKINGMANAGER_H
#define PARKINGMANAGER_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

#include "car.h"

class ParkingManager
{
private:
    QVector<Car> cars;
    quint8 slotStatus = 0b00000000;

public:
    void loadParkingCars();
    quint8 getStoredSlotStatus();

    void addCar(const QString &carNumber, int floor);
    Car *findCar(const QString &carNumber);
    QVector<Car *> findCarByLastFour(const QString &lastFour);
    void removeCar(const QString &carNumber);

    qint64 calculateParkingTime(const QString &carNumber, const QDateTime &exitTime);
    int calculateFee(qint64 parkingTime);

    bool saveParkingRecord(const QString &carNumber);

    void setSlotStatus(quint8 status);
    quint8 getSlotStatus(void) const;

    bool isFull(void) const;
    int getAvailableCount(void) const;
    bool isOccupied(int floor) const;
};

#endif // PARKINGMANAGER_H
