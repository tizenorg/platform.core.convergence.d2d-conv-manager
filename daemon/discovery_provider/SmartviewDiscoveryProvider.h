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

#ifndef __SMARTVIEW_DISCOVERY_PROVIDER_H__
#define __SMARTVIEW_DISCOVERY_PROVIDER_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <map>

#include "../IDiscoveryProvider.h"
#include "../Request.h"
#include "smartview/SmartViewService.h"
#include "smartview/SmartViewDevice.h"

#include "Search.h"
#include "Service.h"

namespace conv {

	class SmartviewDiscoveryProvider : public IDiscoveryProvider  {
		public:
			SmartviewDiscoveryProvider();
			~SmartviewDiscoveryProvider();

			int init();
			int release();

			int start();
			int stop();

			int notifyDiscovered(Service *service, bool bDiscovered);
		private:
			//Service service;
			Search*	search;

			map <string, conv::SmartViewService*> cache; // for double check

			conv::SmartViewService* convertIntoConvService(Service* smartview_service);
			conv::SmartViewDevice* convertIntoConvDevice(Service* smartview_service);
			int checkExistence(conv::SmartViewService* conv_service);
			int removeFromCache(conv::SmartViewService* conv_service);
	};
}

#endif /* __SMARTVIEW_DISCOVERY_PROVIDER_H__ */
