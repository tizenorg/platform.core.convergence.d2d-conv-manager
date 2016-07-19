/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MSF_CHANNEL_H__
#define __MSF_CHANNEL_H__

#include <libwebsockets.h>
#include <list>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>
#include "Result.h"
#include "Error.h"
#include "Message.h"
#include "Client.h"

using namespace std;

class Service;
class Clients;
class Channel;

class ChannelConnectionHandler {
public:
	ChannelConnectionHandler();
	static string PING;
	static string PONG;
	static Channel *channel_ptr;
	void run();
	void setPingTimeout(long);
	void resetLastPingReceived();
	void calculateAverageRT();
	void stopPing();
	void startPing(Channel *);
	static void *Pinging(void *);
	static void ping_again(void *);
	static pthread_t ping_thread;

private:
	long pingTimeout;
	long lastPingReceived;
	int numPings;
	long startTime;
	long pingSent;
	double average;
	long longestRT;
	bool running;
};

enum Result_base_Type {
	Result_Client = 1,
	Result_bool = 2,
	Result_ApplicationInfo = 3
};

typedef void (*_message_receive_cb)(Message msg);

class OnConnectListener {
public:
	virtual void onConnect(Client Client) {}
};

class OnDisconnectListener {
public:
	virtual void onDisconnect(Client Client) {}
};

class OnClientConnectListener {
public:
	virtual void onClientConnect(Client Client) {}
};

class OnClientDisconnectListener {
public:
	virtual void onClientDisconnect(Client Client) {}
};

class OnReadyListener {
public:
	virtual void onReady() {}
};

class OnMessageListener {
public:
	virtual void onMessage(Message msg) {}
};

class OnErrorListener {
public:
	virtual void onError(/*Error error*/) {}
};

class OnPublishListener {
public:
	virtual void onPublished(bool, void*) {}
};

class Channel {
public:
	Channel();
	Channel(Service *, string);
	~Channel();
	static Channel *create(Service *service, string uri);
	ChannelConnectionHandler *connectionHandler;
	Clients *getclients();
	Client getclient(string id);
	void setConnectionTimeout(long timeout);
	bool isConnected();
	bool isWebSocketOpen();
	bool connect();
	void disconnect();
	string getChannelUri(map<string, string> *);
	void addOnMessageListener(string, OnMessageListener *);
	void addOnAllMessageListener(OnMessageListener *);
	void removeAllMessageListener();
	void removeOnMessageListeners(string);
	void removeOnMessageListener(string, OnMessageListener *);
	void removeOnMessageListeners();
	void removeAllListeners();
	void register_message_receive_cb(_message_receive_cb cb);
	void setonConnectListener(OnConnectListener *);
	void unsetonConnectListener();
	void setonClientConnectListener(OnClientConnectListener *);
	void unsetonClientConnectListener();
	void setonDisconnectListener(OnDisconnectListener *);
	void unsetonDisconnectListener();
	void setonClientDisconnectListener(OnClientDisconnectListener *);
	void unsetonClientDisconnectListener();
	void setonReadyListener(OnReadyListener *);
	void unsetonReadyListener();
	void setmessageListeners(list<OnMessageListener>);
	void unsetmessageListeners();
	void setonErrorListener(OnErrorListener *);
	void unsetonErrorListener();
	void setonPublishListener(OnPublishListener *);
	void unsetonPublishListener();
	void publish(string event, const char *data, void *user_data);
	void publish(string event, const char *data, unsigned char payload[],
		int payload_size, void *user_data);
	void publish(string event, const char *data, const char *target, void *user_data);
	void publish(string event, const char *data, const char *target,
		unsigned char payload[], int payload_size, void *user_data);
	void publish(string event, const char *data, Client client, void *user_data);
	void publish(string event, const char *data, Client client,
		unsigned char payload[], int payload_size, void *user_data);
	void publish(string event, const char *data, list<Client> clients, void *user_data);
	void publish(string event, const char *data, list<Client> clients,
		unsigned char payload[], int payload_size, void *user_data);
	void publishMessage(string event, const char *data, const char *to,
			unsigned char payload[], int payload_size, void *user_data);
	void publishMessage(string method, string event, const char *data,
			const char *to, unsigned char payload[],
			int payload_size, void *user_data);
	// unsigned char *createBinaryMessage(string json, unsigned char payload[],
	// int payload_size);
	static void init_json_key_map();
	static void foreach_json_object(JsonObject *object, const gchar *name,
				JsonNode *node, gpointer user_data);
	static void foreach_json_array(JsonArray *object, guint index,
				JsonNode *node, gpointer user_data);

	static void write_socket(Channel*);
	static int callback_lws_mirror(struct lws *wsi,
				enum lws_callback_reasons reason, void *user,
				void *in, size_t len);
	static void *pt_startConnect(void *arg);
	static void get_ip_port_from_uri(string uri, string* dest_ip, int* dest_port);

	Clients *clients; //=new Clients(this );

protected:
	void start_app(char *data, int, string);
	bool connect(Result_Base *result1);
	bool connect(map<string, string>, Result_Base *result1);
	void disconnect(Result_Base *result1);
	void registerCallback(string, void *, int);
	void handleError(string, Error);
	string getapifromUri(string uri);
	string getUID();

	string ChannelID;
	bool connected;
	bool waitForOnReady;
	Service *service;
	string m_uri;
	int errstatus;
	int errcode;
	string errMsg;
	OnErrorListener *onErrorListener = NULL;
	OnConnectListener *onConnectListener = NULL;
	OnDisconnectListener *onDisconnectListener = NULL;
	OnClientConnectListener *onClientConnectListener = NULL;
	OnClientDisconnectListener *onClientDisconnectListener = NULL;
	OnReadyListener *onReadyListener = NULL;
	OnPublishListener *onPublishListener = NULL;
	map<string, list<OnMessageListener *> > messageListeners;
	//Result_Base *connect_cb = NULL;
	//Result_Base *disconnect_cb = NULL;
	static map<Channel *, int> channel_alive_map;

private:
	void emit(string event, const char *msg, string from, unsigned char *payld,
	      int payld_size);
	void release_callback(string uid);
	pair<void *, int> getcallback(string uid);
	void handleSocketClosed();
	void handleSocketClosedAndNotify();
	void handleConnect(string);
	void handleBinaryMessage(unsigned char payload[]);
	void json_parse(const char *in);
	void handleConnectMessage(string UID);
	void handleMessage(string UID);
	void handleMessage(string UID, unsigned char payload[]);
	void handleApplicationMessage(string);
	void doApplicationCallback(Result_Base *);
	void handleClientConnectMessage();
	void handleClientDisconnectMessage();
	void handleErrorMessage(string);
	void handleReadyMessage();
	void handleDisconnectMessage();
	void handleClientMessage(const char *msg, unsigned char payload[]);
	void create_websocket(void *att);
	unsigned char *prepareMessageMap(string, string, const char *data,
				const char *to, long *,
				unsigned char payload[], int payload_size);

	static string ROUTE;
	static string ERROR_EVENT;
	static string CONNECT_EVENT;
	static string CLIENT_CONNECT_EVENT;
	static string CLIENT_DISCONNECT_EVENT;
	static string READY_EVENT;
	unsigned char *write_buf[1000];
	int write_buf_count;
	int write_buf_index;
	int write_buf_last_sent_index;
	void* publish_user_data[1000];
	int write_buf_len[1000];
	bool write_buf_binary_flag[1000];
	//LWS_SEND_BUFFER_PRE_PADDING + 4096 + LWS_SEND_BUFFER_POST_PADDING];
	static pthread_t connect_thread;
	int mirror_lifetime;
	int force_exit;
	char *messagedata;
	//bool isWrite;
	//bool binary_message;
	bool disconnecting;
	//long buflen;
	unsigned char cl_payload[1000];
	int cl_payload_size;
	int was_closed;
	Client client;
	long long clientconnectTime;
	bool clientisHost;
	string clientid;
	map<string, string> clientattributes;
	JsonObject *resultobj;
	bool errobj;
	string data;
	string method;
	string eventType;
	string from;
	bool resultresp;
	string UID;
	struct lws *wsi_mirror;
	struct lws_context *Context;
	list<Client> clientList;
	Client me;
	string msg_subject;
	_message_receive_cb message_receive_cb = NULL;
	OnMessageListener *everyMessageListener = NULL;
	int msg_id;
	bool isLaunched;
	map<string, pair<void *, int> > callbacks;
	map<void *, int> onReadyCallbacks;
	static map<string, int> json_keys;
	static JsonObject *root_json_object;
	string server_ip_address;
	int server_port;
};

#endif
