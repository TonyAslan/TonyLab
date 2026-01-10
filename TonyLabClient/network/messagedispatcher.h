#pragma once

#include <QObject>
#include "third_party/nlohmann_json/include/nlohmann/json.hpp"
#include <unordered_map>
#include <functional>
#include <memory>

using json = nlohmann::json;

// 消息处理回调类型
using MessageHandler = std::function<void(const json&)>;

/**
 * @brief 消息分发器
 * 根据消息类型或模块路由消息到对应的处理器
 */
class MessageDispatcher : public QObject
{
    Q_OBJECT
    
public:
    explicit MessageDispatcher(QObject* parent = nullptr);
    ~MessageDispatcher();
    
    /**
     * @brief 注册消息处理器
     * @param msgType 消息类型（如 "auth.login", "im.message"）
     * @param handler 处理函数
     */
    void registerHandler(const std::string& msgType, MessageHandler handler);
    
    /**
     * @brief 移除消息处理器
     * @param msgType 消息类型
     */
    void unregisterHandler(const std::string& msgType);
    
    /**
     * @brief 分发消息
     * @param message JSON 消息对象
     */
    void dispatch(const json& message);
    
    /**
     * @brief 获取已注册的消息类型列表
     */
    std::vector<std::string> getRegisteredTypes() const;
    
signals:
    /// 消息分发时发出错误
    void dispatchError(const QString& msgType, const QString& error);
    
    /// 消息分发成功
    void messageDispatched(const QString& msgType);
    
private:
    std::unordered_map<std::string, MessageHandler> m_handlers;
};
