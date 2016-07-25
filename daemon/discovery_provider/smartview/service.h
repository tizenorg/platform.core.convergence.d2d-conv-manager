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

#ifndef __CONV_SERVICE_H__
#define __CONV_SERVICE_H__

#include <string>
#include "../../conv_json.h"
#include "../../IService.h"

using namespace std;

namespace conv
{
	class service : public IService {
		public:
			service();
			~service();

			// functions from IService
			string getName();
			string getVersion();
			string getType();
			string getId();
			string getUri();
			int getServiceType();
			string getServiceInfo();

			void setName(string name);
			void setVersion(string version);
			void setType(string type);
			void setId(string id);
			void setUri(string uri);
			void setServiceInfo(string service_info);
			void setServiceType(int service_type);

			void printInfo();

		protected:
			int connection_state;
			string service_name;
			int service_type;
			string service_info;

			string service_version;
			string service_id;
			string service_uri;
			string type;
	};
}

#endif	/* End of __CONV_SERVICE_H__ */
