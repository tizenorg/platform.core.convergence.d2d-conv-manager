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

#include "DiscoveryManager.h"
#include "discovery_provider/SmartviewDiscoveryProvider.h"
#include "discovery_provider/WifiDirectDiscoveryProvider.h"
#include "discovery_provider/IotconDiscoveryProvider.h"
#include "access_control/Privilege.h"

#include "Types.h"
#include "Util.h"
#include <algorithm>
#include <functional>

using namespace std;

static conv::DiscoveryManager *_instance = NULL;
typedef std::map<std::string, conv::IDevice*> discovered_ones_map_t;
static discovered_ones_map_t discovered_results;

conv::DiscoveryManager::DiscoveryManager()
{
	__countDiscoveryRequest = 0;
}

conv::DiscoveryManager::~DiscoveryManager()
{
}

int conv::DiscoveryManager::init()
{
	_D("Discovery_Manager Init!!..");
	registerProvider(new(std::nothrow) conv::SmartviewDiscoveryProvider());
	registerProvider(new(std::nothrow) conv::WifiDirectDiscoveryProvider());
	registerProvider(new(std::nothrow) conv::IotconDiscoveryProvider());

	__requestMap.clear();
	__requestTimerMap.clear();

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::release()
{
	for (DiscoveryProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it)
		(*it)->stop();

	for (DiscoveryProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it)
		delete *it;

	__providerList.clear();

	return CONV_ERROR_NONE;
}

void conv::discovery_manager::setInstance(conv::DiscoveryManager* mgr)
{
	_instance = mgr;
}

int conv::DiscoveryManager::notifyTimeOut(std::string client)
{
	// 1. When no client is using discovery, it should be stopped
	_D("notifyTimeOut.. with current discovery count :%d", __countDiscoveryRequest);
	if (--__countDiscoveryRequest <= 0) {
		__countDiscoveryRequest = 0;
		stopDiscovery();
	}

	// 2. Reqeust to stop timer related to client in the timer_map
	TimerMap::iterator timer_itr = __requestTimerMap.find(client);
	if (timer_itr != __requestTimerMap.end()) {
		int timer_id = timer_itr->second;
		_D("timer_id[%d]", timer_id);
		conv::util::miscStopTimer(reinterpret_cast<void*> (timer_id));
	}

	// 3. Notify the client that the requested discovery has been finished
	RequestMap::iterator request_itr = __requestMap.find(client);
	if (request_itr != __requestMap.end()) {
		Json no_data;
		Request* cur_Req = request_itr->second;
		cur_Req->publish(CONV_DISCOVERY_FINISHED, no_data);
	}

	return CONV_ERROR_NONE;
}

void conv::DiscoveryManager::__timer_worker(void* data)
{
	gpointer* param = reinterpret_cast<gpointer*> (data);
	std::string* req_client = reinterpret_cast<std::string*> (param[0]);
	conv::DiscoveryManager* cur_disc_mgr = reinterpret_cast<conv::DiscoveryManager*> (param[1]);

	_D("Timer_Worker.. req_client[%s] discovery_manager[%x]", (*req_client).c_str(), cur_disc_mgr);

	cur_disc_mgr->notifyTimeOut(*req_client);
}

int conv::DiscoveryManager::checkBoundaryForTimeOut(int givenTimeout)
{
	if ( givenTimeout < CONV_DISCOVERY_MIN_VALUE )
		return CONV_DISCOVERY_MIN_VALUE;
	else if ( givenTimeout > CONV_DISCOVERY_MAX_VALUE )
		return CONV_DISCOVERY_MAX_VALUE;
	else
		return givenTimeout;
}

int conv::DiscoveryManager::handleRequest(Request* requestObj)
{
	_D("handle_request called .. request:%x _instance:%x", requestObj, _instance);
	if ( _instance ) {
		if ( !strcmp(requestObj->getSubject(), CONV_SUBJECT_DISCOVERY_START) ) {
			if ( !conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_INTERNET) ||
					!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_BLUETOOTH) ) {
				_E("permission denied");
				requestObj->reply(CONV_ERROR_PERMISSION_DENIED);
				delete requestObj;
				return CONV_ERROR_PERMISSION_DENIED;
			}

			discovered_results.clear();

			for (DiscoveryProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it)
				// Discovery Provider Starts!!!!
				(*it)->start();

			const char* client = requestObj->getSender();
			if (client == NULL) {
				_D("client is empty..");
				return CONV_ERROR_INVALID_OPERATION;
			}
			_D("requestObj info .. client[%s]", client);

			int timeout = 0;
			Json description = requestObj->getDescription();
			description.get(NULL, "timeout", &timeout);
			timeout = checkBoundaryForTimeOut(timeout);

			RequestMap::iterator map_itr = __requestMap.find(string(client));

			if ( map_itr == __requestMap.end()) {
				// current request inserted into __requestMap..
				__requestMap.insert(RequestMap::value_type(string(client), requestObj));
				_D("client[%s] inserted into __requestMap", client);
				__countDiscoveryRequest++;
			} else {
				_D("client[%s] already in __requestMap.. Replace!!!", client);
				map_itr->second = requestObj;
				// stop the timer if there's one already running
				TimerMap::iterator timer_itr = __requestTimerMap.find(client);
				if (timer_itr != __requestTimerMap.end()) {
					int timer_id = timer_itr->second;
					_D("timer_id[%d]", timer_id);
					conv::util::miscStopTimer(reinterpret_cast<void*> (timer_id));
				}
			}

			// request timer
			gpointer *param = g_new0(gpointer, 2);
			param[0] = reinterpret_cast<void*>(new(std::nothrow) string(client));
			param[1] = reinterpret_cast<void*>(this);

			int timer_id = reinterpret_cast<int>(conv::util::miscStartTimer(__timer_worker, timeout, param));
			__requestTimerMap[ string(client) ] = timer_id;

			requestObj->reply(CONV_ERROR_NONE);
		} else if ( !strcmp(requestObj->getSubject(), CONV_SUBJECT_DISCOVERY_STOP) ){
			const char* client = requestObj->getSender();

			if (__countDiscoveryRequest <= 0) {
				_D("discovery is already stopped");
				requestObj->reply(CONV_ERROR_INVALID_OPERATION);
			} else {
				notifyTimeOut(client);
				requestObj->reply(CONV_ERROR_NONE);
			}

			delete requestObj;
		}

		return CONV_ERROR_NONE;
	}

	return CONV_ERROR_INVALID_OPERATION;;
}

int conv::DiscoveryManager::startDiscovery()
{
	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::stopDiscovery()
{
	_D("Stop_Discovery...");
	for (DiscoveryProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it) {
		(*it)->stop();
	}
	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::registerProvider(IDiscoveryProvider *provider)
{
	if (!provider) {
		_E("Provider NULL");
		return CONV_ERROR_INVALID_PARAMETER;
	}

	if (provider->setManager(this) != CONV_ERROR_NONE) {
		_E("Provider setManager failed");
		delete provider;
		return CONV_ERROR_INVALID_OPERATION;
	}

	if (provider->init() != CONV_ERROR_NONE) {
		_E("Provider initialization failed");
		delete provider;
		return CONV_ERROR_INVALID_OPERATION;
	}

	__providerList.push_back(provider);

	return CONV_ERROR_NONE;
}
int conv::DiscoveryManager::convertDeviceIntoJson(conv::IDevice* device_info, Json* jsonData)
{
	jsonData->set(NULL, CONV_JSON_DEVICE_ID, device_info->getId());
	jsonData->set(NULL, CONV_JSON_DEVICE_NAME, device_info->getName());
	jsonData->set(NULL, CONV_JSON_DEVICE_ADDRESS, device_info->getAddress());

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::convertServiceIntoJson(conv::IService* serviceInfo, Json* jsonData)
{
	string service_info_str = serviceInfo->getServiceInfo();
	_D("Service-2-Json Conversion : %s", service_info_str.c_str());
	Json serviceJson(service_info_str);
	jsonData->appendArray(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_DATA, serviceJson);
	jsonData->appendArray(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_TYPE, serviceInfo->getServiceType());

	return CONV_ERROR_NONE;
}

static bool serviceComparision(conv::IService* obj, int serviceType)
{
	if (obj->getServiceType() == serviceType)
		return true;
	else
		return false;
}

int conv::DiscoveryManager::excludeServices(conv::IDevice* org_device, conv::IDevice* removedDevice)
{
	int remained_serv_count = 0;
	std::list<IService*> org_serv_list;
	std::list<IService*> removed_serv_list;

	org_device->getServiceList(&org_serv_list);
	_D("[%d] Services in the origin device info[%s]", org_serv_list.size(), org_device->getName().c_str());
	remained_serv_count = org_serv_list.size();

	removedDevice->getServiceList(&removed_serv_list);
	_D("[%d] Services in the removed device info[%s]", removed_serv_list.size(), removedDevice->getName().c_str());

	std::list<IService*>::iterator removed_itr = removed_serv_list.begin();
	for (; removed_itr != removed_serv_list.end(); ++removed_itr) {
		IService* cur_removed_serv = *removed_itr;
		std::list<IService*>::iterator org_iter = std::find_if(org_serv_list.begin(), org_serv_list.end(), std::bind(serviceComparision, std::placeholders::_1, cur_removed_serv->getServiceType()));
		if (org_iter != org_serv_list.end()) {
			IService* cur_serv = *org_iter;
			_D("Service[%d],notified as removed one, found in device[%s]", cur_serv->getServiceType(), cur_serv->getName().c_str());
			org_device->removeService(cur_serv);
			remained_serv_count--;
		}
	}

	return remained_serv_count;
}


// return value : the number of new services
int conv::DiscoveryManager::mergeExcludeServices(conv::IDevice* org_device, conv::IDevice* newDevice)
{
	int new_serv_count = 0;
	std::list<IService*> org_serv_list;
	std::list<IService*> new_serv_list;

	org_device->getServiceList(&org_serv_list);
	_D("[%d] Services in the origin device info[%s]", org_serv_list.size(), org_device->getName().c_str() );
	newDevice->getServiceList(&new_serv_list);
	_D("[%d] Services in the new device info[%s]", new_serv_list.size(), newDevice->getName().c_str() );

	std::list<IService*>::iterator new_iter = new_serv_list.begin();
	for (; new_iter != new_serv_list.end(); ++new_iter) {
		IService* cur_serv = *new_iter;
		std::list<IService*>::iterator org_iter =
			std::find_if(org_serv_list.begin(), org_serv_list.end(), std::bind(serviceComparision, std::placeholders::_1, cur_serv->getServiceType()));
		if (org_iter != org_serv_list.end()) {
			// already exists in org_device.. means it's not new!.. so remove the service from new!!
			newDevice->removeService(cur_serv);
			_D("Service[%d] has been already found in Device[%s]", cur_serv->getServiceType(), org_device->getName().c_str() );
		} else {
			_D("New Service[%d] found in Device[%s]", cur_serv->getServiceType(), org_device->getName().c_str() );
			// add the service into the original device
			org_device->addService(cur_serv);
			new_serv_count++;
		}
	}
	return new_serv_count;
}

int conv::DiscoveryManager::notifyLostDevice(IDevice* discoveredDevice)
{
	int num_remained_service = 0;
	// 1. find the device and remove the services included in discoveredDevice from cache (discovered_results)
	discovered_ones_map_t::iterator itor_disc;
	itor_disc = discovered_results.find(discoveredDevice->getId());
	if (itor_disc != discovered_results.end()) {
		IDevice* cur_device = itor_disc->second;
		num_remained_service = excludeServices(cur_device, discoveredDevice);
	} else {
		_D("Lost Notify dismissed - No discovered results corresponding to id[%s]", discoveredDevice->getId().c_str());
		return CONV_ERROR_NO_DATA;
	}

	// 2. if no services is left included in discoveredDevice, then remove the device from the cache and notify to a client
	if (num_remained_service == 0) {
		// remove from the cache
		discovered_results.erase(discoveredDevice->getId());

		// iterate through __requestMap for service
		_D("Iterate through __requestMap to publish..");
		RequestMap::iterator IterPos;
		Json device_json;
		convertDeviceIntoJson(discoveredDevice, &device_json);
		for (IterPos = __requestMap.begin(); IterPos != __requestMap.end(); ++IterPos) {
			Request* cur_Req = IterPos->second;
			cur_Req->publish(CONV_DISCOVERY_DEVICE_LOST, device_json);
		}
	}

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::appendDiscoveredResult(conv::IDevice* discoveredDevice)
{
	conv::IDevice* publish_device_info = NULL;

	_D("Append Discovered Result.. Device:%x, Service:%x");
	IF_FAIL_RETURN_TAG((discoveredDevice != NULL), CONV_ERROR_INVALID_PARAMETER, _E, "IDevice not initialized..");
	// discovery_manager deals with the cache for discovered ones
	_D("Check if key[%s] exists in discovered_results", discoveredDevice->getId().c_str());
	discovered_ones_map_t::iterator itor_disc;
	itor_disc = discovered_results.find(discoveredDevice->getId());
	if (itor_disc != discovered_results.end()) {
		_D("update discovered device's info [%s]", discoveredDevice->getId().c_str());
		IDevice* cur_device = itor_disc->second;

		int count_new_services = mergeExcludeServices(cur_device, discoveredDevice);
		if (count_new_services == 0)
			return CONV_ERROR_NONE;
	} else {
		_D("newbie!! discovered device's info [%s]", discoveredDevice->getId().c_str());
		discovered_results.insert(discovered_ones_map_t::value_type(discoveredDevice->getId(), discoveredDevice));
	}
	publish_device_info = discoveredDevice;

	_D("Convert device_info into Json type..");
	Json device_json;
	convertDeviceIntoJson(publish_device_info, &device_json);

	_D("Convert service info into Json type..");
	typedef std::list<IService*> serv_list_t;
	serv_list_t serv_list;
	publish_device_info->getServiceList(&serv_list);

	for (serv_list_t::iterator iterPos = serv_list.begin(); iterPos != serv_list.end(); ++iterPos) {
		IService* cur_serv = *iterPos;
		convertServiceIntoJson(cur_serv, &device_json);
	}

	// iterate through __requestMap for service
	_D("Iterate through __requestMap to publish..");
	int index = 0;
	RequestMap::iterator IterPos;
	for (IterPos = __requestMap.begin(); IterPos != __requestMap.end(); ++IterPos) {
		Request* cur_Req = IterPos->second;
		cur_Req->publish(CONV_ERROR_NONE, device_json);

		index++;
	}

	return CONV_ERROR_NONE;
}

int conv::discovery_manager::handleRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(_instance, CONV_ERROR_INVALID_PARAMETER, _E, "Not initialized");
	_instance->handleRequest(requestObj);

	return CONV_ERROR_NONE;
}
