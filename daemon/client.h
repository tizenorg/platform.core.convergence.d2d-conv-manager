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
#include "IDevice.h"
#include "IServiceInfo.h"

namespace conv {
	class client {
		typedef std::vector<IDevice*> device_list_t;
		typedef std::pair<std::string, std::string> service_key_t;
		typedef std::map<service_key_t, IServiceInfo*> service_info_map_t;

		public:
			client(std::string id, GDBusMethodInvocation *inv);
			~client();

			int add_device();
			int remove_device();

			std::string get_id();

			IServiceInfo* get_service_info(std::string type, std::string id);
			int add_service_info(std::string type, std::string id, IServiceInfo* info);
		private:
			device_list_t device_list;
			service_info_map_t service_info_map;

		protected:
			std::string id;
			GDBusMethodInvocation *invocation;
	};
}

#endif	/* End of __CONV_CLIENT_H__ */
