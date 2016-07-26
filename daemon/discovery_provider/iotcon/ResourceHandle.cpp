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


int conv::ResourceHandle::setDeviceId(string dev_id)
{
	this->device_id = dev_id;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::getDeviceId()
{
	return this->device_id;
}

int conv::ResourceHandle::setDeviceName(string dev_name)
{
	this->device_name = dev_name;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::getDeviceName()
{
	return device_name;
}

int conv::ResourceHandle::setDeviceType(string dev_type)
{
	this->device_type = dev_type;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::getDeviceType()
{
	return device_type;
}

int	conv::ResourceHandle::setVersion(string ver)
{
	this->version = ver;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::getVersion()
{
	return version;
}

int	conv::ResourceHandle::setServiceList(string list)
{
	this->services_list = Json(list);
	return CONV_ERROR_NONE;
}

int conv::ResourceHandle::setUriPath(string uri_path)
{
	this->uri_path = uri_path;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::getUriPath()
{
	return uri_path;
}

int	conv::ResourceHandle::setHostAddress(string host_address)
{
	this->host_address = host_address;
	return CONV_ERROR_NONE;
}

string conv::ResourceHandle::getHostAddress()
{
	return host_address;
}

list<string>& conv::ResourceHandle::getTypes()
{
	return types;
}

int	conv::ResourceHandle::addType(string type)
{
	types.push_back(type);
	return CONV_ERROR_NONE;
}

list<string>& conv::ResourceHandle::getInterfaces()
{
	return interfaces;
}

int	conv::ResourceHandle::addInterface(string interface)
{
	interfaces.push_back(interface);
	return CONV_ERROR_NONE;
}
