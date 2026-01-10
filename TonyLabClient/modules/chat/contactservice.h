#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include "third_party/nlohmann_json/include/nlohmann/json.hpp"
#include "MessageModel.h"

using json = nlohmann::json;

class WebSocketClient;

/**
 * @brief 联系人服务
 * 负责管理联系人列表、在线状态等
 */
class ContactService : public QObject
{
    Q_OBJECT

public:
    explicit ContactService(WebSocketClient* wsClient, QObject* parent = nullptr);
    ~ContactService() override;

    // 联系人操作
    void requestContactList();
    void addContact(const QString& userId);
    void removeContact(const QString& contactId);
    void updateContactRemark(const QString& contactId, const QString& remark);

    // 群组操作
    void createGroup(const QString& groupName, const QStringList& memberIds);
    void joinGroup(const QString& groupId);
    void leaveGroup(const QString& groupId);
    void inviteToGroup(const QString& groupId, const QStringList& memberIds);

    // 查询
    const QList<Contact>& getContacts() const { return m_contacts; }
    Contact* getContact(const QString& contactId);
    const QList<Group>& getGroups() const { return m_groups; }
    QList<Contact> searchContacts(const QString& keyword);

signals:
    void contactListUpdated(const QList<Contact>& contacts);
    void contactStatusChanged(const QString& contactId, const QString& status);
    void contactRemoved(const QString& contactId);
    void groupListUpdated(const QList<Group>& groups);
    void groupMembersChanged(const QString& groupId, const QStringList& members);
    void errorOccurred(const QString& errorMsg);

private slots:
    void onMessageReceived(const json& message);

private:
    void handleContactListResponse(const json& data);
    void handleStatusUpdate(const json& data);
    void handleGroupListResponse(const json& data);
    void handleGroupUpdate(const json& data);

    WebSocketClient* m_webSocketClient = nullptr;
    QList<Contact> m_contacts;
    QList<Group> m_groups;
};
