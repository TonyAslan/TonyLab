#pragma once

#include <QWidget>

#include "app/public.h"

class QListWidget;
class QListWidgetItem;
class QLineEdit;

class FriendsList : public QWidget
{
    Q_OBJECT

public:
    explicit FriendsList(QWidget* parent = nullptr);

    void setFriends(const QVector<FRIENDINFO>& friends);
    FRIENDINFO currentFriend() const;

signals:
    void friendSelected(const FRIENDINFO& friendInfo);

private slots:
    void onItemActivated(QListWidgetItem* item);
    void onFilterChanged(const QString& text);

private:
    void rebuildList();

    QVector<FRIENDINFO> m_allFriends;
    QVector<FRIENDINFO> m_filteredFriends;

    QLineEdit* m_filterEdit = nullptr;
    QListWidget* m_list = nullptr;
};
