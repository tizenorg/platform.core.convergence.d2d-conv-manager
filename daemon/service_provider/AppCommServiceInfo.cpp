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

#include "AppCommServiceInfo.h"

void conv::AppCommServiceInfo::onSuccess(Service service)
{
	_D("getByUri : service name : %s", service.getName().c_str() ? service.getName().c_str() : "name is NULL");
	serviceObj = service;
	readRequestResult = true;
}

void conv::AppCommServiceInfo::onError(Error)
{
	_D("getByUri Error");
	readRequestResult = false;
}

conv::AppCommServiceInfo::~AppCommServiceInfo()
{
	if (registeredRequest != NULL) {
		delete registeredRequest;
	}

	for (ApplicationInstanceList::iterator iter = applicationInstanceList.begin(); iter != applicationInstanceList.end(); ++iter) {
		AppCommServiceApplication *appInfo = *iter;
		delete appInfo;
		applicationInstanceList.erase(iter);
	}
}
