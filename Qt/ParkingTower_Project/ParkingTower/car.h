#ifndef CAR_H
#define CAR_H

#include <QString>
#include <QDateTime>

class Car
{
private:
    QString carNumber;
    int floor;
    QDateTime entryTime;
    QDateTime exitTime;

public:
    Car(const QString &carNumber, int floor);
    Car(const QString &carNumber, int floor, const QDateTime &entryTime);

    QString getCarNumber(void) const;
    int getFloor(void) const;
    QDateTime getEntryTime(void) const;
    QDateTime getExitTime(void) const;
    void setExitTime(const QDateTime &time);
};

#endif // CAR_H
