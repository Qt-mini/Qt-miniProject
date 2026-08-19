#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    parkingManager.loadParkingCars();

    connect(&serialManager,
            &SerialManager::readyReceived,
            this,
            &MainWindow::onSerialReady);

    connect(&serialManager,
            &SerialManager::slotStatusReceived,
            this,
            &MainWindow::onSlotStatusReceived);

    connect(&serialManager,
            &SerialManager::detectedReceived,
            this,
            &MainWindow::onDetectedReceived);

    connect(&serialManager,
            &SerialManager::parkedReceived,
            this,
            &MainWindow::onParkedReceived);
    connect(&serialManager,
            &SerialManager::exitedReceived,
            this,
            &MainWindow::onExitedReceived);
    serialManager.open("COM6"); // STM32와 연결된 시리얼 포트 이름으로 변경 필요

    /*Home 초기화*/
    ui->btnEntry->setEnabled(false);
    ui->btnExit->setEnabled(false);
    ui->lblAvailableCount->clear();
    ui->stackedWidget->setCurrentWidget(ui->pageHome);

    /*입차 입력 번호 형식 지정*/
    QRegularExpression entryRegex(
        "[0-9]{2,3}"
        "[가나다라마바사거너더러머버서어저고노도로모보소오조구누두루무부수우주아바사자허하호배]"
        "[0-9]{4}");

    ui->leEntryCarNumber->setValidator(new QRegularExpressionValidator(entryRegex, this));

    /*출차 입력 번호 형식 지정*/
    QRegularExpression exitRegex("^[0-9]{4}$");

    ui->leExitLastFour->setValidator(new QRegularExpressionValidator(exitRegex, this));
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateHomeStatus()
{
    ui->lblAvailableCount->setText(QString::number(parkingManager.getAvailableCount()));
    QLabel *slotLabels[8] = {
        ui->lblSlot1, ui->lblSlot2, ui->lblSlot3, ui->lblSlot4,
        ui->lblSlot5, ui->lblSlot6, ui->lblSlot7, ui->lblSlot8};

    QPixmap green(":/images/green.png");
    QPixmap red(":/images/red.png");
    for (int i = 0; i < 8; i++)
    {
        slotLabels[i]->setPixmap(
            parkingManager.isOccupied(i + 1) ? red : green);
        slotLabels[i]->setScaledContents(true);
    }

    ui->btnEntry->setEnabled(false);
    ui->btnExit->setEnabled(true);
}

void MainWindow::onSerialReady()
{
    serialManager.sendData("?");
}

void MainWindow::onSlotStatusReceived(quint8 slotStatus)
{
    quint8 storedStatus = parkingManager.getStoredSlotStatus();
    qDebug() << "Stored SLOT STATUS:"
             << QByteArray::number(storedStatus, 2).rightJustified(8, '0');

    qDebug() << "STM SLOT STATUS:"
             << QByteArray::number(slotStatus, 2).rightJustified(8, '0');
    if (storedStatus == slotStatus)
    {
        qDebug() << "SLOT STATUS MATCHED";
    }
    else
    {
        qDebug() << "SLOT STATUS MISMATCH";
    }
    parkingManager.setSlotStatus(slotStatus);
    updateHomeStatus();
    ui->stackedWidget->setCurrentWidget(ui->pageHome);
}

void MainWindow::onDetectedReceived()
{
    if (!parkingManager.isFull())
    {
        ui->btnEntry->setEnabled(true);
    }
}
void MainWindow::on_btnEntry_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageEntry);
}
void MainWindow::on_btnExit_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageExit);
}

void MainWindow::on_btnEntryToHome_clicked()
{
    ui->leEntryCarNumber->clear();
    serialManager.sendData("?");
}

void MainWindow::on_btnEntryRequest_clicked()
{
    QString carNumber = ui->leEntryCarNumber->text();
    if (!ui->leEntryCarNumber->hasAcceptableInput())
    {
        qDebug() << "Invalid car number";
        return;
    }

    serialManager.sendData("I");
    ui->btnEntryRequest->setEnabled(false); // 중복요청 방지

    qDebug() << "Car Number:" << carNumber;
}

void MainWindow::onParkedReceived(int floor)
{
    QString carNumber = ui->leEntryCarNumber->text();
    parkingManager.addCar(carNumber, floor);
    parkingManager.saveParkingRecord(carNumber);

    ui->stackedWidget->setCurrentWidget(ui->pageEntryComplete);

    QTimer::singleShot(3000, this, [this]()
                       {
        ui->leEntryCarNumber->clear();
        ui->btnEntryRequest->setEnabled(true); // 입차 요청 버튼 원상복구
        
        //request Home Status Update to STM32
        serialManager.sendData("?"); });
}

void MainWindow::on_btnExitSearch_clicked()
{
    if (!ui->leExitLastFour->hasAcceptableInput())
    {
        qDebug() << "Invalid last four digits.";
        return;
    }

    QString lastFour = ui->leExitLastFour->text();
    ui->listExitCars->clear();
    QVector<Car *> results = parkingManager.findCarByLastFour(lastFour);

    for (Car *car : results)
    {
        QString text =
            car->getCarNumber() + "\t" +
            car->getEntryTime().toString("yyyy-MM-dd HH:mm:ss");

        QListWidgetItem *item = new QListWidgetItem(text);

        // 화면에는 번호 + 입차시간을 보여주지만
        // 내부에는 차량번호만 따로 저장
        item->setData(Qt::UserRole, car->getCarNumber());
        ui->listExitCars->addItem(item);
    }
}

void MainWindow::on_btnExitRequest_clicked()
{
    QListWidgetItem *selectedItem = ui->listExitCars->currentItem();
    if (selectedItem == nullptr)
    {
        qDebug() << "No car selected for exit.";
        return;
    }
    selectedExitCarNumber = selectedItem->data(Qt::UserRole).toString();
    pendingExitTime = QDateTime::currentDateTime();
    qint64 parkingTime = parkingManager.calculateParkingTime(selectedExitCarNumber, pendingExitTime);
    int fee = parkingManager.calculateFee(parkingTime);
    ui->lblPaymentCarNumber->setText(selectedExitCarNumber);
    ui->lblPaymentTime->setText(QString("%1시간 %2분").arg(parkingTime / 3600).arg((parkingTime % 3600) / 60));
    ui->lblPaymentFee->setText(QString("%1원").arg(fee));

    ui->stackedWidget->setCurrentWidget(ui->pagePayment);
}

void MainWindow::on_btnExitToHome_clicked()
{
    ui->leExitLastFour->clear();
    ui->listExitCars->clear();

    serialManager.sendData("?");
}

void MainWindow::on_btnPaymentRequest_clicked()
{
    Car *car = parkingManager.findCar(selectedExitCarNumber);
    if (car == nullptr)
    {
        qDebug() << "Car not found for payment.";
        return;
    }
    car->setExitTime(pendingExitTime);
    if (!parkingManager.saveParkingRecord(selectedExitCarNumber))
    {
        qDebug() << "Failed to save parking record.";
        return;
    }

    qDebug() << "Payment completed for car:"
             << selectedExitCarNumber
             << car->getExitTime();

    // STM에 출차 요청
    serialManager.sendData(
        QString("O:%1").arg(car->getFloor()));

    ui->btnPaymentRequest->setEnabled(false);
}

void MainWindow::onExitedReceived()
{
    parkingManager.removeCar(selectedExitCarNumber);

    ui->stackedWidget->setCurrentWidget(ui->pagePaymentComplete);

    QTimer::singleShot(3000, this, [this]()
                       { 
                        clearExitData();
                        ui->btnPaymentRequest->setEnabled(true);
                        serialManager.sendData("?"); });
}
void MainWindow::on_btnPaymentToExit_clicked()
{

    clearExitData();
    ui->stackedWidget->setCurrentWidget(ui->pageExit);
}

void MainWindow::on_btnPaymentToHome_clicked()
{

    clearExitData();
    serialManager.sendData("?");
}

void MainWindow::clearExitData()
{
    pendingExitTime = QDateTime();
    selectedExitCarNumber.clear();

    ui->leExitLastFour->clear();
    ui->listExitCars->clear();

    ui->lblPaymentCarNumber->clear();
    ui->lblPaymentTime->clear();
    ui->lblPaymentFee->clear();
}