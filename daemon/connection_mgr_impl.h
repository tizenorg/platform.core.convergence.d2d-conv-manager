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

#ifndef __CONNECTION_MANAGER_IMPL_H__
#define __CONNECTION_MANAGER_IMPL_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include "manager_iface.h"
#include "request.h"

namespace conv {
	class connection_manager_impl : public manager_iface  {
		public:
			connection_manager_impl();
			~connection_manager_impl();

			int init();
			int release();
			int handle_request(request* request_obj);
	};

	namespace connection_manager {
		void set_instance(connection_manager_impl* mgr);
		int handle_request(request* request_obj);
	}
}

#endif /* __CONNECTION_MANAGER_IMPL_H__ */
