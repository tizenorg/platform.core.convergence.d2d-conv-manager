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

#include "discovery_mgr_impl.h"
#include "discovery_provider/smartview_discovery_provider.h"
#include "discovery_provider/ble_discovery_provider.h"
#include "discovery_provider/iotcon_discovery_provider.h"
#include "access_control/privilege.h"

#include "common.h"
#include "util.h"

static conv::discovery_manager_impl *_instance = NULL;
typedef std::map<std::string, conv::device_iface*> discovered_ones_map_t;
static discovered_ones_map_t discovered_results;

using namespace std;

conv::discovery_manager_impl::discovery_manager_impl()
{
	count_discovery_request = 0;
}

conv::discovery_manager_impl::~discovery_manager_impl()
{
}

int conv::discovery_manager_impl::init()
{
	_D("Discovery_Manager Init!!..");
	register_provider(new conv::smartview_discovery_provider());
	register_provider(new conv::ble_discovery_provider());
	register_provider(new conv::iotcon_discovery_provider());

	request_map.clear();
	request_timer_map.clear();


	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::release()
{
	for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		(*it)->stop();
	}

	for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		delete *it;
	}

	provider_list.clear();

	return CONV_ERROR_NONE;
}

void conv::discovery_manager::set_instance(conv::discovery_manager_impl* mgr)
{
	_instance = mgr;
}

int conv::discovery_manager_impl::notify_time_up(std::string client)
{
	// 1. When no client is using discovery, it should be stopped
	_D("notify_time_up.. with current discovery count :%d", count_discovery_request);
	if (--count_discovery_request == 0)
	{
		stop_discovery();
	}

	// 2. Reqeust to stop timer related to client in the timer_map
	timer_map_t::iterator timer_itr = request_timer_map.find(client);
	if (timer_itr != request_timer_map.end())
	{
		int timer_id = timer_itr->second;
		_D("timer_id[%d]", timer_id);
		conv::util::misc_stop_timer(reinterpret_cast<void*> (timer_id));
	}

	// 3. Notify the client that the requested discovery has been finished
	request_map_t::iterator request_itr = request_map.find(client);
	if (request_itr != request_map.end())
	{
		json no_data;
		request* cur_Req = request_itr->second;
		cur_Req->publish(CONV_DISCOVERY_FINISHED, no_data);
	}

	return CONV_ERROR_NONE;
}

void conv::discovery_manager_impl::timer_worker(void* data)
{
	gpointer* param = reinterpret_cast<gpointer*> (data);
	std::string* req_client = reinterpret_cast<std::string*> (param[0]);
	conv::discovery_manager_impl* cur_disc_mgr = reinterpret_cast<conv::discovery_manager_impl*> (param[1]);

	_D("Timer_Worker.. req_client[%s] discovery_manager[%x]", (*req_client).c_str(), cur_disc_mgr);

	cur_disc_mgr->notify_time_up(*req_client);
}

int conv::discovery_manager_impl::checkBoundaryForTimeout(int givenTimeout)
{
	if ( givenTimeout < CONV_DISCOVERY_MIN_VALUE )
		return CONV_DISCOVERY_MIN_VALUE;
	else if ( givenTimeout > CONV_DISCOVERY_MAX_VALUE )
		return CONV_DISCOVERY_MAX_VALUE;
	else
		return givenTimeout;
}

int conv::discovery_manager_impl::handle_request(request* request_obj)
{
	_D("handle_request called .. request:%x _instance:%x", request_obj, _instance);
	if ( _instance )
	{
		if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_DISCOVERY_START) )
		{
			if ( !conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
					!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) )
			{
				_E("permission denied");
				request_obj->reply(CONV_ERROR_PERMISSION_DENIED);
				delete request_obj;
				return CONV_ERROR_PERMISSION_DENIED;
			}

			discovered_results.clear();
			count_discovery_request++;

			for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it)
			{
				// Discovery Provider Starts!!!!
				(*it)->start();
			}

			const char* client = request_obj->get_sender();
			if (client == NULL)
			{
				_D("client is empty..");
				return CONV_ERROR_INVALID_OPERATION;
			}
			_D("request_obj info .. client[%s]", client);

			int timeout = 0;
			json description = request_obj->get_description();
			description.get(NULL, "timeout", &timeout);
			timeout = checkBoundaryForTimeout(timeout);

			request_map_t::iterator map_itr = request_map.find(string(client));

			if ( map_itr == request_map.end())
			{
				// current request inserted into request_map..
				request_map.insert(request_map_t::value_type(string(client), request_obj));
				_D("client[%s] inserted into request_map", client);
			}
			else
			{
				_D("client[%s] already in request_map.. Replace!!!", client);
				map_itr->second = request_obj;
			}

			// request timer
			gpointer *param = g_new0(gpointer, 2);
			param[0] = reinterpret_cast<void*>(new string(client));
			param[1] = reinterpret_cast<void*>(this);

			int timer_id = reinterpret_cast<int>(conv::util::misc_start_timer(timer_worker, timeout, param));
			request_timer_map[ string(client) ] = timer_id;

			request_obj->reply(CONV_ERROR_NONE);

			//delete request_obj;
		} else if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_DISCOVERY_STOP) ){
			stop_discovery();

			request_obj->reply(CONV_ERROR_NONE);
			//delete request_obj;
		}

		return CONV_ERROR_NONE;
	}

	return CONV_ERROR_INVALID_OPERATION;;
}

int conv::discovery_manager_impl::start_discovery()
{
	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::stop_discovery()
{
	_D("Stop_Discovery...");
	for (discovery_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		(*it)->stop();
	}
	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::register_provider(discovery_provider_base *provider)
{
	if (!provider) {
		_E("Provider NULL");
		throw static_cast<int>(CONV_ERROR_INVALID_PARAMETER);
	}

	provider_list.push_back(provider);

	if (provider->init() != CONV_ERROR_NONE) {
		_E("Provider initialization failed");
		throw CONV_ERROR_INVALID_OPERATION;
	}

	if (provider->set_manager(this) != CONV_ERROR_NONE) {
		_E("Provider set_manager failed");
		throw CONV_ERROR_INVALID_OPERATION;
	}
	return CONV_ERROR_NONE;
}
int conv::discovery_manager_impl::convert_device_into_json(conv::device_iface* device_info, json* json_data)
{
	json_data->set(NULL, CONV_JSON_DEVICE_ID, device_info->getId());
	json_data->set(NULL, CONV_JSON_DEVICE_NAME, device_info->getName());
	json_data->set(NULL, CONV_JSON_DEVICE_ADDRESS, device_info->getAddress());

	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::convert_service_into_json(conv::service_iface* service_info, json* json_data)
{
	string service_info_str = service_info->getServiceInfo();
	_D("Service-2-Json Conversion : %s", service_info_str.c_str());
	json service_json(service_info_str);
	json_data->array_append(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_DATA, service_json);
	json_data->array_append(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_TYPE, service_info->getServiceType());

	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::append_discovered_result(conv::device_iface* disc_device, conv::service_iface* disc_service)
{
	_D("Append Discovered Result.. Device:%x, Service:%x");
	IF_FAIL_RETURN_TAG((disc_device != NULL), CONV_ERROR_INVALID_PARAMETER, _E, "device_iface not initialized..");
	// discovery_manager deals with the cache for discovered ones
	_D("Check if key[%s] exists in discovered_results", disc_device->getId().c_str());
	discovered_ones_map_t::iterator itor_disc;
	itor_disc = discovered_results.find(disc_device->getId());
	if (itor_disc != discovered_results.end())
	{
		_D("update discovered device's info [%s]", disc_device->getId().c_str());
		device_iface* cur_device = itor_disc->second;
		cur_device->add_service(disc_service);
	}
	else
	{
		_D("newbie!! discovered device's info [%s]", disc_device->getId().c_str());
		discovered_results.insert(discovered_ones_map_t::value_type(disc_device->getId(), disc_device));
	}

	_D("Convert device_info into json type..");
	json device_json;
	convert_device_into_json(disc_device, &device_json);

	_D("Convert service info into json type..");
	if (disc_service !=NULL)
		convert_service_into_json(disc_service, &device_json);
	else
	{
		typedef std::list<service_iface*> serv_list_t;
		serv_list_t serv_list;
		disc_device->get_services_list(&serv_list);

		for (serv_list_t::iterator iterPos = serv_list.begin(); iterPos != serv_list.end(); ++iterPos)
		{
			service_iface* cur_serv = *iterPos;
			convert_service_into_json(cur_serv, &device_json);
		}
	}

	// 1.set data info about device

	// 2.set data info about services included in 'device'
	// iterate through request_map for service
	_D("Iterate through request_map to publish..");
	int index = 0;
	request_map_t::iterator IterPos;
	for (IterPos = request_map.begin(); IterPos != request_map.end(); ++IterPos)
	{
		request* cur_Req = IterPos->second;
		cur_Req->publish(CONV_ERROR_NONE, device_json);

		index++;
	}

	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::set_discovery_filter(request* request_obj)
{
	json desc_data = request_obj->get_description();
	json filter_data;
	desc_data.get(NULL, "filter", &filter_data);
	discovery_filter_map.insert(filter_map_t::value_type(request_obj->get_id(), filter_data));

	return CONV_ERROR_NONE;
}

int conv::discovery_manager_impl::isvalid_discovery_on_filter(json& filter_json)
{
	return CONV_ERROR_NONE;
}

int conv::discovery_manager::handle_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_instance, CONV_ERROR_INVALID_PARAMETER, _E, "Not initialized");
	_instance->handle_request(request_obj);

	return CONV_ERROR_NONE;
}
