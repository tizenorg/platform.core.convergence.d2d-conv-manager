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

#ifndef __CONV_MANAGER_INTERFACE_H__
#define __CONV_MANAGER_INTERFACE_H__

#include <string>
#include "conv_json.h"

namespace conv {
	class request;

	class manager_iface {
		public:
			virtual ~manager_iface() {}
			virtual int init() = 0;
			virtual int release() = 0;
			virtual int handle_request(request* request_obj) = 0;
//			virtual int publish(const char* subject, json data_updated, json option) = 0;
	};	/* class manager_iface */
}

#endif	/* End of __CONV_MANAGER_INTERFACE_H__ */
