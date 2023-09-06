#include "MQTTSyncClient.h"
#include "MQTTClient.h"
#include <quuid.h>
#pragma execution_character_set("utf-8")

#define STRING_TO_CSTR(STR) \
  	STR.toLocal8Bit().data()
#define CSTRING_LEN(STR) \
 	STR.toLocal8Bit().length()
#define CSTR_TO_STRING(CSTR, LEN) \
  	QString::fromLocal8Bit(CSTR, LEN)



class MQTTSyncClientPrivate
{
public:
	MQTTSyncClientPrivate()
		: handle(nullptr) { }
	~MQTTSyncClientPrivate() { this->destroy(); }

	bool init(const char* serverURI, const char* clientId)
	{
		return (handle == nullptr) ?
			(MQTTClient_create(&handle, serverURI, clientId, MQTTCLIENT_PERSISTENCE_NONE, nullptr) == MQTTCLIENT_SUCCESS)
			: false;
	}
	void destroy()
	{
		if (handle != nullptr)
		{
			MQTTClient_destroy(&handle); handle = nullptr;
		}
	}

	bool setCallback(MQTTSyncMessageArrived* onMessageArrived, void* context)
	{
		m_MessageArrived.onMessageArrived = onMessageArrived;
		m_MessageArrived.context;
		return MQTTClient_setCallbacks(handle, &m_MessageArrived, nullptr, MessageArrived, nullptr) == MQTTCLIENT_SUCCESS;
	}

	bool connect(const SMQTTSyncConnInfo& info)
	{
		MQTTClient_connectOptions connOpts = MQTTClient_connectOptions_initializer;
		connOpts.keepAliveInterval = info.keepAliveInterval;
		if (CSTRING_LEN(info.username) > 0) { connOpts.username = STRING_TO_CSTR(info.username); }
		if (CSTRING_LEN(info.password) > 0) { connOpts.password = STRING_TO_CSTR(info.password); }
		connOpts.connectTimeout = info.connectTimeout;
		connOpts.retryInterval = info.retryInterval;
		connOpts.MQTTVersion = info.MQTTVersion;

		return MQTTClient_connect(handle, &connOpts) == MQTTCLIENT_SUCCESS;
	}
	bool disconnect(const int& timeout)
	{
		return MQTTClient_disconnect(handle, timeout) == MQTTCLIENT_SUCCESS;
	}

	bool subTopic(const char* topic, const int& qos)
	{
		return MQTTClient_subscribe(handle, topic, qos) == MQTTCLIENT_SUCCESS;
	}
	bool unsubTopic(const char* topic)
	{
		return MQTTClient_unsubscribe(handle, topic) == MQTTCLIENT_SUCCESS;
	}

	bool publishMessage(const char* topicName, const char* msg, const int& len, const int& qos)
	{
		MQTTClient_message pubMsg = MQTTClient_message_initializer;
		pubMsg.payload = (void*)msg;
		pubMsg.payloadlen = len;
		pubMsg.qos = qos;
		return MQTTClient_publishMessage(handle, topicName, &pubMsg, nullptr) == MQTTCLIENT_SUCCESS;
	}

private:
	static int MessageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* message)
	{
		auto messageArrived = (SMessageArrived*)context;
		messageArrived->onMessageArrived(CSTR_TO_STRING(topicName, -1),
			CSTR_TO_STRING((char*)message->payload, message->payloadlen), messageArrived->context);
		MQTTClient_freeMessage(&message);
		MQTTClient_free(topicName);
		return 1;
	}

private:
	MQTTClient handle;

	struct SMessageArrived
	{
		SMessageArrived()
			: onMessageArrived(nullptr)
			, context(nullptr)
		{ }

		MQTTSyncMessageArrived* onMessageArrived;
		void* context;
	} m_MessageArrived;
};



MQTTSyncClient::MQTTSyncClient()
	: m_pClient(new MQTTSyncClientPrivate())
{
}

MQTTSyncClient::~MQTTSyncClient()
{
	delete m_pClient; m_pClient = nullptr;
}

bool MQTTSyncClient::init(const QString& serverURI, const QString& clientId)
{
	const auto id = clientId.isEmpty() ? QUuid::createUuid().toString() : clientId;
	return m_pClient->init(STRING_TO_CSTR(serverURI), STRING_TO_CSTR(id));
}

void MQTTSyncClient::destroy()
{
	m_pClient->destroy();
}

bool MQTTSyncClient::setCallback(MQTTSyncMessageArrived* onMessageArrived, void* context)
{
	return m_pClient->setCallback(onMessageArrived, context);
}

bool MQTTSyncClient::connect(const SMQTTSyncConnInfo& info)
{
	return m_pClient->connect(info);
}

bool MQTTSyncClient::disconnect(const int& timeout)
{
	return m_pClient->disconnect(timeout);
}

bool MQTTSyncClient::subTopic(const QString& topic, const int& qos)
{
	return m_pClient->subTopic(STRING_TO_CSTR(topic), qos);
}

bool MQTTSyncClient::unsubTopic(const QString& topic)
{
	return m_pClient->unsubTopic(STRING_TO_CSTR(topic));
}

bool MQTTSyncClient::publishMessage(const QString& topicName, const QString& msg, const int& qos)
{
	return m_pClient->publishMessage(STRING_TO_CSTR(topicName), STRING_TO_CSTR(msg), CSTRING_LEN(msg), qos);
}
