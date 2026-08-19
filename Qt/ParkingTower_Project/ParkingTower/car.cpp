#include "car.h"

Car::Car(const QString &carNumber, int floor)
    : carNumber(carNumber), floor(floor), entryTime(QDateTime::currentDateTime()), exitTime(QDateTime())
{
}

Car::Car(const QString &carNumber, int floor, const QDateTime &entryTime)
    : carNumber(carNumber),
      floor(floor),
      entryTime(entryTime),
      exitTime(QDateTime())
{
}

QString Car::getCarNumber(void) const
{
    return carNumber;
}

int Car::getFloor(void) const
{
    return floor;
}

QDateTime Car::getEntryTime(void) const
{
    return entryTime;
}

QDateTime Car::getExitTime(void) const
{
    return exitTime;
}

void Car::setExitTime(const QDateTime &time)
{
    exitTime = time;
}