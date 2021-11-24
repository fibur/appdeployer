#ifndef DEPLOYERCLIENT_H
#define DEPLOYERCLIENT_H

#include "NetworkUserBase.h"

#include <QHostAddress>
#include <QObject>
#include <QTcpSocket>

/*!
    \class DeployerClient
    \brief The DeployerClient class wrapps QTcpSockets, and manages deployment process communication with server.
!*/
class DeployerClient : public NetworkUserBase
{
    Q_OBJECT

public:
    explicit DeployerClient(const QString &host, ushort port, const QString &targetFile, QCoreApplication *parent = nullptr);

    void setupSocket();
    QTcpSocket *socket() const;

protected:
    void onMessageReceived(const DeployerMessage &message) override;

private slots:
    void onConnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onDisconnected();

private:
    void runApp();
    void killApp();
    void sendExe();
    QString formatOutput(const QString &output);

private:
    QString m_sourceFile;
    QString m_hostName;
    qint32 m_fileSize = 0;
    QHostAddress m_host;
    ushort m_port;
};

#endif // DEPLOYERCLIENT_H
