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

#include "service_mgr_impl.h"
#include "service_provider/app_comm_service_provider.h"
#include "service_provider/remote_app_control_service_provider.h"
#include "connection_mgr_impl.h"
#include "access_control/privilege.h"
#include "util.h"
#include <iotcon.h>

static conv::service_manager_impl *_instance;
static iotcon_resource_h iotcon_resource = NULL;

using namespace std;

conv::service_manager_impl::service_manager_impl()
{
}

conv::service_manager_impl::~service_manager_impl()
{
}

int conv::service_manager_impl::handle_vconf_update(keynode_t *node)
{
	activation_state = vconf_keynode_get_int(node);

	if ( activation_state == 1 ) {
		register_discovery_info();
	} else {
		unregister_discovery_info();
	}

	return CONV_ERROR_NONE;
}

static void vconf_update_cb(keynode_t *node, void* user_data)
{
	conv::service_manager_impl* instance = static_cast<conv::service_manager_impl*>(user_data);
	IF_FAIL_VOID_TAG(instance, _E, "static_cast failed");

	instance->handle_vconf_update(node);
}

int conv::service_manager_impl::init()
{
	register_provider(new(std::nothrow) conv::app_comm_service_provider());
	register_provider(new(std::nothrow) conv::remote_app_control_service_provider());

	/*int error = vconf_get_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE, &activation_state);
	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);*/

	// temporary code
	activation_state = 1;

	if ( activation_state == 1 ) {
		register_discovery_info();
	} else {
		unregister_discovery_info();
	}

	/*error = vconf_notify_key_changed(VCONFKEY_SETAPPL_D2D_CONVERGENCE, vconf_update_cb, this);
	IF_FAIL_RETURN_TAG(error >= 0, CONV_ERROR_INVALID_OPERATION, _E, "vconf error (%d)", error);*/

	return CONV_ERROR_NONE;
}

int conv::service_manager_impl::release()
{
	unregister_discovery_info();

	for (service_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		(*it)->release();
	}

	for (service_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		delete *it;
	}

	provider_list.clear();

	return CONV_ERROR_NONE;
}

int conv::service_manager_impl::handle_request(request* request_obj)
{
	_D("handle_request called");
	string type;
	int error = CONV_ERROR_INVALID_OPERATION;
	bool json_return;

	json description = request_obj->get_description();
	json_return = description.get(NULL, CONV_JSON_TYPE, &type);

	IF_FAIL_CATCH_TAG(json_return, _E, "json parse error : no type info");

	for (service_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		if ( (*it)->get_type().compare(type) == 0 )
		{
			_D("found service provider");
			error = (*it)->check_activation_state();
			IF_FAIL_CATCH_TAG(error == CONV_ERROR_NONE, _E, "service provider is not activated");

			error = (*it)->load_service_info(request_obj);
			IF_FAIL_CATCH_TAG(error == CONV_ERROR_NONE, _E, "%d, service_info load error", error);

			if (!strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_START)) {
				if ( !conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) ||
						!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_D2D_DATA_SHARING) )
				{
					_E("permission denied");
					request_obj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete request_obj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->start_request(request_obj);
			} else if (!strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_STOP)) {
				if ( !conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) )
				{
					_E("permission denied");
					request_obj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete request_obj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->stop_request(request_obj);
			} else if (!strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_GET)) 	{
				if ( !conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) )
				{
					_E("permission denied");
					request_obj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete request_obj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->get_request(request_obj);
			} else if (!strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_SET)) {
				if ( !conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
						!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) ||
						!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_D2D_DATA_SHARING) )
				{
					_E("permission denied");
					request_obj->reply(CONV_ERROR_PERMISSION_DENIED);
					delete request_obj;
					return CONV_ERROR_PERMISSION_DENIED;
				}
				error = (*it)->set_request(request_obj);
			} else if (!strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_RECV)) {
				return (*it)->register_request(request_obj);
			}
			IF_FAIL_CATCH_TAG(error == CONV_ERROR_NONE, _E, "service manager request handle error");
		}
	}

	request_obj->reply(CONV_ERROR_NONE);
	delete request_obj;
	_D("request_obj deleted");
	return CONV_ERROR_NONE;

CATCH:
	request_obj->reply(error);
	delete request_obj;

	return error;
}

void conv::service_manager::set_instance(conv::service_manager_impl* mgr)
{
	_instance = mgr;
}

int conv::service_manager::handle_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_instance, CONV_ERROR_INVALID_PARAMETER, _E, "Not initialized");
	_instance->handle_request(request_obj);

	return CONV_ERROR_NONE;
}

int conv::service_manager_impl::register_provider(conv::service_provider_base *provider)
{
	if (!provider) {
		_E("Provider NULL");
		return CONV_ERROR_INVALID_PARAMETER;
	}

	if (provider->init() != CONV_ERROR_NONE) {
		_E("Provider initialization failed");
		delete provider;
		return CONV_ERROR_INVALID_OPERATION;
	}

	provider_list.push_back(provider);

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


static iotcon_representation_h _get_d2d_service_representation(conv::service_manager_impl* instance)
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

	char* device_id = (char*) conv::util::get_device_id().c_str();
	char* device_name = (char*) conv::util::get_device_name().c_str();

	iotcon_attributes_add_str(attributes, CONV_JSON_DEVICE_ID, device_id);
	iotcon_attributes_add_str(attributes, CONV_JSON_DEVICE_NAME, device_name);
#ifdef _TV_
	string device_type("TV");
#else
	string device_type("MOBILE");
#endif
	iotcon_attributes_add_str(attributes, CONV_JSON_DEVICE_TYPE, (char*) device_type.c_str());

	json service_json;
	instance->get_service_info_for_discovery(&service_json);
	char* service_json_char = service_json.dup_cstr();

	iotcon_attributes_add_str(attributes, "service_json", service_json_char);

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

int conv::service_manager_impl::get_service_info_for_discovery(json* service_json)
{
	IF_FAIL_RETURN_TAG(service_json, CONV_ERROR_INVALID_OPERATION, _E, "service_json is NULL");

	for (service_provider_list_t::iterator it = provider_list.begin(); it != provider_list.end(); ++it) {
		json service_info;
		if ((*it)->get_service_info_for_discovery(&service_info) == CONV_ERROR_NONE) {
			service_json->array_append(NULL, "service_list", service_info);
		}
	}

	_D("service_info : %s", service_json->str().c_str());

	return CONV_ERROR_NONE;
}

static void iotcon_request_cb(iotcon_resource_h resource, iotcon_request_h request, void *user_data)
{
	_D("request cb called");

	int ret;
	iotcon_request_type_e type;
	char *host_address;

	conv::service_manager_impl *instance = (conv::service_manager_impl*)user_data;

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

int conv::service_manager_impl::register_discovery_info()
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

	error = iotcon_resource_create(CONV_URI_SMARTVIEW_TIZEN_D2D_SERVICE, resource_types, resource_ifaces, properties, iotcon_request_cb, this, &iotcon_resource);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource creation failed");

	_D("device info registered as resource");
	iotcon_resource_types_destroy(resource_types);
	iotcon_resource_interfaces_destroy(resource_ifaces);

	return CONV_ERROR_NONE;
}

int conv::service_manager_impl::unregister_discovery_info()
{
	IF_FAIL_RETURN_TAG(iotcon_resource != NULL, CONV_ERROR_INVALID_PARAMETER, _E, "resource for discovery is already unregistered");

	int error = iotcon_resource_destroy(iotcon_resource);
	IF_FAIL_RETURN_TAG(error == IOTCON_ERROR_NONE, CONV_ERROR_INVALID_OPERATION, _E, "resource destroy failed");
	iotcon_resource = NULL;


	_D("device info unregistered");
	return CONV_ERROR_NONE;
}

