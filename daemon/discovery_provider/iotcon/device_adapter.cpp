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

#include "device_adapter.h"

using namespace std;

conv::device_adapter::device_adapter(resource_handle res_h)
{
	this->m_resource_h = res_h;
}

conv::device_adapter::~device_adapter()
{
}

int conv::device_adapter::add_service(service_iface* service_obj)
{
	service_list.push_back(service_obj);
	return CONV_ERROR_NONE;
}

int conv::device_adapter::remove_service(service_iface* service_obj)
{
	service_list.remove(service_obj);
	return CONV_ERROR_NONE;
}

string conv::device_adapter::getName()
{
	return m_resource_h.get_device_name();
}

string conv::device_adapter::getId()
{
	return m_resource_h.get_device_id();
}

string conv::device_adapter::getAddress()
{
	return m_resource_h.get_host_address();
}

int conv::device_adapter::get_services_list(std::list<service_iface*> *list)
{
	for (service_list_t::iterator iterPos = service_list.begin(); iterPos != service_list.end(); ++iterPos)
	{
		list->push_back(*iterPos);
	}
	return CONV_ERROR_NONE;
}
