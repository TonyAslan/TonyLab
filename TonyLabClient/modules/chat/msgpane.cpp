#include "msgpane.h"

#include "ChatService.h"
#include "chattoptoolbar.h"
#include "friendslist.h"
#include "userdetaildlg.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEnginePage>

MsgPane::MsgPane(QWidget* parent)
    : QWidget(parent)
{
    setProperty("form", "msgpane");

    m_friends = new FriendsList(this);
    m_top = new ChatTopToolBar(this);

    m_web = new QWebEngineView(this);
    m_web->setContextMenuPolicy(Qt::NoContextMenu);
    m_web->load(QUrl(QStringLiteral("qrc:/html/html/index1.html")));

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText(QStringLiteral("输入消息..."));

    m_send = new QPushButton(QStringLiteral("发送"), this);
    m_send->setProperty("sendbtn", "true");

    auto* inputRow = new QHBoxLayout();
    inputRow->setContentsMargins(0, 0, 0, 0);
    inputRow->setSpacing(8);
    inputRow->addWidget(m_input);
    inputRow->addWidget(m_send);

    auto* right = new QVBoxLayout();
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(0);
    right->addWidget(m_top);
    right->addWidget(m_web, 1);

    auto* inputWrap = new QWidget(this);
    inputWrap->setLayout(inputRow);
    inputWrap->setFixedHeight(44);
    right->addWidget(inputWrap);

    auto* rightWidget = new QWidget(this);
    rightWidget->setLayout(right);

    auto* splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(m_friends);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(0, false);
    splitter->setSizes({240, 760});

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(splitter);
    setLayout(root);

    connect(m_friends, &FriendsList::friendSelected, this, &MsgPane::onFriendSelected);
    connect(m_send, &QPushButton::clicked, this, &MsgPane::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed, this, &MsgPane::onSendClicked);
    connect(m_top, &ChatTopToolBar::contactDetailRequested, this, &MsgPane::onContactDetailRequested);
}

void MsgPane::setFriendList(const QVector<FRIENDINFO>& friends)
{
    m_friends->setFriends(friends);
}

void MsgPane::setCurrentUser(int userId, const QString& userName, const QString& avatar)
{
    m_currentUserId = userId;
    m_currentUserName = userName;
    m_currentUserAvatar = avatar;

    if (m_chatService) {
        m_chatService->setCurrentUser(QString::number(userId), userName, avatar);
    }
}

void MsgPane::setChatService(ChatService* chatService)
{
    m_chatService = chatService;
    if (!m_chatService) {
        return;
    }

    connect(m_chatService, &ChatService::messageReceived, this, &MsgPane::onServiceMessageReceived);
    connect(m_chatService, &ChatService::historyLoaded, this, &MsgPane::onServiceHistoryLoaded);

    if (m_currentUserId > 0) {
        m_chatService->setCurrentUser(QString::number(m_currentUserId), m_currentUserName, m_currentUserAvatar);
    }
}

void MsgPane::onFriendSelected(const FRIENDINFO& info)
{
    m_currentContact = info;
    m_top->setCurrentContact(info);
    clearWeb();

    if (m_chatService && info.id != 0) {
        m_chatService->fetchMessageHistory(QString::number(info.id));
    }
}

void MsgPane::onSendClicked()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    if (m_currentContact.id == 0) {
        return;
    }

    m_input->clear();
    appendOutgoingToWeb(text);

    emit sendTextRequested(QString::number(m_currentContact.id), text);

    if (m_chatService) {
        m_chatService->sendTextMessage(QString::number(m_currentContact.id), text);
    }
}

void MsgPane::onContactDetailRequested(const FRIENDINFO& info)
{
    UserDetailDlg dlg(this);
    dlg.setFriendInfo(info);
    dlg.exec();
}

void MsgPane::onServiceMessageReceived(const Message& msg)
{
    // Only show messages for the current conversation.
    const QString contactId = QString::number(m_currentContact.id);
    if (m_currentContact.id == 0) {
        return;
    }

    const QString sender = msg.senderId;
    const QString receiver = msg.receiverId;

    if (sender != contactId && receiver != contactId) {
        return;
    }

    if (sender == contactId) {
        appendIncomingToWeb(msg.content);
    }
}

void MsgPane::onServiceHistoryLoaded(const QList<Message>& messages)
{
    if (m_currentContact.id == 0) {
        return;
    }

    // Current web view is already cleared when switching contacts.
    const QString contactId = QString::number(m_currentContact.id);
    const QString myId = QString::number(m_currentUserId);

    for (const auto& msg : messages) {
        const QString sender = msg.senderId;
        const QString receiver = msg.receiverId;

        if (sender != contactId && receiver != contactId) {
            continue;
        }

        if (sender == contactId) {
            appendIncomingToWeb(msg.content);
        } else if (sender == myId) {
            appendOutgoingToWeb(msg.content);
        }
    }
}

QString MsgPane::toJsStringLiteral(const QString& s)
{
    QString out = s;
    out.replace('\\', "\\\\");
    out.replace('\r', "\\r");
    out.replace('\n', "\\n");
    out.replace('\'', "\\'");
    return QStringLiteral("'%1'").arg(out);
}

void MsgPane::appendOutgoingToWeb(const QString& text)
{
    if (!m_web || !m_web->page()) {
        return;
    }

    const QString js = QStringLiteral("addMsg(%1);").arg(toJsStringLiteral(text));
    m_web->page()->runJavaScript(js);
}

void MsgPane::appendIncomingToWeb(const QString& text)
{
    if (!m_web || !m_web->page()) {
        return;
    }

    const QString js = QStringLiteral("addRecvMsg(%1);").arg(toJsStringLiteral(text));
    m_web->page()->runJavaScript(js);
}

void MsgPane::clearWeb()
{
    if (!m_web || !m_web->page()) {
        return;
    }

    m_web->page()->runJavaScript(QStringLiteral("clear();"));
}
