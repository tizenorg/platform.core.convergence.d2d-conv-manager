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

#ifndef __SERVICE_PROVIDER_BASE_H__
#define __SERVICE_PROVIDER_BASE_H__

#include <string>
#include "request.h"
#include "conv_json.h"

namespace conv {
	class service_provider_base {
		public:
			virtual ~service_provider_base() {}
			virtual int init() = 0;
			virtual int release() = 0;

			virtual int start_request(request* request_obj) = 0;
			virtual int stop_request(request* request_obj) = 0;
			virtual int get_request(request* request_obj) = 0;
			virtual int set_request(request* request_obj) = 0;
			virtual int register_request(request* request_obj) = 0;
			virtual int load_service_info(request* request_obj) = 0;
			virtual int get_service_info_for_discovery(json* json_obj) = 0;

			std::string get_type() {
				return _type;
			}

			std::string get_resource_type() {
					return _resource_type;
			}

			std::string get_uri() {
					return _uri;
			}

		protected:
			std::string _type;
			std::string _resource_type;
			std::string _uri;
	};	/* class service_provider_base */
}

#endif	/* End of __SERVICE_PROVIDER_BASE_H__ */
