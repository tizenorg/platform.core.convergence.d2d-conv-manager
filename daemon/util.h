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

#ifndef __D2D_SERVICE_UTIL_H__
#define __D2D_SERVICE_UTIL_H__
#include <string>

using namespace std;

#define MAC_ADDR_STR_LEN 18

namespace conv {
	namespace util {
		std::string get_bt_mac_address();
		std::string get_device_name();
		std::string get_p2p_mac_address();
		std::string get_device_id();
		bool is_service_activated(int service_value);

		typedef void (*timer_function)(void *data);
		void* misc_start_timer(timer_function function, unsigned int interval, void *data);
		void misc_stop_timer(void *timer);
	}
}
#endif

