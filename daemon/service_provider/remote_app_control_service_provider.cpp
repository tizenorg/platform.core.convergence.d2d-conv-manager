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

#include <bundle.h>
#include <bundle_internal.h>
#include <app_control_internal.h>
#include "remote_app_control_service_provider.h"
#include "../client_mgr_impl.h"

using namespace std;

struct app_control_cb_info_s {
	int req_id;
	iotcon_request_h request_handle;
};

struct response_cb_info_s {
	int req_id;
	conv::request* request_obj;
};

static int get_req_id()
{
	static int req_id = 0;

	if (++req_id < 0) {
		req_id = 1;
	}

	return req_id;
}

static std::map<int, app_control_cb_info_s> app_control_cb_map;
static std::map<int, response_cb_info_s> response_cb_map;

conv::remote_app_control_service_provider::remote_app_control_service_provider()
{
	_type = CONV_SERVICE_TYPE_REMOTE_APP_CONTROL;
	_resource_type = CONV_RESOURCE_TYPE_REMOTE_APP_CONTROL;
	_uri = CONV_URI_SMARTVIEW_REMOTE_APP_CONTROL;
}

conv::remote_app_control_service_provider::~remote_app_control_service_provider()
{
	app_control_cb_map.clear();
	response_cb_map.clear();
}

static int _send_response(iotcon_request_h request, iotcon_representation_h repr,
		iotcon_response_result_e result)
{
	int ret;
	iotcon_response_h response;

	ret = iotcon_response_create(request, &response);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_response_create() Fail(%d)", ret);
		return CONV_ERROR_INVALID_OPERATION;
	}

	ret = iotcon_response_set_result(response, result);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_response_set_result() Fail(%d)", ret);
		iotcon_response_destroy(response);
		return CONV_ERROR_INVALID_OPERATION;
	}

	ret = iotcon_response_set_representation(response, repr);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_response_set_representation() Fail(%d)", ret);
		iotcon_response_destroy(response);
		return CONV_ERROR_INVALID_OPERATION;
	}

	/* send Representation to the client */
	ret = iotcon_response_send(response);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_response_send() Fail(%d)", ret);
		iotcon_response_destroy(response);
		return CONV_ERROR_INVALID_OPERATION;
	}

	iotcon_response_destroy(response);

	return CONV_ERROR_NONE;
}

static void _app_control_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	_D("app control reply received");

	int reply_id = (int)user_data;

	std::map<int, app_control_cb_info_s>::iterator find_iter = app_control_cb_map.find(reply_id);

	IF_FAIL_VOID_TAG(find_iter != app_control_cb_map.end(), _E, "No reply data");

	app_control_cb_info_s cb_info = find_iter->second;

	bundle* p_bundle;
	iotcon_representation_h rep;
	iotcon_representation_create(&rep);
	iotcon_attributes_h attributes;
	iotcon_attributes_create(&attributes);

	app_control_export_as_bundle(reply, &p_bundle);

	bundle_raw* p_bundle_raw;
	int len;
	bundle_encode(p_bundle, &p_bundle_raw, &len);
	char* bundle_raw = reinterpret_cast<char*>(p_bundle_raw);

	iotcon_attributes_add_str(attributes, CONV_JSON_APP_CONTROL, bundle_raw);
	iotcon_attributes_add_int(attributes, CONV_JSON_REQ_ID,  cb_info.req_id);

	iotcon_representation_set_attributes(rep, attributes);

	_D("Send response to sender");
	_send_response(cb_info.request_handle, rep, IOTCON_RESPONSE_OK);
	_D("Response sent");

	bundle_free(p_bundle);
	app_control_cb_map.erase(find_iter);
}

static void handle_request(iotcon_representation_h rep, iotcon_request_h request)
{
	iotcon_attributes_h attributes;
	char* appctl_char;
	app_control_h app_control;
	int reply = 0;

	int ret = iotcon_representation_get_attributes(rep, &attributes);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_representation_get_attributes() Fail(%d)", ret);
		return;
	}

	ret = iotcon_attributes_get_str(attributes, CONV_JSON_APP_CONTROL, &appctl_char);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_get_str() Fail(%d)", ret);
		return;
	}

	bundle_raw* encoded = reinterpret_cast<unsigned char*>(appctl_char);
	bundle* appctl_bundle = bundle_decode(encoded, strlen(appctl_char));

	app_control_create(&app_control);
	app_control_import_from_bundle(app_control, appctl_bundle);

	iotcon_attributes_get_int(attributes, CONV_JSON_REPLY, &reply);

	if (reply == 1) {
		bool waiting_reply = false;

		// check if it's already launched and waiting for app_control response
		for(std::map<int, app_control_cb_info_s>::iterator it = app_control_cb_map.begin(); it != app_control_cb_map.end(); it++) {
			app_control_cb_info_s found_cb_info = it->second;
			if(found_cb_info.request_handle == request) {
				waiting_reply = true;
				_D("app_control_send_launch_request is already called...waiting reply");
				break;
			}
		}

		if (!waiting_reply) {
			int req_id;
			int reply_id = get_req_id();

			iotcon_attributes_get_int(attributes, CONV_JSON_REQ_ID, &req_id);

			app_control_cb_info_s cb_info;
			cb_info.req_id = req_id;
			cb_info.request_handle = request;
			app_control_cb_map[reply_id] = cb_info;

			app_control_send_launch_request(app_control, _app_control_cb, (void*)reply_id);
			_D("app_control_send_launch_request with callback");

			bundle_free(appctl_bundle);
		}
	} else {
		iotcon_response_result_e result;
		ret = app_control_send_launch_request(app_control, NULL, NULL);

		if (ret != APP_CONTROL_ERROR_NONE) {
			_E("Launch request failed(%d)", ret);
			result = IOTCON_RESPONSE_ERROR;
		} else {
			_D("Launch request succeeded");
			result = IOTCON_RESPONSE_OK;
		}
		bundle_free(appctl_bundle);
		app_control_destroy(app_control);

		_send_response(request, NULL, result);
	}
}

void conv::remote_app_control_service_provider::iotcon_request_cb(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
{
	_D("request cb called");

	int ret;
	iotcon_request_type_e type;
	char *host_address;

	IF_FAIL_VOID_TAG(request, _E, "request is NULL");

	ret = iotcon_request_get_host_address(request, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_request_get_host_address() Fail(%d)", ret);
		_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
		return;
	}
	_I("host_address : %s", host_address);

	ret = iotcon_request_get_request_type(request, &type);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_request_get_types() Fail(%d)", ret);
		_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
		return;
	}

	_D("request type : %d", type);

	if (IOTCON_REQUEST_PUT == type) {
		iotcon_representation_h req_repr;
		_I("GET request");

		ret = iotcon_request_get_representation(request, &req_repr);
		if (IOTCON_ERROR_NONE != ret) {
			_E("iotcon_request_get_representation() Fail(%d)", ret);
			return;
		}

		handle_request(req_repr, request);
		iotcon_representation_destroy(req_repr);
	}

	if (IOTCON_ERROR_NONE != ret) {
		_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
		return;
	}
}

int conv::remote_app_control_service_provider::init()
{
	// register resource
	int properties;
	iotcon_resource_interfaces_h resource_ifaces = NULL;
	iotcon_resource_types_h resource_types = NULL;
	int error;

	properties = IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE;

	error = iotcon_resource_types_create(&resource_types);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "rt creation failed");

	iotcon_resource_types_add(resource_types, _resource_type.c_str());

	error = iotcon_resource_interfaces_create(&resource_ifaces);

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "ri creation failed");

	iotcon_resource_interfaces_add(resource_ifaces, IOTCON_INTERFACE_DEFAULT);


	error = iotcon_resource_create(CONV_URI_SMARTVIEW_REMOTE_APP_CONTROL, resource_types, resource_ifaces, properties, iotcon_request_cb, NULL, &iotcon_resource);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource creation failed");


	iotcon_resource_types_destroy(resource_types);
	iotcon_resource_interfaces_destroy(resource_ifaces);

	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::release()
{
	// unregister resource
	int error = iotcon_resource_destroy(iotcon_resource);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource destroy failed");
	iotcon_resource = NULL;

	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::start_request(request* request_obj)
{
	_D("communcation/start requested");
	int error;

	int properties;
	iotcon_resource_interfaces_h resource_ifaces = NULL;
	iotcon_resource_types_h resource_types = NULL;

	remote_app_control_service_info *svc_info = reinterpret_cast<remote_app_control_service_info*>(request_obj->service_info);

	if (svc_info->iotcon_info_obj.iotcon_resource_handle != NULL) {
		_D("already started");
		return CONV_ERROR_INVALID_OPERATION;
	}

	properties = IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE;

	error = iotcon_resource_types_create(&resource_types);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "rt creation failed");

	iotcon_resource_types_add(resource_types, svc_info->iotcon_info_obj.resource_type.c_str());

	error = iotcon_resource_interfaces_create(&resource_ifaces);

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "ri creation failed");

	iotcon_resource_interfaces_add(resource_ifaces, IOTCON_INTERFACE_DEFAULT);

	error = iotcon_remote_resource_create(svc_info->iotcon_info_obj.address.c_str(), IOTCON_CONNECTIVITY_IPV4, svc_info->iotcon_info_obj.uri.c_str(), properties, resource_types, resource_ifaces,
			&(svc_info->iotcon_info_obj.iotcon_resource_handle));

	_D("remote resource created : %s, %s", svc_info->iotcon_info_obj.address.c_str(), svc_info->iotcon_info_obj.uri.c_str());

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "remote resource creation failed %s, %s", svc_info->iotcon_info_obj.address.c_str(), svc_info->iotcon_info_obj.uri.c_str());

	iotcon_resource_types_destroy(resource_types);
	iotcon_resource_interfaces_destroy(resource_ifaces);

	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::stop_request(request* request_obj)
{
	_D("communcation/stop requested");

	remote_app_control_service_info *svc_info = reinterpret_cast<remote_app_control_service_info*>(request_obj->service_info);

	if (svc_info->iotcon_info_obj.iotcon_resource_handle == NULL) {
		_D("not even started");
		return CONV_ERROR_INVALID_OPERATION;
	}

	iotcon_remote_resource_destroy(svc_info->iotcon_info_obj.iotcon_resource_handle);

	svc_info->iotcon_info_obj.iotcon_resource_handle = NULL;

	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::get_request(request* request_obj)
{
	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::send_response(json payload, request* request_obj)
{
	return CONV_ERROR_NONE;
}

static void on_response(iotcon_remote_resource_h resource, iotcon_error_e err,
		iotcon_request_type_e request_type, iotcon_response_h response, void *user_data)
{
	int ret;
	iotcon_response_result_e response_result;
	iotcon_representation_h repr;

	ret = iotcon_response_get_result(response, &response_result);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_response_get_result() Fail(%d)", ret);
		return;
	}

	if (IOTCON_RESPONSE_OK != response_result) {
		_E("_on_response_observe Response error(%d)", response_result);
		return;
	}

	ret = iotcon_response_get_representation(response, &repr);

	int req_id = -1;
	iotcon_attributes_h attributes;
	iotcon_representation_get_attributes(repr, &attributes);
	iotcon_attributes_get_int(attributes, CONV_JSON_REQ_ID, &req_id);

	std::map<int, response_cb_info_s>::iterator find_iter = response_cb_map.find(req_id);

	IF_FAIL_VOID_TAG(find_iter != response_cb_map.end(), _E, "No callback found for response");

	response_cb_info_s cb_info = find_iter->second;

	char* appctl_char;

	ret = iotcon_attributes_get_str(attributes, CONV_JSON_APP_CONTROL, &appctl_char);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_get_str() Fail(%d)", ret);
		return;
	}

	_D(RED("publishing_response"));
	if (cb_info.request_obj) {
		json result;
		json payload;
		json description;

		payload.set(NULL, CONV_JSON_APP_CONTROL, appctl_char);
		result.set(NULL, CONV_JSON_DESCRIPTION, cb_info.request_obj->get_description());
		result.set(NULL, CONV_JSON_PAYLOAD, payload);

		cb_info.request_obj->publish(CONV_ERROR_NONE, result);
		_D("response published");
	}
	response_cb_map.erase(find_iter);
}

int conv::remote_app_control_service_provider::set_request(request* request_obj)
{
	remote_app_control_service_info *svc_info = reinterpret_cast<remote_app_control_service_info*>(request_obj->service_info);
	int error;

	request_obj->communication_info = &(svc_info->iotcon_info_obj);

	iotcon_representation_h representation;
	iotcon_representation_create(&representation);

	json payload;
	request_obj->get_payload_from_description(&payload);

	string app_control;
	int reply;

	payload.get(NULL, CONV_JSON_APP_CONTROL, &app_control);
	payload.get(NULL, CONV_JSON_REPLY, &reply);

	iotcon_attributes_h attributes;
	iotcon_attributes_create(&attributes);

	iotcon_attributes_add_str(attributes, CONV_JSON_APP_CONTROL, (char*)app_control.c_str());
	iotcon_attributes_add_int(attributes, CONV_JSON_REPLY, reply);

	if (reply == 1) {
		int req_id = get_req_id();
		iotcon_attributes_add_int(attributes, CONV_JSON_REQ_ID, req_id);

		response_cb_info_s cb_info;
		cb_info.req_id = req_id;
		cb_info.request_obj = svc_info->registered_request;
		response_cb_map[req_id] = cb_info;
	}

	iotcon_representation_set_attributes(representation, attributes);
	svc_info->iotcon_info_obj.iotcon_representation_handle = representation;

	iotcon_attributes_destroy(attributes);

	error = iotcon_remote_resource_put(svc_info->iotcon_info_obj.iotcon_resource_handle, representation, NULL, on_response, NULL);

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "iotcon_remote_resource_put failed");

	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::register_request(request* request_obj)
{
	_D("communcation/recv requested");
	remote_app_control_service_info *svc_info = reinterpret_cast<remote_app_control_service_info*>(request_obj->service_info);

	switch (request_obj->get_type()) {
	case REQ_SUBSCRIBE:
		if (svc_info->registered_request != NULL) {
			delete svc_info->registered_request;
		}
		svc_info->registered_request = request_obj;
		request_obj->reply(CONV_ERROR_NONE);
		_D("subscribe requested");
		break;
	case REQ_UNSUBSCRIBE:
		svc_info->registered_request = NULL;
		request_obj->reply(CONV_ERROR_NONE);
		delete request_obj;
		break;
	default:
		request_obj->reply(CONV_ERROR_INVALID_OPERATION);
		delete request_obj;
		return CONV_ERROR_INVALID_OPERATION;
		break;
	}

	return CONV_ERROR_NONE;
}


int conv::remote_app_control_service_provider::load_service_info(request* request_obj)
{
	string client;
	conv::client* client_obj = NULL;

	client = request_obj->get_sender();
	_D("client id : %s", client.c_str());
	client_obj = conv::client_manager::get_client(client);
	IF_FAIL_RETURN_TAG(client_obj, CONV_ERROR_OUT_OF_MEMORY, _E, "client info alloc failed");

	json description = request_obj->get_description();
	json service;
	json device;

	string device_id;
	string device_name;
	string device_address;
	string uri;

	description.get(NULL, CONV_JSON_SERVICE, &service);
	description.get(NULL, CONV_JSON_DEVICE, &device);

	service.get(NULL, CONV_SERVICE_ID, &uri);

	device.get(NULL, CONV_DEVICE_ID, &device_id);
	device.get(NULL, CONV_DEVICE_NAME, &device_name);
	device.get(NULL, CONV_JSON_DEVICE_ADDRESS, &device_address);

	remote_app_control_service_info *svc_info = NULL;
	service_info_base* svc_info_base = client_obj->get_service_info(_type, device_id);

	if (svc_info_base != NULL) {
		_D("service instance already exists");
		svc_info = reinterpret_cast<remote_app_control_service_info*>(svc_info_base);

		if (svc_info == NULL)
		{
			_D("casting failed");
			return CONV_ERROR_INVALID_OPERATION;
		}
	} else {
		_D("allocating new service instance");
		svc_info = new(std::nothrow) remote_app_control_service_info();
		IF_FAIL_RETURN_TAG(svc_info, CONV_ERROR_OUT_OF_MEMORY, _E, "svc_info alloc failed");

		svc_info->device_id = device_id;
		svc_info->device_name = device_name;
		svc_info->device_address = device_address;

		svc_info->iotcon_info_obj.address = device_address;
		svc_info->iotcon_info_obj.uri = CONV_URI_SMARTVIEW_REMOTE_APP_CONTROL;
		svc_info->iotcon_info_obj.resource_type = _resource_type;

		//save service info
		client_obj->add_service_info(_type, device_id, (service_info_base*)svc_info);

		_D("remote app control service is created");
	}

	request_obj->service_info = svc_info;
	return CONV_ERROR_NONE;
}

int conv::remote_app_control_service_provider::get_service_info_for_discovery(json* json_obj)
{
	json_obj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_TYPE, CONV_SERVICE_REMOTE_APP_CONTROL);

	// set data for service handle
	json info;
	info.set(NULL, CONV_SERVICE_ID, _uri);
	info.set(NULL, CONV_SERVICE_VERSION, "1.0");

	json_obj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_INFO, info);

	return CONV_ERROR_NONE;
}