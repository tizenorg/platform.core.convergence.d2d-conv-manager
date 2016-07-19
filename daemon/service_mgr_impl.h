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

#ifndef __SERVICE_MANAGER_IMPL_H__
#define __SERVICE_MANAGER_IMPL_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <vconf.h>
#include "manager_iface.h"
#include "request.h"
#include "service_provider_base.h"

namespace conv {
	class service_manager_impl : public manager_iface  {
		public:
			service_manager_impl();
			~service_manager_impl();

			int init();
			int release();
			int handle_request(request* request_obj);

			int get_service_info_for_discovery(json* service_json);
			int handle_vconf_update(keynode_t *node);

		private:
			int activation_state;

			typedef std::list<service_provider_base*> service_provider_list_t;

			int register_provider(service_provider_base *provider_base);
			int register_discovery_info();
			int unregister_discovery_info();
			service_provider_list_t provider_list;
	};

	namespace service_manager {
		void set_instance(service_manager_impl* mgr);
		int handle_request(request* request_obj);
	}
}

#endif /* __SERVICE_MANAGER_IMPL_H__ */
