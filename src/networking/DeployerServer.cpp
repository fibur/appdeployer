#include "DeployerServer.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#include <string>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QProcess>

#include "DeployerMessage.h"

DeployerServer::DeployerServer(ushort port, QObject *parent) : NetworkUserBase(parent), m_port(port)
{
}

void DeployerServer::startupServer()
{
    m_server = new QTcpServer();
    m_server->setMaxPendingConnections(1);
    connect(m_server, &QTcpServer::newConnection, this, &DeployerServer::onConnection);
    if (m_server->listen(QHostAddress::Any, m_port)) {
        qInfo() << "Listening on port" << m_port;
    } else {
        qCritical() << "Couldn't start server";
        qApp->exit(1);
    }
}

void DeployerServer::onConnection()
{
    QTcpSocket *connection = m_server->nextPendingConnection();

    if (baseSocket() || m_appIsRunning) {
        sendMessage(DeployerMessage(DeployerMessage::ServerMessage, "Another client connected."), connection);
        connection->deleteLater();
        return;
    }

    setBaseSocket(connection);
    connect(connection, &QTcpSocket::readyRead, this, &DeployerServer::loadBytes);
    connect(connection, &QTcpSocket::disconnected, this, &DeployerServer::onClientDisconnected);
    qInfo() << "Client connected";
}

void DeployerServer::onError()
{
    qCritical() << m_server->errorString();
}

void DeployerServer::onMessageReceived(const DeployerMessage &message)
{
    switch (message.type()) {
    case DeployerMessage::KillApp: {
        const QString &exe = message.value().toString();
        bool wasRunning = killApp(exe);
        m_exeName = exe;
        sendMessage(DeployerMessage(DeployerMessage::KillApp, wasRunning));
        break;
    }

    case DeployerMessage::Executable: {
        QFile file(m_exeName);

        file.open(QIODevice::ReadWrite);

        file.resize(0);
        file.write(message.value().toByteArray());
        file.waitForBytesWritten(-1);

        sendMessage(DeployerMessage(DeployerMessage::Executable, file.size()));
        file.close();
    }

    case DeployerMessage::RunApp: {
        if (!m_process) {
            qInfo() << "Starting app";
            startApp();
        } else {
            qCritical() << "App is running already";
        }
        break;
    }
    default:
        sendMessage(DeployerMessage(DeployerMessage::ServerMessage, "Message received :)"));
    }
}

void DeployerServer::onClientDisconnected()
{
    qInfo() << "Client disconnected";

    killApp(m_exeName);

    cleanUp();

    qInfo() << "Rerunning server...";

    m_server->close();

    m_server->deleteLater();
    startupServer();
}

void DeployerServer::appClosed(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    if (baseSocket()){
        sendMessage(DeployerMessage(DeployerMessage::ServerMessage, "App has been closed with return value " + QString::number(exitCode)));
    }

    cleanUp();
}

void DeployerServer::logFile(const QString &path)
{
    if (!m_logFile) {
        m_logFile = new QFile(path, this);
        m_logFile->open(QIODevice::ReadWrite);
    }

    QByteArray data;

    data = m_logFile->readAll();

    if (data.size() >= 7) {
        sendMessage(DeployerMessage(DeployerMessage::AppLog, data));
    }
    m_logFile->resize(0);
}

bool DeployerServer::killApp(const QString &executable)
{
    if (m_process) {
        m_process->terminate();
        return true;
    }

    bool closedAny = false;

#ifdef Q_OS_WIN
    std::string exe = executable.toStdString();

    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes) {
        if (strcmp(pEntry.szExeFile, exe.c_str()) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL) {
                if (TerminateProcess(hProcess, 9)) {
                    closedAny = true;
                }

                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }

    CloseHandle(hSnapShot);
#endif

    return closedAny;
}

void DeployerServer::startApp()
{
    const QString filePath = QDir::currentPath() + QDir::separator() + m_exeName;

    m_process = new QProcess();
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &DeployerServer::appClosed);

    m_process->start(filePath, {""});
    m_appIsRunning = true;
    m_logWatcher = new QFileSystemWatcher({QDir::currentPath() + QDir::separator() + "logs" + QDir::separator() + "latest.log"}, this);
    connect(m_logWatcher, &QFileSystemWatcher::fileChanged, this, &DeployerServer::logFile);
}

void DeployerServer::cleanUp()
{
    if (baseSocket()){
        baseSocket()->close();
        baseSocket()->deleteLater();
    }
    setBaseSocket(nullptr);

    m_exeName = "";

    if (m_process) {
        if (m_process->isOpen()) {
            m_process->terminate();
            m_process->waitForFinished();
        }
        delete m_process;
        m_process = nullptr;
    }

    if (m_logWatcher) {
        m_logWatcher->deleteLater();
        m_logWatcher = nullptr;
    }

    if (m_logFile) {
        if (m_logFile->isOpen()) {
            m_logFile->close();
        }
        m_logFile->deleteLater();
        m_logFile = nullptr;
    }

    m_appIsRunning = false;
}
