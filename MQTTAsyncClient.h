#pragma once

/****************************************************************************************************/
/*                              https://github.com/Mrliu88888888/QMQTT                              */
/****************************************************************************************************/

#include <qstring.h>
#pragma execution_character_set("utf-8")

typedef void MQTTSyncMessageArrived(const QString& topicName, const QString& payload, void* context);
typedef void MQTTSyncConnectResult(const int& status, void* context);

struct SMQTTAsyncConnInfo
{
	SMQTTAsyncConnInfo()
		: keepAliveInterval(60)
		, username("")
		, password("")
		, connectTimeout(30)
		, retryInterval(0)
		, MQTTVersion(0)
	{ }

	int keepAliveInterval;
	QString username;
	QString password;
	int connectTimeout;
	int retryInterval;
	int MQTTVersion;
};

class MQTTAsyncClientPrivate;

/// @brief mqtt异步客户端
class MQTTAsyncClient
{
public:
	/// @brief 构造
	MQTTAsyncClient();
	/// @brief 析构
	~MQTTAsyncClient();

	/// @brief 初始化
	/// @param serverURI [in] mqtt服务器地址, 格式:[tcp://ip:port] 示例:[tcp://mqtt.eclipseprojects.io:1883]
	/// @param clientId [in] mqtt客户端ID, 空则自动生成UUID
	/// @return true: 成功, false: 失败
	bool init(const QString& serverURI, const QString& clientId = "");
	/// @brief 销毁
	void destroy();

	/// @brief 设置接收消息回调函数
	/// @param onMessageArrived [in] 接收消息回调函数指针
	/// @param context [in] 万能指针
	/// @return true: 成功, false: 失败
	bool setCallback(MQTTSyncMessageArrived* onMessageArrived = nullptr, void* context = nullptr);

	/// @brief 连接mqtt服务端
	/// @param info [in] 连接参数
	/// @param onConnResult [in] 连接回调函数
	/// @param context [in] 万能指针
	/// @return true: 成功, false: 失败
	bool connect(const SMQTTAsyncConnInfo& info, MQTTSyncConnectResult* onConnResult, void* context = nullptr);
	/// @brief 断开mqtt服务端
	/// @return true: 成功, false: 失败
	bool disconnect();

	/// @brief 订阅主题
	/// @param topic [in] 主题
	/// @param qos [in] QoS等级
	/// @return true: 成功, false: 失败
	bool subTopic(const QString& topic, const int& qos = 0);
	/// @brief 取消订阅主题
	/// @param topic [in] 主题
	/// @return true: 成功, false: 失败
	bool unsubTopic(const QString& topic);

	/// @brief 发送消息
	/// @param topicName [in] 主题
	/// @param msg [in] 消息内容
	/// @param qos [in] QoS等级
	/// @return true: 成功, false: 失败
	bool sendMessage(const QString& topicName, const QString& msg, const int& qos = 0);

private:
	MQTTAsyncClientPrivate* m_pClient;
};



#ifdef MQTT_ASYNC_CLIENT_DEMO
#include <qdebug.h>
#include <qthread.h>
void testMqttAsync()
{
	MQTTAsyncClient cli;
	if (cli.init("tcp://mqtt.eclipseprojects.io:1883"))
	{
		qDebug() << "init succ";
		qDebug() << cli.setCallback([](const QString& topicName, const QString& payload, void*) {
			qDebug() << topicName;
			qDebug() << payload;
			});
		SMQTTAsyncConnInfo info;
		cli.connect(info, [](const int& status, void* context) {
			if (status == 0)
			{
				auto mqClient = (MQTTAsyncClient*)context;
				qDebug() << "conn succ";
				mqClient->subTopic("123");
				qDebug() << mqClient->sendMessage("123", "asddadas好的");
			}
			else
			{
				qDebug() << "conn fail";
			}
			}, &cli);

		QThread::sleep(info.connectTimeout);
		cli.destroy();
	}
	else
	{
		qDebug() << "init fail";
	}
}
#endif
