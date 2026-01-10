#include "friendslist.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

FriendsList::FriendsList(QWidget* parent)
    : QWidget(parent)
{
    setProperty("form", "friendlist");

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(QStringLiteral("搜索"));

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setUniformItemSizes(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_filterEdit);
    layout->addWidget(m_list);
    setLayout(layout);

    connect(m_list, &QListWidget::itemActivated, this, &FriendsList::onItemActivated);
    connect(m_list, &QListWidget::itemClicked, this, &FriendsList::onItemActivated);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &FriendsList::onFilterChanged);
}

void FriendsList::setFriends(const QVector<FRIENDINFO>& friends)
{
    m_allFriends = friends;
    m_filteredFriends = friends;
    rebuildList();

    if (m_list->count() > 0) {
        m_list->setCurrentRow(0);
        onItemActivated(m_list->item(0));
    }
}

FRIENDINFO FriendsList::currentFriend() const
{
    auto* item = m_list->currentItem();
    if (!item) {
        return {};
    }

    const int id = item->data(Qt::UserRole).toInt();
    for (const auto& f : m_filteredFriends) {
        if (f.id == id) {
            return f;
        }
    }
    for (const auto& f : m_allFriends) {
        if (f.id == id) {
            return f;
        }
    }
    return {};
}

void FriendsList::onItemActivated(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    const int id = item->data(Qt::UserRole).toInt();
    for (const auto& f : m_filteredFriends) {
        if (f.id == id) {
            emit friendSelected(f);
            return;
        }
    }
}

void FriendsList::onFilterChanged(const QString& text)
{
    const QString keyword = text.trimmed().toLower();

    if (keyword.isEmpty()) {
        m_filteredFriends = m_allFriends;
        rebuildList();
        return;
    }

    m_filteredFriends.clear();
    for (const auto& f : m_allFriends) {
        if (f.name.toLower().contains(keyword) ||
            f.part.toLower().contains(keyword) ||
            f.email.toLower().contains(keyword) ||
            QString::number(f.id).contains(keyword)) {
            m_filteredFriends.push_back(f);
        }
    }

    rebuildList();
}

void FriendsList::rebuildList()
{
    m_list->clear();

    for (const auto& f : m_filteredFriends) {
        auto* item = new QListWidgetItem(m_list);
        item->setText(f.name.isEmpty() ? QString::number(f.id) : f.name);
        item->setToolTip(QStringLiteral("%1\n%2\n%3")
                             .arg(f.name)
                             .arg(f.part)
                             .arg(f.email));
        item->setData(Qt::UserRole, f.id);
    }
}
