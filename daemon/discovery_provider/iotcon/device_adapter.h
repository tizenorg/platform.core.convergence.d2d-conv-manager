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

#include "../../device_iface.h"
#include "resource_handle.h"
#include <list>
#include <string>

namespace conv {
	class device_adapter : public device_iface {
		public :
			typedef std::list<service_iface*> service_list_t;
			device_adapter(resource_handle res_h);
			~device_adapter();

			int add_service(service_iface* service_obj);
			int remove_service(service_iface* service_obj);
			int get_services_list(std::list<service_iface*> *list);
			string getName();
			string getId();
			string getAddress();

		private :
			resource_handle	m_resource_h;
			service_list_t service_list;
	};
}

#endif
