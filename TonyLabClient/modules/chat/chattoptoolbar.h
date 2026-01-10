#pragma once

#include <QWidget>

#include "app/public.h"

class QLabel;

class ChatTopToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit ChatTopToolBar(QWidget* parent = nullptr);

    void setCurrentContact(const FRIENDINFO& info);
    FRIENDINFO currentContact() const { return m_current; }

signals:
    void contactDetailRequested(const FRIENDINFO& info);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    void initUi();

    FRIENDINFO m_current;

    QLabel* m_name = nullptr;
    QLabel* m_part = nullptr;
    QLabel* m_email = nullptr;
    QLabel* m_sign = nullptr;
};
