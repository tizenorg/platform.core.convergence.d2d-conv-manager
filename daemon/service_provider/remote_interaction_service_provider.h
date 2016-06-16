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

#ifndef __REMOTE_INTERACTION_SERVICE_PROVIDER_H__
#define __REMOTE_INTERACTION_SERVICE_PROVIDER_H__

#include "../service_provider_base.h"

namespace conv {

	class remote_interaction_service_provider : public service_provider_base {

		public:
			remote_interaction_service_provider();
			~remote_interaction_service_provider();

			int init();
			int release();

			int start_request(request *request_obj);
			int stop_request(request *request_obj);
			int get_request(request *request_obj);
			int set_request(request *request_obj);
			int register_request(request *request_obj);
			int load_service_info(request *request_obj);
			int get_service_info_for_discovery(json *json_obj);

		private:
			int send_response(json payload, request *request_obj);
	};
}
#endif /* __REMOTE_INTERACTION_SERVICE_PROVIDER_H__ */