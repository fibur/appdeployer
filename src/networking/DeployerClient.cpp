#include "DeployerClient.h"
#include "DeployerMessage.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QHostInfo>
#include <QProcess>
#include <iostream>

namespace  {
const QString k_consoleWhite = "\u001b[38;5;230m";
const QString k_consoleYellow = "\u001b[38;5;220m";
const QString k_consoleGray = "\u001b[38;5;250m";
const QString k_consoleReset = "\u001b[0m";
const QString k_consoleRed = "\u001b[38;5;203m";
}

DeployerClient::DeployerClient(const QString &host, unsigned short port, const QString &targetFile, QCoreApplication *parent)
    : NetworkUserBase(parent)
    , m_sourceFile(targetFile)
    , m_hostName(host)
    , m_port(port)
{
}

void DeployerClient::setupSocket()
{
    if (!m_hostName.isEmpty()) {
        qInfo() << "Checking hostname...";
        QHostInfo info = QHostInfo::fromName(m_hostName);
        if (info.error() != QHostInfo::NoError) {
            qCritical() << info.errorString();
            return;
        }

        setBaseSocket(new QTcpSocket(this));
        connect(qApp, &QCoreApplication::aboutToQuit, baseSocket(), &QTcpSocket::close);
        connect(baseSocket(), &QTcpSocket::connected, this, &DeployerClient::onConnected);
        connect(baseSocket(), &QTcpSocket::disconnected, this, &DeployerClient::onDisconnected);
        connect(baseSocket(), &QTcpSocket::errorOccurred, this, &DeployerClient::onErrorOccurred);
        connect(baseSocket(), &QTcpSocket::readyRead, this, &DeployerClient::loadBytes);
        qInfo() << "Connecting to host...";
        m_host = info.addresses().at(0);
        baseSocket()->connectToHost(m_host, m_port);
    }
}

QTcpSocket *DeployerClient::socket() const
{
    return baseSocket();
}

void DeployerClient::onConnected()
{
    qInfo() << "Connected to the server. Beginning deploy operations...";
    killApp();
}

void DeployerClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    qCritical() << baseSocket()->errorString();

    if (socketError == QAbstractSocket::ConnectionRefusedError) {
        qInfo() << "Connection was refused. The possible cause may be:";
        qInfo() << "- Server not running";
        qInfo() << "- Other client is connected to the server";
    }

    qApp->exit(1);
}

void DeployerClient::onDisconnected()
{
    qInfo() << "Disconnected from the server";
    qApp->exit(0);
}

void DeployerClient::onMessageReceived(const DeployerMessage &message)
{
    QDebug info = qInfo();

    info.noquote();
    info.nospace();

    switch (message.type()) {
    case DeployerMessage::Executable: {
        qint32 size = message.value().toUInt();

        if (size != m_fileSize) {
            qCritical() << "Written bytes does not match sent";
            qApp->exit(1);
        } else {
            info << "File sent successfully.";
            runApp();
        }
        break;
    }
    case DeployerMessage::KillApp:
        if (message.value().toBool()) {
            info << "App killed successfully.";
        } else {
            info << "App was not running.";
        }

        sendExe();
        break;
    case DeployerMessage::ServerMessage:
        info << formatOutput(message.value().toString());
        break;
    case DeployerMessage::AppLog:
        info << formatOutput(message.value().toString());
        break;
    default:
        break;
    }
}

void DeployerClient::runApp()
{
    qInfo() << "Running app...";
    sendMessage(DeployerMessage(DeployerMessage::RunApp, true));
}

void DeployerClient::killApp()
{
    qInfo() << "Killing remote app (if running)...";
    QFileInfo info(m_sourceFile);

    sendMessage(DeployerMessage(DeployerMessage::KillApp, info.fileName()));
}

void DeployerClient::sendExe()
{
    QFile *file = new QFile(m_sourceFile, this);

    file->open(QIODevice::ReadOnly);
    m_fileSize = file->size();
    const QByteArray &data = fileToQByteArray(file);

    file->close();
    file->deleteLater();

    qInfo() << "Sending executable to host...";

    sendMessage(DeployerMessage(DeployerMessage::Executable, data));
}

QString DeployerClient::formatOutput(const QString &output)
{
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    QString result = "";
    QString copy = output;
    QTextStream stream(&copy);

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QString temp = line.right(line.length() - 9); //removing "xx:xx:xx " characters
        QString color = k_consoleGray;

        if (temp.startsWith("[CRITICAL]")) {
            color = k_consoleRed;
        } else if (temp.startsWith("[INFO]")) {
            color = k_consoleWhite;
        } else if (temp.startsWith("[WARNING]")) {
            color = k_consoleYellow;
        }

        // Reformating message, to:
        // 1) make it more readible
        // 2) allow user to click file links, to access files
        // In order to achieve that we need to split string by ' ' and reorganise it
        QStringList splitted = temp.split(" ");
        if (splitted.length() < 3) {
            continue;
        }

        QString sourceFile = splitted[2].remove(')');

        // Here we also reformat file url, because initially it's not working
        if (sourceFile == "::-1") {
            sourceFile = "";
        } else {
            QStringList sourceFileSplitted = sourceFile.split(":");
            if (sourceFileSplitted.length() < 4) {
                continue;
            }
            sourceFile = sourceFileSplitted[0] + ":" + sourceFileSplitted[1]
                    + ":" + sourceFileSplitted[3] + " (" + sourceFileSplitted[2] + "):";
        }

        splitted.removeAt(1);
        splitted.removeAt(1);
        splitted.removeAt(1);
        splitted.insert(1, sourceFile);

        temp = splitted.join(" ");

        temp = temp.remove('\n');
        temp = temp.remove('\r');
        temp = temp.remove('\t');
        temp = temp.simplified();

        if (temp.isEmpty() || temp.length() < 7) {
            continue;
        }

        result += color + temp + k_consoleReset;
    }

    return result.simplified();
}
