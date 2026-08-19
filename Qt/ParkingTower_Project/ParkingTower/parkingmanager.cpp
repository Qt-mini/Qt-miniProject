#include "parkingmanager.h"

// 다른 PC에서 실행할 경우 본인의 프로젝트 경로로 수정
const QString PARKING_RECORD_DIR =
    "C:/Users/NOY/Python_M4_C++/Qt_PyQt_Lab/practice_qt/ParkingTower_Project/ParkingRecord";

void ParkingManager::loadParkingCars()
{
    QDir dir(PARKING_RECORD_DIR);
    if (!dir.exists())
        return;
    cars.clear();
    QStringList fileList =
        dir.entryList(QStringList() << "*.csv", QDir::Files);
    for (const QString &fileName : fileList)
    {
        QFile file(dir.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString lastLine;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (!line.isEmpty())
                lastLine = line;
        }
        file.close();
        if (lastLine.isEmpty())
            continue;

        QStringList record = lastLine.split(",");
        if (record.size() < 3)
            continue;
        if (!record[2].isEmpty())
            continue;
        bool floorOk = false;
        int floor = record[0].split(" ")[0].toInt(&floorOk);
        if (!floorOk || floor < 1 || floor > 8)
            continue;

        QDateTime entryTime = QDateTime::fromString(record[1], "yyyy-MM-dd HH:mm:ss");
        if (!entryTime.isValid())
            continue;

        QString carNumber = QFileInfo(fileName).completeBaseName();
        Car car(carNumber, floor, entryTime);
        cars.append(car);
    }
}

quint8 ParkingManager::getStoredSlotStatus()
{
    quint8 status = 0b00000000;
    for (const Car &car : cars)
    {
        int floor = car.getFloor();
        if (floor >= 1 && floor <= 8)
        {
            status |= (1 << (floor - 1));
        }
    }
    return status;
}

void ParkingManager::addCar(const QString &carNumber, int floor)
{
    cars.append(Car(carNumber, floor));
}

Car *ParkingManager::findCar(const QString &carNumber)
{
    for (Car &car : cars)
    {
        if (car.getCarNumber() == carNumber)
            return &car;
    }
    return nullptr;
}

QVector<Car *> ParkingManager::findCarByLastFour(const QString &lastFour)
{
    QVector<Car *> results;
    for (Car &car : cars)
    {
        if (car.getCarNumber().endsWith(lastFour))
        {
            results.append(&car);
        }
    }
    return results;
}
void ParkingManager::removeCar(const QString &carNumber)
{
    for (auto it = cars.begin(); it != cars.end(); ++it)
    {
        if (it->getCarNumber() == carNumber)
        {
            cars.erase(it);
            return;
        }
    }
}

qint64 ParkingManager::calculateParkingTime(const QString &carNumber, const QDateTime &exitTime)
{
    Car *car = findCar(carNumber);
    if (car == nullptr)
        return -1; // Return -1 if the car is not found

    return car->getEntryTime().secsTo(exitTime);
}

int ParkingManager::calculateFee(qint64 parkingTime)
{
    if (parkingTime < 0)
        return -1;
    const int BasicTime = 30 * 60;
    const int BasicFee = 2000;
    const int ExtraTime = 10 * 60;
    const int ExtraFee = 500;
    const int MaxFee = 50000;

    int fee = BasicFee;
    if (parkingTime > BasicTime)
    {
        qint64 extraTime = parkingTime - BasicTime;

        qint64 extraCount = (extraTime + ExtraTime - 1) / ExtraTime;
        fee += extraCount * ExtraFee;
    }
    if (fee > MaxFee)
        fee = MaxFee;
    return fee;
}

bool ParkingManager::saveParkingRecord(const QString &carNumber)
{
    Car *car = findCar(carNumber);
    if (car == nullptr)
        return false; // Return false if the car is not found
    QDir dir;
    if (!dir.exists(PARKING_RECORD_DIR))
    {
        if (!dir.mkpath(PARKING_RECORD_DIR))
            return false; // Return false if the directory cannot be created
    }
    QString fileName = PARKING_RECORD_DIR + "/" + carNumber + ".csv";
    QFile file(fileName);

    /*
    입차 기록
    */
    if (!car->getExitTime().isValid())
    {
        if (!file.open(QIODevice::WriteOnly |
                       QIODevice::Append |
                       QIODevice::Text))
        {
            return false; // Return false if the file cannot be opened
        }
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << car->getFloor() << " Floor,"
            << car->getEntryTime().toString("yyyy-MM-dd HH:mm:ss") << ","
            << "\n";
        file.close();
        return true;
    }
    /*
    출차 기록
    */
    if (!file.open(QIODevice::ReadOnly |
                   QIODevice::Text))
    {
        return false; // Return false if the file cannot be opened
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QStringList lines;
    while (!in.atEnd())
    {
        lines.append(in.readLine());
    }
    file.close();
    if (lines.isEmpty())
    {
        return false; // Return false if the file is empty
    }

    QStringList lastRecord = lines.last().split(",");
    if (lastRecord.size() < 2)
    {
        return false; // Return false if the last record is invalid
    }
    lines.last() = lastRecord[0] + "," +
                   lastRecord[1] + "," +
                   car->getExitTime().toString("yyyy-MM-dd HH:mm:ss");

    if (!file.open(QIODevice::WriteOnly |
                   QIODevice::Truncate |
                   QIODevice::Text))
    {
        return false; // Return false if the file cannot be opened
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    for (const QString &line : lines)
    {
        out << line << "\n";
    }
    file.close();
    return true;
}

void ParkingManager::setSlotStatus(quint8 status)
{
    slotStatus = status;
}

quint8 ParkingManager::getSlotStatus(void) const
{
    return slotStatus;
}

bool ParkingManager::isFull(void) const
{
    return slotStatus == 0xFF;
}

int ParkingManager::getAvailableCount(void) const
{
    int occupiedCount = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (slotStatus & (1 << i))
            occupiedCount++;
    }
    return 8 - occupiedCount;
}

bool ParkingManager::isOccupied(int floor) const
{
    if (floor < 1 || floor > 8)
        return false;
    return slotStatus & (1 << (floor - 1));
}
