#pragma once

#include <QString>
#include <QDateTime>
#include "third_party/nlohmann_json/include/nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @brief 消息数据模型
 */
struct Message
{
    QString id;              // 消息ID
    QString senderId;        // 发送者ID
    QString senderName;      // 发送者名称
    QString senderAvatar;    // 发送者头像
    QString receiverId;      // 接收者ID
    QString content;         // 消息内容
    QString type;            // 消息类型: text, image, file
    QDateTime timestamp;     // 时间戳
    bool isSent = false;     // 是否已发送

    json toJson() const
    {
        return json{
            {"id", id.toStdString()},
            {"senderId", senderId.toStdString()},
            {"senderName", senderName.toStdString()},
            {"senderAvatar", senderAvatar.toStdString()},
            {"receiverId", receiverId.toStdString()},
            {"content", content.toStdString()},
            {"type", type.toStdString()},
            {"timestamp", timestamp.toMSecsSinceEpoch()},
            {"isSent", isSent}
        };
    }

    static Message fromJson(const json& j)
    {
        Message msg;
        if (j.contains("id")) msg.id = QString::fromStdString(j["id"]);
        if (j.contains("senderId")) msg.senderId = QString::fromStdString(j["senderId"]);
        if (j.contains("senderName")) msg.senderName = QString::fromStdString(j["senderName"]);
        if (j.contains("senderAvatar")) msg.senderAvatar = QString::fromStdString(j["senderAvatar"]);
        if (j.contains("receiverId")) msg.receiverId = QString::fromStdString(j["receiverId"]);
        if (j.contains("content")) msg.content = QString::fromStdString(j["content"]);
        if (j.contains("type")) msg.type = QString::fromStdString(j["type"]);
        if (j.contains("timestamp")) msg.timestamp = QDateTime::fromMSecsSinceEpoch(j["timestamp"]);
        if (j.contains("isSent")) msg.isSent = j["isSent"];
        return msg;
    }
};

/**
 * @brief 联系人数据模型
 */
struct Contact
{
    QString id;           // 联系人ID
    QString name;         // 名称
    QString avatar;       // 头像URL
    QString status;       // 状态: online, offline, away
    QString remark;       // 备注名

    json toJson() const
    {
        return json{
            {"id", id.toStdString()},
            {"name", name.toStdString()},
            {"avatar", avatar.toStdString()},
            {"status", status.toStdString()},
            {"remark", remark.toStdString()}
        };
    }

    static Contact fromJson(const json& j)
    {
        Contact contact;
        if (j.contains("id")) contact.id = QString::fromStdString(j["id"]);
        if (j.contains("name")) contact.name = QString::fromStdString(j["name"]);
        if (j.contains("avatar")) contact.avatar = QString::fromStdString(j["avatar"]);
        if (j.contains("status")) contact.status = QString::fromStdString(j["status"]);
        if (j.contains("remark")) contact.remark = QString::fromStdString(j["remark"]);
        return contact;
    }
};

/**
 * @brief 群组数据模型
 */
struct Group
{
    QString id;           // 群组ID
    QString name;         // 群组名称
    QString avatar;       // 群组头像
    QString description;  // 群组描述
    QStringList members;  // 成员ID列表

    json toJson() const
    {
        json memberArray = json::array();
        for (const auto& memberId : members) {
            memberArray.push_back(memberId.toStdString());
        }

        return json{
            {"id", id.toStdString()},
            {"name", name.toStdString()},
            {"avatar", avatar.toStdString()},
            {"description", description.toStdString()},
            {"members", memberArray}
        };
    }

    static Group fromJson(const json& j)
    {
        Group group;
        if (j.contains("id")) group.id = QString::fromStdString(j["id"]);
        if (j.contains("name")) group.name = QString::fromStdString(j["name"]);
        if (j.contains("avatar")) group.avatar = QString::fromStdString(j["avatar"]);
        if (j.contains("description")) group.description = QString::fromStdString(j["description"]);
        if (j.contains("members") && j["members"].is_array()) {
            for (const auto& memberId : j["members"]) {
                group.members.append(QString::fromStdString(memberId));
            }
        }
        return group;
    }
};
