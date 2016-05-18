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
#include "device.h"
#include "log.h"
#include "d2d_conv_manager.h"

using namespace std;

conv::device::device()
{
}

conv::device::~device()
{
}

int conv::device::add_service(service_iface* service_obj)
{
	service_list.push_back(service_obj);
	return CONV_ERROR_NONE;
}

int conv::device::remove_service(service_iface* service_obj)
{
	return CONV_ERROR_NONE;
}

string conv::device::getName()
{
	return name;
}

string conv::device::getId()
{
	return id;
}

string conv::device::getAddress()
{
	return ip_address;
}

int conv::device::get_services_list(std::list<service_iface*> *list)
{
	for (service_list_t::iterator iterPos = service_list.begin(); iterPos != service_list.end(); ++iterPos)
	{
		list->push_back(*iterPos);
	}
	return CONV_ERROR_NONE;
}


int conv::device::setName(string name)
{
	this->name = name;
	return CONV_ERROR_NONE;
}

int conv::device::setId(string id)
{
	this->id = id;
	return CONV_ERROR_NONE;
}

int conv::device::setAddress(string id)
{
	ip_address = id;
	return CONV_ERROR_NONE;
}
