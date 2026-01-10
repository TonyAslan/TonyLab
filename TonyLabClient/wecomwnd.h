#pragma once

#include <QObject>
#include <QWidget>

#include "app/public.h"

class WebSocketClient;
class ChatService;
class ContactService;
class NavPane;
class MsgPane;

class WeComWnd : public QWidget
{
    Q_OBJECT

public:
    explicit WeComWnd(WebSocketClient* wsClient = nullptr, QWidget* parent = nullptr);
    ~WeComWnd() override;

    void setUserDetail(int userId, const QString& userName, const QString& userImg, const QString& userEmail, const QString& userPart);
    void setFriendList(const QVector<FRIENDINFO>& friends);

private:
    void initUi();
    void initServices();

    int m_userId = -1;
    QString m_userName;
    QString m_userImg;
    QString m_userEmail;
    QString m_userPart;

    WebSocketClient* m_wsClient = nullptr;
    ChatService* m_chatService = nullptr;
    ContactService* m_contactService = nullptr;

    NavPane* m_nav = nullptr;
    MsgPane* m_msg = nullptr;
};
