#pragma once

#include <QObject>
#include <QWebSocket>
#include <memory>
#include "third_party/nlohmann_json/include/nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @brief WebSocket 通信客户端
 * 负责 WebSocket 连接、发送和接收消息
 */
class WebSocketClient : public QObject
{
    Q_OBJECT
    
public:
    explicit WebSocketClient(QObject* parent = nullptr);
    ~WebSocketClient();
    
    /**
     * @brief 连接到服务器
     * @param url WebSocket 服务器地址（如 ws://localhost:8080）
     */
    void connectToServer(const QString& url);
    
    /**
     * @brief 断开连接
     */
    void disconnect();
    
    /**
     * @brief 发送 JSON 消息
     * @param message JSON 对象
     */
    void sendMessage(const json& message);
    
    /**
     * @brief 发送原始数据
     * @param data 字节数据
     */
    void sendRawData(const QByteArray& data);
    
    /**
     * @brief 是否已连接
     */
    bool isConnected() const;
    
    /**
     * @brief 设置自动重连
     * @param enable 是否启用
     * @param interval 重连间隔（毫秒）
     */
    void setAutoReconnect(bool enable, int interval = 5000);
    
signals:
    /// 连接成功
    void connected();
    
    /// 连接断开
    void disconnected();
    
    /// 连接出错
    void error(const QString& errorMsg);
    
    /// 接收到 JSON 消息
    void messageReceived(const json& message);
    
    /// 接收到原始数据
    void dataReceived(const QByteArray& data);
    
    /// 连接状态改变
    void connectionStateChanged(bool connected);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& data);
    void onAutoReconnectTimeout();
    
private:
    void setupConnections();
    void startAutoReconnectTimer();
    void stopAutoReconnectTimer();
    
    // Timer event for auto-reconnect
    void timerEvent(QTimerEvent* event) override;
    
    std::unique_ptr<QWebSocket> m_webSocket;
    QString m_serverUrl;
    bool m_isConnected = false;
    bool m_autoReconnect = false;
    int m_reconnectInterval = 5000;
    int m_reconnectTimerId = -1;
};
