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

#include "service_adapter.h"

using namespace std;

conv::service_adapter::service_adapter(resource_handle res_h)
{
	this->m_resource_h = res_h;
}

conv::service_adapter::~service_adapter()
{
}

string conv::service_adapter::getName()
{
	return m_resource_h.get_device_name();
}

string conv::service_adapter::getVersion()
{
	return m_resource_h.get_version();
}

string conv::service_adapter::getType()
{
	return m_resource_h.get_device_type();
}

string conv::service_adapter::getId()
{
	return m_resource_h.get_device_id();
}

string conv::service_adapter::getUri()
{
	string uri_path = m_resource_h.get_uri_path();
	string host_address = m_resource_h.get_host_address();

	return host_address + uri_path;
}

int conv::service_adapter::getServiceType()
{
	return service_type;
}

int conv::service_adapter::setServiceType(int serviceType)
{
	this->service_type = serviceType;
	return CONV_ERROR_NONE;
}

int conv::service_adapter::setServiceInfo(string serviceInfo)
{
	this->service_info = serviceInfo;
	return CONV_ERROR_NONE;
}

string conv::service_adapter::getServiceInfo()
{
	return service_info;
}
