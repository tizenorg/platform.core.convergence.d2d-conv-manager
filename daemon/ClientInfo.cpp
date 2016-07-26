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

#include <glib.h>
#include "ClientInfo.h"
#include "Log.h"
#include "d2d_conv_manager.h"

using namespace std;

conv::ClientInfo::ClientInfo(string clientId, GDBusMethodInvocation *inv)
{
	id = clientId;
}

conv::ClientInfo::~ClientInfo()
{
	for (ServiceInfoMap::iterator it = service_info_map.begin(); it != service_info_map.end(); ++it) {
		delete (it->second);
	}
	service_info_map.clear();
}

int conv::ClientInfo::add_device()
{
	return CONV_ERROR_NONE;
}

int conv::ClientInfo::remove_device()
{
	return CONV_ERROR_NONE;
}

string conv::ClientInfo::getId()
{
	return id;
}

conv::IServiceInfo* conv::ClientInfo::getServiceInfo(string type, string id)
{
	ServiceInfoMap::iterator it;
	it = service_info_map.find(std::pair<string, string>(type, id));

	if ( it != service_info_map.end() ) {
		_D("service info found : %s, %s", type.c_str(), id.c_str());
		return (IServiceInfo*)(it->second);
	} else {
		_D("service info not found : %s, %s", type.c_str(), id.c_str());
		return NULL;
	}
}

int conv::ClientInfo::addServiceInfo(string type, string id, IServiceInfo* info)
{
	service_info_map.insert(std::pair<ServiceKey, IServiceInfo*>(std::pair<string, string>(type, id), info));
	_D("service info is added : %s, %s", type.c_str(), id.c_str());

	return CONV_ERROR_NONE;
}
