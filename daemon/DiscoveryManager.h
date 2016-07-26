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
#include "Request.h"

#include "IDevice.h"
#include "IService.h"

namespace conv {
	class SmartViewDevice;

	class DiscoveryManager : public IManager {
		public:
			DiscoveryManager();
			~DiscoveryManager();

			int init();
			int release();
			int handleRequest(Request* requestObj);

			// discovery_manager aggregates devices with services which have been discovered through the registered discover providers
			int appendDiscoveredResult(IDevice* disc_device);
			int notifyLostDevice(IDevice* disc_device);
			int notifyTimeOut(std::string client);
			int stopDiscovery();
			int startDiscovery();

			static void __timer_worker(void* data);

		private:
			int count_discovery_request;


			typedef std::list<IDiscoveryProvider*> discovery_provider_list_t;
			typedef std::map<string, Request*> request_map_t;
			typedef std::map<int, Json>	filter_map_t;
			typedef std::map<std::string, int>	timer_map_t;

			int registerProvider(IDiscoveryProvider *discoveryProvider);
			discovery_provider_list_t __providerList;
			request_map_t	request_map;
			filter_map_t	discovery_filter_map;
			timer_map_t		request_timer_map;

			// internal function
			int convertServiceIntoJson(conv::IService* service_info, Json* json_data);
			int convertDeviceIntoJson(conv::IDevice* service_info, Json* json_data);
			int checkBoundaryForTimeOut(int givenTimeout);
			int mergeExcludeServices(conv::IDevice* org_device, conv::IDevice* new_device);
			int excludeServices(conv::IDevice* org_device, conv::IDevice* removed_device);
	};

	namespace discovery_manager {
		void setInstance(DiscoveryManager* mgr);
		int handleRequest(Request* requestObj);
		int setResult(SmartViewDevice* device_obj);
	};
}

#endif /* __DISCOVERY_MANAGER_IMPL_H__ */
