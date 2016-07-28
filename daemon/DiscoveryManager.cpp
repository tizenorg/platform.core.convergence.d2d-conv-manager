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
typedef std::map<std::string, conv::IDevice*> DiscoveredDeviceMap;
static DiscoveredDeviceMap discoveredResults;

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
	TimerMap::iterator timerIter = __requestTimerMap.find(client);
	if (timerIter != __requestTimerMap.end()) {
		int timerId = timerIter->second;
		_D("timerId[%d]", timerId);
		conv::util::miscStopTimer(reinterpret_cast<void*> (timerId));
		__requestTimerMap.erase(timerIter);
	}

	// 3. Notify the client that the requested discovery has been finished
	RequestMap::iterator requestIter = __requestMap.find(client);
	if (requestIter != __requestMap.end()) {
		Json noData;
		Request* currentRequest = requestIter->second;
		currentRequest->publish(CONV_DISCOVERY_FINISHED, noData);
		__requestMap.erase(requestIter);
	}

	return CONV_ERROR_NONE;
}

void conv::DiscoveryManager::__timer_worker(void* data)
{
	gpointer* param = reinterpret_cast<gpointer*> (data);
	std::string* requestedClient = reinterpret_cast<std::string*> (param[0]);
	conv::DiscoveryManager* currentDiscoveryMgr = reinterpret_cast<conv::DiscoveryManager*> (param[1]);

	_D("Timer_Worker.. req_client[%s] discovery_manager[%x]", (*requestedClient).c_str(), currentDiscoveryMgr);

	currentDiscoveryMgr->notifyTimeOut(*requestedClient);
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

			discoveredResults.clear();

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
				TimerMap::iterator timerIter = __requestTimerMap.find(client);
				if (timerIter != __requestTimerMap.end()) {
					int timerId = timerIter->second;
					_D("timerId[%d]", timerId);
					conv::util::miscStopTimer(reinterpret_cast<void*> (timerId));
					__requestTimerMap.erase(timerIter);
				}
			}

			// request timer
			gpointer *param = g_new0(gpointer, 2);
			param[0] = reinterpret_cast<void*>(new(std::nothrow) string(client));
			param[1] = reinterpret_cast<void*>(this);

			int timerId = reinterpret_cast<int>(conv::util::miscStartTimer(__timer_worker, timeout, param));
			__requestTimerMap[string(client)] = timerId;

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

int conv::DiscoveryManager::excludeServices(conv::IDevice* orgDevice, conv::IDevice* removedDevice)
{
	int remained_serv_count = 0;
	std::list<IService*> orgServiceList;
	std::list<IService*> removed_serv_list;

	orgDevice->getServiceList(&orgServiceList);
	_D("[%d] Services in the origin device info[%s]", orgServiceList.size(), orgDevice->getName().c_str());
	remained_serv_count = orgServiceList.size();

	removedDevice->getServiceList(&removed_serv_list);
	_D("[%d] Services in the removed device info[%s]", removed_serv_list.size(), removedDevice->getName().c_str());

	std::list<IService*>::iterator removedIter = removed_serv_list.begin();
	for (; removedIter != removed_serv_list.end(); ++removedIter) {
		IService* cur_removed_serv = *removedIter;
		std::list<IService*>::iterator orgIter = std::find_if(orgServiceList.begin(), orgServiceList.end(), std::bind(serviceComparision, std::placeholders::_1, cur_removed_serv->getServiceType()));
		if (orgIter != orgServiceList.end()) {
			IService* currentService = *orgIter;
			_D("Service[%d],notified as removed one, found in device[%s]", currentService->getServiceType(), currentService->getName().c_str());
			orgDevice->removeService(currentService);
			remained_serv_count--;
		}
	}

	return remained_serv_count;
}


// return value : the number of new services
int conv::DiscoveryManager::mergeExcludeServices(conv::IDevice* orgDevice, conv::IDevice* newDevice)
{
	int newServiceCount = 0;
	std::list<IService*> orgServiceList;
	std::list<IService*> newServiceList;

	orgDevice->getServiceList(&orgServiceList);
	_D("[%d] Services in the origin device info[%s]", orgServiceList.size(), orgDevice->getName().c_str() );
	newDevice->getServiceList(&newServiceList);
	_D("[%d] Services in the new device info[%s]", newServiceList.size(), newDevice->getName().c_str() );

	std::list<IService*>::iterator newIter = newServiceList.begin();
	for (; newIter != newServiceList.end(); ++newIter) {
		IService* currentService = *newIter;
		std::list<IService*>::iterator orgIter =
			std::find_if(orgServiceList.begin(), orgServiceList.end(), std::bind(serviceComparision, std::placeholders::_1, currentService->getServiceType()));
		if (orgIter != orgServiceList.end()) {
			// already exists in orgDevice.. means it's not new!.. so remove the service from new!!
			newDevice->removeService(currentService);
			_D("Service[%d] has been already found in Device[%s]", currentService->getServiceType(), orgDevice->getName().c_str() );
		} else {
			_D("New Service[%d] found in Device[%s]", currentService->getServiceType(), orgDevice->getName().c_str() );
			// add the service into the original device
			orgDevice->addService(currentService);
			newServiceCount++;
		}
	}
	return newServiceCount;
}

int conv::DiscoveryManager::notifyLostDevice(IDevice* discoveredDevice)
{
	int num_remained_service = 0;
	// 1. find the device and remove the services included in discoveredDevice from cache (discoveredResults)
	DiscoveredDeviceMap::iterator iterDiscovered;
	iterDiscovered = discoveredResults.find(discoveredDevice->getId());
	if (iterDiscovered != discoveredResults.end()) {
		IDevice* currentDevice = iterDiscovered->second;
		num_remained_service = excludeServices(currentDevice, discoveredDevice);
	} else {
		_D("Lost Notify dismissed - No discovered results corresponding to id[%s]", discoveredDevice->getId().c_str());
		return CONV_ERROR_NO_DATA;
	}

	// 2. if no services is left included in discoveredDevice, then remove the device from the cache and notify to a client
	if (num_remained_service == 0) {
		// remove from the cache
		discoveredResults.erase(discoveredDevice->getId());

		// iterate through __requestMap for service
		_D("Iterate through __requestMap to publish..");
		RequestMap::iterator IterPos;
		Json deviceJson;
		convertDeviceIntoJson(discoveredDevice, &deviceJson);
		for (IterPos = __requestMap.begin(); IterPos != __requestMap.end(); ++IterPos) {
			Request* currentRequest = IterPos->second;
			currentRequest->publish(CONV_DISCOVERY_DEVICE_LOST, deviceJson);
		}
	}

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::appendDiscoveredResult(conv::IDevice* discoveredDevice)
{
	conv::IDevice* deviceInfoToPublish = NULL;

	_D("Append Discovered Result.. Device:%x, Service:%x");
	IF_FAIL_RETURN_TAG((discoveredDevice != NULL), CONV_ERROR_INVALID_PARAMETER, _E, "IDevice not initialized..");
	// discovery_manager deals with the cache for discovered ones
	_D("Check if key[%s] exists in discoveredResults", discoveredDevice->getId().c_str());
	DiscoveredDeviceMap::iterator iterDiscovered;
	iterDiscovered = discoveredResults.find(discoveredDevice->getId());
	if (iterDiscovered != discoveredResults.end()) {
		_D("update discovered device's info [%s]", discoveredDevice->getId().c_str());
		IDevice* currentDevice = iterDiscovered->second;

		int countNewServices = mergeExcludeServices(currentDevice, discoveredDevice);
		if (countNewServices == 0)
			return CONV_ERROR_NONE;
	} else {
		_D("newbie!! discovered device's info [%s]", discoveredDevice->getId().c_str());
		discoveredResults.insert(DiscoveredDeviceMap::value_type(discoveredDevice->getId(), discoveredDevice));
	}
	deviceInfoToPublish = discoveredDevice;

	_D("Convert device_info into Json type..");
	Json deviceJson;
	convertDeviceIntoJson(deviceInfoToPublish, &deviceJson);

	_D("Convert service info into Json type..");
	typedef std::list<IService*> ServiceList;
	ServiceList serviceList;
	deviceInfoToPublish->getServiceList(&serviceList);

	for (ServiceList::iterator iterPos = serviceList.begin(); iterPos != serviceList.end(); ++iterPos) {
		IService* currentService = *iterPos;
		convertServiceIntoJson(currentService, &deviceJson);
	}

	// iterate through __requestMap for service
	_D("Iterate through __requestMap to publish..");
	int index = 0;
	RequestMap::iterator IterPos;
	for (IterPos = __requestMap.begin(); IterPos != __requestMap.end(); ++IterPos) {
		Request* currentRequest = IterPos->second;
		currentRequest->publish(CONV_ERROR_NONE, deviceJson);

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
