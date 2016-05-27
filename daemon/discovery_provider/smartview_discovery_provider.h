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

#ifndef __SMARTVIEW_DISCOVERY_PROVIDER_H__
#define __SMARTVIEW_DISCOVERY_PROVIDER_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include <map>

#include "../discovery_provider_base.h"
#include "../request.h"
#include "smartview/service.h"
#include "smartview/device.h"

#include <msf-api/Search.h>
#include <msf-api/Service.h>

namespace conv {

	class smartview_discovery_provider : public discovery_provider_base  {
		public:
			smartview_discovery_provider();
			~smartview_discovery_provider();

			int init();
			int release();

			int start();
			int stop();

			int notice_discovered(Service *service);
		private:
			Service service;
			Search*	search;

			map <string, conv::service*> cache; // for double check

			conv::service* convert_into_conv_service(Service* smartview_service);
			conv::device* convert_into_conv_device(Service* smartview_service);
			int checkExistence(conv::service* conv_service);
	};
}

#endif /* __SMARTVIEW_DISCOVERY_PROVIDER_H__ */
