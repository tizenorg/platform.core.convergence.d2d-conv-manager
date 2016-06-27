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

#ifndef __MSF_MSFD_SEARCH_PROVIDER_H__
#define __MSF_MSFD_SEARCH_PROVIDER_H__

#include <map>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>
#include"SearchProvider.h"

using namespace std;

class Search;

class MSFDSearchProvider: public SearchProvider
{
	bool receive;
	string state;
	string id;
	long ttl;
	string url;

public:

	static string KEY_TYPE_STATE;
	static string KEY_TTL;
	static string KEY_SID;
	static string KEY_DATA;
	static string KEY_VERSION_1;
	static string KEY_VERSION_2;
	static string KEY_URI;
	static string TYPE_DISCOVER;
	static string STATE_UP;
	static string STATE_DOWN;
	static string STATE_ALIVE;
	static int SERVICE_CHECK_TIMEOUT;
	static int flag;
	int fd;
	map<string, long> aliveMap;
	MSFDSearchProvider();
	MSFDSearchProvider(Search *);
	~MSFDSearchProvider();
	void start() ;
	bool stop();
	void createMSFD();
	void processReceivedMsg(char *, int);
	static SearchProvider create();
	static SearchProvider create(Search *);
	void json_parse(const char *in);
	void reapServices();
	void updateAlive(string id , long ttl);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
};

#endif //MSFDSEARCHPROVIDER_H