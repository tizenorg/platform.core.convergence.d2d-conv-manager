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

#include "ClientManager.h"

static conv::ClientManager *_instance;

using namespace std;

conv::ClientManager::ClientManager()
{
}

conv::ClientManager::~ClientManager()
{
}

int conv::ClientManager::init()
{
	return CONV_ERROR_NONE;
}

int conv::ClientManager::release()
{
	for (ClientList::iterator it = clientList.begin(); it != clientList.end(); ++it) {
		delete *it;
	}

	clientList.clear();

	return CONV_ERROR_NONE;
}

void conv::client_manager::setInstance(conv::ClientManager* mgr)
{
	_instance = mgr;
}


int conv::ClientManager::handleRequest(Request* requestObj)
{
	_D("handle_request called");

	return CONV_ERROR_NONE;
}

conv::ClientInfo* conv::client_manager::getClient(std::string clientId)
{
	IF_FAIL_RETURN_TAG(_instance, NULL, _E, "Not initialized");

	return _instance->getClient(clientId);
}

conv::ClientInfo* conv::ClientManager::getClient(std::string clientId)
{
	for (ClientList::iterator it = clientList.begin(); it != clientList.end(); ++it) {
		if ( !(*it)->getId().compare(clientId) )
		{
			// already exists
			_D("client exists");
			return *it;
		}
	}

	_D("new client obj created");
	ClientInfo* clientObj = new(std::nothrow) conv::ClientInfo(clientId, NULL);

	clientList.push_back(clientObj);
	return clientObj;
}
