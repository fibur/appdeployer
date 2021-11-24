#ifndef DEPLOYERSERVER_H
#define DEPLOYERSERVER_H

#include "NetworkUserBase.h"

#include <QFileSystemWatcher>
#include <QProcess>
#include <QTcpServer>

/*!
    \class DeployerServer
    \brief The DeployerServer class wrapps QTcpServer, manages connections and handles their requests.
!*/
class DeployerServer : public NetworkUserBase
{
    Q_OBJECT

public:
    explicit DeployerServer(ushort port, QObject *parent = nullptr);

    void startupServer();

protected:
    void onMessageReceived(const DeployerMessage &message) override;

private slots:
    void onConnection();
    void onError();
    void onClientDisconnected();
    void appClosed(int exitCode, QProcess::ExitStatus exitStatus);
    void logFile(const QString &path);

private:
    bool killApp(const QString &executable);
    void startApp();
    void cleanUp();

private:
    QTcpServer *m_server = nullptr;
    QProcess *m_process = nullptr;
    QFileSystemWatcher *m_logWatcher = nullptr;

    ushort m_port;
    QString m_exeName;
    QFile *m_logFile = nullptr;
    bool m_appIsRunning = false;
};

#endif // DEPLOYERSERVER_H
