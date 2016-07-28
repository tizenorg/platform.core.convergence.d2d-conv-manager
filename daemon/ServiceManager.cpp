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

#include "ServiceManager.h"
#include "service_provider/AppCommServiceProvider.h"
#include "service_provider/RemoteAppControlServiceProvider.h"
#include "access_control/Privilege.h"
#include "Util.h"
#include <iotcon.h>

static conv::ServiceManager *_instance;
static iotcon_resource_h iotcon_resource = NULL;

using namespace std;

conv::ServiceManager::ServiceManager()
{
}

conv::ServiceManager::~ServiceManager()
{
}

int conv::ServiceManager::handleVconfUpdate(keynode_t *node)
{
	__activationState = vconf_keynode_get_int(node);

	if ( __activationState == 1 ) {
		registerDiscoveryInfo();
	} else {
		unregisterDiscoveryInfo();
	}

	return CONV_ERROR_NONE;
}

static void vconf_update_cb(keynode_t *node, void* user_data)
{
	conv::ServiceManager* instance = static_cast<conv::ServiceManager*>(user_data);
	IF_FAIL_VOID_TAG(instance, _E, "static_cast failed");

	instance->handleVconfUpdate(node);
}

int conv::ServiceManager::init()
{
	registerProvider(new(std::nothrow) conv::AppCommServiceProvider());
	registerProvider(new(std::nothrow) conv::RemoteAppControlServiceProvider());

	int error = vconf_get_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE, &__activationState);

	if ( error < 0 ) {
		_E("vconf error (%d)", error);
//		temporary code for binary without vconf
		__activationState = 1;
	}

	if ( __activationState == 1 ) {
		registerDiscoveryInfo();
	} else {
		unregisterDiscoveryInfo();
	}

	error = vconf_notify_key_changed(VCONFKEY_SETAPPL_D2D_CONVERGENCE, vconf_update_cb, this);
//	temporarily commented out for binary without vconf
//	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);

	return CONV_ERROR_NONE;
}

int conv::ServiceManager::release()
{
	unregisterDiscoveryInfo();

	for (ServiceProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it) {
		(*it)->release();
	}

	for (ServiceProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it) {
		delete *it;
	}

	__providerList.clear();

	return CONV_ERROR_NONE;
}

int conv::ServiceManager::handleRequest(Request* requestObj)
{
	_D("handle_request called");
	string type;
	int error = CONV_ERROR_INVALID_OPERATION;
	bool result;

	Json description = requestObj->getDescription();
	result = description.get(NULL, CONV_JSON_TYPE, &type);

	IF_FAIL_CATCH_TAG(result, _E, "json parse error : no type info");

	for (ServiceProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it) {
		if ( (*it)->getType().compare(type) == 0 )
		{
			_D("found service provider");
			error = (*it)->checkActivationState();
			IF_FAIL_CATCH_TAG(error == CONV_ERROR_NONE, _E, "service provider is not activated");

			error = (*it)->loadServiceInfo(requestObj);
			IF_FAIL_CATCH_TAG(error == CONV_ERROR_NONE, _E, "%d, service_info load error", error);

			if (!strcmp(requestObj->getSubject(), CONV_SUBJECT_COMMUNICATION_START)) {
				if ( !conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_BLUETOOTH) ||
						!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_D2D_DATA_SHARING) )
				{
					_E("permission denied");
					requestObj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete requestObj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->startRequest(requestObj);
			} else if (!strcmp(requestObj->getSubject(), CONV_SUBJECT_COMMUNICATION_STOP)) {
				if ( !conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_BLUETOOTH) )
				{
					_E("permission denied");
					requestObj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete requestObj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->stopRequest(requestObj);
			} else if (!strcmp(requestObj->getSubject(), CONV_SUBJECT_COMMUNICATION_GET)) 	{
				if ( !conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_BLUETOOTH) )
				{
					_E("permission denied");
					requestObj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete requestObj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->readRequest(requestObj);
			} else if (!strcmp(requestObj->getSubject(), CONV_SUBJECT_COMMUNICATION_SET)) {
				if ( !conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_BLUETOOTH) ||
						!conv::privilege_manager::isAllowed(requestObj->getCreds(), CONV_PRIVILEGE_D2D_DATA_SHARING) )
				{
					_E("permission denied");
					requestObj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete requestObj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->publishRequest(requestObj);
			} else if (!strcmp(requestObj->getSubject(), CONV_SUBJECT_COMMUNICATION_RECV)) {
				return (*it)->registerRequest(requestObj);
			}
			IF_FAIL_CATCH_TAG(error == CONV_ERROR_NONE, _E, "service manager request handle error");
		}
	}

	requestObj->reply(CONV_ERROR_NONE);
	delete requestObj;
	_D("requestObj deleted");
	return CONV_ERROR_NONE;

CATCH:
	requestObj->reply(error);
	delete requestObj;

	return error;
}

void conv::service_manager::setInstance(conv::ServiceManager* mgr)
{
	_instance = mgr;
}

int conv::service_manager::handleRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(_instance, CONV_ERROR_INVALID_PARAMETER, _E, "Not initialized");
	_instance->handleRequest(requestObj);

	return CONV_ERROR_NONE;
}

int conv::ServiceManager::registerProvider(conv::IServiceProvider *provider)
{
	if (!provider) {
		_E("Provider NULL");
		return CONV_ERROR_INVALID_PARAMETER;
	}

	if (provider->checkActivationState() == CONV_ERROR_NONE) {
		if (provider->init() != CONV_ERROR_NONE) {
			_E("Provider initialization failed");
			delete provider;
			return CONV_ERROR_INVALID_OPERATION;
		}
	} else {
		_D("provider is not activated. init pending");
	}

	__providerList.push_back(provider);

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


static iotcon_representation_h _get_d2d_service_representation(conv::ServiceManager* instance)
{
	int ret;
	iotcon_attributes_h attributes;
	iotcon_representation_h repr;

	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_representation_create() Fail(%d)", ret);
		return NULL;
	}

	ret = iotcon_attributes_create(&attributes);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_attributes_create() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return NULL;
	}

	char* deviceId = (char*) conv::util::getDeviceId().c_str();
	char* deviceName = (char*) conv::util::getDeviceName().c_str();

	iotcon_attributes_add_str(attributes, CONV_JSON_DEVICE_ID, deviceId);
	iotcon_attributes_add_str(attributes, CONV_JSON_DEVICE_NAME, deviceName);
#ifdef _TV_
	string device_type("TV");
#else
	string device_type("MOBILE");
#endif
	iotcon_attributes_add_str(attributes, CONV_JSON_DEVICE_TYPE, (char*) device_type.c_str());

	conv::Json serviceJson;
	instance->getServiceInfoForDiscovery(&serviceJson);
	char* service_json_char = serviceJson.dupCstr();

	iotcon_attributes_add_str(attributes, "serviceJson", service_json_char);

	ret = iotcon_representation_set_attributes(repr, attributes);
	g_free(service_json_char);
	if (IOTCON_ERROR_NONE != ret) {
		_E("iotcon_representation_set_attributes() Fail(%d)", ret);
		iotcon_attributes_destroy(attributes);
		iotcon_representation_destroy(repr);
		return NULL;
	}

	iotcon_attributes_destroy(attributes);

	return repr;
}

int conv::ServiceManager::getServiceInfoForDiscovery(Json* serviceJson)
{
	IF_FAIL_RETURN_TAG(serviceJson, CONV_ERROR_INVALID_OPERATION, _E, "serviceJson is NULL");

	for (ServiceProviderList::iterator it = __providerList.begin(); it != __providerList.end(); ++it) {
		Json serviceInfo;
		if ((*it)->getServiceInfoForDiscovery(&serviceInfo) == CONV_ERROR_NONE) {
			serviceJson->appendArray(NULL, "service_list", serviceInfo);
		}
	}

	_D("service_info : %s", serviceJson->str().c_str());

	return CONV_ERROR_NONE;
}

static void iotcon_request_cb(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
{
	_D("request cb called");

	int ret;
	iotcon_request_type_e type;
	char *host_address;

	conv::ServiceManager *instance = (conv::ServiceManager*)user_data;

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

	if (IOTCON_REQUEST_GET == type) {
		iotcon_representation_h resp_repr;
		_I("GET request");

		resp_repr = _get_d2d_service_representation(instance);
		if (resp_repr == NULL) {
			_I("_get_d2d_service_representation() Fail");
			ret = CONV_ERROR_INVALID_OPERATION;
		} else {
			ret = _send_response(request, resp_repr, IOTCON_RESPONSE_OK);
			if (ret != IOTCON_ERROR_NONE) {
				_E("_send_response() Fail(%d)", ret);
				iotcon_representation_destroy(resp_repr);
				return;
			}

			iotcon_representation_destroy(resp_repr);
		}
	}

	if (IOTCON_ERROR_NONE != ret) {
		_send_response(request, NULL, IOTCON_RESPONSE_ERROR);
		return;
	}
}

int conv::ServiceManager::registerDiscoveryInfo()
{
	IF_FAIL_RETURN_TAG(iotcon_resource == NULL, CONV_ERROR_INVALID_PARAMETER, _E, "resource for discovery is already registered");

	// register resource
	int properties;
	iotcon_resource_interfaces_h resource_ifaces = NULL;
	iotcon_resource_types_h resource_types = NULL;
	int error;

	properties = IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE;

	error = iotcon_resource_types_create(&resource_types);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "rt creation failed");

	iotcon_resource_types_add(resource_types, CONV_RESOURCE_TYPE_TIZEN_D2D_SERVICE);

	error = iotcon_resource_interfaces_create(&resource_ifaces);

	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "ri creation failed");

	iotcon_resource_interfaces_add(resource_ifaces, IOTCON_INTERFACE_DEFAULT);

	error = iotcon_resource_create(CONV_URI_TIZEN_D2D_SERVICE, resource_types, resource_ifaces, properties, iotcon_request_cb, this, &iotcon_resource);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource creation failed");

	_D("device info registered as resource");
	iotcon_resource_types_destroy(resource_types);
	iotcon_resource_interfaces_destroy(resource_ifaces);

	return CONV_ERROR_NONE;
}

int conv::ServiceManager::unregisterDiscoveryInfo()
{
	IF_FAIL_RETURN_TAG(iotcon_resource != NULL, CONV_ERROR_INVALID_PARAMETER, _E, "resource for discovery is already unregistered");

	int error = iotcon_resource_destroy(iotcon_resource);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource destroy failed");
	iotcon_resource = NULL;


	_D("device info unregistered");
	return CONV_ERROR_NONE;
}

