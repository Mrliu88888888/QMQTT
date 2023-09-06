#pragma once

/****************************************************************************************************/
/*                              https://github.com/Mrliu88888888/QMQTT                              */
/****************************************************************************************************/

#include <qstring.h>
#pragma execution_character_set("utf-8")

typedef void MQTTSyncMessageArrived(const QString& topicName, const QString& payload, void* context);

struct SMQTTSyncConnInfo
{
	SMQTTSyncConnInfo()
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

class MQTTSyncClientPrivate;

class MQTTSyncClient
{
public:
	/// @brief 构造
	MQTTSyncClient();
	/// @brief 析构
	~MQTTSyncClient();

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
	/// @return true: 成功, false: 失败
	bool connect(const SMQTTSyncConnInfo& info);
	/// @brief 断开mqtt服务端
	/// @param timeout [in] 超时时间(milliseconds)
	/// @return true: 成功, false: 失败
	bool disconnect(const int& timeout = 1000 * 10);

	/// @brief 订阅主题
	/// @param topic [in] 主题
	/// @param qos [in] QoS等级
	/// @return true: 成功, false: 失败
	bool subTopic(const QString& topic, const int& qos = 0);
	/// @brief 取消订阅主题
	/// @param topic [in] 主题
	/// @return true: 成功, false: 失败
	bool unsubTopic(const QString& topic);

	/// @brief 推送消息
	/// @param topicName [in] 主题
	/// @param msg [in] 消息内容
	/// @param qos [in] QoS等级
	/// @return true: 成功, false: 失败
	bool publishMessage(const QString& topicName, const QString& msg, const int& qos = 0);

private:
	MQTTSyncClientPrivate* m_pClient;
};



#ifdef MQTT_SYNC_CLIENT_DEMO
#include <qdebug.h>
#include <qthread.h>
void testMqttSync()
{
	MQTTSyncClient cli;
	if (cli.init("tcp://mqtt.eclipseprojects.io:1883"))
	{
		qDebug() << "init succ";
		qDebug() << cli.setCallback([](const QString& topicName, const QString& payload, void*) {
			qDebug() << topicName;
			qDebug() << payload;
			}, &cli);
		SMQTTSyncConnInfo info;
		if (cli.connect(info))
		{
			qDebug() << "conn succ";
			qDebug() << "conn succ";
			cli.subTopic("123");
			qDebug() << cli.publishMessage("123", "asddadas好的");

			QThread::sleep(10);
			cli.disconnect(10);
		}
		else
		{
			qDebug() << "conn fail";
		}
		cli.destroy();
	}
	else
	{
		qDebug() << "init fail";
	}
}
#endif
