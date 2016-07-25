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

#ifndef __DISCOVERY_MANAGER_IMPL_H__
#define __DISCOVERY_MANAGER_IMPL_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <map>

#include "IManager.h"
#include "IDiscoveryProvider.h"
#include "request.h"

#include "IDevice.h"
#include "IService.h"

namespace conv {
	class request;
	class device;

	class DiscoveryManager : public IManager {
		public:
			DiscoveryManager();
			~DiscoveryManager();

			int init();
			int release();
			int handleRequest(request* requestObj);

			// discovery_manager aggregates devices with services which have been discovered through the registered discover providers
			int append_discovered_result(IDevice* disc_device);
			int notify_lost_device(IDevice* disc_device);
			int notify_time_up(std::string client);
			int stop_discovery();
			int start_discovery();

			static void timer_worker(void* data);

		private:
			int count_discovery_request;


			typedef std::list<IDiscoveryProvider*> discovery_provider_list_t;
			typedef std::map<string, request*> request_map_t;
			typedef std::map<int, json>	filter_map_t;
			typedef std::map<std::string, int>	timer_map_t;

			int register_provider(IDiscoveryProvider *provider_base);
			discovery_provider_list_t provider_list;
			request_map_t	request_map;
			filter_map_t	discovery_filter_map;
			timer_map_t		request_timer_map;

			// internal function
			int convert_service_into_json(conv::IService* service_info, json* json_data);
			int convert_device_into_json(conv::IDevice* service_info, json* json_data);
			int set_discovery_filter(request* req_obj);
			int isvalid_discovery_on_filter(json& filter_json);
			int checkBoundaryForTimeout(int givenTimeout);
			int merge_exclude_services(conv::IDevice* org_device, conv::IDevice* new_device);
			int exclude_services(conv::IDevice* org_device, conv::IDevice* removed_device);
	};

	namespace discovery_manager {
		void set_instance(DiscoveryManager* mgr);
		int handleRequest(request* requestObj);
		int set_result(device* device_obj);
	};
}

#endif /* __DISCOVERY_MANAGER_IMPL_H__ */
