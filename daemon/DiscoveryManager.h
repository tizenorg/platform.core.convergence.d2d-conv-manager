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
			int appendDiscoveredResult(IDevice* discoveredDevice);
			int notifyLostDevice(IDevice* discoveredDevice);
			int notifyTimeOut(std::string client);
			int stopDiscovery();
			int startDiscovery();

			static void __timer_worker(void* data);

		private:
			int __countDiscoveryRequest;


			typedef std::list<IDiscoveryProvider*> DiscoveryProviderList;
			typedef std::map<string, Request*> RequestMap;
			typedef std::map<std::string, int>	TimerMap;

			int registerProvider(IDiscoveryProvider *discoveryProvider);
			DiscoveryProviderList __providerList;
			RequestMap __requestMap;
			TimerMap __requestTimerMap;

			// internal function
			int convertServiceIntoJson(conv::IService* serviceInfo, Json* jsonData);
			int convertDeviceIntoJson(conv::IDevice* serviceInfo, Json* jsonData);
			int checkBoundaryForTimeOut(int givenTimeout);
			int mergeExcludeServices(conv::IDevice* org_device, conv::IDevice* newDevice);
			int excludeServices(conv::IDevice* org_device, conv::IDevice* removedDevice);
	};

	namespace discovery_manager {
		void setInstance(DiscoveryManager* mgr);
		int handleRequest(Request* requestObj);
		int setResult(SmartViewDevice* deviceObj);
	};
}

#endif /* __DISCOVERY_MANAGER_IMPL_H__ */
