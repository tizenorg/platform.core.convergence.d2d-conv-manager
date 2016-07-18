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

#include "Service.h"
#include "Search.h"
#include "Result.h"
#include "Channel.h"
#include "Debug.h"
#include "Device.h"
#include "Application.h"

#include <curl/curl.h>

string Service::ID_PROPERTY         = "id";
string Service::VERSION_PROPERTY    = "ve";
string Service::NAME_PROPERTY       = "fn";
string Service::TYPE_PROPERTY       = "md";
string Service::ENDPOINT_PROPERTY   = "se";

Result_Base *Service::Resulturi = NULL;
ServiceInfo Service::serviceval;

Service Service::local_service;

bool Service::success_get_id = false;
string Service::remote_device_id = "";
std::map<std::string, std::string> Service::dev_id_map;

string Service::curl_data = "";

Service::Service()
{
	//Resulturi = NULL;
	//Resultdevice=NULL;
	//devicedata = NULL;
}

Service::Service(string id1, string version1, string name1, string type1, string endPoint1)
{
	id = id1;
	version = version1;
	name = name1;
	type = type1;
	uri = endPoint1;
}

void Service:: getById(string id, Result_Base *result)
{
	Search search;
	Service service;

	bool status = search.start();
	if (!status)	{
		service = search.getServiceById(id);
	} else {
		sleep(3);
		service = search.getServiceById(id);
		search.stop();
	}

	if (service.getId().length() == 0) {
		if (result != NULL)
			result->onError(Error::create("Service Not Found"));
	} else {
		if (result != NULL)
			result->onSuccess(service);
	}
}

string Service::getName()
{
	return name;
}

string Service::getVersion()
{
	return version;
}

string Service::getType()
{
	return type;
}

string Service::getId()
{
	return id;
}

string Service::getUri()
{
	return uri;
}

void Service::create(char *service_txt)
{
	position posvalues;
	string txt(service_txt);

	posvalues = findServiceValue(ID_PROPERTY, service_txt);
	id = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(VERSION_PROPERTY, service_txt);
	version = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(NAME_PROPERTY, service_txt);
	name = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(TYPE_PROPERTY, service_txt);
	type = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));

	posvalues = findServiceValue(ENDPOINT_PROPERTY, service_txt);
	uri = txt.substr((posvalues.start)-1, ((posvalues.end)-(posvalues.start)+1));
}

Service Service::create(ServiceInfo servicevalue)
{
	//if (
	string id = servicevalue.infoId;
	string version = servicevalue.infoVersion;
	string name = servicevalue.infoName;
	string type = servicevalue.infotype;
	string endPoint = servicevalue.infoURI;//TODO
	Service service = Service(id, version, name, type, endPoint);
	return service;
}

Service Service::create(map<string, string> serviceMap)
{
	string id = serviceMap.find(ID_PROPERTY)->second;
	string version = serviceMap.find("version")->second;
	string name = serviceMap.find("name")->second;
	string type = serviceMap.find("type")->second;
	string endPoint = serviceMap.find("uri")->second;//TODO
	Service service = Service(id, version, name, type, endPoint);
	return service;
}

position Service::findServiceValue(string substrng, char *strng)
{
	const char *src = strstr(strng, substrng.c_str());
	int startpoint = (src-strng)+sizeof(substrng);
	std::string temp = "\"";
	const char *dest = strstr(src, temp.c_str());
	int endpoint = (dest-strng);
	position values;
	values.start = startpoint;
	values.end = endpoint;
	return values;
}

void Service::getDeviceInfo(Result_Base *dev_result)
{
	curl_service_calling(uri, 5000, dev_result);
}

string Service::getUniqueId(string address)
{
	string ret_id;

	class GetLocalServiceCallback : public Result_Base
	{
		public:
			void onSuccess(Service service)
			{
				MSF_DBG("getUniqueId() : success to get remote service info");
				remote_device_id = service.getId();
				success_get_id = true;
			}

			void onError(Error)
			{
				MSF_ERR("getUniqueId() : Fail to get remote service info");
				success_get_id = false;
			}
	};

	GetLocalServiceCallback r1Service;
	string remote_uri = "http://" + address + "/api/v2/";
	success_get_id = false;
	getByURI(remote_uri, 5000, &r1Service);

	if (success_get_id) {
		MSF_DBG("remote_device_id[%s] for address[%s]", remote_device_id.c_str(), address.c_str());
		ret_id = remote_device_id;
	} else {
		ret_id = "";
	}

	return ret_id;
}

Service Service::getLocal(void)
{
	class GetLocalServiceCallback : public Result_Base
	{
		public:
			void onSuccess(Service service)
			{
				MSF_DBG("service name : %s\n", service.getName().c_str() ? service.getName().c_str() : "name is NULL");
				local_service = service;
			}

			void onError(Error)
			{
				MSF_DBG("getLocal() : Fail to get local service info");
			}
	};

	GetLocalServiceCallback r1Service;
	string uri = "http://127.0.0.1:8001/api/v2/";
	getByURI(uri, 2000, &r1Service);

	return local_service;
}

void Service::getByURI(string uri, Result_Base *result)
{
	getByURI(uri, 5000, result);
}

void Service::getByURI(string uri, long timeout, Result_Base *result)
{
	MSF_DBG("getByURI() uri = %s", uri.c_str());

	Resulturi = result;
	//Resultdevice = NULL;
	curl_service_calling(uri, timeout, NULL);
	Resulturi = NULL;
}

int Service::curl_service_calling(string uri, long timeout, void *dev_result_ptr)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl = curl_easy_init();
	if (curl) {
		const char *c = uri.c_str();
		curl_easy_setopt(curl, CURLOPT_URL, c);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Service::createdata);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			MSF_DBG("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			createdata_process("", dev_result_ptr);
		} else {
			createdata_process(curl_data, dev_result_ptr);
			curl_data.clear();
		}

		curl_easy_cleanup(curl);
	}

	return 0;
}

size_t Service::createdata(char *buf, size_t size, size_t nmemb, void *up)
{
	if (buf != NULL) {
		curl_data.append(buf, size*nmemb);
	} else {
		MSF_DBG("createdata() buf is null");
	}

	return size*nmemb;
}

void Service::createdata_process(string data, void *dev_result_ptr)
{
	if (data.length() != 0) {
		json_parse_service(data.c_str(), dev_result_ptr);
	} else {
		if (Resulturi != NULL) {
			Resulturi->onError(Error::create("Timeout"));
		}
		if (dev_result_ptr != NULL) {
			(static_cast<Result_Base*> (dev_result_ptr))->onError(Error::create("Not Found"));
		}
	}
}

int Service::json_parse_service(const char *in, void *ptr)
{
	JsonParser *parser = json_parser_new();

	if (json_parser_load_from_data(parser, in, -1, NULL)) {
	} else {
		MSF_DBG("json_parsing error");
	}

	JsonObject *root = json_node_get_object(json_parser_get_root(parser));

	if (json_object_has_member(root, "id")) {
		serviceval.infoId = json_object_get_string_member(root, "id");
	}

	if (json_object_has_member(root, "version")) {
		serviceval.infoVersion = json_object_get_string_member(root, "version");
	}

	if (json_object_has_member(root, "name")) {
		serviceval.infoName = json_object_get_string_member(root, "name");
	}

	if (json_object_has_member(root, "type")) {
		serviceval.infotype = json_object_get_string_member(root, "type");
	}

	if (json_object_has_member(root, "uri")) {
		serviceval.infoURI = json_object_get_string_member(root, "uri");
	}

	if (json_object_has_member(root, "device")) {
		if (ptr != NULL) {
			Device dev = Device::create(json_object_get_member(root, "device"));
			(static_cast<Result_Base *>(ptr))->onSuccess(dev);
		}

		JsonObject *dev_object = json_node_get_object(json_object_get_member(root, "device"));

		if (json_object_has_member(dev_object, "WifiMac")) {
			serviceval.infoId = json_object_get_string_member(dev_object, "WifiMac");
		}

	}

	if ((Resulturi!= NULL)) {
		Resulturi->onSuccess(create(serviceval));
	}

	return 0;
}

Application  Service::createApplication(string uri)
{
	if (!(uri == "")) {//check length and return error
		Application application = Application::create(this, uri);
		return application;
	} else {
		Application app;
		return app;
	}
}

Application Service::createApplication(string uri, string channelId)
{
	if (!((uri == "") || (channelId == ""))) {
		Application application = Application::create(this, uri, channelId, map_type());
		return application;
	} else {
		Application app;
		return app;
	}
}

Application Service::createApplication(string uri, string channelId, map<string, string> startArgs)
{
	if (!((uri == "") || (channelId == ""))) {
		Application application = Application::create(this, uri, channelId, startArgs);
		return application;
	} else {
		Application app;
		return app;
	}
}

Channel *Service::createChannel(string uri)
{
	Channel *channel = Channel::create(this, uri);
	return channel;
}
