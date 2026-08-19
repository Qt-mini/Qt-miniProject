#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QListWidgetItem>

#include "parkingmanager.h"
#include "serialmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSerialReady();
    void onSlotStatusReceived(quint8 slotStatus);
    void onDetectedReceived();
    void on_btnEntry_clicked();
    void on_btnExit_clicked();

    void on_btnEntryToHome_clicked();
    void on_btnEntryRequest_clicked();
    void onParkedReceived(int floor);

    void on_btnExitSearch_clicked();
    void on_btnExitRequest_clicked();
    void on_btnExitToHome_clicked();
    void onExitedReceived();

    void on_btnPaymentRequest_clicked();
    void on_btnPaymentToExit_clicked();
    void on_btnPaymentToHome_clicked();

private:
    Ui::MainWindow *ui;
    ParkingManager parkingManager;
    SerialManager serialManager;

    QString selectedExitCarNumber;
    QDateTime pendingExitTime;
    void updateHomeStatus();
    void clearExitData();
};
#endif // MAINWINDOW_H
