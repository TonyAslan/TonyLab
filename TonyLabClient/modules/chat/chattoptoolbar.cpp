#include "chattoptoolbar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

ChatTopToolBar::ChatTopToolBar(QWidget* parent)
    : QWidget(parent)
{
    setProperty("form", "toptoolbar");
    initUi();
}

void ChatTopToolBar::initUi()
{
    setFixedHeight(56);

    m_name = new QLabel(this);
    m_part = new QLabel(this);
    m_email = new QLabel(this);
    m_sign = new QLabel(this);

    m_name->setStyleSheet(QStringLiteral("QLabel{font: bold 14px; color:#111;}"));
    m_part->setStyleSheet(QStringLiteral("QLabel{font: 12px; color:#666;}"));
    m_email->setStyleSheet(QStringLiteral("QLabel{font: 12px; color:#666;}"));
    m_sign->setStyleSheet(QStringLiteral("QLabel{font: 12px; color:#888;}"));

    auto* left = new QVBoxLayout();
    left->setContentsMargins(10, 6, 10, 6);
    left->setSpacing(2);
    left->addWidget(m_name);

    auto* row = new QHBoxLayout();
    row->setSpacing(10);
    row->addWidget(m_part);
    row->addWidget(m_email);
    row->addStretch();
    left->addLayout(row);
    left->addWidget(m_sign);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addLayout(left);
    setLayout(root);

    setCurrentContact({});
}

void ChatTopToolBar::setCurrentContact(const FRIENDINFO& info)
{
    m_current = info;

    const QString title = info.name.isEmpty() ? QStringLiteral("未选择会话") : info.name;
    m_name->setText(title);
    m_part->setText(info.part);
    m_email->setText(info.email);
    m_sign->setText(info.sign);
}

void ChatTopToolBar::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton && m_current.id != 0) {
        emit contactDetailRequested(m_current);
    }
}
