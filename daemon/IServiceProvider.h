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
#include "Request.h"
#include "Json.h"

namespace conv {
	class IServiceProvider {
		public:
			virtual ~IServiceProvider() {}
			virtual int init() = 0;
			virtual int release() = 0;

			virtual int startRequest(Request* request_obj) = 0;
			virtual int stopRequest(Request* request_obj) = 0;
			virtual int readRequest(Request* request_obj) = 0;
			virtual int publishRequest(Request* request_obj) = 0;
			virtual int registerRequest(Request* request_obj) = 0;
			virtual int loadServiceInfo(Request* request_obj) = 0;
			virtual int getServiceInfoForDiscovery(Json* json_obj) = 0;
			int checkActivationState() {
				if (__activationState == 1) {
					return CONV_ERROR_NONE;
				} else {
					return CONV_ERROR_INVALID_OPERATION;
				}
			}

			std::string getType() {
				return __type;
			}

			std::string get_resource_type() {
					return __resourceType;
			}

			std::string get_uri() {
					return __uri;
			}

		protected:
			std::string __type;
			std::string __resourceType;
			std::string __uri;
			int __activationState;
	};	/* class IServiceProvider */
}

#endif	/* End of __SERVICE_PROVIDER_BASE_H__ */
