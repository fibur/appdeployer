#include "AppDeployer.h"

#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>
#include <QIcon>
#include <QPainter>
#include <QRandomGenerator>
#include <QSystemTrayIcon>
#include <QMenu>

#include "networking/DeployerClient.h"
#include "networking/DeployerServer.h"

namespace  {
constexpr ushort k_defaultPort = 8129;
}

AppDeployer::AppDeployer(int argc, char *argv[]) : QApplication(argc, argv)
{
    setApplicationName("AppDeployer");
    setApplicationVersion("1.0");

    m_parser.setApplicationDescription("Network app deployer");
    m_parser.addHelpOption();
    m_parser.addVersionOption();
    m_parser.addOption(QCommandLineOption({"p", "port"}, "Server port", "port"));
    m_parser.addOption(QCommandLineOption({"s", "source"}, "Source exe file path", "source"));
    m_parser.addOption(QCommandLineOption({"r", "remote"}, "Remote server address", "remote"));

    m_trayIcon = new QSystemTrayIcon(randomIcon(64, 64), this);
    m_trayIcon->setToolTip("AppDeployer");
    m_trayMenu = new QMenu("AppDeployer");
    m_trayMenu->addAction("Exit", this, &QCoreApplication::quit);
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setVisible(true);
}

AppDeployer::~AppDeployer()
{
    m_trayMenu->deleteLater();
}

int AppDeployer::exec()
{
    m_parser.process(*this);
    ushort port;
    bool portIsOk = false;
    if (m_parser.isSet("port")) {
        port = m_parser.value("port").toUShort(&portIsOk);
    }

    if (!portIsOk) {
        qWarning() << "Provided port is not valid. Using default port";
        port = k_defaultPort;
    }

    qInfo() << "Using port:" << port;

    if (m_parser.isSet("source") && m_parser.isSet("remote")) {
        qInfo() << "Running in client mode.";
        QString sourceFile = m_parser.value("source");
        QString host = m_parser.value("remote");

        if (host.indexOf(":") != -1) {
            qWarning() << "Provided host address contains ':' symbol. Taking first part of hostname.";
            host = host.split(":")[0];
        }

        qInfo() << "Remote hostname" << host;

        QFileInfo fileInfo(sourceFile);

        if (!fileInfo.exists()) {

            fileInfo.setFile(QDir::currentPath(), sourceFile);
            if (!fileInfo.exists()) {
                qCritical() << "Provided source file doesn't exist." << fileInfo.absoluteFilePath();
                return 1;
            } else {
                sourceFile = QDir::currentPath() + QDir::separator() + sourceFile;
            }
        }

        if (!fileInfo.isFile() || fileInfo.completeSuffix() != "exe") {
            qCritical() << "Provided source is not an exe file.";
            return 1;
        }


        qInfo() << "Target file:" << sourceFile;

        qInfo() << "Setting up connection";
        m_deployerClient = new DeployerClient(host, port, sourceFile, this);
        m_deployerClient->setupSocket();

        if (!m_deployerClient->socket()) {
            return -1;
        }
    } else {
        m_trayIcon->showMessage("AppDeployer", "Running server in serverMode", QSystemTrayIcon::Information, 1000);
        qInfo() << "Running in server mode.";
        m_deployerServer = new DeployerServer(port, this);
        qInfo() << "Setting up server...";
        m_deployerServer->startupServer();
    }

    return QCoreApplication::exec();
}

QIcon AppDeployer::randomIcon(int w, int h)
{
    QPixmap pixmap(w, h);
    pixmap.fill(Qt::transparent);
    QPainter p (&pixmap);
    QRandomGenerator *rand = QRandomGenerator::global();

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            const QColor color = QColor(rand->generate() % 255,
                                        rand->generate() % 255,
                                        rand->generate() % 255);
            p.setPen(color);
            p.drawPoint(i, j);
        }
    }

    return QIcon(pixmap);
}
