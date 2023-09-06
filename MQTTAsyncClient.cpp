#include "MQTTAsyncClient.h"
#include "MQTTAsync.h"
#include <quuid.h>
#pragma execution_character_set("utf-8")

#define STRING_TO_CSTR(STR) \
  	STR.toLocal8Bit().data()
#define CSTRING_LEN(STR) \
 	STR.toLocal8Bit().length()
#define CSTR_TO_STRING(CSTR, LEN) \
  	QString::fromLocal8Bit(CSTR, LEN)



class MQTTAsyncClientPrivate
{
public:
	MQTTAsyncClientPrivate()
		: handle(nullptr) { }
	~MQTTAsyncClientPrivate() { this->destroy(); }

	bool init(const char* serverURI, const char* clientId)
	{
		return (handle == nullptr) ?
			(MQTTAsync_create(&handle, serverURI, clientId, MQTTCLIENT_PERSISTENCE_NONE, nullptr) == MQTTASYNC_SUCCESS)
			: false;
	}
	void destroy()
	{
		if (handle != nullptr)
		{
			MQTTAsync_destroy(&handle); handle = nullptr;
		}
	}

	bool setCallback(MQTTSyncMessageArrived* onMessageArrived, void* context)
	{
		m_MessageArrivedContext.onMessageArrived = onMessageArrived;
		m_MessageArrivedContext.context = context;
		return MQTTAsync_setCallbacks(handle, &m_MessageArrivedContext, nullptr, MessageArrived, nullptr) == MQTTASYNC_SUCCESS;
	}

	bool connect(const SMQTTAsyncConnInfo& info, MQTTSyncConnectResult* onConnResult, void* context)
	{
		MQTTAsync_connectOptions connOpts = MQTTAsync_connectOptions_initializer;
		connOpts.keepAliveInterval = info.keepAliveInterval;
		if (CSTRING_LEN(info.username) > 0) { connOpts.username = STRING_TO_CSTR(info.username); }
		if (CSTRING_LEN(info.password) > 0) { connOpts.password = STRING_TO_CSTR(info.password); }
		connOpts.connectTimeout = info.connectTimeout;
		connOpts.retryInterval = info.retryInterval;
		connOpts.MQTTVersion = info.MQTTVersion;
		if (onConnResult != nullptr)
		{
			connOpts.onSuccess = onSuccess;
			connOpts.onFailure = onFailure;
			m_ConnectResultContext.onConnResult = onConnResult;
			m_ConnectResultContext.context = context;
			connOpts.context = &m_ConnectResultContext;
		}

		return MQTTAsync_connect(handle, &connOpts) == MQTTASYNC_SUCCESS;
	}
	bool disconnect()
	{
		return MQTTAsync_disconnect(handle, nullptr) == MQTTASYNC_SUCCESS;
	}

	bool subTopic(const char* topic, const int& qos)
	{
		return MQTTAsync_subscribe(handle, topic, qos, nullptr) == MQTTASYNC_SUCCESS;
	}
	bool unsubTopic(const char* topic)
	{
		return MQTTAsync_unsubscribe(handle, topic, nullptr) == MQTTASYNC_SUCCESS;
	}

	bool sendMessage(const char* topicName, const char* msg, const int& len, const int& qos)
	{
		MQTTAsync_message pubMsg = MQTTAsync_message_initializer;
		pubMsg.payload = (void*)msg;
		pubMsg.payloadlen = len;
		pubMsg.qos = qos;
		return MQTTAsync_sendMessage(handle, topicName, &pubMsg, nullptr) == MQTTASYNC_SUCCESS;
	}

private:
	static int MessageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
	{
		auto messageArrivedContext = (SMessageArrivedContext*)context;
		messageArrivedContext->onMessageArrived(CSTR_TO_STRING(topicName, -1),
			CSTR_TO_STRING((char*)message->payload, message->payloadlen),
			messageArrivedContext->context);
		MQTTAsync_freeMessage(&message);
		MQTTAsync_free(topicName);
		return 1;
	}

	static void onSuccess(void* context, MQTTAsync_successData* response)
	{
		auto connectResultContext = (SConnectResultContext*)context;
		connectResultContext->onConnResult(0, connectResultContext->context);
	}

	static void onFailure(void* context, MQTTAsync_failureData* response)
	{
		auto connectResultContext = (SConnectResultContext*)context;
		connectResultContext->onConnResult(1, connectResultContext->context);
	}

private:
	MQTTAsync handle;

	struct SMessageArrivedContext
	{
		SMessageArrivedContext()
			: onMessageArrived(nullptr)
			, context(nullptr)
		{ }

		MQTTSyncMessageArrived* onMessageArrived;
		void* context;
	} m_MessageArrivedContext;

	struct SConnectResultContext
	{
		SConnectResultContext()
			: onConnResult(nullptr)
			, context(nullptr)
		{ }

		MQTTSyncConnectResult* onConnResult;
		void* context;
	} m_ConnectResultContext;
};

MQTTAsyncClient::MQTTAsyncClient()
	: m_pClient(new MQTTAsyncClientPrivate())
{
}

MQTTAsyncClient::~MQTTAsyncClient()
{
	delete m_pClient; m_pClient = nullptr;
}

bool MQTTAsyncClient::init(const QString& serverURI, const QString& clientId)
{
	const auto id = clientId.isEmpty() ? QUuid::createUuid().toString() : clientId;
	return m_pClient->init(STRING_TO_CSTR(serverURI), STRING_TO_CSTR(id));
}

void MQTTAsyncClient::destroy()
{
	m_pClient->destroy();
}

bool MQTTAsyncClient::setCallback(MQTTSyncMessageArrived* onMessageArrived, void* context)
{
	return m_pClient->setCallback(onMessageArrived, context);
}

bool MQTTAsyncClient::connect(const SMQTTAsyncConnInfo& info, MQTTSyncConnectResult* onConnResult, void* context)
{
	return m_pClient->connect(info, onConnResult, context);
}

bool MQTTAsyncClient::disconnect()
{
	return m_pClient->disconnect();
}

bool MQTTAsyncClient::subTopic(const QString& topic, const int& qos)
{
	return m_pClient->subTopic(STRING_TO_CSTR(topic), qos);
}

bool MQTTAsyncClient::unsubTopic(const QString& topic)
{
	return m_pClient->unsubTopic(STRING_TO_CSTR(topic));
}

bool MQTTAsyncClient::sendMessage(const QString& topicName, const QString& msg, const int& qos)
{
	return m_pClient->sendMessage(STRING_TO_CSTR(topicName), STRING_TO_CSTR(msg), CSTRING_LEN(msg), qos);
}
