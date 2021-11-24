#include "DeployerMessage.h"

DeployerMessage::DeployerMessage()
{
}

DeployerMessage::DeployerMessage(MessageType type, const QVariant &value) :
    m_type(type),
    m_value(value)
{
}

const DeployerMessage::MessageType DeployerMessage::type() const
{
    return m_type;
}

const QVariant &DeployerMessage::value() const
{
    return m_value;
}
