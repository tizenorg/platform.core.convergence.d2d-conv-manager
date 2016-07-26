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
#ifndef _DEVICE_INTERFACE_H__
#define	_DEVICE_INTERFACE_H__

#include "IService.h"
#include <string>
#include <list>

using namespace std;

namespace conv {
	class IDevice {
		public:
			virtual ~IDevice() {}

			virtual int addService(IService* service_obj) = 0;
			virtual int removeService(IService* service_obj) = 0;
			virtual int getServiceList(std::list<IService*> *list) = 0;

			virtual string getName() = 0;
			virtual string getId() = 0;
			virtual string getAddress() = 0;
	};
}

#endif
