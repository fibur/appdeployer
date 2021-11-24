#include "NetworkUserBase.h"

#include <QCoreApplication>

#include "DeployerMessage.h"

namespace  {

QByteArray intToQByteArray(qint32 source)
{
    QByteArray result;
    QDataStream data(&result, QIODevice::ReadWrite);
    data << source;
    return result;
}

qint32 intFromQByteArray(const QByteArray &array) {
    QDataStream data(array);
    qint32 result;
    data >> result;
    return result;
}

constexpr qint32 k_maxFileSize = 10485760; // 10MB
}

NetworkUserBase::NetworkUserBase(QObject *parent) : QObject(parent)
{
}

void NetworkUserBase::loadBytes()
{
    QByteArray  temp;
    QDataStream stream(m_baseSocket);

    int sizeOfInt = sizeof(qint32);

    while (m_baseSocket->bytesAvailable() > 0) {
        temp.append(m_baseSocket->readAll());
        while (temp.size() > 0) {
            if (m_currentPacket == 0) {
                if (temp.size() < sizeOfInt) {
                    break;
                }
                m_currentPacket = ::intFromQByteArray(temp.left(sizeOfInt));
                temp.remove(0, sizeOfInt);
            }

            qint32 toRemove = m_currentPacket;

            if (toRemove > temp.size()) {
                toRemove = temp.size();
                m_currentPacket -= toRemove;
            } else {
                m_currentPacket = 0;
            }

            if (toRemove != 0) {
                m_buffer.append(temp.left(toRemove));
                temp.remove(0, toRemove);
            }
        }

        temp.clear();
    }

    if (m_currentPacket == 0) {
        DeployerMessage message;

        QDataStream inputStream(&m_buffer, QIODevice::ReadOnly);
        inputStream >> message;

        m_buffer.clear();

        onMessageReceived(message);
    }
}

QByteArray NetworkUserBase::messageToPacket(const DeployerMessage &message)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << message;

    return data;
}

void NetworkUserBase::sendMessage(const DeployerMessage &message)
{
    sendMessage(message, m_baseSocket);
}

void NetworkUserBase::sendMessage(const DeployerMessage &message, QTcpSocket *target)
{
    if (target) {
        QByteArray data = messageToPacket(message);

        QByteArray result;
        result.append(::intToQByteArray(data.size()));
        result.append(data);

        target->write(result);
    }
}

QByteArray NetworkUserBase::fileToQByteArray(QFile *file)
{
    QByteArray temp;
    if (file->isOpen() && file->isReadable()) {
        const qint32 size = file->size();

        if (size > ::k_maxFileSize) {
            qCritical() << "Provided source file is too large";
            qApp->exit(1);
        } else {
            while (file->bytesAvailable()) {
                temp.append(file->readAll());
            }

            return temp;
        }
    }

    return temp;
}

QTcpSocket *NetworkUserBase::baseSocket() const
{
    return m_baseSocket;
}

void NetworkUserBase::setBaseSocket(QTcpSocket *baseSocket)
{
    m_baseSocket = baseSocket;
}
