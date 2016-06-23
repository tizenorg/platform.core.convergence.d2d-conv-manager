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

#include <dlog.h>
#include "Message.h"

string Message::TARGET_BROADCAST             = "broadcast";
string Message::TARGET_ALL                   = "all";
string Message::TARGET_HOST                  = "host";

string Message::METHOD_APPLICATION_GET        = "ms.application.get";
string Message::METHOD_APPLICATION_START      = "ms.application.start";
string Message::METHOD_APPLICATION_STOP       = "ms.application.stop";
string Message::METHOD_APPLICATION_INSTALL    = "ms.application.install";
string Message::METHOD_WEB_APPLICATION_GET    = "ms.webapplication.get";
string Message::METHOD_WEB_APPLICATION_START  = "ms.webapplication.start";
string Message::METHOD_WEB_APPLICATION_STOP   = "ms.webapplication.stop";
string Message::METHOD_EMIT                  = "ms.channel.emit";

string Message::PROPERTY_MESSAGE             = "message";
string Message::PROPERTY_METHOD              = "method";
string Message::PROPERTY_MESSAGE_ID          = "id";
string Message::PROPERTY_PARAMS              = "params";
string Message::PROPERTY_ID                  = "id";
string Message::PROPERTY_URL                 = "url";
string Message::PROPERTY_ARGS                = "args";
string Message::PROPERTY_EVENT               = "event";
string Message::PROPERTY_DATA                = "data";
string Message::PROPERTY_TO                  = "to";
string Message::PROPERTY_FROM                = "from";
string Message::PROPERTY_CLIENTS             = "clients";
string Message::PROPERTY_RESULT              = "result";
string Message::PROPERTY_ERROR               = "error";

Message::Message(Channel *ch, string event, string data, string from, unsigned char *payload, int payload_size)
{
	m_channel = ch;
	m_event = event;
	m_data = data;
	m_from = from;
	m_payload_size = payload_size;

	m_payload = new unsigned char[payload_size+1];

	for (int i = 0; i < payload_size; i++) {
		m_payload[i] = payload[i];
	}

	m_payload[payload_size] = 0;

	dlog_print(DLOG_INFO, "MSF", "Message Test 1");
}

Message::Message()
{
}

Message::~Message()
{
	delete[] m_payload;
}

Message::Message(const Message& m)
{
	m_channel = m.m_channel;
	m_event = m.m_event;
	m_data = m.m_data;
	m_from = m.m_from;
	m_payload_size = m.m_payload_size;

	m_payload = new unsigned char[m_payload_size+1];

	for (int i = 0; i < m_payload_size; i++) {
		m_payload[i] = m.m_payload[i];
	}
}
