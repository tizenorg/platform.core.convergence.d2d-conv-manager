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

#include "Application.h"
#include <curl/curl.h>
#include <dlog.h>
#include "Debug.h"
#include <cstring>
#include "URIparser.h"
#include "Message.h"
#include "Clients.h"

using namespace std;
using namespace uri_parser;

string Application::ROUTE_APPLICATION = "applications/";
string Application::curl_data = "";

class start_result_callback : public Result_Base
{
public:
	OnStartAppListener** in_result_cb  = NULL;

	void onSuccess(bool result)
	{
		MSF_DBG("start result callback onSuccess()");

		if (in_result_cb != NULL) {
			if (((*in_result_cb) != NULL)) {
				(*in_result_cb)->onStart(result);
			} else {
			}
		}

		delete this;
	}

	void onError(Error)
	{
		MSF_DBG("start result callback onError()");
		if (in_result_cb != NULL) {
			if (((*in_result_cb) != NULL)) {
				(*in_result_cb)->onStart(false);
			} else {
			}
		}
		delete this;
	}
};

class stop_result_callback : public Result_Base
{
public:
	OnStopAppListener** in_result_cb  = NULL;

	void onSuccess(bool result)
	{
		MSF_DBG("stop result callback onSuccess()");
		if (in_result_cb != NULL) {
			if (((*in_result_cb) != NULL)) {
				(*in_result_cb)->onStop(result);
			} else {
			}
		}

		delete this;
	}

	void onError(Error)
	{
		MSF_DBG("stop result callback onError()");
		if (in_result_cb != NULL) {
			if (((*in_result_cb) != NULL)) {
				(*in_result_cb)->onStop(false);
			} else {
			}
		}
		delete this;
	}
};

Application::Application()
{
	MSF_DBG("Application()");
	waitForOnReady = false;
	webapp = false;
	install_listener = NULL;
	install_result = false;
}

Application::Application(Service *se, string Uri, string Id)
{
	MSF_DBG("Application()");
	map<string, string> startargs = std::map<std::string, std::string>();
	service = se;
	m_uri = Uri;
	ChannelID = Id;
	bool webApp = false;
	uri_parser::URIParse uri2(m_uri);
	if (!(uri2.getURIScheme() == "")) {
		MSF_DBG("webapp set as true");
		webApp = true;
	}

	this->webapp = webApp;
	this->startArgs = &startargs;
	waitForOnReady = false;
	install_listener = NULL;
	install_result = false;
}

Application::Application(Service *se, string Uri, string Id, map<string, string> startargs)
{
	MSF_DBG("Application()");
	service = se;
	m_uri = Uri;
	ChannelID = Id;
	bool webApp = false;
	uri_parser::URIParse uri2(m_uri);
	if (!(uri2.getURIScheme() == "")) {
		MSF_DBG("webapp set as true");
		webApp = true;
	}

	this->webapp = webApp;
	this->startArgs = &startargs;
	waitForOnReady = false;
	install_listener = NULL;
	install_result = false;
}

Application::~Application()
{
	MSF_DBG("~Application()");
}

bool Application::isConnected()
{
	return (Channel::isConnected());
}

bool Application::isWebapp()
{
	return webapp;
}

void Application::getinfo(Result_Base *result1)
{
	string uri = service->getUri();
	if (isWebapp()) {
		uri += "webapplication/";
	} else {
		//TODO
		//uri += "applications/" + service->getid() + "/";
	}

	result = result1;
	int ret = curl_application_calling(uri);
	if (ret == -1) {
		if (result1 != NULL)
			result1->onError(Error::create("Failed to get info"));
	}
}

void Application::start()
{
	Clients *clientstemp = Channel::getclients();

	if (clientstemp->getHost() == NULL)
		waitForOnReady = true;

	map<string, string> params = getparams();

	start_result_callback* start_app_result_listener = new start_result_callback();
	start_app_result_listener->in_result_cb = &start_app_listener;
	invokeMethod(webapp?Message::METHOD_WEB_APPLICATION_START : Message::METHOD_APPLICATION_START, params, start_app_result_listener);
}

void Application::stop()
{
	map<string, string> params = getparams();
	stop_result_callback* stop_app_result_listener = new stop_result_callback();
	stop_app_result_listener->in_result_cb = &stop_app_listener;
	invokeMethod(webapp?Message::METHOD_WEB_APPLICATION_STOP : Message::METHOD_APPLICATION_STOP, params, stop_app_result_listener);
}

void Application::install()
{
	int ret;
	if (webapp) {
		string randID = Channel::getUID();
		if (install_listener) {
			install_listener->onError(Error::create("Unsupported Method"));
		}
		/* Channel::registerCallback(randID,(void*)result,Result_bool); *
		 *  handleError(randID,Error::create("Unsupported Method"));   */
	} else {
		string Uri = service->getUri().append(ROUTE_APPLICATION).append(m_uri);
		ret = Application::curl_install(Uri);
		if (ret == -1) {
			if (install_listener) {
				install_listener->onInstall(false);
				install_listener->onError(Error::create("Failed to Install"));
			}
		}
	}
}

map<string, string> Application::getparams()
{
	string messagekey = Message::PROPERTY_ID;
	string id = m_uri;
	if (webapp) {
		messagekey = Message::PROPERTY_URL;
	}
	map <string, string> params;
	params[messagekey] = id;
	return params;
}


void Application::connect()
{
	connect(NULL);
}

void Application::connect(Result_Base *res)
{
	connect(map<string, string>(), res);
}

void Application::connect(map<string, string> attributes, Result_Base *res)
{
	Channel::connect(attributes, res);
}

void Application::disconnect()
{
	realDisconnect(NULL);
}

void Application:: realDisconnect(Result_Base *result)
{
	Channel::disconnect(result);
}

void Application::invokeMethod(string method, map<string, string> params, string messageID, Result_Base *callback)
{
	if (!Channel::isConnected()) {
		Channel::handleError(messageID, Error::create("Not Connected"));
		return;
	}

	int l = 0;
	string id = webapp?"url":"id";
	char buffer[2000];
	l += snprintf((char *)&buffer, sizeof(buffer), " {\n \"method\": \"%s\",\n \"id\": %s, \n \"params\" : { \n \"%s\": \"%s\" \n } \n }", method.c_str(), messageID.c_str(), id.c_str(), params[id].c_str());
	buffer[l] ='\0';
	Channel::start_app(buffer, l, messageID);
}

void Application::invokeMethod(string method, map<string, string> params, Result_Base *callback)
{
	string messageID = Channel::getUID();
	Channel::registerCallback(messageID, (void*)callback, Result_bool);
	invokeMethod(method, params,  messageID, callback);
}


Application Application::create(Service *service, string uri)
{
	MSF_DBG("Application::create(service, uri)");
	if (!((service == NULL) || (uri == ""))) {
		string id = uri;
		uri_parser::URIParse uri2(uri);
		if (!(uri2.getURIScheme() == "")) {
			id  = uri2.getURIHost();
		}
		Application app(service, uri, id , std::map<std::string, std::string>());
		return app;
	} else {
		Application app;
		return app;
	}
}

Application Application::create(Service *service, string uri, string id, map<string, string> startargs)
{
	MSF_DBG("Application::create(service, uri)");
	if (!((service == NULL) || (uri == "") || (id == ""))) {
		Application app(service, uri, id , startargs);
		return app;
	} else {
		Application app;
		return app;
	}
}

void Application::createdata_process(string data)
{
	ApplicationInfo InfoObj;
	InfoObj.create(data);
	result->onSuccess(InfoObj);
}

size_t Application::createdata(char* buf, size_t size, size_t nmemb, void* up)
{
	if (buf != NULL) {
		curl_data.append(buf, size*nmemb);
	} else {
		MSF_DBG("Application::createdata() buf is null");
	}

	return size*nmemb;
}

size_t Application::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	if (ptr != NULL)
		static_cast<Application*>(stream)->curl_install_data.append(static_cast<const char*>(ptr), size*nmemb);

	return size*nmemb;
}

int Application::curl_application_calling(string uri)
{
	CURL *curl;
	CURLcode res = CURLE_FAILED_INIT;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl = curl_easy_init();
	dlog_print(DLOG_INFO, "MSF", "uri = %s", uri.c_str());

	if (curl) {
		const char *c = uri.c_str();
		curl_easy_setopt(curl, CURLOPT_URL, c);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Application::createdata);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			MSF_DBG("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			createdata_process(curl_data);
		}

		curl_install_data.clear();
		curl_easy_cleanup(curl);

		if (res != CURLE_OK)
			return -1;
		else
			return 0;
	}

	return -1;
}

void Application::foreach_json_object(JsonObject* object, const gchar* key, JsonNode* node, gpointer user_data)
{
	Application* p = static_cast<Application*>(user_data);

	if (!strncmp(key, "ok", 2)) {
		p->install_result = json_node_get_boolean(node);
	} else if (!strncmp(key, "status", 6)) {
		p->errstatus = json_node_get_int(node);
	} else if (!strncmp(key, "message", 7)) {
		p->errMsg = json_node_get_string(node);
	} else if (!strncmp(key, "code", 4)) {
		p->errcode = json_node_get_int(node);
		p->install_result = false;
	}
}

void Application::json_parse(const char* in)
{
	JsonParser* parser = json_parser_new();
	if (json_parser_load_from_data(parser, in, -1, NULL)) {
		JsonNode* node = json_parser_get_root(parser);

		if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
			json_object_foreach_member(json_node_get_object(node), foreach_json_object, this);
		}
	}
}

int Application::curl_install(string uri)
{
	CURL *curl;
	CURLcode res = CURLE_FAILED_INIT;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Application::read_callback);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			MSF_DBG("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			json_parse(curl_install_data.c_str());

			if (install_listener != NULL) {
				if (install_result) {
					install_listener->onInstall(install_result);
				} else {
					string err = "";
					err.append("status:");
					err.append(to_string(errstatus));
					err.append(" message:");
					err.append(errMsg);
					err.append(" code:");
					err.append(to_string(errcode));
					install_listener->onError(Error::create(err));
				}
			}
		}
		curl_install_data.clear();

		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}

	if (res != CURLE_OK)
		return -1;
	else
		return 0;
}

void Application::setonInstallListener(OnInstallListener * listener)
{
	install_listener = listener;
}

void Application::unsetonInstallListener()
{
	 install_listener = NULL;
}

void Application::setonStartAppListener(OnStartAppListener *listener)
{
	start_app_listener = listener;
}

void Application::unsetonStartAppListener()
{
	start_app_listener = NULL;
}

void Application::setonStopAppListener(OnStopAppListener *listener)
{
	stop_app_listener = listener;
}

void Application::unsetonStopAppListener()
{
	stop_app_listener = NULL;
}


