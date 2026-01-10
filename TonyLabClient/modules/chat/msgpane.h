#pragma once

#include <QList>
#include <QWidget>

#include "app/public.h"
#include "MessageModel.h"

class ChatService;
class FriendsList;
class ChatTopToolBar;
class QLineEdit;
class QPushButton;
class QWebEngineView;

class MsgPane : public QWidget
{
    Q_OBJECT

public:
    explicit MsgPane(QWidget* parent = nullptr);

    void setFriendList(const QVector<FRIENDINFO>& friends);
    void setCurrentUser(int userId, const QString& userName, const QString& avatar);

    void setChatService(ChatService* chatService);

signals:
    void sendTextRequested(const QString& receiverId, const QString& content);

private slots:
    void onFriendSelected(const FRIENDINFO& info);
    void onSendClicked();
    void onContactDetailRequested(const FRIENDINFO& info);

    void onServiceMessageReceived(const Message& msg);
    void onServiceHistoryLoaded(const QList<Message>& messages);

private:
    static QString toJsStringLiteral(const QString& s);
    void appendOutgoingToWeb(const QString& text);
    void appendIncomingToWeb(const QString& text);
    void clearWeb();

    FRIENDINFO m_currentContact;
    int m_currentUserId = -1;
    QString m_currentUserName;
    QString m_currentUserAvatar;

    ChatService* m_chatService = nullptr;

    FriendsList* m_friends = nullptr;
    ChatTopToolBar* m_top = nullptr;
    QWebEngineView* m_web = nullptr;
    QLineEdit* m_input = nullptr;
    QPushButton* m_send = nullptr;
};
