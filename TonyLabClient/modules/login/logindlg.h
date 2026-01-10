#ifndef CLOGINDLG_H
#define CLOGINDLG_H

#include <QWidget>

#include "third_party/nlohmann_json/include/nlohmann/json.hpp"

#include "basedlg.h"

#include "app/public.h"

class QLineEdit;
class CPushButtonEx;
class WeComWnd;
class WebSocketClient;

using json = nlohmann::json;
class CLoginDlg : public CBaseDlg
{
    Q_OBJECT
public:
	explicit CLoginDlg(QWidget* parent = nullptr);

private:
	void createAllChildWnd();
	void initCtrl();
	void initSlots();
	void relayout();
	void handleServerMessage(const json& response);
	void handleLoginResponse(const json& response);
	void handleFriendsDetailResponse(const json& response);
	void sendFriendsDetailRequest(const QVector<int>& vFriendIds);

signals:
    //void SignalLoginFinish();

private slots:
	void onLoginFinish();
	void onLogging();
	void onBtnCloseClicked();

	void onSendLoginRequest();
	void onReadyRead(const QByteArray& data);

private:
	
	//登录用户的基本信息
	int m_userId = -1;
	QString m_userName;
	QString m_userPart;
	QString m_userEmail;
	QString m_userImg;
	//好友Id
	QVector<int> m_vFriendsId;
	//Id，详情Map
	QMap<int, FRIENDINFO> m_mapFriends;

	QLabel* m_labCompany = nullptr;
	QLabel* m_labUserIcon = nullptr;
	QLabel* m_labUserName = nullptr;
	QLabel* m_labTip = nullptr;
	CPushButtonEx* m_btnCfg = nullptr;
	QLabel* m_labQRcode = nullptr;

	QLabel* m_labUserId = nullptr;
	QLabel* m_labUserPassword = nullptr;
	QLineEdit* m_userIdEdit = nullptr;
	QLineEdit* m_passwordEdit = nullptr;
	QPushButton* m_loginBtn = nullptr;

	//QTcpSocket* m_socket; //通信的端口
	WebSocketClient* m_wsClient = nullptr;
	//主窗口
	WeComWnd* m_weComWnd = nullptr;
};

#endif // CLOGINDLG_H
