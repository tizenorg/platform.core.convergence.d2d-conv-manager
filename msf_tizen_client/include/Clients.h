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

#ifndef __MSF_CLIENTS_H__
#define __MSF_CLIENTS_H__

#include"Client.h"
#include"Channel.h"
#include<vector>

using namespace std;

class Channel;
class Client;

class Clients
{
	map<string, Client> clients;
	string myClientId;
	Client *host;
	Client varme;
	Channel *m_channel;

public:
	void setMyClientId(string clientId);
	void reset();
	void add(list <Client> clientList);
	void add(Client client);
	void remove(Client client);
	int size();
	Clients();
	Clients(Channel *channel);
	Client get(string clientId);
	Client *getHost() ;//need better way to find host
	Client me(void);
	bool isMe(Client client);
	list<Client> lists();
	Channel *getChannel();
};

#endif
