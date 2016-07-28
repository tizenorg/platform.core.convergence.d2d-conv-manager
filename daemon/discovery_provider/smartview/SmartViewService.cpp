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

#include <glib.h>
#include "SmartViewService.h"

using namespace std;

conv::SmartViewService::SmartViewService()
{
}

conv::SmartViewService::~SmartViewService()
{
}

string conv::SmartViewService::getName()
{
	return service_name;
}

string conv::SmartViewService::getVersion()
{
	return service_version;
}

string conv::SmartViewService::getType()
{
	return this->type;
}

string conv::SmartViewService::getId()
{
	return service_id;
}

string conv::SmartViewService::getUri()
{
	return service_uri;
}

int conv::SmartViewService::getServiceType()
{
	return service_type;
}

string conv::SmartViewService::getServiceInfo()
{
	return __serviceInfo;
}

void conv::SmartViewService::setName(string name)
{
	this->service_name = name;
}

void conv::SmartViewService::setVersion(string version)
{
	this->service_version = version;
}

void conv::SmartViewService::setType(string type)
{
	this->type = type;
}

void conv::SmartViewService::setId(string id)
{
	this->service_id = id;
}

void conv::SmartViewService::setUri(string uri)
{
	this->service_uri = uri;
}

void conv::SmartViewService::setServiceInfo(string serviceInfo)
{
	this->__serviceInfo = serviceInfo;
}

void conv::SmartViewService::setServiceType(int service_type)
{
	this->service_type = service_type;
}

void conv::SmartViewService::printInfo()
{
	_D("Flow Service Info : Name[%s] Version[%s] Type[%d] Id[%s] Uri[%s] ",
			service_name.c_str(), service_version.c_str(), service_type,
			service_id.c_str(), service_uri.c_str());
}
