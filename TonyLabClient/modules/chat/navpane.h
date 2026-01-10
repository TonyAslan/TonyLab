#pragma once

#include <QWidget>

#include "app/public.h"

class QButtonGroup;
class CPushButtonEx;

class NavPane : public QWidget
{
    Q_OBJECT

public:
    explicit NavPane(QWidget* parent = nullptr);

signals:
    void tabSelected(EMainTabTitle tab);

private:
    void initUi();

    QButtonGroup* m_group = nullptr;
    CPushButtonEx* m_btnMessage = nullptr;
    CPushButtonEx* m_btnContacts = nullptr;
};
