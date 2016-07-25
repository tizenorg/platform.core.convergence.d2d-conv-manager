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
	count_discovery_request = 0;
}

conv::DiscoveryManager::~DiscoveryManager()
{
}

int conv::DiscoveryManager::init()
{
	_D("Discovery_Manager Init!!..");
	register_provider(new(std::nothrow) conv::SmartviewDiscoveryProvider());
	register_provider(new(std::nothrow) conv::WifiDirectDiscoveryProvider());
	register_provider(new(std::nothrow) conv::IotconDiscoveryProvider());

	request_map.clear();
	request_timer_map.clear();

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::release()
{
	for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it)
		(*it)->stop();

	for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it)
		delete *it;

	provider_list.clear();

	return CONV_ERROR_NONE;
}

void conv::discovery_manager::set_instance(conv::DiscoveryManager* mgr)
{
	_instance = mgr;
}

int conv::DiscoveryManager::notify_time_up(std::string client)
{
	// 1. When no client is using discovery, it should be stopped
	_D("notify_time_up.. with current discovery count :%d", count_discovery_request);
	if (--count_discovery_request <= 0) {
		count_discovery_request = 0;
		stop_discovery();
	}

	// 2. Reqeust to stop timer related to client in the timer_map
	timer_map_t::iterator timer_itr = request_timer_map.find(client);
	if (timer_itr != request_timer_map.end()) {
		int timer_id = timer_itr->second;
		_D("timer_id[%d]", timer_id);
		conv::util::miscStopTimer(reinterpret_cast<void*> (timer_id));
	}

	// 3. Notify the client that the requested discovery has been finished
	request_map_t::iterator request_itr = request_map.find(client);
	if (request_itr != request_map.end()) {
		json no_data;
		request* cur_Req = request_itr->second;
		cur_Req->publish(CONV_DISCOVERY_FINISHED, no_data);
	}

	return CONV_ERROR_NONE;
}

void conv::DiscoveryManager::timer_worker(void* data)
{
	gpointer* param = reinterpret_cast<gpointer*> (data);
	std::string* req_client = reinterpret_cast<std::string*> (param[0]);
	conv::DiscoveryManager* cur_disc_mgr = reinterpret_cast<conv::DiscoveryManager*> (param[1]);

	_D("Timer_Worker.. req_client[%s] discovery_manager[%x]", (*req_client).c_str(), cur_disc_mgr);

	cur_disc_mgr->notify_time_up(*req_client);
}

int conv::DiscoveryManager::checkBoundaryForTimeout(int givenTimeout)
{
	if ( givenTimeout < CONV_DISCOVERY_MIN_VALUE )
		return CONV_DISCOVERY_MIN_VALUE;
	else if ( givenTimeout > CONV_DISCOVERY_MAX_VALUE )
		return CONV_DISCOVERY_MAX_VALUE;
	else
		return givenTimeout;
}

int conv::DiscoveryManager::handleRequest(request* requestObj)
{
	_D("handle_request called .. request:%x _instance:%x", requestObj, _instance);
	if ( _instance ) {
		if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_DISCOVERY_START) ) {
			if ( !conv::privilege_manager::isAllowed(requestObj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
					!conv::privilege_manager::isAllowed(requestObj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) ) {
				_E("permission denied");
				requestObj->reply(CONV_ERROR_PERMISSION_DENIED);
				delete requestObj;
				return CONV_ERROR_PERMISSION_DENIED;
			}

			discovered_results.clear();

			for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it)
				// Discovery Provider Starts!!!!
				(*it)->start();

			const char* client = requestObj->get_sender();
			if (client == NULL) {
				_D("client is empty..");
				return CONV_ERROR_INVALID_OPERATION;
			}
			_D("requestObj info .. client[%s]", client);

			int timeout = 0;
			json description = requestObj->get_description();
			description.get(NULL, "timeout", &timeout);
			timeout = checkBoundaryForTimeout(timeout);

			request_map_t::iterator map_itr = request_map.find(string(client));

			if ( map_itr == request_map.end()) {
				// current request inserted into request_map..
				request_map.insert(request_map_t::value_type(string(client), requestObj));
				_D("client[%s] inserted into request_map", client);
				count_discovery_request++;
			} else {
				_D("client[%s] already in request_map.. Replace!!!", client);
				map_itr->second = requestObj;
				// stop the timer if there's one already running
				timer_map_t::iterator timer_itr = request_timer_map.find(client);
				if (timer_itr != request_timer_map.end()) {
					int timer_id = timer_itr->second;
					_D("timer_id[%d]", timer_id);
					conv::util::miscStopTimer(reinterpret_cast<void*> (timer_id));
				}
			}

			// request timer
			gpointer *param = g_new0(gpointer, 2);
			param[0] = reinterpret_cast<void*>(new(std::nothrow) string(client));
			param[1] = reinterpret_cast<void*>(this);

			int timer_id = reinterpret_cast<int>(conv::util::miscStartTimer(timer_worker, timeout, param));
			request_timer_map[ string(client) ] = timer_id;

			requestObj->reply(CONV_ERROR_NONE);
		} else if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_DISCOVERY_STOP) ){
			const char* client = requestObj->get_sender();

			if (count_discovery_request <= 0) {
				_D("discovery is already stopped");
				requestObj->reply(CONV_ERROR_INVALID_OPERATION);
			} else {
				notify_time_up(client);
				requestObj->reply(CONV_ERROR_NONE);
			}

			delete requestObj;
		}

		return CONV_ERROR_NONE;
	}

	return CONV_ERROR_INVALID_OPERATION;;
}

int conv::DiscoveryManager::start_discovery()
{
	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::stop_discovery()
{
	_D("Stop_Discovery...");
	for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		(*it)->stop();
	}
	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::register_provider(IDiscoveryProvider *provider)
{
	if (!provider) {
		_E("Provider NULL");
		return CONV_ERROR_INVALID_PARAMETER;
	}

	if (provider->set_manager(this) != CONV_ERROR_NONE) {
		_E("Provider set_manager failed");
		delete provider;
		return CONV_ERROR_INVALID_OPERATION;
	}

	if (provider->init() != CONV_ERROR_NONE) {
		_E("Provider initialization failed");
		delete provider;
		return CONV_ERROR_INVALID_OPERATION;
	}

	provider_list.push_back(provider);

	return CONV_ERROR_NONE;
}
int conv::DiscoveryManager::convert_device_into_json(conv::IDevice* device_info, json* json_data)
{
	json_data->set(NULL, CONV_JSON_DEVICE_ID, device_info->getId());
	json_data->set(NULL, CONV_JSON_DEVICE_NAME, device_info->getName());
	json_data->set(NULL, CONV_JSON_DEVICE_ADDRESS, device_info->getAddress());

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::convert_service_into_json(conv::IService* service_info, json* json_data)
{
	string service_info_str = service_info->getServiceInfo();
	_D("Service-2-Json Conversion : %s", service_info_str.c_str());
	json service_json(service_info_str);
	json_data->array_append(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_DATA, service_json);
	json_data->array_append(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_TYPE, service_info->getServiceType());

	return CONV_ERROR_NONE;
}

static bool serviceComparision(conv::IService* obj, int serviceType)
{
	if (obj->getServiceType() == serviceType)
		return true;
	else
		return false;
}

int conv::DiscoveryManager::exclude_services(conv::IDevice* org_device, conv::IDevice* removed_device)
{
	int remained_serv_count = 0;
	std::list<IService*> org_serv_list;
	std::list<IService*> removed_serv_list;

	org_device->get_services_list(&org_serv_list);
	_D("[%d] Services in the origin device info[%s]", org_serv_list.size(), org_device->getName().c_str());
	remained_serv_count = org_serv_list.size();

	removed_device->get_services_list(&removed_serv_list);
	_D("[%d] Services in the removed device info[%s]", removed_serv_list.size(), removed_device->getName().c_str());

	std::list<IService*>::iterator removed_itr = removed_serv_list.begin();
	for (; removed_itr != removed_serv_list.end(); ++removed_itr) {
		IService* cur_removed_serv = *removed_itr;
		std::list<IService*>::iterator org_iter = std::find_if(org_serv_list.begin(), org_serv_list.end(), std::bind(serviceComparision, std::placeholders::_1, cur_removed_serv->getServiceType()));
		if (org_iter != org_serv_list.end()) {
			IService* cur_serv = *org_iter;
			_D("Service[%d],notified as removed one, found in device[%s]", cur_serv->getServiceType(), cur_serv->getName().c_str());
			org_device->remove_service(cur_serv);
			remained_serv_count--;
		}
	}

	return remained_serv_count;
}


// return value : the number of new services
int conv::DiscoveryManager::merge_exclude_services(conv::IDevice* org_device, conv::IDevice* new_device)
{
	int new_serv_count = 0;
	std::list<IService*> org_serv_list;
	std::list<IService*> new_serv_list;

	org_device->get_services_list(&org_serv_list);
	_D("[%d] Services in the origin device info[%s]", org_serv_list.size(), org_device->getName().c_str() );
	new_device->get_services_list(&new_serv_list);
	_D("[%d] Services in the new device info[%s]", new_serv_list.size(), new_device->getName().c_str() );

	std::list<IService*>::iterator new_iter = new_serv_list.begin();
	for (; new_iter != new_serv_list.end(); ++new_iter) {
		IService* cur_serv = *new_iter;
		std::list<IService*>::iterator org_iter =
			std::find_if(org_serv_list.begin(), org_serv_list.end(), std::bind(serviceComparision, std::placeholders::_1, cur_serv->getServiceType()));
		if (org_iter != org_serv_list.end()) {
			// already exists in org_device.. means it's not new!.. so remove the service from new!!
			new_device->remove_service(cur_serv);
			_D("Service[%d] has been already found in Device[%s]", cur_serv->getServiceType(), org_device->getName().c_str() );
		} else {
			_D("New Service[%d] found in Device[%s]", cur_serv->getServiceType(), org_device->getName().c_str() );
			// add the service into the original device
			org_device->add_service(cur_serv);
			new_serv_count++;
		}
	}
	return new_serv_count;
}

int conv::DiscoveryManager::notify_lost_device(IDevice* disc_device)
{
	int num_remained_service = 0;
	// 1. find the device and remove the services included in disc_device from cache (discovered_results)
	discovered_ones_map_t::iterator itor_disc;
	itor_disc = discovered_results.find(disc_device->getId());
	if (itor_disc != discovered_results.end()) {
		IDevice* cur_device = itor_disc->second;
		num_remained_service = exclude_services(cur_device, disc_device);
	} else {
		_D("Lost Notify dismissed - No discovered results corresponding to id[%s]", disc_device->getId().c_str());
		return CONV_ERROR_NO_DATA;
	}

	// 2. if no services is left included in disc_device, then remove the device from the cache and notify to a client
	if (num_remained_service == 0) {
		// remove from the cache
		discovered_results.erase(disc_device->getId());

		// iterate through request_map for service
		_D("Iterate through request_map to publish..");
		request_map_t::iterator IterPos;
		json device_json;
		convert_device_into_json(disc_device, &device_json);
		for (IterPos = request_map.begin(); IterPos != request_map.end(); ++IterPos) {
			request* cur_Req = IterPos->second;
			cur_Req->publish(CONV_DISCOVERY_DEVICE_LOST, device_json);
		}
	}

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::append_discovered_result(conv::IDevice* disc_device)
{
	conv::IDevice* publish_device_info = NULL;

	_D("Append Discovered Result.. Device:%x, Service:%x");
	IF_FAIL_RETURN_TAG((disc_device != NULL), CONV_ERROR_INVALID_PARAMETER, _E, "IDevice not initialized..");
	// discovery_manager deals with the cache for discovered ones
	_D("Check if key[%s] exists in discovered_results", disc_device->getId().c_str());
	discovered_ones_map_t::iterator itor_disc;
	itor_disc = discovered_results.find(disc_device->getId());
	if (itor_disc != discovered_results.end()) {
		_D("update discovered device's info [%s]", disc_device->getId().c_str());
		IDevice* cur_device = itor_disc->second;

		int count_new_services = merge_exclude_services(cur_device, disc_device);
		if (count_new_services == 0)
			return CONV_ERROR_NONE;
	} else {
		_D("newbie!! discovered device's info [%s]", disc_device->getId().c_str());
		discovered_results.insert(discovered_ones_map_t::value_type(disc_device->getId(), disc_device));
	}
	publish_device_info = disc_device;

	_D("Convert device_info into json type..");
	json device_json;
	convert_device_into_json(publish_device_info, &device_json);

	_D("Convert service info into json type..");
	typedef std::list<IService*> serv_list_t;
	serv_list_t serv_list;
	publish_device_info->get_services_list(&serv_list);

	for (serv_list_t::iterator iterPos = serv_list.begin(); iterPos != serv_list.end(); ++iterPos) {
		IService* cur_serv = *iterPos;
		convert_service_into_json(cur_serv, &device_json);
	}

	// iterate through request_map for service
	_D("Iterate through request_map to publish..");
	int index = 0;
	request_map_t::iterator IterPos;
	for (IterPos = request_map.begin(); IterPos != request_map.end(); ++IterPos) {
		request* cur_Req = IterPos->second;
		cur_Req->publish(CONV_ERROR_NONE, device_json);

		index++;
	}

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::set_discovery_filter(request* request_obj)
{
	json desc_data = request_obj->get_description();
	json filter_data;
	desc_data.get(NULL, "filter", &filter_data);
	discovery_filter_map.insert(filter_map_t::value_type(request_obj->get_id(), filter_data));

	return CONV_ERROR_NONE;
}

int conv::DiscoveryManager::isvalid_discovery_on_filter(json& filter_json)
{
	return CONV_ERROR_NONE;
}

int conv::discovery_manager::handleRequest(request* requestObj)
{
	IF_FAIL_RETURN_TAG(_instance, CONV_ERROR_INVALID_PARAMETER, _E, "Not initialized");
	_instance->handleRequest(requestObj);

	return CONV_ERROR_NONE;
}
