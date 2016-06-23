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

#ifndef __MSF_APPLICATION_INFO_H__
#define __MSF_APPLICATION_INFO_H__

#include <stdio.h>
#include<iostream>
#include <string.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-parser.h>

using namespace std;

class ApplicationInfo
{
	string name;
	string id;
	string msf_version;
	bool running;
	static string PROPERTY_ID;
	static string PROPERTY_STATE;
	static string PROPERTY_NAME;
	static string PROPERTY_VERSION;


public:
	string getID();
	string getName();
	string getVersion();
	bool isRunning();
	ApplicationInfo create(string data);
	ApplicationInfo();
	ApplicationInfo(string iD, bool state, string Name, string Version);
	void json_parse(const char *in);
	static void foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data);
};

#endif
