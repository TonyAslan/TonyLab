#include "ContactService.h"
#include "network/WebSocketClient.h"
#include <QDebug>

ContactService::ContactService(WebSocketClient* wsClient, QObject* parent)
    : QObject(parent)
    , m_webSocketClient(wsClient)
{
    if (!wsClient) {
        qCritical() << "WebSocketClient is null";
        return;
    }
    
    connect(m_webSocketClient, &WebSocketClient::messageReceived,
            this, &ContactService::onMessageReceived);
}

ContactService::~ContactService()
{
}

void ContactService::requestContactList()
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json request = {
            {"type", "contact.list"},
            {"action", "fetch"}
        };
        
        m_webSocketClient->sendMessage(request);
        qDebug() << "Requesting contact list";
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

Contact* ContactService::getContact(const QString& contactId)
{
    for (auto& contact : m_contacts) {
        if (contact.id == contactId) {
            return &contact;
        }
    }
    return nullptr;
}

void ContactService::addContact(const QString& userId)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json request = {
            {"type", "contact.manage"},
            {"action", "add"},
            {"userId", userId.toStdString()}
        };
        
        m_webSocketClient->sendMessage(request);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::removeContact(const QString& contactId)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json request = {
            {"type", "contact.manage"},
            {"action", "remove"},
            {"contactId", contactId.toStdString()}
        };
        
        m_webSocketClient->sendMessage(request);
        
        // 本地移除
        for (int i = 0; i < m_contacts.size(); ++i) {
            if (m_contacts[i].id == contactId) {
                m_contacts.removeAt(i);
                break;
            }
        }
        
        emit contactRemoved(contactId);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::updateContactRemark(const QString& contactId, const QString& remark)
{
    try {
        json request = {
            {"type", "contact.manage"},
            {"action", "updateRemark"},
            {"contactId", contactId.toStdString()},
            {"remark", remark.toStdString()}
        };
        
        m_webSocketClient->sendMessage(request);
        
        // 更新本地
        for (auto& contact : m_contacts) {
            if (contact.id == contactId) {
                contact.remark = remark;
                break;
            }
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::createGroup(const QString& groupName, const QStringList& memberIds)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json memberArray = json::array();
        for (const auto& memberId : memberIds) {
            memberArray.push_back(memberId.toStdString());
        }
        
        json request = {
            {"type", "group.manage"},
            {"action", "create"},
            {"name", groupName.toStdString()},
            {"members", memberArray}
        };
        
        m_webSocketClient->sendMessage(request);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::joinGroup(const QString& groupId)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json request = {
            {"type", "group.manage"},
            {"action", "join"},
            {"groupId", groupId.toStdString()}
        };
        
        m_webSocketClient->sendMessage(request);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::leaveGroup(const QString& groupId)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json request = {
            {"type", "group.manage"},
            {"action", "leave"},
            {"groupId", groupId.toStdString()}
        };
        
        m_webSocketClient->sendMessage(request);
        
        // 本地移除
        for (int i = 0; i < m_groups.size(); ++i) {
            if (m_groups[i].id == groupId) {
                m_groups.removeAt(i);
                break;
            }
        }
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::inviteToGroup(const QString& groupId, const QStringList& memberIds)
{
    if (!m_webSocketClient->isConnected()) {
        emit errorOccurred("WebSocket not connected");
        return;
    }
    
    try {
        json memberArray = json::array();
        for (const auto& memberId : memberIds) {
            memberArray.push_back(memberId.toStdString());
        }
        
        json request = {
            {"type", "group.manage"},
            {"action", "invite"},
            {"groupId", groupId.toStdString()},
            {"members", memberArray}
        };
        
        m_webSocketClient->sendMessage(request);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

QList<Contact> ContactService::searchContacts(const QString& keyword)
{
    QList<Contact> results;
    QString lowerKeyword = keyword.toLower();
    
    for (const auto& contact : m_contacts) {
        if (contact.name.toLower().contains(lowerKeyword) ||
            contact.remark.toLower().contains(lowerKeyword) ||
            contact.id.contains(lowerKeyword)) {
            results.append(contact);
        }
    }
    
    return results;
}

void ContactService::onMessageReceived(const json& message)
{
    try {
        if (!message.contains("type")) {
            return;
        }
        
        std::string msgType = message["type"];
        
        if (msgType == "contact.list") {
            handleContactListResponse(message);
        } else if (msgType == "contact.status") {
            handleStatusUpdate(message);
        } else if (msgType == "group.list") {
            handleGroupListResponse(message);
        } else if (msgType == "group.update") {
            handleGroupUpdate(message);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error processing message:" << e.what();
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void ContactService::handleContactListResponse(const json& data)
{
    try {
        m_contacts.clear();
        
        if (data.contains("contacts") && data["contacts"].is_array()) {
            for (const auto& contactJson : data["contacts"]) {
                Contact contact = Contact::fromJson(contactJson);
                m_contacts.append(contact);
            }
        }
        
        emit contactListUpdated(m_contacts);
        qDebug() << "Contact list updated, total:" << m_contacts.count();
        
    } catch (const std::exception& e) {
        qWarning() << "Error handling contact list response:" << e.what();
    }
}

void ContactService::handleStatusUpdate(const json& data)
{
    try {
        if (data.contains("contactId") && data.contains("status")) {
            QString contactId = QString::fromStdString(data["contactId"]);
            QString status = QString::fromStdString(data["status"]);
            
            // 更新本地状态
            for (auto& contact : m_contacts) {
                if (contact.id == contactId) {
                    contact.status = status;
                    break;
                }
            }
            
            emit contactStatusChanged(contactId, status);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error handling status update:" << e.what();
    }
}

void ContactService::handleGroupListResponse(const json& data)
{
    try {
        m_groups.clear();
        
        if (data.contains("groups") && data["groups"].is_array()) {
            for (const auto& groupJson : data["groups"]) {
                Group group = Group::fromJson(groupJson);
                m_groups.append(group);
            }
        }
        
        emit groupListUpdated(m_groups);
        qDebug() << "Group list updated, total:" << m_groups.count();
        
    } catch (const std::exception& e) {
        qWarning() << "Error handling group list response:" << e.what();
    }
}

void ContactService::handleGroupUpdate(const json& data)
{
    try {
        if (data.contains("groupId") && data.contains("members")) {
            QString groupId = QString::fromStdString(data["groupId"]);
            QStringList members;
            
            if (data["members"].is_array()) {
                for (const auto& member : data["members"]) {
                    members.append(QString::fromStdString(member));
                }
            }
            
            // 更新本地群组成员
            for (auto& group : m_groups) {
                if (group.id == groupId) {
                    group.members = members;
                    break;
                }
            }
            
            emit groupMembersChanged(groupId, members);
        }
    } catch (const std::exception& e) {
        qWarning() << "Error handling group update:" << e.what();
    }
}
