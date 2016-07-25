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

#include "ResourceHandle.h"
#include "d2d_conv_manager.h"

using namespace std;


int conv::ResourceHandle::set_device_id(string dev_id)
{
	this->device_id = dev_id;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::get_device_id()
{
	return this->device_id;
}

int conv::ResourceHandle::set_device_name(string dev_name)
{
	this->device_name = dev_name;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::get_device_name()
{
	return device_name;
}

int conv::ResourceHandle::set_device_type(string dev_type)
{
	this->device_type = dev_type;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::get_device_type()
{
	return device_type;
}

int	conv::ResourceHandle::set_version(string ver)
{
	this->version = ver;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::get_version()
{
	return version;
}

int	conv::ResourceHandle::set_services_list(string list)
{
	this->services_list = json(list);
	return CONV_ERROR_NONE;
}

int conv::ResourceHandle::set_uri_path(string uri_path)
{
	this->uri_path = uri_path;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::get_uri_path()
{
	return uri_path;
}

int	conv::ResourceHandle::set_host_address(string host_address)
{
	this->host_address = host_address;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::get_host_address()
{
	return host_address;
}

list<string>& conv::ResourceHandle::get_types()
{
	return types;
}

int	conv::ResourceHandle::add_types(string type)
{
	types.push_back(type);
	return CONV_ERROR_NONE;
}

list<string>& conv::ResourceHandle::get_interfaces()
{
	return interfaces;
}

int	conv::ResourceHandle::add_interfaces(string interface)
{
	interfaces.push_back(interface);
	return CONV_ERROR_NONE;
}
