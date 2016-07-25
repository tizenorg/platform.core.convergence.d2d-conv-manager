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

#ifndef __APP_COMMUNICATION_SERVICE_PROVIDER_H__
#define __APP_COMMUNICATION_SERVICE_PROVIDER_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <vconf.h>
#include "../service_provider_base.h"
#include "app_comm_service_info.h"

namespace conv {
	class app_comm_service_provider : public service_provider_base  {
		public:
			app_comm_service_provider();
			~app_comm_service_provider();

			int init();
			int release();

			app_comm_service_info* get_svc_info(request* request_obj);
			int start_request(request* request_obj);
			int stop_request(request* request_obj);
			int get_request(request* request_obj);
			int set_request(request* request_obj);
			int register_request(request* request_obj);
			int load_service_info(request* request_obj);
			int get_service_info_for_discovery(json* json_obj);
			int handle_vconf_update(keynode_t *node);

		private:
			int send_read_response(json payload, const char* read_type, conv_error_e error, request* request_obj);
	};
}
#endif /* __APP_COMMUNICATION_SERVICE_PROVIDER_H__ */
