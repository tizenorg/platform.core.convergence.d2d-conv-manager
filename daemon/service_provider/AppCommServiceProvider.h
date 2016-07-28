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

#ifndef __APP_COMMUNICATION_SERVICE_PROVIDER_H__
#define __APP_COMMUNICATION_SERVICE_PROVIDER_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <vconf.h>
#include "../IServiceProvider.h"
#include "AppCommServiceInfo.h"

namespace conv {
	class AppCommServiceProvider : public IServiceProvider  {
		public:
			AppCommServiceProvider();
			~AppCommServiceProvider();

			int init();
			int release();

			AppCommServiceInfo* get_svc_info(Request* requestObj);
			int startRequest(Request* requestObj);
			int stopRequest(Request* requestObj);
			int readRequest(Request* requestObj);
			int publishRequest(Request* requestObj);
			int registerRequest(Request* requestObj);
			int loadServiceInfo(Request* requestObj);
			int getServiceInfoForDiscovery(Json* jsonObj);
			int handleVconfUpdate(keynode_t *node);

		private:
			int sendReadResponse(Json payload, const char* read_type, conv_error_e error, Request* requestObj);
	};
}
#endif /* __APP_COMMUNICATION_SERVICE_PROVIDER_H__ */