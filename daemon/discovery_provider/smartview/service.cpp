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
#include "service.h"

using namespace std;

conv::service::service()
{
}

conv::service::~service()
{
}

string conv::service::getName()
{
	return service_name;
}

string conv::service::getVersion()
{
	return service_version;
}

string conv::service::getType()
{
	return this->type;
}

string conv::service::getId()
{
	return service_id;
}

string conv::service::getUri()
{
	return service_uri;
}

int conv::service::getServiceType()
{
	return 0;
}

string conv::service::getServiceInfo()
{
	return service_info;
}

void conv::service::setName(string name)
{
	this->service_name = name;
}

void conv::service::setVersion(string version)
{
	this->service_version = version;
}

void conv::service::setType(string type)
{
	this->type = type;
}

void conv::service::setId(string id)
{
	this->service_id = id;
}

void conv::service::setUri(string uri)
{
	this->service_uri = uri;
}

void conv::service::setServiceInfo(string service_info)
{
	this->service_info = service_info;
}

void conv::service::setServiceType(int service_type)
{
	this->service_type = service_type;
}

void conv::service::printInfo()
{
	_D("Flow Service Info : Name[%s] Version[%s] Type[%d] Id[%s] Uri[%s] ",
			service_name.c_str(), service_version.c_str(), service_type,
			service_id.c_str(), service_uri.c_str());
}
