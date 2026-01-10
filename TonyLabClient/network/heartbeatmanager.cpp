#include "HeartbeatManager.h"
#include "WebSocketClient.h"
#include <chrono>
#include <QDebug>
#include <QTimerEvent>

HeartbeatManager::HeartbeatManager(WebSocketClient* client, QObject* parent)
    : QObject(parent)
    , m_webSocketClient(client)
{
    if (!client) {
        qCritical() << "WebSocketClient is null";
        return;
    }
    
    // 设置默认心跳包
    m_heartbeatPacket = {
        {"type", "heartbeat"},
        {"timestamp", 0}
    };
    
    // 监听客户端断开连接事件
    connect(m_webSocketClient, &WebSocketClient::disconnected,
            this, &HeartbeatManager::onClientDisconnected);
}

HeartbeatManager::~HeartbeatManager()
{
    stop();
}

void HeartbeatManager::start(int interval)
{
    if (m_timerId != -1) {
        qWarning() << "Heartbeat already running";
        return;
    }
    
    if (!m_webSocketClient->isConnected()) {
        qWarning() << "WebSocket client is not connected";
        return;
    }
    
    m_interval = interval;
    m_missedCount = 0;
    
    qDebug() << "Starting heartbeat, interval:" << interval << "ms";
    m_timerId = startTimer(interval);
}

void HeartbeatManager::stop()
{
    if (m_timerId != -1) {
        killTimer(m_timerId);
        m_timerId = -1;
        qDebug() << "Heartbeat stopped";
    }
}

void HeartbeatManager::setHeartbeatPacket(const json& heartbeat)
{
    m_heartbeatPacket = heartbeat;
    qDebug() << "Heartbeat packet updated:" << QString::fromStdString(heartbeat.dump());
}

void HeartbeatManager::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timerId) {
        if (!m_webSocketClient->isConnected()) {
            m_missedCount++;
            qWarning() << "WebSocket not connected, missed heartbeats:" << m_missedCount;
            
            // 如果连续丢失 3 个心跳，认为连接已断开
            if (m_missedCount >= 3) {
                emit heartbeatTimeout();
                stop();
            }
            return;
        }
        
        try {
            // 更新时间戳
            auto packet = m_heartbeatPacket;
            packet["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
            
            m_webSocketClient->sendMessage(packet);
            m_missedCount = 0;
            
            emit heartbeatSent();
            qDebug() << "Heartbeat sent";
        } catch (const std::exception& e) {
            qCritical() << "Failed to send heartbeat:" << e.what();
            m_missedCount++;
        }
    } else {
        QObject::timerEvent(event);
    }
}

void HeartbeatManager::onClientDisconnected()
{
    qDebug() << "WebSocket client disconnected, stopping heartbeat";
    stop();
}

void HeartbeatManager::onHeartbeatTimeout()
{
    emit heartbeatTimeout();
    stop();
}
