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
#include "../IServiceInfo.h"
#include "../IotconCommunicationInfo.h"
#include "../Request.h"
#include <iotcon.h>

namespace conv {
	class RemoteAppControlServiceInfo : public IServiceInfo {
		public:
			~RemoteAppControlServiceInfo()
			{
				if ( registeredRequest != NULL )
				{
					delete registeredRequest;
				}

				if ( iotconInfoObj.iotconResourceHandle != NULL )
				{
					iotcon_remote_resource_destroy(iotconInfoObj.iotconResourceHandle);
				}
				if ( iotconInfoObj.iotconRepresentationHandle != NULL )
				{
					iotcon_representation_destroy(iotconInfoObj.iotconRepresentationHandle);
				}
			}
			std::string deviceId;
			std::string deviceName;
			std::string deviceAddress;
			Request* registeredRequest;
			IotconCommunicationInfo iotconInfoObj;
	};

}
#endif /* __REMOTE_APP_CONTROL_SERVICE_INFO_H__ */
