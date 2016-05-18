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

#ifndef __CONV_DBUS_SERVER_INTERFACE_H__
#define __CONV_DBUS_SERVER_INTERFACE_H__

#include <sys/types.h>

namespace conv {
	class dbus_server_iface {
		public:
			virtual ~dbus_server_iface() {}
//			virtual int64_t signal_subscribe(const char* sender, const char* path, const char* iface, const char* name, dbus_listener_iface* listener) = 0;
//			virtual void signal_unsubscribe(int64_t subscription_id) = 0;
	};	/* class conv::dbus_server */

}	/* namespace ctx */

#endif	/* End of __CONV_DBUS_SERVER_INTERFACE_H__ */
