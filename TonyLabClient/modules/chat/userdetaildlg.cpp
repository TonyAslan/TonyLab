#include "userdetaildlg.h"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

UserDetailDlg::UserDetailDlg(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("用户详情"));
    setModal(true);
    setFixedWidth(360);
    initUi();
}

void UserDetailDlg::initUi()
{
    m_id = new QLabel(this);
    m_name = new QLabel(this);
    m_part = new QLabel(this);
    m_email = new QLabel(this);
    m_sign = new QLabel(this);

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("ID"), m_id);
    form->addRow(QStringLiteral("姓名"), m_name);
    form->addRow(QStringLiteral("部门"), m_part);
    form->addRow(QStringLiteral("邮箱"), m_email);
    form->addRow(QStringLiteral("签名"), m_sign);

    auto* closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addStretch();
    root->addWidget(closeBtn);
    setLayout(root);
}

void UserDetailDlg::setFriendInfo(const FRIENDINFO& info)
{
    m_id->setText(QString::number(info.id));
    m_name->setText(info.name);
    m_part->setText(info.part);
    m_email->setText(info.email);
    m_sign->setText(info.sign);
}
