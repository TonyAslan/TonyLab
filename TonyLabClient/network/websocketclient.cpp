#include "WebSocketClient.h"
#include <QDebug>
#include <QTimer>
#include <QTimerEvent>
#include <QUrl>

WebSocketClient::WebSocketClient(QObject* parent)
    : QObject(parent)
    , m_webSocket(std::make_unique<QWebSocket>())
{
    setupConnections();
}

WebSocketClient::~WebSocketClient()
{
    if (m_webSocket) {
        m_webSocket->close();
    }
}

void WebSocketClient::setupConnections()
{
    connect(m_webSocket.get(), &QWebSocket::connected, 
            this, &WebSocketClient::onConnected);
    
    connect(m_webSocket.get(), &QWebSocket::disconnected, 
            this, &WebSocketClient::onDisconnected);
    
    connect(m_webSocket.get(), 
            static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            this, &WebSocketClient::onError);
    
    connect(m_webSocket.get(), &QWebSocket::textMessageReceived, 
            this, &WebSocketClient::onTextMessageReceived);
    
    connect(m_webSocket.get(), &QWebSocket::binaryMessageReceived, 
            this, &WebSocketClient::onBinaryMessageReceived);
}

void WebSocketClient::connectToServer(const QString& url)
{
    m_serverUrl = url;
    
    if (m_webSocket->isValid()) {
        m_webSocket->close();
    }
    
    qDebug() << "Connecting to WebSocket server:" << url;
    m_webSocket->open(QUrl(url));
}

void WebSocketClient::disconnect()
{
    stopAutoReconnectTimer();
    
    if (m_webSocket && m_webSocket->isValid()) {
        m_webSocket->close();
    }
}

void WebSocketClient::sendMessage(const json& message)
{
    if (!m_isConnected) {
        qWarning() << "WebSocket not connected, cannot send message";
        return;
    }
    
    try {
        QString jsonStr = QString::fromStdString(message.dump());
        m_webSocket->sendTextMessage(jsonStr);
    } catch (const std::exception& e) {
        qCritical() << "Failed to send message:" << e.what();
        emit error(QString::fromStdString(e.what()));
    }
}

void WebSocketClient::sendRawData(const QByteArray& data)
{
    if (!m_isConnected) {
        qWarning() << "WebSocket not connected, cannot send data";
        return;
    }
    
    m_webSocket->sendBinaryMessage(data);
}

bool WebSocketClient::isConnected() const
{
    return m_isConnected;
}

void WebSocketClient::setAutoReconnect(bool enable, int interval)
{
    m_autoReconnect = enable;
    m_reconnectInterval = interval;
}

void WebSocketClient::onConnected()
{
    m_isConnected = true;
    stopAutoReconnectTimer();
    
    qInfo() << "WebSocket connected";
    emit connected();
    emit connectionStateChanged(true);
}

void WebSocketClient::onDisconnected()
{
    m_isConnected = false;
    
    qInfo() << "WebSocket disconnected";
    emit disconnected();
    emit connectionStateChanged(false);
    
    if (m_autoReconnect) {
        startAutoReconnectTimer();
    }
}

void WebSocketClient::onError(QAbstractSocket::SocketError error)
{
    QString errorMsg = m_webSocket->errorString();
    qCritical() << "WebSocket error:" << errorMsg;
    emit this->error(errorMsg);
}

void WebSocketClient::onTextMessageReceived(const QString& message)
{
    try {
        json jsonMessage = json::parse(message.toStdString());
        emit messageReceived(jsonMessage);
    } catch (const json::exception& e) {
        qWarning() << "Failed to parse JSON message:" << e.what();
        emit error(QString("JSON parse error: %1").arg(e.what()));
        
        // 仍然转发原始数据
        emit dataReceived(message.toUtf8());
    }
}

void WebSocketClient::onBinaryMessageReceived(const QByteArray& data)
{
    emit dataReceived(data);
}

void WebSocketClient::startAutoReconnectTimer()
{
    if (m_reconnectTimerId == -1) {
        qDebug() << "Starting auto-reconnect timer, interval:" << m_reconnectInterval << "ms";
        m_reconnectTimerId = startTimer(m_reconnectInterval);
    }
}

void WebSocketClient::stopAutoReconnectTimer()
{
    if (m_reconnectTimerId != -1) {
        killTimer(m_reconnectTimerId);
        m_reconnectTimerId = -1;
    }
}

void WebSocketClient::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_reconnectTimerId) {
        onAutoReconnectTimeout();
    } else {
        QObject::timerEvent(event);
    }
}

void WebSocketClient::onAutoReconnectTimeout()
{
    qDebug() << "Auto-reconnecting...";
    connectToServer(m_serverUrl);
}
