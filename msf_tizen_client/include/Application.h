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

#ifndef __MSF_APPLICATION_H__
#define __MSF_APPLICATION_H__

#include"Result.h"
#include"ApplicationInfo.h"
#include"Channel.h"
#include"Service.h"

class OnStartAppListener {
public:
	virtual void onStart(bool result) {}
	virtual void onError(Error e) {}
};

class OnStopAppListener {
public:
	virtual void onStop(bool result) {}
	virtual void onError(Error e) {}
};


class OnInstallListener {
public:
	virtual void onInstall(bool result) {}
	virtual void onError(Error e) {}
};

class Application:public Channel
{
public:
	bool webapp;
	static string ROUTE_APPLICATION;
	static string ROUTE_WEBAPPLICATION;
	static string curl_data;
	bool install_result = false;
	OnInstallListener *install_listener = NULL;
	OnStartAppListener *start_app_listener = NULL;
	OnStopAppListener *stop_app_listener = NULL;

	string createdata_data;
	string curl_install_data;
	typedef std::map<std::string, std::string> map_null;
	map<string, string> *startArgs;
	int curl_install(string uri);
	int curl_application_calling(string uri);
	map<string, string> getparams();
	void invokeMethod(string method, map<string, string> params, Result_Base *);
	void invokeMethod(string method, map<string, string> params,  string messageID, Result_Base *);
	void realDisconnect(Result_Base *result);

public:
	Application();
	Application(Service *service, string uri, string id);
	Application(Service *service, string uri, string id, map<string, string> startArgs);
	virtual ~Application();
	Result_Base *result ;
	bool isConnected();
	bool isWebapp();
	void createdata_process(string data);
	void getinfo(Result_Base *result);
	void connect();
	void connect(Result_Base *r);
	void connect(map<string, string> attributes, Result_Base *r);
	void disconnect();
	//void disconnect(Result_Base *r);
	//void disconnect(bool stopOnDisconnect);
	void install();
	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t createdata(char *buf, size_t size, size_t nmemb, void *up);
	static Application create(Service *service, string uri);
	static Application create(Service *service, string uri, string id, map<string, string>);
	void start();
	void stop();
	void stop(Result_Base*);
	void json_parse(const char *in);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
	void setonInstallListener(OnInstallListener *);
	void unsetonInstallListener();
	void setonStartAppListener(OnStartAppListener *);
	void unsetonStartAppListener();
	void setonStopAppListener(OnStopAppListener *);
	void unsetonStopAppListener();
};

#endif
