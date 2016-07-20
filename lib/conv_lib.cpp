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

#include <stdlib.h>
#include <gio/gio.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <cstring>
#include <vconf.h>

#include "common.h"
#include "d2d_conv_service.h"
#include "conv_lib_json.h"
#include "internal_types.h"
#include "dbus_client.h"
#include "conv_lib_util.h"

struct device_callback_info_s {
	conv_discovery_cb cb;
	void* user_data;
	conv_h handle;
};

typedef std::map<int, device_callback_info_s*> callback_map_t;
static callback_map_t callback_map;

//LCOV_EXCL_START
static void conv_subject_cb(const char* subject, int req_id, int error, json data)
{
	_D("Callback response %d : subject[%s] json_data[%s]", req_id, subject, data.str().c_str());

	conv_device_h device = new(std::nothrow) _conv_device_handle_s();
	IF_FAIL_VOID_TAG(device, _E, "Memory Allocation Failed");

	device->jbody = data;

	callback_map_t::iterator itor = callback_map.find(req_id);
	if (itor == callback_map.end()) {
		_D("No Callback found for the response[%d]", req_id);
		return;
	}
	device_callback_info_s* callback_info = itor->second;
	callback_info->cb(device, (conv_discovery_result_e)error, callback_info->user_data);
}

static void register_subject_callbacks()
{
	conv::dbus_client::register_callback(CONV_SUBJECT_DISCOVERY_START, conv_subject_cb);

	_I("Done with registering subject callback");
}
//LCOV_EXCL_STOP

EXTAPI int conv_create(conv_h* handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	*handle = new(std::nothrow) _conv_handle();
	ASSERT_ALLOC(*handle);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_destroy(conv_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	_D("conv_destroy..");
	conv::dbus_client::release();
	std::list<int>::iterator itr_end = handle->request_ids.end();
	callback_map_t::iterator map_iter_end = callback_map.end();
	for (std::list<int>::iterator iter_pos = handle->request_ids.begin(); iter_pos != itr_end; iter_pos++) {
		callback_map_t::iterator map_iter = callback_map.find(*iter_pos);
		if (map_iter != map_iter_end)
			callback_map.erase(map_iter);
	}
	delete handle;
	return CONV_ERROR_NONE;
}

EXTAPI int conv_discovery_start(conv_h handle, const int timeout_seconds, conv_discovery_cb callback, void* user_data)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(callback);

	register_subject_callbacks();

	device_callback_info_s *cb_info = new(std::nothrow)device_callback_info_s();
	cb_info->cb = callback;
	cb_info->user_data = user_data;
	cb_info->handle = handle;

	// input data when discovery requested..
	json input_data;
	input_data.set(NULL, "timeout", timeout_seconds);

	int req_id;
	const char* input = const_cast<const char*> (input_data.str().c_str());
	_D("input:%s", input);
	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_DISCOVERY_START, input_data.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	callback_map[req_id] = cb_info;
	handle->request_ids.push_back(req_id);
	_D("req_id:%d", req_id);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_discovery_stop(conv_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	int req_id;
	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_DISCOVERY_STOP, NULL, NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	// unset callback..
	std::list<int>::iterator req_itr = handle->request_ids.begin();
	for (; req_itr != handle->request_ids.end(); req_itr++) {
		int cur_req_id = *req_itr;
		if (callback_map[cur_req_id] !=  NULL) {
			_D("free memory for callback[id:%d]", cur_req_id);
			delete callback_map[cur_req_id];
			callback_map.erase(callback_map.find(cur_req_id));
		}
	}

	return CONV_ERROR_NONE;
}

// === Device
EXTAPI int conv_device_clone(conv_device_h original_handle, conv_device_h* target_handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(original_handle);
	ASSERT_NOT_NULL(target_handle);

	//LCOV_EXCL_START
	_conv_device_handle* device = new(std::nothrow) _conv_device_handle();
	device->jbody = original_handle->jbody;

	*target_handle = device;
	return CONV_ERROR_NONE;
	//LCOV_EXCL_STOP
}

EXTAPI int conv_device_destroy(conv_device_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	//LCOV_EXCL_START
	delete handle;

	return CONV_ERROR_NONE;
	//LCOV_EXCL_STOP
}

EXTAPI int conv_device_get_property_string(conv_device_h handle, const char* key, char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key && value);

	//LCOV_EXCL_START
	std::string strval;
	bool ret = handle->jbody.get(NULL, key, &strval);	// path is NULL..
	if (ret == false || strval.empty())
		return CONV_ERROR_NO_DATA;
	*value = strdup(strval.c_str());

	return CONV_ERROR_NONE;
	//LCOV_EXCL_STOP
}

EXTAPI int conv_device_foreach_service(conv_device_h handle, conv_service_foreach_cb cb, void* user_data)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(cb);

	//LCOV_EXCL_START
	json json_data = handle->jbody;
	std::string strval;

	int service_count = 0;
	service_count = json_data.array_get_size(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_DATA);	// json data..

	for (int index =0; index < service_count; index++) {
		_conv_service_handle* service = new(std::nothrow) _conv_service_handle();
		ASSERT_ALLOC(service);

		// service json data
		json json_service;
		std::string version, type, uri;
		conv_service_e service_type;
		json_data.get_array_elem(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_DATA, index, &json_service);
		json_data.get_array_elem(CONV_JSON_SERVICE_PATH, CONV_JSON_SERVICE_TYPE, index, (int*)&service_type);

		service->jservice = json_service;
		service->service_type = service_type;
		service->jdevice = handle->jbody;
		service->connection_state = CONV_SERVICE_CONNECTION_STATE_CONNECTED;

		_D("index[%d] service json : %s", index, json_service.str().c_str());

		cb(service, user_data);
		delete service;
	}

	return CONV_ERROR_NONE;
	//LCOV_EXCL_STOP
}

EXTAPI int conv_internal_set_activation_state(int activation_state)
{
	int error = vconf_set_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE, activation_state);
	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_internal_get_activation_state(int* activation_state)
{
	int error = vconf_get_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE, activation_state);
	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_internal_set_service_activation_state(int activation_state)
{
	int error = vconf_set_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE_SERVICE, activation_state);
	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_internal_get_service_activation_state(int* activation_state)
{
	int error = vconf_get_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE_SERVICE, activation_state);
	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);

	return CONV_ERROR_NONE;
}
