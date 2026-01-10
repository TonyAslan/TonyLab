#include "ChatService.h"
#include "network/WebSocketClient.h"
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

ChatService::ChatService(WebSocketClient* wsClient, QObject* parent)
    : QObject(parent)
    , m_webSocketClient(wsClient)
{
    if (!wsClient) {
        qCritical() << "WebSocketClient is null";
        return;
    }
    
    // 连接消息接收信号
    connect(m_webSocketClient, &WebSocketClient::messageReceived,
            this, &ChatService::onMessageReceived);
    
    connect(m_webSocketClient, &WebSocketClient::disconnected,
            this, &ChatService::onWebSocketDisconnected);
}

ChatService::~ChatService()
{
}

void ChatService::setCurrentUser(const QString& userId, const QString& userName, const QString& avatar)
{
    m_currentUserId = userId;
    m_currentUserName = userName;
    m_currentUserAvatar = avatar;
}

void ChatService::sendTextMessage(const QString& receiverId, const QString& content)
{
    if (m_currentUserId.isEmpty()) {
        qWarning() << "Current user not set";
        emit errorOccurred("User not authenticated");
        return;
    }
    
    if (!m_webSocketClient->isConnected()) {
        qWarning() << "WebSocket not connected, message will be sent when connected";
        // 缓存消息
        Message msg;
        msg.id = QUuid::createUuid().toString();
        msg.senderId = m_currentUserId;
        msg.senderName = m_currentUserName;
        msg.senderAvatar = m_currentUserAvatar;
        msg.receiverId = receiverId;
        msg.content = content;
        msg.type = "text";
        msg.timestamp = QDateTime::currentDateTime();
        msg.isSent = false;
        m_pendingMessages.append(msg);
        return;
    }
    
    try {
        json message = {
            {"type", "im.message"},
            {"action", "send"},
            {"messageId", QUuid::createUuid().toString().toStdString()},
            {"senderId", m_currentUserId.toStdString()},
            {"senderName", m_currentUserName.toStdString()},
            {"senderAvatar", m_currentUserAvatar.toStdString()},
            {"receiverId", receiverId.toStdString()},
            {"content", content.toStdString()},
            {"contentType", "text"},
            {"timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch()}
        };
        
        m_webSocketClient->sendMessage(message);
        qDebug() << "Message sent to" << receiverId;
        
    } catch (const std::exception& e) {
        qCritical() << "Error sending message:" << e.what();
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ChatService::sendFile(const QString& receiverId, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open file: " + filePath);
        return;
    }
    
    QFileInfo fileInfo(filePath);
    QByteArray fileData = file.readAll();
    file.close();
    
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json message = {
            {"type", "im.file"},
            {"action", "send"},
            {"senderId", m_currentUserId.toStdString()},
            {"receiverId", receiverId.toStdString()},
            {"fileName", fileInfo.fileName().toStdString()},
            {"fileSize", fileData.size()},
            {"timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch()}
        };
        
        // 先发送文件元数据
        m_webSocketClient->sendMessage(message);
        // 然后发送文件内容
        m_webSocketClient->sendRawData(fileData);
        
        qDebug() << "File sent to" << receiverId;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ChatService::fetchMessageHistory(const QString& contactId, int limit)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json request = {
            {"type", "im.history"},
            {"action", "fetch"},
            {"contactId", contactId.toStdString()},
            {"limit", limit}
        };
        
        m_webSocketClient->sendMessage(request);
        qDebug() << "Fetching message history for" << contactId;
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ChatService::markMessageAsRead(const QString& messageId)
{
    if (!m_webSocketClient->isConnected()) {
        return;
    }
    
    try {
        json message = {
            {"type", "im.ack"},
            {"action", "read"},
            {"messageId", messageId.toStdString()},
            {"userId", m_currentUserId.toStdString()}
        };
        
        m_webSocketClient->sendMessage(message);
        emit messageReadStatusChanged(messageId);
        
    } catch (const std::exception& e) {
        qWarning() << "Error marking message as read:" << e.what();
    }
}

void ChatService::notifyTyping(const QString& targetId, bool isTyping)
{
    if (!m_webSocketClient->isConnected()) {
        return;
    }
    
    try {
        json notification = {
            {"type", "im.typing"},
            {"userId", m_currentUserId.toStdString()},
            {"targetId", targetId.toStdString()},
            {"isTyping", isTyping}
        };
        
        m_webSocketClient->sendMessage(notification);
        
    } catch (const std::exception& e) {
        qWarning() << "Error sending typing notification:" << e.what();
    }
}

void ChatService::onMessageReceived(const json& message)
{
    try {
        if (!message.contains("type")) {
            return;
        }
        
        std::string msgType = message["type"];
        
        if (msgType == "im.message") {
            handleIncomingMessage(message);
        } else if (msgType == "im.ack") {
            handleMessageAck(message);
        } else if (msgType == "im.typing") {
            handleTypingNotification(message);
        } else if (msgType == "im.history") {
            handleHistoryResponse(message);
        }
        
    } catch (const std::exception& e) {
        qWarning() << "Error processing message:" << e.what();
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ChatService::handleIncomingMessage(const json& data)
{
    try {
        Message msg = Message::fromJson(data);
        m_messages.append(msg);
        emit messageReceived(msg);
        
        // 自动标记为已读
        markMessageAsRead(msg.id);
        
        qDebug() << "Message received from" << msg.senderName;
        
    } catch (const std::exception& e) {
        qWarning() << "Error handling incoming message:" << e.what();
    }
}

void ChatService::handleMessageAck(const json& data)
{
    try {
        if (data.contains("messageId")) {
            QString messageId = QString::fromStdString(data["messageId"]);
            emit messageSent(messageId);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error handling message ack:" << e.what();
    }
}

void ChatService::handleTypingNotification(const json& data)
{
    try {
        if (data.contains("userId") && data.contains("isTyping")) {
            QString userId = QString::fromStdString(data["userId"]);
            bool isTyping = data["isTyping"];
            emit typingStatusChanged(userId, isTyping);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error handling typing notification:" << e.what();
    }
}

void ChatService::handleHistoryResponse(const json& data)
{
    try {
        QList<Message> historyMessages;
        
        if (data.contains("messages") && data["messages"].is_array()) {
            for (const auto& msgJson : data["messages"]) {
                Message msg = Message::fromJson(msgJson);
                historyMessages.append(msg);
                m_messages.append(msg);
            }
        }
        
        emit historyLoaded(historyMessages);
        qDebug() << "Loaded" << historyMessages.count() << "messages from history";
        
    } catch (const std::exception& e) {
        qWarning() << "Error handling history response:" << e.what();
    }
}

void ChatService::onWebSocketDisconnected()
{
    qDebug() << "WebSocket disconnected";
}
