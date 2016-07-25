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

#ifndef __REMOTE_APP_CONTROL_SERVICE_PROVIDER_H__
#define __REMOTE_APP_CONTROL_SERVICE_PROVIDER_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <vconf.h>
#include "../IServiceProvider.h"
#include "RemoteAppControlServiceInfo.h"

namespace conv {
	class RemoteAppControlServiceProvider : public IServiceProvider  {
		public:
			RemoteAppControlServiceProvider();
			~RemoteAppControlServiceProvider();

			int init();
			int release();

			int start_request(request* request_obj);
			int stop_request(request* request_obj);
			int get_request(request* request_obj);
			int set_request(request* request_obj);
			int register_request(request* request_obj);
			int load_service_info(request* request_obj);
			int get_service_info_for_discovery(json* json_obj);
			int handle_vconf_update(keynode_t *node);

		private:
			iotcon_resource_h iotcon_resource;
			int send_response(json payload, const char* request_type, conv_error_e error, request* request_obj);

			static void iotcon_request_cb(iotcon_resource_h resource, iotcon_request_h request, void *user_data);
	};
}
#endif /* __REMOTE_APP_CONTROL_SERVICE_PROVIDER_H__ */
