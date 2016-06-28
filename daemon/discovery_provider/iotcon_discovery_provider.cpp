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

#include "iotcon_discovery_provider.h"

#include <iotcon.h>
#include <map>
#include <algorithm>
#include "iotcon/resource_handle.h"
#include "../conv_json.h"

#include "../discovery_mgr_impl.h"
#include "iotcon/service_adapter.h"
#include "iotcon/device_adapter.h"

#include "../util.h"

using namespace std;

typedef std::map<std::string, conv::resource_handle> resource_h_map_t;
static resource_h_map_t* resource_h_map = NULL;

typedef std::list <string> discovery_complete_list_t;
typedef std::map <string, conv::resource_handle> discovery_process_map_t;
static discovery_complete_list_t	discovery_complete_list;
static discovery_process_map_t		discovery_process_map;

conv::iotcon_discovery_provider::iotcon_discovery_provider()
{
}

conv::iotcon_discovery_provider::~iotcon_discovery_provider()
{
}

int conv::iotcon_discovery_provider::init()
{
	int ret = iotcon_initialize(CONV_IOTCON_FILEPATH);
	if (ret != IOTCON_ERROR_NONE) {
		_E("Failed iotcon_connect... Error:%d", ret);
		return CONV_ERROR_INVALID_OPERATION;
	}

	if (resource_h_map != NULL)
		delete resource_h_map;
	resource_h_map = new(std::nothrow) resource_h_map_t;

	_D("iotcon_discovery_provider init done");

	return CONV_ERROR_NONE;
}

int conv::iotcon_discovery_provider::release()
{
	iotcon_deinitialize();

	if (resource_h_map != NULL)
		delete resource_h_map;

	return CONV_ERROR_NONE;
}

static bool _get_resource_foreach_types(const char* type, void* user_data)
{
	_D("resource type : %s", type);

	list<string>* list_types = (list<string>*) user_data;
	list_types->push_back(string(type));

	return true;
}

static bool response_attributes_cb(iotcon_attributes_h attributes, const char* key, void* user_data)
{
	_D("response_attributes_cb with the key[%s]", key);
	return true;
}

int conv::iotcon_discovery_provider::notice_discovered(service_iface* service)
{
	return CONV_ERROR_NONE;
}

void conv::iotcon_discovery_provider::_on_response_get(iotcon_remote_resource_h resource,
									iotcon_response_h response, void* user_data)
{
	_D("On Response Get..");

	int ret;
	iotcon_response_result_e	response_result;
	iotcon_representation_h		recv_repr;
	iotcon_attributes_h				recv_attributes;

	unsigned int key_count;

	// uri_path , host_address
	char *resource_uri_path, *resource_host;
	iotcon_remote_resource_get_uri_path(resource, &resource_uri_path);
	iotcon_remote_resource_get_host_address(resource, &resource_host);

	// Check if it's already done publishing..
	string discoveryKey = generateDiscoveryKey(resource_uri_path, resource_host);
	discovery_complete_list_t::iterator itor;
	_D("Check if key[%s] already exists in complete_list", discoveryKey.c_str());
	itor = find(discovery_complete_list.begin(), discovery_complete_list.end(), discoveryKey);
	if (itor != discovery_complete_list.end() )
		return;	// Already done pushing upwards..

	// Retreive info from process_map..
	resource_handle cur_resource_h;
	discovery_process_map_t::iterator itor_process;
	itor_process = discovery_process_map.find(discoveryKey);
	_D("Check if key[%s] exists in process_map", discoveryKey.c_str());
	if (itor_process != discovery_process_map.end()) {
		cur_resource_h = itor_process->second;
	} else {
		_D("Not supposed to happen");
		return;
	}


	//============= Dealing with Response ==============//

	ret = iotcon_response_get_result(response, &response_result);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "Failed[%d] to get response result", ret);
	IF_FAIL_VOID_TAG((response_result == IOTCON_RESPONSE_OK), _E,
			"_on_response_get Response Error[%d]", response_result);

	ret = iotcon_response_get_representation(response, &recv_repr);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_response_get_representation() Fail[%d]", ret);

	ret = iotcon_representation_get_attributes(recv_repr, &recv_attributes);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_representation_get_attributes() Fail[%d]", ret);

	ret = iotcon_attributes_get_keys_count(recv_attributes, &key_count);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_attributes_get_keys_count() Fail[%d]", ret);
	_D("attributes key count : %d", key_count);

	ret = iotcon_attributes_foreach(recv_attributes, response_attributes_cb, NULL);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_attributes_for_each() Fail[%d]", ret);

	char *device_id = NULL, *device_name = NULL, *device_type = NULL, *version = NULL, *service_list = NULL;
	ret = iotcon_attributes_get_str(recv_attributes, "device_id", &device_id);		// device id (ex. bt address)
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_attributes_get_str() Fail[%d]", ret);
	cur_resource_h.set_device_id(string(device_id));

	if ( conv::util::get_device_id().compare(device_id) == 0 ) {
		_D("the device has found itself..[device_id:%s].. out!", device_id);
		return;
	}

	ret = iotcon_attributes_get_str(recv_attributes, "device_name", &device_name);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_attributes_get_str() Fail[%d]", ret);
	cur_resource_h.set_device_name(string(device_name));

	ret = iotcon_attributes_get_str(recv_attributes, "device_type", &device_type);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "iotcon_attributes_get_str() Fail[%d]", ret);
	cur_resource_h.set_device_type(string(device_type));

	char* service_json_char = NULL;
	ret = iotcon_attributes_get_str(recv_attributes, "service_json", &service_json_char);
	if (service_json_char == NULL) {
		_D("service_json does not exist...");
		service_json_char = const_cast<char*>("");
	}
	std::string service_json(service_json_char);

	cur_resource_h.set_services_list(service_json);
	json service_list_json(service_json);
	_D("on_response : device_id[%s] device_name[%s] device_type[%s] version[%s] service_list[%s]"
									, device_id, device_name, device_type, version, service_list);

	if (_discovery_manager != NULL) {
		device_adapter*	device = new(std::nothrow) device_adapter (cur_resource_h);

		int num_service = service_list_json.array_get_size(NULL, "service_list");
		for (int index = 0; index < num_service; index++) {
			int serv_type_int;

			json cur_service_json;
			bool ret = service_list_json.get_array_elem(NULL, "service_list", index, &cur_service_json);
			if (ret != true) {
				_D("json get array elem with service_list error..");
				continue;
			}

			service_adapter* serv = new(std::nothrow) service_adapter(cur_resource_h);
			json serv_info_json;

			cur_service_json.get(NULL, CONV_JSON_DISCOVERY_SERVICE_TYPE, &serv_type_int);
			serv->setServiceType(serv_type_int);
			cur_service_json.get(NULL, CONV_JSON_DISCOVERY_SERVICE_INFO, &serv_info_json);
			serv->setServiceInfo(serv_info_json.str());

			device->add_service(serv);
		}
		_discovery_manager->append_discovered_result(device);
	}

	discovery_complete_list.push_back(discoveryKey);
}

bool conv::iotcon_discovery_provider::_get_str_list_cb(int pos, const char* value, void* user_data)
{
	json* service_list = (json*) user_data;
	service_list->array_append(NULL, "services_list", string(value));
	return IOTCON_FUNC_CONTINUE;
}

void conv::iotcon_discovery_provider::on_received_detail_info(iotcon_remote_resource_h resource, iotcon_error_e error,
									iotcon_request_type_e request_type, iotcon_response_h response,
									void* user_data)
{
	_D("On Received Detail Info.. with type[%d]", request_type);

	switch (request_type) {
	case IOTCON_REQUEST_GET :
		_on_response_get(resource, response, NULL);
		break;
	case IOTCON_REQUEST_PUT:
	case IOTCON_REQUEST_POST:
	case IOTCON_REQUEST_DELETE:
		_D("Not supported request_type");
		break;
	default:
		_E("Invalid Request Type");
		return;
	}
}

void conv::iotcon_discovery_provider::_get_detail_info(iotcon_remote_resource_h resource)
{
	iotcon_query_h query;
	iotcon_remote_resource_h resource_clone = NULL;

	_D("_get_detail_info called..");

	int ret = iotcon_remote_resource_clone(resource, &resource_clone);
	if (ret != IOTCON_ERROR_NONE) {
		_E("iotcon_remote_resource_clone() Failed(%d)", ret);
	}

	ret = iotcon_query_create(&query);
	IF_FAIL_VOID_TAG((ret == IOTCON_ERROR_NONE), _E, "Failed[%d] to create Query", ret);

	ret = iotcon_query_add(query, "query_key", "query_value");
	if (ret != IOTCON_ERROR_NONE) {
		_E("Failed[%d] to add query", ret);
		iotcon_query_destroy(query);
		return;
	}

	/* send Get Request */
	ret = iotcon_remote_resource_get(resource_clone, query, on_received_detail_info, NULL);
	if (ret != IOTCON_ERROR_NONE) {
		_E("Failed[%d] to get remote resource query", ret);
		iotcon_query_destroy(query);
		return;
	}
}

string conv::iotcon_discovery_provider::generateDiscoveryKey(char* uri_path, char* host_address)
{
	return string(host_address) + string(uri_path);
}

int conv::iotcon_discovery_provider::add_iot_resource(iotcon_remote_resource_h resource)
{
	_D("add_iot_resource called..");
	resource_handle	iot_resource_h;

	char* resource_uri_path = NULL;
	char* resource_host = NULL;
	iotcon_resource_types_h resource_types = NULL;
	string resource_type;
	iotcon_resource_interfaces_h resource_interfaces;

	// uri_path , host_address
	iotcon_remote_resource_get_uri_path(resource, &resource_uri_path);
	iotcon_remote_resource_get_host_address(resource, &resource_host);

	_D("found Resource : uri[%s], host_address[%s]", resource_uri_path, resource_host);

	// resource_types
	iotcon_remote_resource_get_types(resource, &resource_types);

	// interface - IOTCON_INTERFACE_ DEFAULT / LINK / BATCH / GROUP
	iotcon_remote_resource_get_interfaces(resource, &resource_interfaces);

	resource_handle res_handle;
	// uri_path
	res_handle.set_uri_path(string(resource_uri_path));
	// host_address
	res_handle.set_host_address(string(resource_host));
	// resource types
	iotcon_resource_types_foreach(resource_types, _get_resource_foreach_types, (void*)(&res_handle.get_types()) );

	// Add resource handle into Temp Cache
	string discovery_key = generateDiscoveryKey(resource_uri_path, resource_host);
	_D("discovery_process_map insertion with key[%s]", discovery_key.c_str());
	discovery_process_map.insert(discovery_process_map_t::value_type(discovery_key, res_handle));

	return CONV_ERROR_NONE;
}

bool conv::iotcon_discovery_provider::_found_resource(iotcon_remote_resource_h resource, iotcon_error_e result, void *user_data)
{
	_D("_found_resource called..");

	IF_FAIL_RETURN_TAG((resource != NULL), IOTCON_FUNC_STOP, _E, "resource is NULL..");

	int ret;
	// add the found resource into Temp cache
	ret = add_iot_resource(resource);
	IF_FAIL_RETURN_TAG((ret == CONV_ERROR_NONE), IOTCON_FUNC_CONTINUE, _E, "Failed to add iot resource");

	// Get additional info from resource
	_get_detail_info(resource);

	return IOTCON_FUNC_CONTINUE;
}

int conv::iotcon_discovery_provider::start()
{
	int ret;
	_D("iotcon discovery provider start..");
	discovery_complete_list.clear();
	discovery_process_map.clear();

	ret = iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, IOTCON_CONNECTIVITY_IPV4,
			CONV_RESOURCE_TYPE_TIZEN_D2D_SERVICE, false, _found_resource, NULL);

	IF_FAIL_RETURN_TAG((ret == 0), CONV_ERROR_INVALID_OPERATION, _E, "failed on iotcon_find_resource[%d]", ret);

	return CONV_ERROR_NONE;
}

int conv::iotcon_discovery_provider::stop()
{
	return CONV_ERROR_NONE;
}

