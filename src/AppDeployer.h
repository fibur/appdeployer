#ifndef APPDEPLOYER_H
#define APPDEPLOYER_H

#include <QCommandLineParser>
#include <QApplication>
#include <QMenu>
#include <QTcpServer>
#include <QSystemTrayIcon>

class DeployerServer;
class DeployerClient;

/*!
    \class AppDeployer
    \brief The AppDeployer class is a main QCoreApplication class,
           that handles command line input, and initializes client/server
!*/
class AppDeployer : public QApplication
{
    Q_OBJECT

public:
    explicit AppDeployer(int argc, char *argv[]);
    ~AppDeployer();

    /*!
        \brief Reimplementation of QCoreApplication::exec.
    !*/
    int exec();

private:
    QIcon randomIcon(int w, int h);

private:
    QCommandLineParser m_parser;

    DeployerClient *m_deployerClient = nullptr;
    DeployerServer *m_deployerServer = nullptr;
    QMenu *m_trayMenu = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
};

#endif // APPDEPLOYER_H
