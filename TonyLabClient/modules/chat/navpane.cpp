#include "navpane.h"

#include <QButtonGroup>
#include <QVBoxLayout>

#include "pushbuttonex.h"

NavPane::NavPane(QWidget* parent)
    : QWidget(parent)
{
    setProperty("form", "nav");
    initUi();
}

void NavPane::initUi()
{
    setFixedWidth(56);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    m_btnMessage = new CPushButtonEx(this);
    m_btnMessage->setCheckable(true);
    m_btnMessage->setText(QStringLiteral("消息"));

    m_btnContacts = new CPushButtonEx(this);
    m_btnContacts->setCheckable(true);
    m_btnContacts->setText(QStringLiteral("通讯录"));

    m_group->addButton(m_btnMessage, TABTITLE_MESSAGE);
    m_group->addButton(m_btnContacts, TABTITLE_CONTACTS);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 10, 6, 10);
    layout->setSpacing(8);
    layout->addWidget(m_btnMessage);
    layout->addWidget(m_btnContacts);
    layout->addStretch();
    setLayout(layout);

    m_btnMessage->setChecked(true);

    connect(m_group, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [this](int id) {
        emit tabSelected(static_cast<EMainTabTitle>(id));
    });
}
