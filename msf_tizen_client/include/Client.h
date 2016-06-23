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

#ifndef __MSF_CLIENT_H__
#define __MSF_CLIENT_H__

#include <stdio.h>
#include<iostream>
#include <string.h>
#include<map>
//#include"Channel.h"

using namespace std;

class Channel;

class Client
{
	static string ID_KEY;
	static string IS_HOST_KEY;
	static string CONNECT_TIME_KEY;
	static string ATTRIBUTES_KEY;

	string id;
	bool host;
	long long connectTime;
	map<string, string> attributes;
	Channel *m_channel;

public:
	Client();
	void create(Client &client);
	void create(Channel *channel, string, long long, /*map<string, string> data,*/bool, map<string, string>);
	bool isHost();
	const char *getId();
	long long getConnectTime();
	map<string, string> getAttributesKey();
	Channel *getChannel();
};

#endif
