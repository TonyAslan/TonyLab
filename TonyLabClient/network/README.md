# 网络模块使用指南

## 模块概述

网络模块提供了现代化的 WebSocket 通信框架，基于 Qt5 和 nlohmann/json：

- **WebSocketClient** - WebSocket 连接管理和通信
- **MessageDispatcher** - 消息路由和分发
- **HeartbeatManager** - 心跳保活管理

## WebSocketClient 使用示例

### 基础连接

```cpp
#include "network/WebSocketClient.h"

// 创建客户端
auto client = new WebSocketClient(this);

// 监听信号
connect(client, &WebSocketClient::connected, this, []{
    qDebug() << "Connected to server";
});

connect(client, &WebSocketClient::disconnected, this, []{
    qDebug() << "Disconnected from server";
});

connect(client, &WebSocketClient::messageReceived, this, 
    [](const json& msg){
        qDebug() << "Received:" << QString::fromStdString(msg.dump());
    });

// 启用自动重连
client->setAutoReconnect(true, 5000);  // 5秒重连一次

// 连接服务器
client->connectToServer("ws://localhost:8080");
```

### 发送消息

```cpp
// 发送 JSON 消息
json message = {
    {"type", "auth.login"},
    {"username", "user123"},
    {"password", "pass123"}
};

client->sendMessage(message);

// 发送原始数据
QByteArray rawData = "...";
client->sendRawData(rawData);
```

## MessageDispatcher 使用示例

### 注册消息处理器

```cpp
#include "network/MessageDispatcher.h"

auto dispatcher = new MessageDispatcher(this);

// 为 "auth.login" 消息注册处理器
dispatcher->registerHandler("auth.login", [this](const json& msg){
    std::string username = msg["username"];
    std::string password = msg["password"];
    qDebug() << "Login attempt:" << QString::fromStdString(username);
    // 处理登录逻辑...
});

// 为 "im.message" 消息注册处理器
dispatcher->registerHandler("im.message", [this](const json& msg){
    std::string content = msg["content"];
    qDebug() << "Chat message:" << QString::fromStdString(content);
    // 处理聊天消息...
});

// 连接客户端的 messageReceived 信号到分发器
connect(client, &WebSocketClient::messageReceived,
        dispatcher, &MessageDispatcher::dispatch);
```

### 使用 module.action 格式

若消息格式为：
```json
{
    "module": "auth",
    "action": "login",
    "username": "user123"
}
```

处理器会自动转换为 `"auth.login"` 类型，你可以：

```cpp
dispatcher->registerHandler("auth.login", [](const json& msg){
    // 处理...
});
```

## HeartbeatManager 使用示例

```cpp
#include "network/HeartbeatManager.h"

auto heartbeat = new HeartbeatManager(client, this);

// 监听心跳事件
connect(heartbeat, &HeartbeatManager::heartbeatTimeout, this, []{
    qWarning() << "Heartbeat timeout - connection may be lost";
});

connect(heartbeat, &HeartbeatManager::heartbeatSent, this, []{
    qDebug() << "Heartbeat sent";
});

// 自定义心跳包格式
json customHeartbeat = {
    {"type", "heartbeat"},
    {"client_id", "client_001"},
    {"version", "1.0"}
};
heartbeat->setHeartbeatPacket(customHeartbeat);

// 启动心跳（每 30 秒发送一次）
heartbeat->start(30000);

// 在适当时候停止
heartbeat->stop();

// 检查丢失的心跳数
int missedCount = heartbeat->getMissedHeartbeats();
```

## 完整集成示例

```cpp
#include "network/WebSocketClient.h"
#include "network/MessageDispatcher.h"
#include "network/HeartbeatManager.h"

class NetworkManager : public QObject
{
    Q_OBJECT
    
public:
    explicit NetworkManager(QObject* parent = nullptr)
        : QObject(parent)
        , m_client(new WebSocketClient(this))
        , m_dispatcher(new MessageDispatcher(this))
        , m_heartbeat(new HeartbeatManager(m_client, this))
    {
        setupConnections();
        setupMessageHandlers();
    }
    
    void connect(const QString& serverUrl)
    {
        m_client->setAutoReconnect(true, 5000);
        m_client->connectToServer(serverUrl);
    }
    
    void sendMessage(const json& msg)
    {
        m_client->sendMessage(msg);
    }
    
private:
    void setupConnections()
    {
        // 连接消息信号到分发器
        connect(m_client, &WebSocketClient::messageReceived,
                m_dispatcher, &MessageDispatcher::dispatch);
        
        // 监听连接状态
        connect(m_client, &WebSocketClient::connected, this, [this]{
            qInfo() << "Network connected";
            m_heartbeat->start(30000);  // 启动心跳
        });
        
        connect(m_client, &WebSocketClient::disconnected, this, [this]{
            qInfo() << "Network disconnected";
            m_heartbeat->stop();
        });
    }
    
    void setupMessageHandlers()
    {
        // 登录消息处理
        m_dispatcher->registerHandler("auth.login", [this](const json& msg){
            handleAuthLogin(msg);
        });
        
        // 聊天消息处理
        m_dispatcher->registerHandler("im.message", [this](const json& msg){
            handleChatMessage(msg);
        });
        
        // 设备控制消息处理
        m_dispatcher->registerHandler("device.control", [this](const json& msg){
            handleDeviceControl(msg);
        });
    }
    
    void handleAuthLogin(const json& msg)
    {
        // 处理登录...
    }
    
    void handleChatMessage(const json& msg)
    {
        // 处理聊天消息...
    }
    
    void handleDeviceControl(const json& msg)
    {
        // 处理设备控制命令...
    }
    
private:
    WebSocketClient* m_client;
    MessageDispatcher* m_dispatcher;
    HeartbeatManager* m_heartbeat;
};
```

## 依赖安装

### nlohmann/json

#### Windows (vcpkg)
```bash
vcpkg install nlohmann-json:x64-windows
```

#### CMake (自动)
如果 CMakeLists.txt 未找到 nlohmann/json，会自动使用 `third_party/nlohmann_json/` 中的版本。

### Qt5 WebSockets

在 Qt 安装时选择 WebSockets 组件，或通过包管理器安装：

```bash
# Windows (vcpkg)
vcpkg install qt5-websockets:x64-windows
```

## 编译

```bash
cd build
cmake ..
cmake --build . --config Release
```

## 注意事项

1. **线程安全** - WebSocketClient 的所有操作必须在创建它的线程中进行
2. **内存管理** - 建议将 WebSocketClient、MessageDispatcher 等对象作为主窗口的成员，由 Qt 的父子关系自动管理
3. **错误处理** - 监听 `error` 信号以捕获所有通信错误
4. **JSON 格式** - 确保服务器发送的消息都是有效的 JSON，或实现自定义处理逻辑
5. **心跳超时** - 如果丢失 3 个连续心跳，HeartbeatManager 会自动停止

## 架构设计优势

- ✅ 现代 C++ (C++17) 设计
- ✅ 基于信号槽的解耦通信
- ✅ 自动化消息路由和分发
- ✅ 内置心跳保活机制
- ✅ 灵活的 JSON 支持 (nlohmann/json)
- ✅ 易于扩展和维护
