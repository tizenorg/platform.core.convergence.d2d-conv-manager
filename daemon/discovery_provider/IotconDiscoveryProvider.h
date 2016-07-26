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

#ifndef __IOTCON_DISCOVERY_PROVIDER_H__
#define __IOTCON_DISCOVERY_PROVIDER_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <map>
#include "../IDiscoveryProvider.h"
#include "../Request.h"

#include "../IService.h"
#include "iotcon/ResourceHandle.h"

namespace conv {
	class IotconDiscoveryProvider : public IDiscoveryProvider  {
		private:

			static bool __found_resource(iotcon_remote_resource_h resource, iotcon_error_e result,
									void *user_data);
			static void __get_detail_info(iotcon_remote_resource_h resource);

			static void __on_received_detail_info(iotcon_remote_resource_h resource, iotcon_error_e error,
									iotcon_request_type_e request_type, iotcon_response_h response,
									void* user_data);

			static void __on_response_get(iotcon_remote_resource_h resource,
									iotcon_response_h response, void* user_data);

			static int __add_iot_resource(iotcon_remote_resource_h resource);
			static string __generate_discovery_key(char* uri_path, char* host_address);
			static bool __get_str_list_cb(int pos, const char* value, void* user_data);

		public:
			IotconDiscoveryProvider();
			~IotconDiscoveryProvider();

			int init();
			int release();

			int start();
			int stop();

			static int notifyDiscovered(IService* service);
	};
}
#endif /* __SMARTVIEW_DISCOVERY_PROVIDER_H__ */
