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

#ifndef __CONV_SMART_VIEW_DEVICE_H__
#define __CONV_SMART_VIEW_DEVICE_H__

#include <string>
#include <vector>
#include "../../IService.h"
#include "../../Json.h"

#include "../../IDevice.h"

namespace conv {
	class SmartViewDevice : public IDevice {
		public:
			typedef std::list<IService*> service_list_t;
			SmartViewDevice();
			~SmartViewDevice();

			// functions from IDevice
			int addService(IService* serviceObj);
			int removeService(IService* serviceObj);
			int getServiceList(std::list<IService*> *list);
			std::string getName();
			std::string getId();
			std::string getAddress();
			int setName(std::string name);
			int setId(std::string id);
			int setAddress(std::string host_address);

		private:
			service_list_t service_list;

		protected:
			std::string id;
			std::string ip_address;
			std::string name;
			std::string type;
			std::string wd_mac_address;
			std::string bt_mac_address;
	};
}

#endif	/* End of __CONV_SMART_VIEW_DEVICE_H__ */
