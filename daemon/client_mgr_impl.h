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

#ifndef __CLIENT_MANAGER_IMPL_H__
#define __CLIENT_MANAGER_IMPL_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <string>
#include "manager_iface.h"
#include "client.h"

namespace conv {
	class client_manager_impl : public manager_iface  {
		typedef std::vector<client*> client_list_t;
		public:
			client_manager_impl();
			~client_manager_impl();

	//		int start(int time, request_info* request, json option);
	//		int stop();

			int init();
			int release();
			int handle_request(request* request_obj);
			conv::client* get_client(std::string client_id);

		private:
			client_list_t client_list;
	};

	namespace client_manager {
		void set_instance(client_manager_impl* mgr);
		conv::client* get_client(std::string client_id);
	}
}

#endif /* __CLIENT_MANAGER_IMPL_H__ */
