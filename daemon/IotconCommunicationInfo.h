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

#ifndef __IOTCON_COMMUNICATION_INFO_H__
#define __IOTCON_COMMUNICATION_INFO_H__

#include <iotcon.h>
#include <glib.h>
#include <iotcon.h>
#include <string>
#include "ICommunicationInfo.h"

namespace conv {
	class IotconCommunicationInfo : public ICommunicationInfo {
		public:
			std::string address;
			std::string uri;
			std::string resource_type;

			iotcon_remote_resource_h iotcon_resource_handle;
			iotcon_representation_h iotcon_representation_handle;
	};

}
#endif /* __IOTCON_COMMUNICATION_INFO_H__ */
