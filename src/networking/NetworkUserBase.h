#ifndef NETWORKUSERBASE_H
#define NETWORKUSERBASE_H

#include <QDataStream>
#include <QFile>
#include <QTcpSocket>

struct DeployerMessage;

/*!
    \class NetworkUserBase
    \brief The NetworkUserBase is an interface, that allows to read/write DeployerMessages through QTcpSockets.
!*/
class NetworkUserBase : public QObject
{

public:
    explicit NetworkUserBase(QObject *parent = nullptr);

protected:
    /*!
        \brief loadBytes method processes single data packet, and adds read data to the buffer.

        \note If buffer size equals received packet size information, then loadBytes constructs
              DeployerMessage struct instance, and invokes onMessageReceived method.
    !*/
    virtual void loadBytes();

    /*!
        \brief sendMessage converts provided DeplouyerMessage instance to packet, and sends it to target QTcpSocket.
        \param message is a reference to DeployerMessage instance.
        \param target is a pointer to QTcpSocket instance, that will process packet data.
    !*/
    virtual void sendMessage(const DeployerMessage &message, QTcpSocket *target);

    /*!
        \brief This is sendMessage method overload. It uses local baseSocket() to send message.
        \param message is a reference to DeployerMessage instance.
    !*/
    virtual void sendMessage(const DeployerMessage &message);

    /*!
        \brief fileToQByteArray checks, if provided file meets size restrictions, and then converts it to QByteArray.
        \param file is a pointer to opened QFile instance.
    !*/
    virtual QByteArray fileToQByteArray(QFile *file);

    /*!
        \brief onMessageReceived is a method, that receives constructed DeployerMessage when buffer is completed.
               That method should be reimplemented, to handle specific DeployerMessage::MessageType types.
        \param message is a reference to constructed DeployerMessage instance.
    !*/
    virtual void onMessageReceived(const DeployerMessage &message) = 0;

    /*!
        \brief baseSocket is pointer to local QTcpSocket instance.
               Each NetworkUserBase derivates contains it's own QTcpSocket insstance.
               That instance is used in sendMessage/loadBytes methods.
               However initially it's setted to nullptr, so deriving class must create it first.
               To set baseSocket use setBaseSocket() method.
    !*/
    QTcpSocket *baseSocket() const;
    void setBaseSocket(QTcpSocket *baseSocket);

private:
    virtual QByteArray messageToPacket(const DeployerMessage &message);

private:
    QByteArray m_buffer;
    qint32 m_currentPacket = 0;
    QTcpSocket *m_baseSocket = nullptr;
};

#endif // NETWORKUSERBASE_H
