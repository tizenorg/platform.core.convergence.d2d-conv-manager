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

#include "common.h"
#include "d2d_conv_manager.h"
#include "conv_lib_json.h"
#include "internal_types.h"
#include "dbus_client.h"
#include "conv_lib_util.h"

static std::map<std::string, _conv_service_callback_info*> callback_map;
static std::map<std::string, _conv_service_connect_callback_info*> connect_callback_map;

std::string convert_type_to_string(conv_service_e service_type);
conv_service_e convert_string_to_type(std::string type_name);

static void conv_subject_cb(const char* subject, int req_id, int error, json data){
	_D("Callback response %d", req_id);

	json description;
	json service;
	json channel;
	std::string service_type;
	json payload;
	json key;
	int is_local = 0;

	data.get(NULL, CONV_JSON_DESCRIPTION, &description);

	_D("description:%s", description.str().c_str());
	description.get(NULL, CONV_JSON_SERVICE, &service);
	description.get(NULL, CONV_JSON_CHANNEL, &channel);
	description.get(NULL, CONV_JSON_TYPE, &service_type);
	description.get(NULL, CONV_JSON_IS_LOCAL, &is_local);

	data.get(NULL, CONV_JSON_PAYLOAD, &payload);
	_D("payload:%s", payload.str().c_str());

	key.set(NULL, CONV_JSON_SERVICE, service);
	key.set(NULL, CONV_JSON_TYPE, service_type);
	key.set(NULL, CONV_JSON_IS_LOCAL, is_local);
	_D("key:%s", key.str().c_str());

	std::map<std::string, _conv_service_callback_info*>::iterator it = callback_map.find(key.str());
	if (it == callback_map.end()) {
		_D("No callback found for response.");
		return;
	}

	_conv_service_callback_info* callback_info = it->second;

	conv_service_h service_handle = new(std::nothrow) _conv_service_handle();
	IF_FAIL_VOID_TAG(service_handle, _E, "Memory allocation failed");

	service_handle->jservice = service;
	service_handle->service_type = convert_string_to_type(service_type);
	service_handle->is_local = is_local;
	service_handle->connection_state = CONV_SERVICE_CONNECTION_STATE_CONNECTED;

	conv_channel_h channel_handle = new(std::nothrow) _conv_channel_handle();
	IF_FAIL_VOID_TAG(channel_handle, _E, "Memory allocation failed");

	channel_handle->jchannel = channel;

	conv_payload_h payload_handle = new(std::nothrow) _conv_payload_handle();
	IF_FAIL_VOID_TAG(payload_handle, _E, "Memory allocation failed");

	payload_handle->jpayload = payload;

	callback_info->cb(service_handle, channel_handle, CONV_ERROR_NONE, payload_handle, callback_info->user_data);

	delete payload_handle;
	delete channel_handle;
	delete service_handle;
}

static void conv_connect_subject_cb(const char* subject, int req_id, int error, json data){
	_D("Callback response %d", req_id);

	json description;
	json service;
	json channel;
	json device;
	std::string service_type;
	json payload;
	int is_local = 0;

	data.get(NULL, CONV_JSON_DESCRIPTION, &description);
	description.get(NULL, CONV_JSON_SERVICE, &service);
	description.get(NULL, CONV_JSON_TYPE, &service_type);
	description.get(NULL, CONV_JSON_DEVICE, &device);
	description.get(NULL, CONV_JSON_IS_LOCAL, &is_local);

	data.get(NULL, CONV_JSON_PAYLOAD, &payload);
	_D("payload:%s", payload.str().c_str());

	std::map<std::string, _conv_service_connect_callback_info*>::iterator it = connect_callback_map.find(description.str());
	if (it == connect_callback_map.end()) {
		_D("No callback found for response.");
		return;
	}

	_conv_service_connect_callback_info* callback_info = it->second;

	conv_service_h service_handle = new(std::nothrow) _conv_service_handle();
	IF_FAIL_VOID_TAG(service_handle, _E, "Memory allocation failed");

	service_handle->jservice = service;
	service_handle->service_type = convert_string_to_type(service_type);
	service_handle->is_local = is_local;
	service_handle->connection_state = CONV_SERVICE_CONNECTION_STATE_CONNECTED;

	conv_payload_h payload_handle = new(std::nothrow) _conv_payload_handle();
	IF_FAIL_VOID_TAG(payload_handle, _E, "Memory allocation failed");

	payload_handle->jpayload = payload;

	callback_info->cb(service_handle, CONV_ERROR_NONE, payload_handle, callback_info->user_data);

	delete payload_handle;
	delete service_handle;
}

static void register_subject_callbacks()
{
	static bool done = false;

	if (!done) {
		conv::dbus_client::register_callback(CONV_SUBJECT_COMMUNICATION_RECV, conv_subject_cb);
		conv::dbus_client::register_callback(CONV_SUBJECT_CONNECTION_START, conv_connect_subject_cb);
		done = true;
	}
	_I("Done with registering subject callback");
}

EXTAPI int conv_service_create(conv_service_h* handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	*handle = new(std::nothrow) _conv_service_handle();
	ASSERT_ALLOC(*handle);

	(*handle)->is_local = 1;
	(*handle)->service_type = CONV_SERVICE_NONE;
	(*handle)->connection_state = CONV_SERVICE_CONNECTION_STATE_CONNECTED;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_clone(conv_service_h original_handle, conv_service_h* target_handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(target_handle);
	ASSERT_NOT_NULL(original_handle);

	*target_handle = new(std::nothrow) _conv_service_handle();
	ASSERT_ALLOC(*target_handle);

	(*target_handle)->jservice = original_handle->jservice;
	(*target_handle)->service_type = original_handle->service_type;
	(*target_handle)->is_local = original_handle->is_local;
	(*target_handle)->jdevice = original_handle->jdevice;
	(*target_handle)->connection_state = original_handle->connection_state;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_destroy(conv_service_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	if ( handle->callback )
		delete handle->callback;

	delete handle;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_set_listener_cb(conv_service_h handle, conv_service_listener_cb callback, void* user_data)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(callback);

	register_subject_callbacks();

	json description;

	json service = handle->jservice;

	std::string type = convert_type_to_string(handle->service_type);
	if (type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_TYPE, type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	_conv_service_callback_info *cb_info = new(std::nothrow)_conv_service_callback_info();
	cb_info->cb = callback;
	cb_info->user_data = user_data;
	cb_info->handle = handle;
	cb_info->description = description;

	callback_map[description.str()] = cb_info;

	_D("key:%s", description.str().c_str());

	int req_id;

	int err = conv::dbus_client::request(REQ_SUBSCRIBE, &req_id, CONV_SUBJECT_COMMUNICATION_RECV, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Getting list failed");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_unset_listener_cb(conv_service_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	json description;

	json service = handle->jservice;

	std::string type = convert_type_to_string(handle->service_type);
	if (type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_TYPE, type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int req_id;
	int err = conv::dbus_client::request(REQ_UNSUBSCRIBE, &req_id, CONV_SUBJECT_COMMUNICATION_RECV, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Unset observe failed");

	std::map<std::string, _conv_service_callback_info*>::iterator it = callback_map.find(description.str());
	if (it == callback_map.end()) {
		_D("No callback found for response.");
		return CONV_ERROR_INVALID_OPERATION;
	}

	_conv_service_callback_info* callback_info = it->second;

	delete callback_info;
    callback_map.erase(it);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_start(conv_service_h handle, conv_channel_h channel_handle, conv_payload_h payload)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	int req_id;

	json description;
	json channel;

	if ( channel_handle != NULL )
		channel = channel_handle->jchannel;

	json service = handle->jservice;
	json device = handle->jdevice;
	std::string type = convert_type_to_string(handle->service_type);
	if (type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_CHANNEL, channel);
	description.set(NULL, CONV_JSON_DEVICE, device);
	description.set(NULL, CONV_JSON_TYPE, type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_COMMUNICATION_START, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_read(conv_service_h handle, conv_channel_h channel_handle, conv_payload_h payload_handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	int req_id;

	json description;
	json channel;

	if ( channel_handle != NULL )
		channel = channel_handle->jchannel;

	json payload;

	if ( payload_handle != NULL)
		payload = payload_handle->jpayload;

	json service = handle->jservice;
	std::string type = convert_type_to_string(handle->service_type);
	if (type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_CHANNEL, channel);
	description.set(NULL, CONV_JSON_PAYLOAD, payload);
	description.set(NULL, CONV_JSON_TYPE, type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_COMMUNICATION_GET, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_stop(conv_service_h handle, conv_channel_h channel_handle, conv_payload_h payload)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	int req_id;

	json description;
	json channel;

	if ( channel_handle != NULL )
		channel = channel_handle->jchannel;

	json service = handle->jservice;
	json device = handle->jdevice;
	std::string type = convert_type_to_string(handle->service_type);
	if (type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_CHANNEL, channel);
	description.set(NULL, CONV_JSON_DEVICE, device);
	description.set(NULL, CONV_JSON_TYPE, type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_COMMUNICATION_STOP, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_publish(conv_service_h handle, conv_channel_h channel_handle, conv_payload_h payload_handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	int req_id;

	json description;
	json channel;
	json payload;

	if (channel_handle != NULL)
		channel = channel_handle->jchannel;

	if (payload_handle != NULL)
		payload = payload_handle->jpayload;

	json service = handle->jservice;
	json device = handle->jdevice;

	std::string type = convert_type_to_string(handle->service_type);
	if (type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_CHANNEL, channel);
	description.set(NULL, CONV_JSON_DEVICE, device);
	description.set(NULL, CONV_JSON_PAYLOAD, payload);
	description.set(NULL, CONV_JSON_TYPE, type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_COMMUNICATION_SET, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	return CONV_ERROR_NONE;
}

static int conv_service_set_connected_cb(conv_service_h handle, json description, conv_service_connected_cb callback, void* user_data)
{
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(callback);

	register_subject_callbacks();

	_conv_service_connect_callback_info *cb_info = new(std::nothrow)_conv_service_connect_callback_info();
	cb_info->cb = callback;
	cb_info->user_data = user_data;
	cb_info->handle = handle;
	cb_info->description = description;

	connect_callback_map[description.str()] = cb_info;

	_D("key:%s", description.str().c_str());

	int req_id;

	int err = conv::dbus_client::request(REQ_SUBSCRIBE, &req_id, CONV_SUBJECT_CONNECTION_START, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Getting list failed");

	return CONV_ERROR_NONE;
}

static int conv_service_unset_connected_cb(conv_service_h handle)
{
	ASSERT_NOT_NULL(handle);

	json description;
	json service = handle->jservice;

	std::string service_type = convert_type_to_string(handle->service_type);
	if (service_type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_TYPE, service_type);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int req_id;
	int err = conv::dbus_client::request(REQ_UNSUBSCRIBE, &req_id, CONV_SUBJECT_CONNECTION_START, NULL, NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Unset observe failed");

	std::map<std::string, _conv_service_connect_callback_info*>::iterator it = connect_callback_map.find(description.str());
	if (it == connect_callback_map.end()) {
		_D("No callback found for response.");
		return CONV_ERROR_INVALID_OPERATION;
	}

	_conv_service_connect_callback_info* callback_info = it->second;

	delete callback_info;
    connect_callback_map.erase(it);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_get_connection_state(conv_service_h handle, conv_service_connection_state_e* state)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(state);

	*state = handle->connection_state;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_connect(conv_service_h handle, conv_service_connected_cb callback, void* user_data)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(callback);

	int req_id;

	json description;
	json service = handle->jservice;
	json device = handle->jdevice;

	std::string service_type = convert_type_to_string(handle->service_type);
	if (service_type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_TYPE, service_type);
	description.set(NULL, CONV_JSON_DEVICE, device);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	conv_service_set_connected_cb(handle, description, callback, user_data);

	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_CONNECTION_START, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_disconnect(conv_service_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	conv_service_unset_connected_cb(handle);

	int req_id;

	json description;
	json service = handle->jservice;
	json device = handle->jdevice;
	std::string service_type = convert_type_to_string(handle->service_type);
	if (service_type.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	description.set(NULL, CONV_JSON_SERVICE, service);
	description.set(NULL, CONV_JSON_TYPE, service_type);
	description.set(NULL, CONV_JSON_DEVICE, device);
	description.set(NULL, CONV_JSON_IS_LOCAL, handle->is_local);

	int err = conv::dbus_client::request(REQ_WRITE, &req_id, CONV_SUBJECT_CONNECTION_STOP, description.str().c_str(), NULL, NULL);
	IF_FAIL_RETURN_TAG(err == CONV_ERROR_NONE, err, _E, "Failed in starting flow service");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_get_type(conv_service_h handle, conv_service_e* value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(value);

	*value = handle->service_type;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_get_property_string(conv_service_h handle, const char* key, char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key && value);

	std::string strval;
	bool ret = handle->jservice.get(NULL, key, &strval);	// path is NULL..
	if (ret == false || strval.empty())
		return CONV_ERROR_NO_DATA;
	*value = strdup(strval.c_str());

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_set_type(conv_service_h handle, conv_service_e value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	std::string value_str = convert_type_to_string(value);
	if (value_str.empty())
		return CONV_ERROR_INVALID_PARAMETER;

	handle->service_type = value;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_service_set_property_string(conv_service_h handle, const char* key, const char* value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	handle->jservice.set(NULL, key, value);

	return CONV_ERROR_NONE;
}

std::string convert_type_to_string(conv_service_e service_type)
{
	std::string str;
	switch (service_type) {
	case CONV_SERVICE_APP_TO_APP_COMMUNICATION:
		str = CONV_SERVICE_TYPE_SMARTVIEW_APP_COMMUNICATION;
		break;
	case CONV_SERVICE_REMOTE_APP_CONTROL:
		str = CONV_SERVICE_TYPE_REMOTE_APP_CONTROL;
		break;
	default:
		break;
	}
	return str;
}

conv_service_e convert_string_to_type(std::string type_name)
{
	conv_service_e service_type = CONV_SERVICE_NONE;

	if ( !type_name.compare(CONV_SERVICE_TYPE_SMARTVIEW_APP_COMMUNICATION)) {
		service_type = CONV_SERVICE_APP_TO_APP_COMMUNICATION;
	} else if ( !type_name.compare(CONV_SERVICE_TYPE_REMOTE_APP_CONTROL)) {
		service_type = CONV_SERVICE_REMOTE_APP_CONTROL;
	}
	return service_type;
}
