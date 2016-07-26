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

#ifndef __DEVICE_ADAPTER_H__
#define __DEVICE_ADAPTER_H__

#include "../../IDevice.h"
#include "ResourceHandle.h"
#include <list>
#include <string>

namespace conv {
	class DeviceAdapter : public IDevice {
		public :
			typedef std::list<IService*> service_list_t;
			DeviceAdapter(ResourceHandle res_h);
			~DeviceAdapter();

			int addService(IService* service_obj);
			int removeService(IService* service_obj);
			int getServiceList(std::list<IService*> *list);
			string getName();
			string getId();
			string getAddress();

		private :
			ResourceHandle	m_resource_h;
			service_list_t service_list;
	};
}

#endif
