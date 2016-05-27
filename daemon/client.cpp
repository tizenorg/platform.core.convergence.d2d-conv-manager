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
#include "client.h"
#include "log.h"
#include "d2d_conv_manager.h"

using namespace std;

conv::client::client(string client_id, GDBusMethodInvocation *inv)
{
	id = client_id;
}

conv::client::~client()
{
	for (service_info_map_t::iterator it = service_info_map.begin(); it != service_info_map.end(); ++it) {
		delete (it->second);
	}
	service_info_map.clear();
}

int conv::client::add_device()
{
	return CONV_ERROR_NONE;
}

int conv::client::remove_device()
{
	return CONV_ERROR_NONE;
}

string conv::client::get_id()
{
	return id;
}

conv::service_info_base* conv::client::get_service_info(string type, string id)
{
	service_info_map_t::iterator it;
	it = service_info_map.find(std::pair<string, string>(type, id));

	if ( it != service_info_map.end() ) {
		_D("service info found : %s, %s", type.c_str(), id.c_str());
		return (service_info_base*)(it->second);
	} else {
		_D("service info not found : %s, %s", type.c_str(), id.c_str());
		return NULL;
	}
}

int conv::client::add_service_info(string type, string id, service_info_base* info)
{
	service_info_map.insert(std::pair<service_key_t, service_info_base*>(std::pair<string, string>(type, id), info));
	_D("service info is added : %s, %s", type.c_str(), id.c_str());

	return CONV_ERROR_NONE;
}
