#include "appreciatedlg.h"

AppreciateDlg::AppreciateDlg(QWidget *parent) :
    CBaseDlg(parent)
{
    CreateAllChildWnd();
    InitCtrl();
    Relayout();
}

AppreciateDlg::~AppreciateDlg()
{
}

void AppreciateDlg::CreateAllChildWnd()
{
    m_label1 = new QLabel(this);
    m_label2 = new QLabel(this);
}

void AppreciateDlg::InitCtrl()
{
    m_label1->setText("感谢支持！");
    m_label2->setText("请扫描二维码打赏");
}

void AppreciateDlg::Relayout()
{
    QVBoxLayout *layoutVMain = new QVBoxLayout();
    layoutVMain->addWidget(m_label1);
    layoutVMain->addWidget(m_label2);
    layoutVMain->addStretch();
    layoutVMain->setSpacing(20);
    layoutVMain->setContentsMargins(40, 10, 40, 60);

    centralWidget()->setLayout(layoutVMain);
}
