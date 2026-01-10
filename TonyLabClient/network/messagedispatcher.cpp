#include "MessageDispatcher.h"
#include <QDebug>

MessageDispatcher::MessageDispatcher(QObject* parent)
    : QObject(parent)
{
}

MessageDispatcher::~MessageDispatcher()
{
    m_handlers.clear();
}

void MessageDispatcher::registerHandler(const std::string& msgType, MessageHandler handler)
{
    if (!handler) {
        qWarning() << "Cannot register null handler for message type:" << QString::fromStdString(msgType);
        return;
    }
    
    m_handlers[msgType] = handler;
    qDebug() << "Handler registered for message type:" << QString::fromStdString(msgType);
}

void MessageDispatcher::unregisterHandler(const std::string& msgType)
{
    auto it = m_handlers.find(msgType);
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
        qDebug() << "Handler unregistered for message type:" << QString::fromStdString(msgType);
    }
}

void MessageDispatcher::dispatch(const json& message)
{
    try {
        // 从消息中提取类型 (假设格式为 {"type": "auth.login", ...} 或 {"module": "auth", "action": "login", ...})
        std::string msgType;
        
        if (message.contains("type") && message["type"].is_string()) {
            msgType = message["type"].get<std::string>();
        } else if (message.contains("module") && message.contains("action")) {
            msgType = message["module"].get<std::string>() + "." + message["action"].get<std::string>();
        } else {
            qWarning() << "Message does not contain 'type' or 'module'/'action' fields";
            emit dispatchError("unknown", "Missing message type");
            return;
        }
        
        auto it = m_handlers.find(msgType);
        if (it != m_handlers.end()) {
            it->second(message);
            qDebug() << "Message dispatched to handler:" << QString::fromStdString(msgType);
            emit messageDispatched(QString::fromStdString(msgType));
        } else {
            qWarning() << "No handler registered for message type:" << QString::fromStdString(msgType);
            emit dispatchError(QString::fromStdString(msgType), "No handler registered");
        }
    } catch (const std::exception& e) {
        qCritical() << "Dispatch error:" << e.what();
        emit dispatchError("unknown", QString::fromStdString(e.what()));
    }
}

std::vector<std::string> MessageDispatcher::getRegisteredTypes() const
{
    std::vector<std::string> types;
    for (const auto& pair : m_handlers) {
        types.push_back(pair.first);
    }
    return types;
}
