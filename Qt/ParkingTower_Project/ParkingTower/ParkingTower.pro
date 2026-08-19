QT += widgets serialport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    car.cpp \
    main.cpp \
    mainwindow.cpp \
    parkingmanager.cpp \
    serialmanager.cpp

HEADERS += \
    car.h \
    mainwindow.h \
    parkingmanager.h \
    serialmanager.h

FORMS += \
    mainwindow.ui

#나중에 제거
CONFIG += console


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
