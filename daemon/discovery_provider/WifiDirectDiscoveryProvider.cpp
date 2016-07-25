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

#include "WifiDirectDiscoveryProvider.h"
#include "../DiscoveryManager.h"

#include <net_connection.h>
#include <wifi-direct.h>

#define TEMP_TEST

#define MACSTR_LEN 18
#define MAX_PEER_NUM 20

using namespace std;

conv::WifiDirectDiscoveryProvider::WifiDirectDiscoveryProvider()
{
}

conv::WifiDirectDiscoveryProvider::~WifiDirectDiscoveryProvider()
{
}

void _cb_activation(int error_code, wifi_direct_device_state_e device_state, void *user_data)
{
	switch (device_state) {
	case WIFI_DIRECT_DEVICE_STATE_ACTIVATED:
		_D("event -WIFI_DIRECT_DEVICE_STATE_ACTIVATED");
		break;

	case WIFI_DIRECT_DEVICE_STATE_DEACTIVATED:
		_D("event - WIFI_DIRECT_DEVICE_STATE_DEACTIVATED");
		break;

	default:
		break;
	}
}

bool _cb_discovered_peers_impl(wifi_direct_discovered_peer_info_s* peer, void* user_data)
{
#if 0//def TEMP_TEST
	static int first = 1;
	if (!strcmp(peer->device_name, "D2d note" ) ) {
		first = 0;
		int	result = wifi_direct_connect(peer->mac_address);
		_D("connect result = %d", result);
	}
#endif

#if 0 // TODO: make and notice if it's device support d2d
	conv::WifiDirectDiscoveryProvider* disc_provider = (conv::WifiDirectDiscoveryProvider*)user_data;
	conv::service *conv_service = new(std::nothrow) conv::service;

	conv_service->setName(peer->device_name);
	conv_service->setVersion("0.0");
	conv_service->setType("");
	conv_service->setId(peer->mac_address);
	conv_service->setUri("");

	disc_provider->notice_discovered(conv_service);
#endif

	if (NULL != peer) {
		_D("device_name : %s", peer->device_name);
	}

	return true; /* continue with the next iteration of the loop */
}

void _cb_discover(int error_code, wifi_direct_discovery_state_e discovery_state, void *user_data)
{
	switch (discovery_state) {
	case WIFI_DIRECT_DISCOVERY_STARTED:
		_D("_cb_discover - WIFI_DIRECT_DISCOVERY_STARTED");
		break;

	case WIFI_DIRECT_ONLY_LISTEN_STARTED:
		_D("_cb_discover - WIFI_DIRECT_ONLY_LISTEN_STARTED");
		break;

	case WIFI_DIRECT_DISCOVERY_FINISHED:
		wifi_direct_foreach_discovered_peers(_cb_discovered_peers_impl, user_data);
		_D("_cb_discover - WIFI_DIRECT_DISCOVERY_FINISHED");
		break;

	case WIFI_DIRECT_DISCOVERY_FOUND:
	{
		_D("_cb_discover - WIFI_DIRECT_DISCOVERY_FOUND");
	}
	break;

	default:
	break;
	}
}

bool _cb_connected_peers_impl(wifi_direct_connected_peer_info_s* peer, void* user_data)
{
	_D("_cb_connected_peers_impl - %s", peer->ip_address);

	return true; /* continue with the next iteration of the loop */
}

void _cb_connection(int error_code, wifi_direct_connection_state_e connection_state, const char* mac_address, void *user_data)
{
	char *ip_addr = NULL;
	bool owner;

	switch (connection_state) {
	case WIFI_DIRECT_CONNECTION_IN_PROGRESS:
	{
		_D("event - WIFI_DIRECT_CONNECTION_IN_PROGRESS");

		if (error_code == WIFI_DIRECT_ERROR_NONE) {
			char _peer_mac[MACSTR_LEN+1]={0, };

			if (NULL != mac_address)
				g_strlcpy(_peer_mac, mac_address, sizeof(_peer_mac));

			_D("Connection start with [%s] ", _peer_mac);
		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_RSP:
	{
		_D("event - WIFI_DIRECT_CONNECTION_RSP");

#ifdef TEMP_TEST
		wifi_direct_foreach_connected_peers(_cb_connected_peers_impl, NULL);
#endif

		if (error_code == WIFI_DIRECT_ERROR_NONE) {
			char incomming_peer_mac[MACSTR_LEN+1]={0, };
			char status[100] = {0, };

			if (NULL != mac_address)
				g_strlcpy(incomming_peer_mac, mac_address, sizeof(incomming_peer_mac));

			_D("Connection response with [%s] ", incomming_peer_mac);

			wifi_direct_is_group_owner(&owner);
			if (owner) {
				wifi_direct_get_ip_address(&ip_addr);
				if (NULL != ip_addr) {
					snprintf(status, sizeof(status), "<color=#FFFFFF>GO - IP : %s", ip_addr);
					free(ip_addr);
				}

			} else {
				wifi_direct_get_ip_address(&ip_addr);
				if (NULL != ip_addr) {
					snprintf(status, sizeof(status), "<color=#FFFFFF>STA - IP : %s", ip_addr);
					free(ip_addr);
				}
			}
		} else {
			_E("Error : wifi direct (Error Code:%d)", error_code);
		}
	}
	break;

	case WIFI_DIRECT_CONNECTION_WPS_REQ:
	{
		_D("event - WIFI_DIRECT_CONNECTION_WPS_REQ");
	}
	break;

	case WIFI_DIRECT_CONNECTION_REQ:
	{
		_D("event - WIFI_DIRECT_CONNECTION_REQ");
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_IND:
	{
		_D("event - WIFI_DIRECT_DISCONNECTION_IND");

		if (error_code == WIFI_DIRECT_ERROR_NONE) {
			char incomming_peer_mac[MACSTR_LEN+1]={0, };

			if (NULL != mac_address)
				g_strlcpy(incomming_peer_mac, mac_address, sizeof(incomming_peer_mac));

			_D("Disconnection IND from [%s] ", incomming_peer_mac);
		}
	}
	break;

	case WIFI_DIRECT_DISCONNECTION_RSP:
	{
		_D("event - WIFI_DIRECT_DISCONNECTION_RSP");

		if (error_code == WIFI_DIRECT_ERROR_NONE) {
			char incomming_peer_mac[MACSTR_LEN+1]={0, };

			if (NULL != mac_address)
				g_strlcpy(incomming_peer_mac, mac_address, sizeof(incomming_peer_mac));

			_D("Disconnection RSP with [%s] ", incomming_peer_mac);
		}
	}
	break;

	case WIFI_DIRECT_DISASSOCIATION_IND:
	{
		_D("event - WIFI_DIRECT_DISASSOCIATION_IND");

		if (error_code == WIFI_DIRECT_ERROR_NONE) {
			char incomming_peer_mac[MACSTR_LEN+1]={0, };

			if (NULL != mac_address)
				g_strlcpy(incomming_peer_mac, mac_address, sizeof(incomming_peer_mac));

			_D("Disassociation IND from [%s] ", incomming_peer_mac);
		}
	}
	break;

	case WIFI_DIRECT_GROUP_CREATED:
	{
		_D("event - WIFI_DIRECT_GROUP_CREATED");
	}
	break;

	case WIFI_DIRECT_GROUP_DESTROYED:
	{
		_D("event - WIFI_DIRECT_GROUP_DESTROYED");
	}
	break;

	default:
		_D("event - %d", connection_state);
		break;
	}
}

int init_wfd_client(void* disc_provider)
{
	int ret;

	ret = wifi_direct_initialize();
	_D("wifi_direct_initialize() result=[%d]", ret);

	ret = wifi_direct_set_device_state_changed_cb(_cb_activation, (void*)NULL);
	_D("wifi_direct_set_device_state_changed_cb() result=[%d]", ret);

	ret = wifi_direct_set_discovery_state_changed_cb(_cb_discover, disc_provider);
	_D("wifi_direct_set_discovery_state_changed_cb() result=[%d]", ret);

	ret = wifi_direct_set_connection_state_changed_cb(_cb_connection, (void*)NULL);
	_D("wifi_direct_set_connection_state_changed_cb() result=[%d]", ret);

#if defined(TIZEN_TV) && defined(TIZEN_TV_PRODUCT) && defined(ENABLE_EXTRA_INFO)
	ret = wifi_direct_set_peer_info_connection_state_changed_cb(_cb_peer_info_connection, (void*)NULL);
	_D("wifi_direct_set_peer_info_connection_state_changed_cb() result=[%d]", ret);
#endif /* defined(TIZEN_TV) && defined(TIZEN_TV_PRODUCT) && defined(ENABLE_EXTRA_INFO) */

	return ret;
}

int start_wfd_discovery(void)
{
	int result;
	connection_h connection;
	connection_wifi_state_e wifi_state;

	IF_FAIL_RETURN_TAG(connection_create(&connection) == CONNECTION_ERROR_NONE, CONV_ERROR_NOT_SUPPORTED, _E, "connection error");
	result = connection_get_wifi_state(connection, &wifi_state);
	if ((result != CONNECTION_ERROR_NONE) || (wifi_state == CONNECTION_WIFI_STATE_DEACTIVATED)) {
		_E("ERROR : wifi is not on!");
		connection_destroy(connection);
		return CONV_ERROR_NOT_SUPPORTED;
	}
	connection_destroy(connection);

	result = wifi_direct_activate();
	_D("wifi_direct_activate() result=[%d]", result);

	if (result == WIFI_DIRECT_ERROR_WIFI_USED) {
		_E("ERROR : WIFI_DIRECT_ERROR_WIFI_USED ");
	} else if (result == WIFI_DIRECT_ERROR_MOBILE_AP_USED) {
		_E("ERROR : WIFI_DIRECT_ERROR_MOBILE_AP_USED ");
	}

	result = wifi_direct_start_discovery(FALSE, 5);
	_D("wifi_direct_start_discovery() listen_only=[false] result=[%d]", result);

	return result;
}
int conv::WifiDirectDiscoveryProvider::init()
{
	//init_wfd_client((void*)this);
	_D("WifiDirectDiscoveryProvider init done");

	return CONV_ERROR_NONE;
}

int conv::WifiDirectDiscoveryProvider::release()
{
	return CONV_ERROR_NONE;
}

int conv::WifiDirectDiscoveryProvider::start()
{
	//return start_wfd_discovery();
	return CONV_ERROR_NONE;
}

int conv::WifiDirectDiscoveryProvider::stop()
{
	return CONV_ERROR_NONE;
}

int conv::WifiDirectDiscoveryProvider::checkExistence(conv::service* conv_service)
{
	// print conv_service Info..
	_D("Check Existence : %s", conv_service->getName().c_str());
	conv_service->printInfo();

	// insert into cache
	string cache_key = conv_service->getId();	// Serivce URI as Map Key..
	if (cache.find(cache_key) == cache.end()) {
		_D("conv_service with key[%s] does not exist..so go into the cache", cache_key.c_str());
		cache.insert(map<string, conv::service*>::value_type(cache_key, conv_service));
		return CONV_ERROR_NONE;
	} else {
		_D("conv_service with key[%s] already exists..", cache_key.c_str());
		return 1;
	}
}

int conv::WifiDirectDiscoveryProvider::notice_discovered(conv::service* conv_service)
{
	_D("Notice Discovered called with service[%x]", conv_service);

	// Double check if the noticed one already got delivered to..
	int alreadyExisted = checkExistence(conv_service);
	_D("double check .. existence[%s]", (alreadyExisted == 0)? "No" : "Yes");

	if (!alreadyExisted) {
		//the discovered one is NEW!!
		// TO-DO : need to re-write this code
		//_discovery_manager->append_discovered_result(NULL, conv_service);
	}

	return CONV_ERROR_NONE;
}

