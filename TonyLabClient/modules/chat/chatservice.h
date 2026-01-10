#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include "third_party/nlohmann_json/include/nlohmann/json.hpp"
#include "MessageModel.h"

using json = nlohmann::json;

class WebSocketClient;

/**
 * @brief 聊天服务
 * 负责聊天消息的发送、接收和管理
 */
class ChatService : public QObject
{
    Q_OBJECT

public:
    explicit ChatService(WebSocketClient* wsClient, QObject* parent = nullptr);
    ~ChatService() override;

    // 消息操作
    void sendTextMessage(const QString& receiverId, const QString& content);
    void sendFile(const QString& receiverId, const QString& filePath);
    void fetchMessageHistory(const QString& contactId, int limit = 50);
    void markMessageAsRead(const QString& messageId);
    void notifyTyping(const QString& targetId, bool isTyping);

    // 用户信息
    void setCurrentUser(const QString& userId, const QString& userName, const QString& avatar);
    QString getCurrentUserId() const { return m_currentUserId; }

    // 消息列表
    const QList<Message>& getMessages() const { return m_messages; }
    void clearMessages() { m_messages.clear(); }

signals:
    void messageReceived(const Message& message);
    void messageSent(const QString& messageId);
    void messageSendFailed(const QString& messageId, const QString& error);
    void messageReadStatusChanged(const QString& messageId);
    void historyLoaded(const QList<Message>& messages);
    void typingStatusChanged(const QString& contactId, bool isTyping);
    void errorOccurred(const QString& errorMsg);

private slots:
    void onMessageReceived(const json& message);
    void onWebSocketDisconnected();

private:
    void handleIncomingMessage(const json& data);
    void handleMessageAck(const json& data);
    void handleTypingNotification(const json& data);
    void handleHistoryResponse(const json& data);

    WebSocketClient* m_webSocketClient = nullptr;
    QString m_currentUserId;
    QString m_currentUserName;
    QString m_currentUserAvatar;
    QList<Message> m_messages;
    QList<Message> m_pendingMessages;
};

