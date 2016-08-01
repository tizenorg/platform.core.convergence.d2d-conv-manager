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
#include "RemoteAppControlServiceProvider.h"
#include "../ClientManager.h"
#include "../Util.h"

using namespace std;

struct app_control_cb_info_s {
	int req_id;
	iotcon_request_h request_handle;
};

struct response_cb_info_s {
	int req_id;
	conv::Request* requestObj;
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

static void vconf_update_cb(keynode_t *node, void* user_data)
{
	conv::RemoteAppControlServiceProvider* instance = static_cast<conv::RemoteAppControlServiceProvider*>(user_data);
	IF_FAIL_VOID_TAG(instance, _E, "static_cast failed");

	instance->handleVconfUpdate(node);
}

conv::RemoteAppControlServiceProvider::RemoteAppControlServiceProvider()
{
	__type = CONV_SERVICE_TYPE_REMOTE_APP_CONTROL;
	__resourceType = CONV_RESOURCE_TYPE_REMOTE_APP_CONTROL;
	__uri = CONV_URI_REMOTE_APP_CONTROL;
	iotcon_resource = NULL;
	if (conv::util::isServiceActivated(CONV_INTERNAL_SERVICE_REMOTE_APP_CONTROL))
		__activationState = 1;
	else
		__activationState = 0;

	vconf_notify_key_changed(VCONFKEY_SETAPPL_D2D_CONVERGENCE_SERVICE, vconf_update_cb, this);
}

conv::RemoteAppControlServiceProvider::~RemoteAppControlServiceProvider()
{
	app_control_cb_map.clear();
	response_cb_map.clear();
}

int conv::RemoteAppControlServiceProvider::handleVconfUpdate(keynode_t *node)
{
	int current_state = vconf_keynode_get_int(node);

	if ((CONV_INTERNAL_SERVICE_REMOTE_APP_CONTROL & current_state) > 0) {
		__activationState = 1;
		init();
	} else {
		__activationState = 0;
		release();
	}

	return CONV_ERROR_NONE;
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
	bundle* p_bundle_request;
	iotcon_representation_h rep;
	iotcon_representation_create(&rep);
	iotcon_attributes_h attributes;
	iotcon_attributes_create(&attributes);

	app_control_export_as_bundle(reply, &p_bundle);
	app_control_export_as_bundle(request, &p_bundle_request);

	bundle_raw* p_bundle_raw;
	bundle_raw* p_bundle_request_raw;

	int len, len_request;
	bundle_encode(p_bundle, &p_bundle_raw, &len);
	char* bundle_raw = reinterpret_cast<char*>(p_bundle_raw);

	bundle_encode(p_bundle_request, &p_bundle_request_raw, &len_request);
	char* bundle_request_raw = reinterpret_cast<char*>(p_bundle_request_raw);

	iotcon_attributes_add_str(attributes, CONV_JSON_APP_CONTROL_REPLY, bundle_raw);
	iotcon_attributes_add_str(attributes, CONV_JSON_APP_CONTROL_REQUEST, bundle_request_raw);
	iotcon_attributes_add_int(attributes, CONV_JSON_REQ_ID, cb_info.req_id);
	iotcon_attributes_add_int(attributes, CONV_JSON_APP_CONTROL_RESULT,  result);

	iotcon_representation_set_attributes(rep, attributes);

	_D("Send response to sender");
	_send_response(cb_info.request_handle, rep, IOTCON_RESPONSE_OK);
	_D("Response sent");

	bundle_free(p_bundle);
	bundle_free(p_bundle_request);
	app_control_cb_map.erase(find_iter);
}

static int handle_request(iotcon_representation_h rep, iotcon_request_h request)
{
	iotcon_attributes_h attributes;
	char* appctl_char;
	app_control_h app_control;
	int reply = 0;

	int ret = iotcon_representation_get_attributes(rep, &attributes);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_representation_get_attributes() Fail(%d)", ret);
		return CONV_ERROR_INVALID_PARAMETER;
	}

	ret = iotcon_attributes_get_str(attributes, CONV_JSON_APP_CONTROL, &appctl_char);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_get_str() Fail(%d)", ret);
		return CONV_ERROR_INVALID_PARAMETER;
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

			ret = app_control_send_launch_request(app_control, _app_control_cb, (void*)reply_id);
			_D("app_control_send_launch_request with callback");

			if (ret != APP_CONTROL_ERROR_NONE) {
				_E("Launch request failed(%d)", ret);
				iotcon_response_result_e result = IOTCON_RESPONSE_ERROR;
				_send_response(request, NULL, result);
				app_control_cb_map.erase(reply_id);
			}

			bundle_free(appctl_bundle);
			app_control_destroy(app_control);
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

	return CONV_ERROR_NONE;
}

void conv::RemoteAppControlServiceProvider::__iotcon_request_cb(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
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
		_I("PUT request");

		ret = iotcon_request_get_representation(request, &req_repr);
		if (IOTCON_ERROR_NONE != ret) {
			_E("iotcon_request_get_representation() Fail(%d)", ret);
			_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
			return;
		}

		if (handle_request(req_repr, request) != CONV_ERROR_NONE) {
			_E("handle_request() Fail");
			_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
		}

		iotcon_representation_destroy(req_repr);
	}
}

int conv::RemoteAppControlServiceProvider::init()
{
	if (iotcon_resource == NULL) {
		// register resource
		int properties;
		iotcon_resource_interfaces_h resource_ifaces = NULL;
		iotcon_resource_types_h resource_types = NULL;
		int error;

		properties = IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE;

		error = iotcon_resource_types_create(&resource_types);
		IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "rt creation failed");

		iotcon_resource_types_add(resource_types, __resourceType.c_str());

		error = iotcon_resource_interfaces_create(&resource_ifaces);

		IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "ri creation failed");

		iotcon_resource_interfaces_add(resource_ifaces, IOTCON_INTERFACE_DEFAULT);


		error = iotcon_resource_create(CONV_URI_REMOTE_APP_CONTROL, resource_types, resource_ifaces, properties, __iotcon_request_cb, NULL, &iotcon_resource);
		IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource creation failed");

		iotcon_resource_types_destroy(resource_types);
		iotcon_resource_interfaces_destroy(resource_ifaces);

		_D("RemoteAppControlServiceProvider init done");
	} else {
		_D("RemoteAppControlServiceProvider is already initiated");
	}

	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::release()
{
	if (iotcon_resource == NULL) {
		_D("RemoteAppControlServiceProvider is already released");
	} else {
		// unregister resource
		int error = iotcon_resource_destroy(iotcon_resource);
		IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource destroy failed");
		iotcon_resource = NULL;
	}

	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::startRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/start requested");
	int error;
	Json result;

	int properties;
	iotcon_resource_interfaces_h resource_ifaces = NULL;
	iotcon_resource_types_h resource_types = NULL;

	RemoteAppControlServiceInfo *svcInfo = reinterpret_cast<RemoteAppControlServiceInfo*>(requestObj->getServiceInfo());

	if (svcInfo->iotconInfoObj.iotconResourceHandle != NULL) {
		_D("already started");
		sendResponse(result, CONV_JSON_ON_START, CONV_ERROR_INVALID_OPERATION, svcInfo->registeredRequest);
		return CONV_ERROR_INVALID_OPERATION;
	}

	properties = IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE;

	error = iotcon_resource_types_create(&resource_types);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "rt creation failed");

	iotcon_resource_types_add(resource_types, svcInfo->iotconInfoObj.resourceType.c_str());

	error = iotcon_resource_interfaces_create(&resource_ifaces);

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "ri creation failed");

	iotcon_resource_interfaces_add(resource_ifaces, IOTCON_INTERFACE_DEFAULT);

	error = iotcon_remote_resource_create(svcInfo->iotconInfoObj.address.c_str(), IOTCON_CONNECTIVITY_IPV4, svcInfo->iotconInfoObj.uri.c_str(), properties, resource_types, resource_ifaces,
			&(svcInfo->iotconInfoObj.iotconResourceHandle));

	_D("remote resource created : %s, %s", svcInfo->iotconInfoObj.address.c_str(), svcInfo->iotconInfoObj.uri.c_str());

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "remote resource creation failed %s, %s", svcInfo->iotconInfoObj.address.c_str(), svcInfo->iotconInfoObj.uri.c_str());

	iotcon_resource_types_destroy(resource_types);
	iotcon_resource_interfaces_destroy(resource_ifaces);

	sendResponse(result, CONV_JSON_ON_START, CONV_ERROR_NONE, svcInfo->registeredRequest);

	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::sendResponse(Json payload, const char* request_type, conv_error_e error, Request* requestObj)
{
	_D(RED("publishing_response"));
	IF_FAIL_RETURN_TAG(requestObj != NULL, CONV_ERROR_INVALID_OPERATION, _E, "listener_cb is not registered");

	Json result;
	Json description = requestObj->getDescription();

	payload.set(NULL, CONV_JSON_RESULT_TYPE, request_type);

	result.set(NULL, CONV_JSON_DESCRIPTION, description);
	result.set(NULL, CONV_JSON_PAYLOAD, payload);
	requestObj->publish(error, result);

	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::stopRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/stop requested");
	Json result;

	RemoteAppControlServiceInfo *svcInfo = reinterpret_cast<RemoteAppControlServiceInfo*>(requestObj->getServiceInfo());

	if (svcInfo->iotconInfoObj.iotconResourceHandle == NULL) {
		_D("not even started");
		sendResponse(result, CONV_JSON_ON_STOP, CONV_ERROR_INVALID_OPERATION, svcInfo->registeredRequest);
		return CONV_ERROR_INVALID_OPERATION;
	}

	iotcon_remote_resource_destroy(svcInfo->iotconInfoObj.iotconResourceHandle);
	svcInfo->iotconInfoObj.iotconResourceHandle = NULL;
	sendResponse(result, CONV_JSON_ON_STOP, CONV_ERROR_NONE, svcInfo->registeredRequest);

	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::readRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	return CONV_ERROR_NONE;
}

static void __on_response(iotcon_remote_resource_h resource, iotcon_error_e err,
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

	ret = iotcon_attributes_get_str(attributes, CONV_JSON_APP_CONTROL_REPLY, &appctl_char);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_get_str() Fail(%d)", ret);
		return;
	}

	char* appctl_request_char;
	ret = iotcon_attributes_get_str(attributes, CONV_JSON_APP_CONTROL_REQUEST, &appctl_request_char);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_get_str() Fail(%d)", ret);
		return;
	}

	int appctl_result;
	ret = iotcon_attributes_get_int(attributes, CONV_JSON_APP_CONTROL_RESULT, &appctl_result);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_get_int() Fail(%d)", ret);
		return;
	}

	_D(RED("publishing_response"));
	if (cb_info.requestObj) {
		conv::Json result;
		conv::Json payload;
		conv::Json description;

		if (cb_info.requestObj == NULL) {
			_E("listener_cb is not registered");
		} else {
			payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_PUBLISH);
			payload.set(NULL, CONV_JSON_APP_CONTROL_REPLY, appctl_char);
			payload.set(NULL, CONV_JSON_APP_CONTROL_REQUEST, appctl_request_char);
			payload.set(NULL, CONV_JSON_APP_CONTROL_RESULT, appctl_result);
			result.set(NULL, CONV_JSON_DESCRIPTION, cb_info.requestObj->getDescription());
			result.set(NULL, CONV_JSON_PAYLOAD, payload);

			cb_info.requestObj->publish(CONV_ERROR_NONE, result);
			_D("response published");
		}
	}
	response_cb_map.erase(find_iter);
}

int conv::RemoteAppControlServiceProvider::publishRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	RemoteAppControlServiceInfo *svcInfo = reinterpret_cast<RemoteAppControlServiceInfo*>(requestObj->getServiceInfo());
	int error;

	requestObj->setCommunicationInfo(&(svcInfo->iotconInfoObj));

	iotcon_representation_h representation;
	iotcon_representation_create(&representation);

	Json payload;
	requestObj->getPayloadFromDescription(&payload);

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
		cb_info.requestObj = svcInfo->registeredRequest;
		response_cb_map[req_id] = cb_info;
	}

	iotcon_representation_set_attributes(representation, attributes);
	svcInfo->iotconInfoObj.iotconRepresentationHandle = representation;

	iotcon_attributes_destroy(attributes);

	error = iotcon_remote_resource_put(svcInfo->iotconInfoObj.iotconResourceHandle, representation, NULL, __on_response, NULL);

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "iotcon_remote_resource_put failed");

	if (reply != 1) {
		Json result;
		sendResponse(result, CONV_JSON_ON_PUBLISH, CONV_ERROR_NONE, svcInfo->registeredRequest);
	}

	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::registerRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/recv requested");
	RemoteAppControlServiceInfo *svcInfo = reinterpret_cast<RemoteAppControlServiceInfo*>(requestObj->getServiceInfo());

	switch (requestObj->getType()) {
	case REQ_SUBSCRIBE:
		if (svcInfo->registeredRequest != NULL) {
			delete svcInfo->registeredRequest;
		}
		svcInfo->registeredRequest = requestObj;
		requestObj->reply(CONV_ERROR_NONE);
		_D("subscribe requested");
		break;
	case REQ_UNSUBSCRIBE:
		svcInfo->registeredRequest = NULL;
		requestObj->reply(CONV_ERROR_NONE);
		delete requestObj;
		break;
	default:
		requestObj->reply(CONV_ERROR_INVALID_OPERATION);
		delete requestObj;
		return CONV_ERROR_INVALID_OPERATION;
		break;
	}

	return CONV_ERROR_NONE;
}


int conv::RemoteAppControlServiceProvider::loadServiceInfo(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	string client;
	conv::ClientInfo* clientObj = NULL;

	client = requestObj->getSender();
	_D("client id : %s", client.c_str());
	clientObj = conv::client_manager::getClient(client);
	IF_FAIL_RETURN_TAG(clientObj, CONV_ERROR_OUT_OF_MEMORY, _E, "client info alloc failed");

	Json description = requestObj->getDescription();
	Json service;
	Json device;

	string deviceId;
	string deviceName;
	string deviceAddress;
	string uri;

	description.get(NULL, CONV_JSON_SERVICE, &service);
	description.get(NULL, CONV_JSON_DEVICE, &device);

	service.get(NULL, CONV_SERVICE_ID, &uri);

	device.get(NULL, CONV_DEVICE_ID, &deviceId);
	device.get(NULL, CONV_DEVICE_NAME, &deviceName);
	device.get(NULL, CONV_JSON_DEVICE_ADDRESS, &deviceAddress);

	RemoteAppControlServiceInfo *svcInfo = NULL;
	IServiceInfo* svcInfoBase = clientObj->getServiceInfo(__type, deviceId);

	if (svcInfoBase != NULL) {
		_D("service instance already exists");
		svcInfo = reinterpret_cast<RemoteAppControlServiceInfo*>(svcInfoBase);

		if (svcInfo == NULL)
		{
			_D("casting failed");
			return CONV_ERROR_INVALID_OPERATION;
		}
	} else {
		_D("allocating new service instance");
		svcInfo = new(std::nothrow) RemoteAppControlServiceInfo();
		IF_FAIL_RETURN_TAG(svcInfo, CONV_ERROR_OUT_OF_MEMORY, _E, "svcInfo alloc failed");

		svcInfo->deviceId = deviceId;
		svcInfo->deviceName = deviceName;
		svcInfo->deviceAddress = deviceAddress;

		svcInfo->iotconInfoObj.address = deviceAddress;
		svcInfo->iotconInfoObj.uri = CONV_URI_REMOTE_APP_CONTROL;
		svcInfo->iotconInfoObj.resourceType = __resourceType;

		//save service info
		clientObj->addServiceInfo(__type, deviceId, (IServiceInfo*)svcInfo);

		_D("remote app control service is created");
	}

	requestObj->setServiceInfo(svcInfo);
	return CONV_ERROR_NONE;
}

int conv::RemoteAppControlServiceProvider::getServiceInfoForDiscovery(Json* jsonObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_NOT_SUPPORTED, _E, "service provider is not activated");

	jsonObj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_TYPE, CONV_SERVICE_REMOTE_APP_CONTROL);

	// set data for service handle
	Json info;
	info.set(NULL, CONV_SERVICE_ID, __uri);
	info.set(NULL, CONV_SERVICE_VERSION, "1.0");

	jsonObj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_INFO, info);

	return CONV_ERROR_NONE;
}
