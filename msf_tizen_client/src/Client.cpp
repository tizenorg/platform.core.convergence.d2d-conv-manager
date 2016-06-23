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

#include "Channel.h"
#include "Client.h"

string	Client::ID_KEY              = "id";
string  Client::IS_HOST_KEY         = "isHost";
string  Client::CONNECT_TIME_KEY    = "connectTime";
string  Client::ATTRIBUTES_KEY      = "attributes";

Client::Client()
{
	id   = string();
	host = false ;
	connectTime = 0;
}

void Client::create(Client &client)
{
	id   = client.id;
	host = client.host ;
	connectTime = client.connectTime;
}

void Client::create(Channel *channel, string Id, long long ConnectTime,  /*map<string, string> data,*/ bool Host, map<string, string> attr)
{
	id = Id;
	host = Host;
	connectTime = ConnectTime;
	attributes = attr;
	m_channel = channel;
	//return client;
}

bool Client::isHost()
{
	return host;
}

const char *Client::getId()
{
	return id.c_str();
}

long long Client::getConnectTime()
{
	return connectTime;
}
map<string, string> Client::getAttributesKey()
{
	return attributes;
}

Channel *Client::getChannel()
{
	return m_channel;
}

