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

#ifndef __DISCOVERY_PROVIDER_BASE_H__
#define __DISCOVERY_PROVIDER_BASE_H__

#include <string>
#include "conv_json.h"

namespace conv {

	class DiscoveryManager;

	class IDiscoveryProvider {
		public:
			virtual ~IDiscoveryProvider() {}
			virtual int init() = 0;
			virtual int release() = 0;
			virtual int start() = 0;
			virtual int stop() = 0;

			int set_manager(DiscoveryManager* discovery_manager);

		protected:
			static DiscoveryManager* _discovery_manager;
	};	/* class IDiscoveryProvider */
}

#endif	/* End of __DISCOVERY_PROVIDER_BASE_H__ */
