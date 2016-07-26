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

#ifndef	__RESOURCE_HANDLE_H__
#define __RESOURCE_HANDLE_H__

#include <list>
#include <string>
#include "../../Json.h"

using namespace std;

namespace conv{
class ResourceHandle {
	private:
		string	uri_path;
		string	host_address;
		list<string>	types;
		list<string>	interfaces;

		string	device_id;
		string	device_name;
		string	device_type;
		string	version;
		Json	services_list;

	public:
		int		setDeviceId(string dev_id);
		string	getDeviceId();
		int		setDeviceName(string dev_name);
		string	getDeviceName();
		int		setDeviceType(string dev_type);
		string	getDeviceType();
		int		setVersion(string ver);
		string	getVersion();
		int		setServiceList(string list);

		int		setUriPath(string uri_path);
		string	getUriPath();
		int		setHostAddress(string host_address);
		string	getHostAddress();
		list<string>&	getTypes();
		int		addType(string type);
		list<string>&	getInterfaces();
		int		addInterface(string interface);
};
}
#endif
