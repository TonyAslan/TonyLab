#pragma once

#include <QObject>
#include "third_party/nlohmann_json/include/nlohmann/json.hpp"
#include <memory>

using json = nlohmann::json;

class WebSocketClient;

/**
 * @brief 心跳管理器
 * 负责定期发送心跳包保活连接
 */
class HeartbeatManager : public QObject
{
    Q_OBJECT
    
public:
    explicit HeartbeatManager(WebSocketClient* client, QObject* parent = nullptr);
    ~HeartbeatManager();
    
    /**
     * @brief 启动心跳
     * @param interval 心跳间隔（毫秒，默认 30000）
     */
    void start(int interval = 30000);
    
    /**
     * @brief 停止心跳
     */
    void stop();
    
    /**
     * @brief 是否运行中
     */
    bool isRunning() const { return m_timerId != -1; }
    
    /**
     * @brief 设置心跳包格式
     * @param heartbeat JSON 格式的心跳包
     */
    void setHeartbeatPacket(const json& heartbeat);
    
    /**
     * @brief 获取丢失的心跳数
     */
    int getMissedHeartbeats() const { return m_missedCount; }
    
    /**
     * @brief 重置丢失计数
     */
    void resetMissedCount() { m_missedCount = 0; }
    
signals:
    /// 心跳超时（可能连接已断开）
    void heartbeatTimeout();
    
    /// 心跳发送成功
    void heartbeatSent();
    
    /// 收到心跳响应
    void heartbeatAcked();
    
private slots:
    void onHeartbeatTimeout();
    void onClientDisconnected();
    
protected:
    void timerEvent(QTimerEvent* event) override;
    
private:
    WebSocketClient* m_webSocketClient;
    int m_timerId = -1;
    int m_interval = 30000;
    int m_missedCount = 0;
    json m_heartbeatPacket;
};
