#pragma once

#include <QDialog>

#include "app/public.h"

class QLabel;

class UserDetailDlg : public QDialog
{
    Q_OBJECT

public:
    explicit UserDetailDlg(QWidget* parent = nullptr);

    void setFriendInfo(const FRIENDINFO& info);

private:
    void initUi();

    QLabel* m_name = nullptr;
    QLabel* m_part = nullptr;
    QLabel* m_email = nullptr;
    QLabel* m_sign = nullptr;
    QLabel* m_id = nullptr;
};
