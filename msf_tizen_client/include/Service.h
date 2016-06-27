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

#ifndef __MSF_SERVICE_H__
#define __MSF_SERVICE_H__

#include <string>
#include <map>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>


using namespace std;

class Application;
class Search;
class Result_Base;
class Channel;

struct position {
	int start;
	int end;
};

struct ServiceInfo {
	string infoId;
	string infoVersion;
	string infoName;
	string infotype;
	string infoURI;
};

class Service
{
	static string ID_PROPERTY;
	static string VERSION_PROPERTY;
	static string NAME_PROPERTY;
	static string TYPE_PROPERTY;
	static string ENDPOINT_PROPERTY;
	string id = "";
	string version = "";
	string name = "";
	string type = "";
	string uri = "";
	static string curl_data;
	typedef std::map<std::string, std::string> map_type;
	static Service local_service;
	void *result_ptr;

	static bool success_get_id;
	static string remote_device_id;
	static std::map<std::string, std::string> dev_id_map;

public:
	Service();
	Service(string, string, string, string, string);
	static ServiceInfo serviceval;
	static Result_Base *Resulturi;
	Result_Base *Resultdevice;

	position findServiceValue(string, char *);
	void getDeviceInfo(Result_Base*);
	void create(char*);
	string getUri();
	string getName();
	string getType();
	string getId();
	string getVersion();
	static string getUniqueId(string address);
	static Service getLocal(void);
	static void getByURI(string, Result_Base*);
	static void getByURI(string, long, Result_Base *result);
	static void getById(string id, Result_Base *result);
	static int curl_service_calling(string uri, long, void *);
	//static Search *search();
	static size_t createdata(char *buf, size_t size, size_t nmemb, void *up);
	static void createdata_process(string data,  void *);
	static int json_parse_service(const char *in,  void *);
	static Service create(ServiceInfo);
	static Service create(map<string, string>);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
	Channel *createChannel(string uri);
	Application createApplication(string uri);
	Application createApplication(string uri, string channelId);
	Application createApplication(string uri, string channelId, map<string, string> startArgs) ;
};
#endif