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

#ifndef __MSF_MESSAGE_H__
#define __MSF_MESSAGE_H__

#include "Client.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

class Message
{
public:
	Channel *m_channel;
	string m_event;
	string m_data;
	string m_from;

	unsigned char *m_payload;
	int m_payload_size;
	//std::vector<unsigned char> m_payload;

public:
	static string TARGET_BROADCAST;
	static string TARGET_ALL;
	static string TARGET_HOST;

	static string METHOD_APPLICATION_GET;
	static string METHOD_APPLICATION_START;
	static string METHOD_APPLICATION_STOP;
	static string METHOD_APPLICATION_INSTALL;
	static string METHOD_WEB_APPLICATION_GET;
	static string METHOD_WEB_APPLICATION_START;
	static string METHOD_WEB_APPLICATION_STOP;
	static string METHOD_EMIT;

	static string PROPERTY_MESSAGE;
	static string PROPERTY_METHOD;
	static string PROPERTY_MESSAGE_ID;
	static string PROPERTY_PARAMS;
	static string PROPERTY_ID;
	static string PROPERTY_URL;
	static string PROPERTY_ARGS;
	static string PROPERTY_EVENT;
	static string PROPERTY_DATA;
	static string PROPERTY_TO;
	static string PROPERTY_FROM;
	static string PROPERTY_CLIENTS;
	static string PROPERTY_RESULT;
	static string PROPERTY_ERROR;

	Message(Channel *ch, string event, string data, string from, unsigned char *payload, int payload_size);
	Message();
	virtual ~Message();
	Message(const Message&);
};

#endif
