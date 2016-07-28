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

#include "ServiceAdapter.h"

using namespace std;

conv::ServiceAdapter::ServiceAdapter(ResourceHandle resourceHandle)
{
	this->m_resource_h = resourceHandle;
}

conv::ServiceAdapter::~ServiceAdapter()
{
}

string conv::ServiceAdapter::getName()
{
	return m_resource_h.getDeviceName();
}

string conv::ServiceAdapter::getVersion()
{
	return m_resource_h.getVersion();
}

string conv::ServiceAdapter::getType()
{
	return m_resource_h.getDeviceType();
}

string conv::ServiceAdapter::getId()
{
	return m_resource_h.getDeviceId();
}

string conv::ServiceAdapter::getUri()
{
	string uri_path = m_resource_h.getUriPath();
	string host_address = m_resource_h.getHostAddress();

	return host_address + uri_path;
}

int conv::ServiceAdapter::getServiceType()
{
	return service_type;
}

int conv::ServiceAdapter::setServiceType(int serviceType)
{
	this->service_type = serviceType;
	return CONV_ERROR_NONE;
}

int conv::ServiceAdapter::setServiceInfo(string serviceInfo)
{
	this->__serviceInfo = serviceInfo;
	return CONV_ERROR_NONE;
}

string conv::ServiceAdapter::getServiceInfo()
{
	return __serviceInfo;
}
