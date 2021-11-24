QT -= gui console
# Adding QWidgets, to support system tray icon
QT += network winextras widgets

CONFIG += c++17
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        src/networking/DeployerClient.cpp \
        src/networking/DeployerMessage.cpp \
        src/networking/DeployerServer.cpp \
        src/networking/NetworkUserBase.cpp \
        src\AppDeployer.cpp \
        src\main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/networking/DeployerClient.h \
    src/networking/DeployerMessage.h \
    src/networking/DeployerServer.h \
    src/networking/NetworkUserBase.h \
    src\AppDeployer.h \

RESOURCES +=
