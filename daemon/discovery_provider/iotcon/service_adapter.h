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

#ifndef __SERVICE_ADAPTER_H__
#define __SERVICE_ADAPTER_H__

#include "../../service_iface.h"
#include "resource_handle.h"

namespace conv {

	class service_adapter : public service_iface {
		public:
			service_adapter(resource_handle res_h);
			~service_adapter();

			string getName();
			string getVersion();
			string getType();
			string getId();
			string getUri();
			int getServiceType();
			string getServiceInfo();

			int		setServiceType(int serviceType);
			int		setServiceInfo(string serviceInfo);

		private:
			resource_handle	m_resource_h;

			int service_type;
			string service_info;
	};

}

#endif
