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

#ifndef __CONV_CLIENT_H__
#define __CONV_CLIENT_H__

#include <string>
#include <vector>
#include <map>
#include <gio/gio.h>
#include "client.h"
#include "conv_json.h"
#include "device_iface.h"
#include "service_info_base.h"

namespace conv {
	class client {
		typedef std::vector<device_iface*> device_list_t;
		typedef std::pair<std::string, std::string> service_key_t;
		typedef std::map<service_key_t, service_info_base*> service_info_map_t;

		public:
			client(std::string id, GDBusMethodInvocation *inv);
			~client();

			int add_device();
			int remove_device();

			std::string get_id();

			service_info_base* get_service_info(std::string type, std::string id);
			int add_service_info(std::string type, std::string id, service_info_base* info);
		private:
			device_list_t device_list;
			service_info_map_t service_info_map;

		protected:
			std::string id;
			GDBusMethodInvocation *invocation;
	};
}

#endif	/* End of __CONV_CLIENT_H__ */