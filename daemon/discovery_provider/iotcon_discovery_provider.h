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
#include "../request.h"

#include "../IService.h"
#include "iotcon/resource_handle.h"

using namespace std;

namespace conv {
	class iotcon_discovery_provider : public IDiscoveryProvider  {
		private:

			static bool _found_resource(iotcon_remote_resource_h resource, iotcon_error_e result,
									void *user_data);
			static void _get_detail_info(iotcon_remote_resource_h resource);

			static void on_received_detail_info(iotcon_remote_resource_h resource, iotcon_error_e error,
									iotcon_request_type_e request_type, iotcon_response_h response,
									void* user_data);

			static void _on_response_get(iotcon_remote_resource_h resource,
									iotcon_response_h response, void* user_data);

			static int add_iot_resource(iotcon_remote_resource_h resource);
			static string generateDiscoveryKey(char* uri_path, char* host_address);
			static bool _get_str_list_cb(int pos, const char* value, void* user_data);

		public:
			iotcon_discovery_provider();
			~iotcon_discovery_provider();

			int init();
			int release();

			int start();
			int stop();

			static int notice_discovered(IService* service);
	};
}
#endif /* __SMARTVIEW_DISCOVERY_PROVIDER_H__ */
