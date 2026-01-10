#include "logindlg.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#include "wecomwnd.h"
#include "utils/iconhelper.h"
#include "pushbuttonex.h"
#include "network/WebSocketClient.h"

int gCurrentLoginId;
QString gCurrentLoginName;
QString gCurrentLoginImg;

CLoginDlg::CLoginDlg(QWidget* parent)
	: CBaseDlg(parent)
{
	createAllChildWnd();
	initCtrl(); //show 输入用户id密码 其他隐藏
	initSlots();
	relayout();

	m_wsClient = new WebSocketClient(this);
	connect(m_wsClient, &WebSocketClient::dataReceived, this, &CLoginDlg::onReadyRead);
	connect(m_wsClient, &WebSocketClient::messageReceived, this, [this](const json& message) {
		handleServerMessage(message);
	});

	//连接服务器
	m_wsClient->setAutoReconnect(true);
	m_wsClient->connectToServer(QStringLiteral("ws://127.0.0.1:6666"));
	//QTimer::singleShot(2000, this, &CLoginDlg::onLogging);
	//应该是按钮点击然后开始登录 
	//向服务器发送一个查询的json指令
	connect(m_loginBtn, &QPushButton::clicked, this, &CLoginDlg::onSendLoginRequest);
}

void CLoginDlg::createAllChildWnd()
{
#define NEW_OBJECT(pObj, TYPE) \
	if ((pObj) == nullptr) { (pObj) = new TYPE(this); }

    NEW_OBJECT(m_labCompany, QLabel);
    NEW_OBJECT(m_labUserIcon, QLabel);
    NEW_OBJECT(m_labUserName, QLabel);
    NEW_OBJECT(m_labTip, QLabel);
    NEW_OBJECT(m_btnCfg, CPushButtonEx);
    NEW_OBJECT(m_labQRcode, QLabel);

	NEW_OBJECT(m_labUserId, QLabel);
	NEW_OBJECT(m_labUserPassword, QLabel);
	NEW_OBJECT(m_userIdEdit, QLineEdit);
	NEW_OBJECT(m_passwordEdit, QLineEdit);
	NEW_OBJECT(m_loginBtn, QPushButton);
}

void CLoginDlg::initCtrl()
{
    setFixedSize(300, 425);
	//允许拖动
	enableMoveWindow(true);
	//获取桌面对象
    QDesktopWidget *desktopWidget = QApplication::desktop();
	//获取屏幕尺寸
    QRect screenRect = desktopWidget->screenGeometry();
	//将窗口移动到屏幕中央
    move((screenRect.width() - this->width())/2, (screenRect.height() - this->height()) / 2);

	//设置用户头像标签 - 使用图片作为背景
    m_labUserIcon->setStyleSheet("border-image: url(:/qss/res/usricon.jpeg);");
    m_labUserIcon->setFixedSize(110, 110);
	//设置二维码标签 - 使用图片作为背景
    m_labQRcode->setStyleSheet("border-image: url(:/qss/res/QRcode.png);");
    m_labQRcode->setFixedSize(200, 200);

    m_btnCfg->setFixedSize(40, 40);
    m_labCompany->setText(tr("造化天宫"));
    m_labUserName->setText(tr("逍遥子"));
    m_labTip->setText(tr("登录"));
    m_labCompany->setAlignment(Qt::AlignCenter);
    m_labUserName->setAlignment(Qt::AlignCenter);
    m_labTip->setAlignment(Qt::AlignCenter);

    IconHelper::SetIcon(m_btnCfg, QChar(0xe642));
    m_btnCfg->setProperty("white_bk", "true");

    m_labCompany->setStyleSheet("QLabel{font: bold 13px; color:#36608F;}");
    m_labUserName->setStyleSheet("QLabel{font: bold 16px; color:#000000;}");
    m_labTip->setStyleSheet("QLabel{font: 14px; color:#000000;}");

    //m_labQRcode->show();
	m_labQRcode->hide();
    m_labCompany->hide();
    m_labUserIcon->hide();
    m_labUserName->hide();

	m_labUserId->setText("用户ID: ");
	m_labUserPassword->setText("密码: ");
	m_labUserId->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_labUserPassword->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_labUserId->setStyleSheet("QLabel{font: bold 16px; color:#000000;}");
	m_labUserPassword->setStyleSheet("QLabel{font: bold 16px; color:#000000;}");
	m_userIdEdit->setFixedSize(150,30);
	m_labUserId->setFixedSize(80, 30);
	m_labUserPassword->setFixedSize(80, 30);
	m_passwordEdit->setFixedSize(150, 30);
	m_passwordEdit->setEchoMode(QLineEdit::Password); // 设置为密码模式
	m_loginBtn->setFixedSize(80, 30);
	m_loginBtn->setText("登录");

}

void CLoginDlg::initSlots()
{

}

void CLoginDlg::relayout()
{
    QHBoxLayout *layoutIcon = new QHBoxLayout();
    layoutIcon->addStretch();
    layoutIcon->addWidget(m_labQRcode);
    layoutIcon->addWidget(m_labUserIcon);
    layoutIcon->addStretch();

    QHBoxLayout *layoutCfg = new QHBoxLayout();
    layoutCfg->addStretch();
    layoutCfg->addWidget(m_btnCfg);
    layoutCfg->setContentsMargins(0, 0, 10, 0);

    QVBoxLayout *layoutUser = new QVBoxLayout();
    layoutUser->addLayout(layoutIcon);
    layoutUser->addWidget(m_labUserName);
    layoutUser->setSpacing(0);
    layoutUser->setMargin(0);

	QHBoxLayout *layoutInputId = new QHBoxLayout();
	layoutInputId->addStretch();
	layoutInputId->addWidget(m_labUserId);
	layoutInputId->addWidget(m_userIdEdit);
	layoutInputId->addStretch();
	layoutInputId->setSpacing(10);

	QHBoxLayout *layoutInputPassword = new QHBoxLayout();
	layoutInputPassword->addStretch();
	layoutInputPassword->addWidget(m_labUserPassword);
	layoutInputPassword->addWidget(m_passwordEdit);
	layoutInputPassword->addStretch();
	layoutInputPassword->setSpacing(10);

	QHBoxLayout *layoutLoginBtn = new QHBoxLayout();
	layoutLoginBtn->addStretch();
	layoutLoginBtn->addWidget(m_loginBtn);
	layoutLoginBtn->addStretch();
	layoutLoginBtn->setSpacing(20);

    QVBoxLayout *layoutVMain = new QVBoxLayout();
    layoutVMain->addWidget(m_labCompany);
    layoutVMain->addLayout(layoutUser);
    layoutVMain->addWidget(m_labTip);
    layoutVMain->addStretch();

	layoutVMain->addLayout(layoutInputId);
	layoutVMain->addLayout(layoutInputPassword);
	layoutVMain->addLayout(layoutLoginBtn);

    layoutVMain->addLayout(layoutCfg);
    layoutVMain->setSpacing(40);
    layoutVMain->setContentsMargins(0, 40, 0, 10);
	//centralWidget() 返回基类内定义的中间部件
    centralWidget()->setLayout(layoutVMain);
}

void CLoginDlg::onLoginFinish()
{
	//当前登录者的id
	gCurrentLoginId = m_userId;
	gCurrentLoginName = m_userName;
	gCurrentLoginImg = m_userImg;
	m_weComWnd->show();
    hide();
    //emit SignalLoginFinish();
}

void CLoginDlg::onLogging()
{
    m_labTip->setText(tr("正在登录..."));
    m_labQRcode->hide();
	
	m_labUserId->hide();
	m_labUserPassword->hide();
	m_userIdEdit->hide();
	m_passwordEdit->hide();
	m_loginBtn->hide();
	m_labCompany->setText(m_userPart);
    m_labCompany->show();
	m_labUserIcon->setStyleSheet(QString("border-image: url(%1);").arg(m_userImg));
    m_labUserIcon->show();
	m_labUserName->setText(m_userName);
    m_labUserName->show();

	//登录中查好友详情
	if (!m_vFriendsId.isEmpty())
	{
		//根据需求查登入后界面可视部分
		QVector<int> vRequestId;
		//假定只显示5个
		for (int i = 0; i < qMin(5, m_vFriendsId.size()); ++i)
		{
			vRequestId.append(m_vFriendsId[i]);
			qDebug() << "vRequestId = " << m_vFriendsId[i] << " ";
		}
		//发送查详情请求
		sendFriendsDetailRequest(vRequestId);
	}
	//在显示主窗口之前，先设置号好友信息


	//QTimer::singleShot(3000, this, &CLoginDlg::onLoginFinish);
}

void CLoginDlg::onBtnCloseClicked()
{
    qApp->quit();
    reject();
}

void CLoginDlg::onSendLoginRequest()
{

	//先测试wecomwnd显示
	m_weComWnd = new WeComWnd(m_wsClient);
	if (m_wsClient) {
		m_wsClient->setParent(m_weComWnd);
		disconnect(m_wsClient, nullptr, this, nullptr);
	}
	m_weComWnd->show();
	hide();
	return ;

	//发送登录请求
	qDebug() << "send login request";
	/*if (m_socket->state() != QAbstractSocket::ConnectedState) {
		qDebug() << "connection error";
		return;
	}*/
	if (!m_wsClient) {
		return;
	}

	json request;
	request["type"] = "0";
	request["login"] = {
		{"account", m_userIdEdit->text().toStdString()},
		{"password", m_passwordEdit->text().toStdString()},
	};

	m_wsClient->sendMessage(request);
}


void CLoginDlg::onReadyRead(const QByteArray& data)
{
	try {
		json message = json::parse(data.constData());
		handleServerMessage(message);
	} catch (const std::exception&) {
		qDebug() << "received invalid JSON response";
	}
}

void CLoginDlg::handleServerMessage(const json& response)
{
	const QString type = QString::fromStdString(response.value("type", std::string{}));
	const int status = response.value("status", 0);
	const QString desc = QString::fromStdString(response.value("desc", std::string{}));

	qDebug() << "Received response type:" << type << "status:" << status << desc;

	if (type == QStringLiteral("0")) {
		handleLoginResponse(response);
		return;
	}
	if (type == QStringLiteral("2")) {
		handleFriendsDetailResponse(response);
		return;
	}

	qDebug() << "Unsupported response type:" << type;
}
//处理登录回复
void CLoginDlg::handleLoginResponse(const json& response)
{
	const json dataObj = response.value("data", json::object());
	m_userId = dataObj.value("userId", -1);
	m_userName = QString::fromStdString(dataObj.value("userName", std::string{}));
	m_userPart = QString::fromStdString(dataObj.value("userPart", std::string{}));
	m_userEmail = QString::fromStdString(dataObj.value("userEmail", std::string{}));
	m_userImg = QString::fromStdString(dataObj.value("userImg", std::string{}));
	const int nFriendCount = dataObj.value("friendCount", 0);

	qDebug() << "User info:"
		<< "\nID:" << m_userId
		<< "\nName:" << m_userName
		<< "\nDepartment:" << m_userPart
		<< "\nEmail:" << m_userEmail
		<< "\nAvatar:" << m_userImg
		<< "\nFriends count:" << nFriendCount;

	if (dataObj.contains("list") && dataObj["list"].is_array()) {
		for (const auto& value : dataObj["list"]) {
			const int friendId = value.value("id", 0);
			const QString friendName = QString::fromStdString(value.value("name", std::string{}));
		m_vFriendsId.append(friendId);
		m_mapFriends[friendId] = { friendId,friendName,"","","" };
		}
	}

	//拿到一些基本信息 设置登录界面
	QTimer::singleShot(2000, this, &CLoginDlg::onLogging);
}

//处理详情回复
void CLoginDlg::handleFriendsDetailResponse(const json& response)
{
	const json dataObj = response.value("data", json::object());
	//解析好友详情
	QVector<FRIENDINFO> vFriendInfo;
	vFriendInfo.clear();
	if (dataObj.contains("friendDetails") && dataObj["friendDetails"].is_array()) {
		for (const auto& detail : dataObj["friendDetails"]) {
			const int nId = detail.value("id", 0);
			if (!m_mapFriends.contains(nId)) {
				continue;
			}

			auto& friendInfo = m_mapFriends[nId];
			friendInfo.id = nId;
			friendInfo.part = QString::fromStdString(detail.value("part", std::string{}));
			friendInfo.email = QString::fromStdString(detail.value("email", std::string{}));
			friendInfo.img = QString::fromStdString(detail.value("img", std::string{}));
			friendInfo.sign = QString::fromStdString(detail.value("sign", std::string{}));
			qDebug() << "friend details" << nId << friendInfo.name;
			vFriendInfo.append(friendInfo);
		}
	}
	//拿到好友详情后 拿信息构建主窗口
	m_weComWnd = new WeComWnd(m_wsClient);
	if (m_wsClient) {
		m_wsClient->setParent(m_weComWnd);
		disconnect(m_wsClient, nullptr, this, nullptr);
	}
	//设置个人信息
	m_weComWnd->setUserDetail(m_userId, m_userName, m_userImg, m_userEmail, m_userPart);
	//设置好友列表
	m_weComWnd->setFriendList(vFriendInfo);
	//设置好后 再登录完成 显示页面
	QTimer::singleShot(1000, this, &CLoginDlg::onLoginFinish);
	
}

void CLoginDlg::sendFriendsDetailRequest(const QVector<int>& vFriendIds)
{
	if (!m_wsClient) {
		return;
	}

	json request;
	request["type"] = "2";
	request["friendIds"] = json::array();
	for (int nId : vFriendIds) {
		request["friendIds"].push_back(nId);
	}

	m_wsClient->sendMessage(request);
}



