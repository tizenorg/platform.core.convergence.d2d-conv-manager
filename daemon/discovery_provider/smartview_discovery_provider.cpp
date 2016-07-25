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

#include "smartview_discovery_provider.h"

#include "../DiscoveryManager.h"
#include "../conv_json.h"

#include "../Util.h"

using namespace std;

class SearchListenerImpl : public SearchListener {
	private:
		conv::DiscoveryManager* disc_manager;
		conv::smartview_discovery_provider*	disc_provider;

	public:
		void set_discovery_manager(conv::DiscoveryManager* discovery_manager)
		{
			this->disc_manager = discovery_manager;
		}

		void set_discovery_provider(conv::smartview_discovery_provider* discovery_provider)
		{
			this->disc_provider = discovery_provider;
		}

		void onStart()
		{
			_I("onStart on SearchListener");
		}
		void onStop()
		{
			_I("onStop on SearchListener");
		}
		void onFound(Service service)
		{
			_I("onFound on SearchListener.. towards disc_manager:%x", disc_manager);
			if (disc_provider != NULL) {
				disc_provider->notice_discovered(&service, true);
			}
		}
		void onLost(Service service)
		{
			_I("onLost on SearchListener");
			if (disc_provider != NULL) {
				disc_provider->notice_discovered(&service, false);
			}
		}
};

static SearchListenerImpl* listener_impl = NULL;

conv::smartview_discovery_provider::smartview_discovery_provider()
{
}

conv::smartview_discovery_provider::~smartview_discovery_provider()
{
}

int conv::smartview_discovery_provider::init()
{
	_D("smartview_discovery init");
	search = new(std::nothrow) Search();
	ASSERT_ALLOC(search);
	if (listener_impl == NULL) {
		listener_impl = new(std::nothrow) SearchListenerImpl;
		if ( listener_impl == NULL) {
			_E("listener_impl allocation failed");
			delete search;
			return CONV_ERROR_OUT_OF_MEMORY;
		}
		listener_impl->set_discovery_provider(this);
		listener_impl->set_discovery_manager(_discovery_manager);
	}
	search->setSearchListener(listener_impl);

	_D("smartview_discovery init Done");

	return CONV_ERROR_NONE;
}

int conv::smartview_discovery_provider::release()
{
	if ( search != NULL ) {
		delete search;
	}

	return CONV_ERROR_NONE;
}

int conv::smartview_discovery_provider::start()
{
	// Cache init..
	cache.clear();

	_D("smartview_discovery start");
	search->start();
	_D("smartview_discovery start done");

	return CONV_ERROR_NONE;
}

int conv::smartview_discovery_provider::stop()
{
	_D("smartview_discovery stop");
	search->stop();
	return CONV_ERROR_NONE;
}

conv::device* conv::smartview_discovery_provider::convert_into_conv_device(Service* smartview_service)
{
	string serv_name, serv_version, serv_type, serv_id, serv_uri;

	serv_name = smartview_service->getName();
	serv_version = smartview_service->getVersion();
	serv_type = smartview_service->getType();
	serv_id = smartview_service->getId();
	serv_uri = smartview_service->getUri();

	_D("SmartView Discovered Service : Name[%s] Version[%s] Type[%s] Id[%s] Uri[%s] ",
								serv_name.c_str(), serv_version.c_str(), serv_type.c_str(),
								serv_id.c_str(), serv_uri.c_str() );

	conv::device*	device_info = new(std::nothrow) conv::device;
	device_info->setId(serv_id);
	device_info->setName(serv_name);

	return device_info;
}

conv::service* conv::smartview_discovery_provider::convert_into_conv_service(Service* smartview_service)
{
	string serv_name, serv_version, serv_type, serv_id, serv_uri;

	serv_name = smartview_service->getName();
	serv_version = smartview_service->getVersion();
	serv_type = smartview_service->getType();
	serv_id = smartview_service->getId();
	serv_uri = smartview_service->getUri();

	_D("SmartView Discovered Service : Name[%s] Version[%s] Type[%s] Id[%s] Uri[%s] ",
								serv_name.c_str(), serv_version.c_str(), serv_type.c_str(),
								serv_id.c_str(), serv_uri.c_str() );

	json json_serv_info;
	json_serv_info.set(CONV_JSON_SERVICE_DATA_PATH, CONV_JSON_SERVICE_DATA_URI, serv_uri);
	json_serv_info.set(CONV_JSON_SERVICE_DATA_PATH, CONV_JSON_SERVICE_DATA_ID, serv_id);
	json_serv_info.set(CONV_JSON_SERVICE_DATA_PATH, CONV_JSON_SERVICE_DATA_NAME, serv_name);
	json_serv_info.set(CONV_JSON_SERVICE_DATA_PATH, CONV_JSON_SERVICE_DATA_VERSION, serv_version);
	json_serv_info.set(CONV_JSON_SERVICE_DATA_PATH, CONV_JSON_SERVICE_DATA_TYPE, serv_type);

	conv::service *conv_service = new(std::nothrow) conv::service;

	if ( conv_service == NULL ) {
		_E("conv_service allocation failed");
		return NULL;
	}
	conv_service->setName(serv_name);
	conv_service->setVersion(serv_version);
	conv_service->setType(serv_type);
	conv_service->setId(serv_id);
	conv_service->setUri(serv_uri);

	conv_service->setServiceInfo(json_serv_info.str());
	conv_service->setServiceType(CONV_TYPE_APP_TO_APP_COMMUNICATION);

	return conv_service;
}

int conv::smartview_discovery_provider::removeFromCache(conv::service* conv_service)
{
	string cache_key = conv_service->getUri();
	cache.erase(cache_key);
	return CONV_ERROR_NONE;
}

int conv::smartview_discovery_provider::checkExistence(conv::service* conv_service)
{
	_D("Check Existence : ");
	conv_service->printInfo();

	// insert into cache
	string cache_key = conv_service->getUri();	// Serivce URI as Map Key
	if (cache.find(cache_key) == cache.end()){
		_D("Flow_service with key[%s] does not exist..so go into the cache", cache_key.c_str());
		cache.insert(map<string, conv::service*>::value_type(cache_key, conv_service));
		return CONV_ERROR_NONE;
	} else {
		_D("Flow_service with key[%s] already exists..", cache_key.c_str());
		return CONV_ERROR_INVALID_PARAMETER;
	}
}

int conv::smartview_discovery_provider::notice_discovered(Service* service, bool bDiscovered)
{
	_D("Notice Discovered called with service[%x]", service);

	// Covert MSF-API's Service into D2D Flow's Device n Service
	conv::device*	conv_device = convert_into_conv_device(service);
	IF_FAIL_RETURN_TAG((conv_device != NULL), CONV_ERROR_INVALID_PARAMETER, _E, "failed to convert into flow device..");

	conv::service* conv_service = convert_into_conv_service(service);
	IF_FAIL_RETURN_TAG((conv_service != NULL), CONV_ERROR_INVALID_PARAMETER, _E, "failed to convert into flow service..");
	conv_device->add_service(conv_service);

	_D("Success in converting into flow.service[%x] .device[%x]", conv_service, conv_device);

	if ( conv::util::getDeviceId().compare(conv_service->getId()) == 0 )	{
		_D("the device has found itself..[device_id:%s].. out!", conv_service->getId().c_str());
		if(conv_device != NULL)		delete conv_device;
		if(conv_service != NULL)	delete conv_service;

		return CONV_ERROR_NONE;
	}

	if (bDiscovered == false) {
		//1. delete it from the cache..
		removeFromCache(conv_service);
		//2. notify
		_discovery_manager->notify_lost_device(conv_device);

		return CONV_ERROR_NONE;
	} else {
		// Double check if the noticed one already got delivered
		int alreadyExisted = checkExistence(conv_service);
		_D("double check .. existence[%s]", (alreadyExisted == 0)? "No" : "Yes");

		if (!alreadyExisted) {
			//the discovered one is NEW!!
			_discovery_manager->append_discovered_result(conv_device);
		}
	}

	return CONV_ERROR_NONE;
}
