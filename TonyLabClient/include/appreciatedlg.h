#ifndef APPRECIATEDLG_H
#define APPRECIATEDLG_H

#include <QWidget>
#include <QLabel>
#include "basedlg.h"

class AppreciateDlg : public CBaseDlg
{
    Q_OBJECT

public:
    explicit AppreciateDlg(QWidget *parent = 0);
    ~AppreciateDlg();

private:
    void CreateAllChildWnd();
    void InitCtrl();
    void Relayout();

private:
    QLabel *m_label1;
    QLabel *m_label2;
};

#endif // APPRECIATEDLG_H
