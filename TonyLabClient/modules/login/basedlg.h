#pragma once

#include <QDialog>

class QEvent;
class QHBoxLayout;
class QLabel;
class QMouseEvent;
class QObject;
class QVBoxLayout;
class QWidget;

class CPushButtonEx;

// 无边框基础对话框（WeCom 风格）：
// 1) 提供一套可复用的标题栏（标题文本 + 最小化/最大化/关闭按钮）
// 2) 提供 centralWidget() 作为“内容承载面板”，派生类只需给它 setLayout()
// 3) 通过 enableMoveWindow(true) 启用拖拽移动（与 WeCom 一致：默认仅拖动标题栏）
// 4) 通过对象属性 form/toolbar/toolbar_close 与 QSS 进行外观联动（见 resources/WeComCopy.css）
class CBaseDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CBaseDlg(QWidget* parent = nullptr);
    ~CBaseDlg() override = default;

    // 内容承载面板（标题栏下方区域）：派生类通常这样用：centralWidget()->setLayout(...)
    QWidget* centralWidget() const;
    // 启用/禁用拖拽移动（与 WeCom 一致：拖动标题栏移动窗口）
     void enableMoveWindow(bool enable);

    // 设置标题栏文本（仅影响标题栏 QLabel）
     void setTitle(const QString& title);
    // 设置标题栏高度（单位：px）
     void setTitleHeight(int height);
    // 控制标题栏按钮显隐（最小化/最大化/关闭），传入 Qt::WindowFlags 组合即可
     void setDlgFlags(Qt::WindowFlags type);
    // 显示/隐藏标题栏（隐藏后可实现“无标题栏”的纯内容对话框）
     void setTitleVisible(bool visible);

protected:
    // 事件过滤器：用于实现“拖动标题栏移动窗口”的无边框拖拽逻辑
    bool eventFilter(QObject* obj, QEvent* evt) override;

private slots:
     void onBtnMinClicked();
     void onBtnMaxClicked();
     void onBtnCloseClicked();

private:
    // 初始化控件树、布局、样式属性、信号槽（构造函数中调用）
     void initCtrl();

private:
    // 主布局（外边距用于留出阴影/边框空间）
    QVBoxLayout* m_layoutMain = nullptr;
    // 带阴影的主容器（包含标题栏 + 内容区）
    QWidget* m_widgetMain = nullptr;
    // 主容器内部布局
    QVBoxLayout* m_vLayoutWidgetMain = nullptr;

    // 标题栏：左侧标题文本 + 右侧菜单按钮区
    QWidget* m_widgetTitle = nullptr;
    QHBoxLayout* m_hLayoutTitle = nullptr;
    QLabel* m_labelTitle = nullptr;

    // 标题栏右侧按钮区域
    QWidget* m_widgetMenu = nullptr;
    QHBoxLayout* m_hLayoutMenu = nullptr;
    CPushButtonEx* m_btnMin = nullptr;
    CPushButtonEx* m_btnMax = nullptr;
    CPushButtonEx* m_btnClose = nullptr;

    // 标题栏下方内容面板（centralWidget() 返回此控件）
    QWidget* m_widgetMainPane = nullptr;

    bool m_bWndMaxSize = false;   // 当前是否处于最大化状态
    bool m_bEnableMove = false;  // 是否允许拖动移动
};
