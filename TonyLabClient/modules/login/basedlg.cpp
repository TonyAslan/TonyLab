#include "basedlg.h"

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QWidget>

#include "pushbuttonex.h"
#include "utils/iconhelper.h"

CBaseDlg::CBaseDlg(QWidget* parent)
    : QDialog(parent)
{
    // 构造时一次性搭好“外壳”：标题栏 + 内容面板 + 阴影
    initCtrl();
}

QWidget* CBaseDlg::centralWidget() const
{
    Q_ASSERT(m_widgetMainPane != nullptr);
    // 给外部填充标题栏下方的内容面板：派生类一般只关心这里
    return m_widgetMainPane;
}

void CBaseDlg::enableMoveWindow(bool enable)
{
    // 控制是否允许拖拽移动窗口（与 WeCom 一致：拖动标题栏）
    m_bEnableMove = enable;
}

void CBaseDlg::setTitle(const QString& title)
{
    // 仅更新标题栏文字，不影响窗口标题（无边框时通常也不显示系统标题）
    if (m_labelTitle) {
        m_labelTitle->setText(title);
    }
}

void CBaseDlg::setTitleHeight(int height)
{
    // 固定标题栏高度（px）
    if (m_widgetTitle) {
        m_widgetTitle->setFixedHeight(height);
    }
}

void CBaseDlg::setDlgFlags(Qt::WindowFlags type)
{
    // 通过 Qt::WindowFlags 控制标题栏按钮显隐
    // - Qt::WindowMinimizeButtonHint  显示最小化按钮
    // - Qt::WindowMaximizeButtonHint  显示最大化按钮
    // - Qt::WindowMinMaxButtonsHint   同时显示最小化和最大化按钮
    // - Qt::WindowCloseButtonHint     显示关闭按钮

    if (!m_btnMin || !m_btnMax || !m_btnClose) {
        return;
    }

    m_btnMin->setVisible(false);
    m_btnMax->setVisible(false);
    m_btnClose->setVisible(false);

    if (type & Qt::WindowMinimizeButtonHint) {
        m_btnMin->setVisible(true);
    }
    if (type & Qt::WindowMaximizeButtonHint) {
        m_btnMax->setVisible(true);
    }
    if (type & Qt::WindowCloseButtonHint) {
        m_btnClose->setVisible(true);
    }
}

void CBaseDlg::setTitleVisible(bool visible)
{
    // 控制整条标题栏显示/隐藏
    if (m_widgetTitle) {
        m_widgetTitle->setVisible(visible);
    }
}

bool CBaseDlg::eventFilter(QObject* obj, QEvent* evt)
{
    // WeCom 版行为：
    // 1) 仅在标题栏区域拖动移动窗口（避免内容区的点击/拖动影响输入控件）
    // 2) 最大化时不允许拖动（保持最大化窗口交互一致）
    if (!m_bEnableMove || m_bWndMaxSize) {
        return QWidget::eventFilter(obj, evt);
    }

    static QPoint ptMouse;
    static bool bMousePressed = false;

    // 注意：这里沿用 WeCom 版写法（函数内 static 状态），不做成员化以保证一致
    if (evt->type() == QEvent::MouseButtonPress) {
        auto* event = static_cast<QMouseEvent*>(evt);
        if (obj == m_widgetTitle && event->button() == Qt::LeftButton) {
            bMousePressed = true;
            ptMouse = event->globalPos() - pos();
            return true;
        }
    } else if (evt->type() == QEvent::MouseButtonRelease) {
        bMousePressed = false;
        return true;
    } else if (evt->type() == QEvent::MouseMove) {
        auto* event = static_cast<QMouseEvent*>(evt);
        if (bMousePressed && (event->buttons() & Qt::LeftButton)) {
            // 鼠标全局坐标 - 按下时的相对偏移 = 新的窗口左上角
            move(event->globalPos() - ptMouse);
            return true;
        }
    }

    return QWidget::eventFilter(obj, evt);
}

void CBaseDlg::onBtnMinClicked()
{
    // 最小化
    showMinimized();
}

void CBaseDlg::onBtnMaxClicked()
{
    // 最大化/还原（内部用 m_bWndMaxSize 记录当前状态，与 WeCom 一致）
    if (m_bWndMaxSize) {
        setWindowState(Qt::WindowNoState);
    } else {
        setWindowState(Qt::WindowMaximized);
    }

    m_bWndMaxSize = !m_bWndMaxSize;
}

void CBaseDlg::onBtnCloseClicked()
{
    // 关闭对话框（QDialog::reject）
    reject();
}

void CBaseDlg::initCtrl()
{
    // WeCom 版行为：无边框窗口 + 系统菜单 + 最小/最大化按钮提示（按钮是否显示由 SetDlgFlags 控制）
    setWindowFlags(windowFlags() | (Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint));

    // 该类的控件树结构如下（外层到内层）：
    // this
    //  └─ m_layoutMain (QVBoxLayout, margins=8)
    //      └─ m_widgetMain (QWidget, id="m_widgetMain"，承载阴影/边框)
    //          └─ m_vLayoutWidgetMain (QVBoxLayout, margins=1)
    //              ├─ m_widgetTitle (标题栏)
    //              │   ├─ m_labelTitle (标题文本)
    //              │   └─ m_widgetMenu (按钮区：min/max/close)
    //              └─ m_widgetMainPane (内容面板，centralWidget())

    // 主窗口布局（外边距用于留出阴影空间）
    m_layoutMain = new QVBoxLayout(this);
    m_layoutMain->setSpacing(0);
    m_layoutMain->setContentsMargins(8, 8, 8, 8);

    // 主容器（边框 + 阴影），承载标题栏和内容面板
    m_widgetMain = new QWidget(this);
    m_widgetMain->setObjectName(QStringLiteral("m_widgetMain"));
    m_widgetMain->setStyleSheet(QStringLiteral("QWidget#m_widgetMain{border: 1px solid #366CB3;}"));

    // 主容器内部垂直布局
    m_vLayoutWidgetMain = new QVBoxLayout(m_widgetMain);
    m_vLayoutWidgetMain->setSpacing(0);
    m_vLayoutWidgetMain->setContentsMargins(1, 1, 1, 1);

    // 标题栏
    m_widgetTitle = new QWidget(m_widgetMain);
    m_widgetTitle->setMinimumSize(QSize(0, 45));
    m_widgetTitle->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    m_hLayoutTitle = new QHBoxLayout(m_widgetTitle);
    m_hLayoutTitle->setSpacing(0);
    m_hLayoutTitle->setContentsMargins(11, 0, 0, 0);

    m_labelTitle = new QLabel(m_widgetTitle);
    m_labelTitle->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    m_hLayoutTitle->addWidget(m_labelTitle);

    m_widgetMenu = new QWidget(m_widgetTitle);
    m_widgetMenu->setObjectName(QStringLiteral("m_widgetMenu"));

    m_hLayoutMenu = new QHBoxLayout(m_widgetMenu);
    m_hLayoutMenu->setSpacing(0);
    m_hLayoutMenu->setContentsMargins(0, 0, 0, 0);

    m_btnMin = new CPushButtonEx(m_widgetMenu);
    // 字体图标：对应 IconHelper 内加载的图标字体编码（与 WeCom 一致）
    IconHelper::SetIcon(m_btnMin, QChar(0xe7d8));
    m_btnMin->setFocusPolicy(Qt::NoFocus);
    m_hLayoutMenu->addWidget(m_btnMin);

    m_btnMax = new CPushButtonEx(m_widgetMenu);
    IconHelper::SetIcon(m_btnMax, QChar(0xe693));
    m_btnMax->setFocusPolicy(Qt::NoFocus);
    m_hLayoutMenu->addWidget(m_btnMax);

    m_btnClose = new CPushButtonEx(m_widgetMenu);
    IconHelper::SetIcon(m_btnClose, QChar(0xe64f));
    m_btnClose->setFocusPolicy(Qt::NoFocus);
    m_hLayoutMenu->addWidget(m_btnClose);

    m_hLayoutTitle->addWidget(m_widgetMenu);
    m_vLayoutWidgetMain->addWidget(m_widgetTitle);

    // 内容面板（派生类往这里 setLayout）
    m_widgetMainPane = new QWidget(m_widgetMain);
    m_widgetMainPane->setObjectName(QStringLiteral("centralWidget"));
    // QSS 钩子：resources/WeComCopy.css 中 QWidget[form="basedlg"]
    m_widgetMainPane->setProperty("form", "basedlg");
    m_vLayoutWidgetMain->addWidget(m_widgetMainPane);

    m_layoutMain->addWidget(m_widgetMain);

    connect(m_btnMin, &QPushButton::clicked, this, &CBaseDlg::onBtnMinClicked);
    connect(m_btnMax, &QPushButton::clicked, this, &CBaseDlg::onBtnMaxClicked);
    connect(m_btnClose, &QPushButton::clicked, this, &CBaseDlg::onBtnCloseClicked);

    m_btnMin->setFixedSize(32, 25);
    m_btnMax->setFixedSize(32, 25);
    m_btnClose->setFixedSize(36, 26);
    m_btnMin->setProperty("toolbar", "true");
    m_btnMax->setProperty("toolbar", "true");
    // QSS 钩子：resources/WeComCopy.css 中 toolbar_close="true"
    m_btnClose->setProperty("toolbar_close", "true");

    // WeCom 版：绑定事件过滤器监听标题栏鼠标移动
    m_widgetTitle->installEventFilter(this);

    // 设置标题栏默认值与按钮显示
    // QSS 钩子：resources/WeComCopy.css 中 QWidget[form="title"]
    setProperty("form", true);
    m_widgetTitle->setProperty("form", "title");
    setTitle(QString());
    setTitleHeight(26);
    setDlgFlags(Qt::WindowCloseButtonHint);

    // 设置边框阴影
    // 透明背景 + 阴影效果：阴影附加到 m_widgetMain 上，边距由 m_layoutMain 留出来
    setAttribute(Qt::WA_TranslucentBackground, true);
    QGraphicsDropShadowEffect* shadowBorder = new QGraphicsDropShadowEffect(this);
    shadowBorder->setOffset(0, 0);
    shadowBorder->setColor(QColor(0x44, 0x44, 0x44, 127));
    shadowBorder->setBlurRadius(8);
    m_widgetMain->setGraphicsEffect(shadowBorder);
}
