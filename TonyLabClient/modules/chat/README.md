# Chat 模块说明（TonyLabClient）

## 目标

- 保留现有聊天 UI（好友列表 + 顶部栏 + WebEngine 聊天渲染 + 发送按钮）的交互体验。
- 将底层通信升级为 WebSocket（Qt `QWebSocket`），并统一 JSON 协议为 nlohmann/json。
- 在逐步迁移过程中，允许旧 UI/login 代码暂时继续调用 `ComSocket`（通过兼容层实现）。

## 模块结构

- `modules/chat/`
  - `msgpane.*`：聊天主面板（左侧好友列表 + 右侧聊天 WebEngine + 输入框）。
  - `friendslist.*`：好友列表 UI。
  - `chattoptoolbar.*`：聊天顶部栏（用户名/部门/邮箱/签名）。
  - `navpane.*`：左侧主导航栏。
  - `wecomwnd.*`：主窗口容器（已改为纯代码 UI，不依赖 `.ui` 文件）。
  - `userdetaildlg.*`：用户详情浮层（最小实现，用于 NavPane/TopToolbar 点击展示）。
  - `ChatService.*`：聊天业务服务（发送消息、历史、已读、typing 等）。
  - `ContactService.*`：联系人/群组服务（拉取联系人、增删、群组管理等）。
  - `MessageModel.h`：消息/联系人/群组数据模型（nlohmann/json 序列化）。
  - `pushbuttonex.*`：通用按钮控件（供 login/device 等模块复用）。

- `ui/chat/`
  - `wecomwnd.h`：兼容 include 路径的薄封装（旧代码使用 `#include "ui/chat/wecomwnd.h"` 时仍可编译）。

- `network/`
  - `WebSocketClient.*`：WebSocket 连接、发送/接收 JSON。
  - `ComSocket.*`：兼容层（保留旧代码 API 与信号），内部基于 `WebSocketClient`。

## 当前接入方式（兼容模式）

现有 UI/login 仍使用旧接口：

- `ComSocket::getInstance()->connectServer(host, port)`
- `ComSocket::getInstance()->sendJsonRequest(QJsonObject)`
- 信号：`signalTcpReceivedData(QByteArray)` / `signalUdpReceivedData(QByteArray)`

兼容层实现说明：

- `network/ComSocket` 内部持有 `WebSocketClient`。
- WebSocket 收到 JSON 后，会把 `message.dump()` 转成 UTF-8 `QByteArray` 同时发到两个旧信号，以兼容 login/chat 的既有槽函数。
- `sendJsonRequest(QJsonObject)` 会先转成紧凑 JSON 字符串，再解析为 nlohmann/json，通过 `WebSocketClient::sendMessage(json)` 以 text frame 发送。

兼容层扩展（便于 Service 化复用连接）：

- `ComSocket::client()` 暴露底层 `WebSocketClient*`，新 UI/Service 可以复用同一条连接，不必额外再建连接。

> 这样可以先恢复“能编译 + UI 基本收发链路可走”，后续再逐步把 `msgpane`/`logindlg` 迁移为直接依赖 `ChatService`/`ContactService`。

## JSON 模型

- [MessageModel.h](MessageModel.h)
  - `Message`：`id/senderId/receiverId/content/type/timestamp/isSent`。
  - `Contact`：`id/name/avatar/status/remark`。
  - `Group`：`id/name/avatar/description/members`。

## 后续推荐迁移（Service 化）

建议最终形态：

- UI 层（`msgpane`）只负责：
  - 选择会话、渲染消息、采集输入。
- 业务层（`ChatService`）：
  - 统一发送/接收协议、维护消息列表、发出 `messageReceived/historyLoaded/...` 信号。
- 网络层（`WebSocketClient`）：
  - 专注连接与消息收发，不包含业务字段。

迁移步骤建议：

1. 在 `chatwindow` 初始化时创建 `WebSocketClient` + `ChatService` + `ContactService`。
2. `msgpane` 通过 signal/slot 订阅 `ChatService::messageReceived`，并把“发送按钮点击”改为调用 `ChatService::sendTextMessage(...)`。
3. 移除 UI 对 `ComSocket` 的直接依赖（届时 `ComSocket` 可以只保留给 login 或直接删除）。

## 当前实现状态（仓库内已落地）

- 主窗口使用纯代码 UI：`WeComWnd` 在构造时创建 `NavPane` + `MsgPane`，并在内部创建 `ChatService`/`ContactService`。
- WebEngine 渲染：右侧聊天区域加载 `qrc:/html/html/index1.html`，通过 `runJavaScript` 调用页面内的 `addMsg/addRecvMsg/clear` 函数。
- 消息链路：
  - 发送：输入框/发送按钮 -> `ChatService::sendTextMessage(receiverId, content)`。
  - 接收：`WebSocketClient::messageReceived(json)` -> `ChatService::messageReceived(Message)` -> `MsgPane` 渲染。
  - 历史：切换会话时触发 `ChatService::fetchMessageHistory(contactId)`，收到 `historyLoaded` 后渲染到 WebEngine。
- 好友列表来源：当前仍由 login 返回的数据驱动（login 解析好友详情后调用 `WeComWnd::setFriendList(...)`）。

## 构建说明

- `CMakeLists.txt` 已关闭 `AUTOUIC`（因为目前仓库中不再包含 `.ui` 文件）。
- nlohmann/json：优先 `find_package(nlohmann_json)`，否则使用内置路径：`third_party/nlohmann_json/include`。

### Windows 编译（推荐）

注意：如果不在 Visual Studio Developer Shell 环境中构建，可能会出现标准库头文件找不到（例如 `type_traits`/`memory`）的问题。

推荐步骤：

1. 打开 “x64 Native Tools Command Prompt for VS 2022” 或 “Developer PowerShell for VS 2022”。
2. 生成并编译：
  - `cmake -S d:\TonyLab\TonyLabClient -B d:\TonyLab\TonyLabClient\build`
  - `cmake --build d:\TonyLab\TonyLabClient\build --config Debug`
