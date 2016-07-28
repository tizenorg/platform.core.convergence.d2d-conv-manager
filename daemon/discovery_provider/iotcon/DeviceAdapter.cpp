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

#include "DeviceAdapter.h"
#include <algorithm>
#include <functional>

using namespace std;

conv::DeviceAdapter::DeviceAdapter(ResourceHandle resourceHandle)
{
	this->m_resource_h = resourceHandle;
}

conv::DeviceAdapter::~DeviceAdapter()
{
}

static bool serviceComparision(conv::IService* obj, int serviceType)
{
	if (obj->getServiceType() == serviceType)
		return true;
	else
		return false;
}

int conv::DeviceAdapter::addService(IService* serviceObj)
{
	service_list_t::iterator itr;
	itr = std::find_if(service_list.begin(), service_list.end(), std::bind(serviceComparision, std::placeholders::_1, serviceObj->getServiceType()));

	if (itr == service_list.end()) {
		_D("New Service Type[%d] added to the device[%s]",
											serviceObj->getServiceType(), getId().c_str());
		service_list.push_back(serviceObj);
	} else {
		_D("Service Type[%d] is already included in device[%s] so skipped!",
											serviceObj->getServiceType(), getId().c_str());
	}
	return CONV_ERROR_NONE;
}

int conv::DeviceAdapter::removeService(IService* serviceObj)
{
	service_list.remove(serviceObj);
	return CONV_ERROR_NONE;
}

string conv::DeviceAdapter::getName()
{
	return m_resource_h.getDeviceName();
}

string conv::DeviceAdapter::getId()
{
	return m_resource_h.getDeviceId();
}

string conv::DeviceAdapter::getAddress()
{
	return m_resource_h.getHostAddress();
}

int conv::DeviceAdapter::getServiceList(std::list<IService*> *list)
{
	for (service_list_t::iterator iterPos = service_list.begin(); iterPos != service_list.end(); ++iterPos) {
		list->push_back(*iterPos);
	}
	return CONV_ERROR_NONE;
}
