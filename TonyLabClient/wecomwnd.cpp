#include "wecomwnd.h"

#include "modules/chat/chatservice.h"
#include "modules/chat/contactservice.h"
#include "modules/chat/msgpane.h"
#include "modules/chat/navpane.h"

#include "network/websocketclient.h"

#include <QHBoxLayout>
#include <QMessageBox>

WeComWnd::WeComWnd(WebSocketClient* wsClient, QWidget* parent)
    : QWidget(parent)
{
    m_wsClient = wsClient;

    setObjectName(QStringLiteral("WeComWnd"));
    setWindowTitle(QStringLiteral("TonyLabClient"));
    setMinimumSize(860, 650);
    setAttribute(Qt::WA_StyledBackground, true);
    setProperty("form", "mainwnd");
    setProperty("canMove", true);

    initUi();
    initServices();
}

WeComWnd::~WeComWnd()
{
    // QObject parent ownership covers children; keep explicit pointers null.
}

void WeComWnd::initUi()
{
    m_nav = new NavPane(this);
    m_msg = new MsgPane(this);

    m_nav->setProperty("form", "nav");
    m_msg->setProperty("form", "msgpane");

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(m_nav);
    root->addWidget(m_msg, 1);
    setLayout(root);
}

void WeComWnd::initServices()
{
    if (!m_wsClient) {
        // Should not happen, but keep UI alive.
        QMessageBox::warning(this, QStringLiteral("网络"), QStringLiteral("WebSocketClient 未初始化"));
        return;
    }

    m_chatService = new ChatService(m_wsClient, this);
    m_contactService = new ContactService(m_wsClient, this);

    m_msg->setChatService(m_chatService);
}

void WeComWnd::setUserDetail(int userId, const QString& userName, const QString& userImg, const QString& userEmail, const QString& userPart)
{
    m_userId = userId;
    m_userName = userName;
    m_userImg = userImg;
    m_userEmail = userEmail;
    m_userPart = userPart;

    if (m_msg) {
        m_msg->setCurrentUser(userId, userName, userImg);
    }

    if (m_chatService) {
        m_chatService->setCurrentUser(QString::number(userId), userName, userImg);
    }
}

void WeComWnd::setFriendList(const QVector<FRIENDINFO>& friends)
{
    if (m_msg) {
        m_msg->setFriendList(friends);
    }
}
