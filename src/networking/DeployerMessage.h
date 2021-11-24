#ifndef DEPLOYERMESSAGE_H
#define DEPLOYERMESSAGE_H

#include <QDataStream>
#include <QVariant>

/*!
    \class DeployerMessage
    \brief The DeployerMessage struct wrapps data based on QVariatnt, allowing to send it throght network.
           Each DeployerMessage contains information about it's type, so client/server knows how to read associated data.
!*/
struct DeployerMessage
{
    Q_GADGET

    friend QDataStream &operator<<(QDataStream &stream, const DeployerMessage &out) {
        stream << out.type();
        stream << out.value();
        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, DeployerMessage &in) {
        int type;
        QVariant value;
        stream >> type >> value;

        in.m_type = (DeployerMessage::MessageType) type;
        in.m_value = value;
        return stream;
    }

public:
    enum MessageType {
        General = 0,
        Executable,
        KillApp,
        RunApp,
        ServerMessage,
        AppLog
    };

    DeployerMessage();

    DeployerMessage(MessageType type, const QVariant &value);

    const MessageType type() const;
    const QVariant &value() const;

private:
    MessageType m_type = General;
    QVariant m_value;
};
Q_DECLARE_METATYPE(DeployerMessage)

#endif // DEPLOYERMESSAGE_H
