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

#ifndef __REMOTE_APP_CONTROL_SERVICE_INFO_H__
#define __REMOTE_APP_CONTROL_SERVICE_INFO_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include "../service_info_base.h"
#include "../iotcon_communication_info.h"
#include "../request.h"
#include <iotcon.h>

namespace conv {
	class remote_app_control_service_info : public service_info_base {
		public:
			~remote_app_control_service_info()
			{
				if ( registered_request != NULL )
				{
					delete registered_request;
				}

				if ( iotcon_info_obj.iotcon_resource_handle != NULL )
				{
					iotcon_remote_resource_destroy(iotcon_info_obj.iotcon_resource_handle);
				}
				if ( iotcon_info_obj.iotcon_representation_handle != NULL )
				{
					iotcon_representation_destroy(iotcon_info_obj.iotcon_representation_handle);
				}
			}
			std::string device_id;
			std::string device_name;
			std::string device_address;
			request* registered_request;
			iotcon_communication_info iotcon_info_obj;
	};

}
#endif /* __REMOTE_APP_CONTROL_SERVICE_INFO_H__ */