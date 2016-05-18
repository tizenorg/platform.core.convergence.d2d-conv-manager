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

#ifndef	_RESOURCE_HANDLE_H__
#define _RESOURCE_HANDLE_H__

#include <list>
#include <string>
#include "../../conv_json.h"

using namespace std;

namespace conv{
class resource_handle {
	private:
		string	uri_path;
		string	host_address;
		list<string>	types;
		list<string>	interfaces;

		string	device_id;
		string	device_name;
		string	device_type;
		string	version;
		json	services_list;

	public:
		int		set_device_id(string dev_id);
		string	get_device_id();
		int		set_device_name(string dev_name);
		string	get_device_name();
		int		set_device_type(string dev_type);
		string	get_device_type();
		int		set_version(string ver);
		string	get_version();
		int		set_services_list(string list);

		int		set_uri_path(string uri_path);
		string	get_uri_path();
		int		set_host_address(string host_address);
		string	get_host_address();
		list<string>&	get_types();
		int		add_types(string type);
		list<string>&	get_interfaces();
		int		add_interfaces(string interface);
};
}
#endif
