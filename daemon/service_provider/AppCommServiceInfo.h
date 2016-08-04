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

#ifndef __APP_COMMUNICATION_SERVICE_INFO_H__
#define __APP_COMMUNICATION_SERVICE_INFO_H__

#include <glib.h>
#include <vector>
#include "AppCommServiceApplication.h"
#include "../IServiceInfo.h"
#include "../Request.h"
#include "Service.h"
#include "Clients.h"
#include "Application.h"
#include "Result.h"

namespace conv {
	typedef vector<AppCommServiceApplication*> ApplicationInstanceList;
	// service information to handle app-to-app service with specific device 'id'
	class AppCommServiceInfo : public IServiceInfo, public Result_Base {
		public:
			void onSuccess(Service service);
			void onError(Error);

			~AppCommServiceInfo();

			std::string id;
			Service serviceObj;
			bool isLocal;
			bool readRequestResult;
			conv::Request* registeredRequest;

			ApplicationInstanceList applicationInstanceList;
	};

}
#endif /* __APP_COMMUNICATION_SERVICE_INFO_H__ */
