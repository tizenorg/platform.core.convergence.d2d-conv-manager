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

#include "remote_interaction_service_provider.h"

#include <string>
#include <net_connection.h>
#include <aul.h>
#include <aul_svc.h>
#include <bundle.h>
#include <tzplatform_config.h>
#include <curl/curl.h>
#include <app_manager.h>
#include <package_manager.h>

#define APP_FOCUS 5
#define CONTROLLER_APP_ID "org.tizen.remote-controller"

using namespace std;

conv::remote_interaction_service_provider::remote_interaction_service_provider()
{
	_type = CONV_SERVICE_TYPE_REMOTE_INTERACTION;
}

conv::remote_interaction_service_provider::~remote_interaction_service_provider()
{

}

int conv::remote_interaction_service_provider::init()
{
	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::release()
{
	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::load_service_info(request* request_obj)
{
	return CONV_ERROR_NONE;
}

/* Luanch Controller Hidden Application */
/*
int launch_controller(const char *uri)
{
int err = TIZEN_ERROR_NONE;

app_control_h my_service = NULL;

err = app_control_create(&my_service);
if (err != APP_CONTROL_ERROR_NONE)
{
return CONV_ERROR_INVALID_OPERATION;
}

err = app_control_set_app_id(my_service, CONTROLLER_APP_ID);
if (err != APP_CONTROL_ERROR_NONE)
{
app_control_destroy(my_service);
return CONV_ERROR_INVALID_OPERATION;
}

err = app_control_set_uri(my_service, uri);
if (err != APP_CONTROL_ERROR_NONE)
{
app_control_destroy(my_service);
return CONV_ERROR_INVALID_OPERATION;
}

err = app_control_send_launch_request(my_service, NULL, NULL);
if (err != APP_CONTROL_ERROR_NONE)
{
app_control_destroy(my_service);
return CONV_ERROR_INVALID_OPERATION;
}

app_control_destroy(my_service);

return err;
}
*/

/* Luanch Browser (Temporary) */
int launch_controller(const char *uri)
{	
	int ret;

	bundle *pBundle = bundle_create();

	if (BUNDLE_ERROR_NONE != bundle_add_str(pBundle, "AppId", "org.tizen.browser")) 
	{
		ret = bundle_free(pBundle);
		if (ret != BUNDLE_ERROR_NONE)
		{
			_E("bundle_free function worng...");
		}
		return CONV_ERROR_INVALID_OPERATION;
	}

	ret = aul_svc_set_uri(pBundle, uri);
	if (ret < 0)
	{
		ret = bundle_free(pBundle);
		if(ret != BUNDLE_ERROR_NONE)
		{
			_E("bundle_free function worng...");
		}
		return CONV_ERROR_INVALID_OPERATION;
	}

	ret = aul_app_is_running_for_uid("org.tizen.browser", tzplatform_getuid(TZ_SYS_DEFAULT_USER));
	if (ret != true)
	{
		ret = aul_launch_app_for_uid("org.tizen.browser", pBundle, tzplatform_getuid(TZ_SYS_DEFAULT_USER));
		if (ret != AUL_R_OK)
		{
			_E("aul_launch_app_for_uid function worng...");

			ret = bundle_free(pBundle);
			if (ret != BUNDLE_ERROR_NONE)
			{
				_E("bundle_free function worng...");
			}
			return CONV_ERROR_INVALID_OPERATION;
		}		
	} 
	else
	{
		ret = aul_resume_app_for_uid("org.tizen.browser", tzplatform_getuid(TZ_SYS_DEFAULT_USER));	
		if (ret != AUL_R_OK)
		{
			_E("aul_resume_app_for_uid function worng...");

			ret = bundle_free(pBundle);
			if (ret != BUNDLE_ERROR_NONE)
			{
				_E("bundle_free function worng...");
			}
			return CONV_ERROR_INVALID_OPERATION;
		}
	}

	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::start_request(request* request_obj)
{	
	json description = request_obj->get_description();
	json service;

	string uri;

	description.get(NULL, CONV_JSON_SERVICE, &service);
	service.get(NULL, CONV_SERVICE_ID, &uri);	

	int ret = launch_controller(uri.c_str());
	if (ret != CONV_ERROR_NONE)
	{
		_E("launch_controller function worng...");
		return CONV_ERROR_INVALID_OPERATION;
	}
	return CONV_ERROR_NONE;
}

/* Terminate Controller Hidden Application */
/*
int terminate_controller()
{
int err = CONV_ERROR_NONE;

app_control_h m_service;

err = app_control_create(&m_service);
if (err != APP_CONTROL_ERROR_NONE)
{
return CONV_ERROR_INVALID_OPERATION;
}

err = app_control_set_app_id(m_service, CONTROLLER_APP_ID);
if (err != APP_CONTROL_ERROR_NONE)
{
app_control_destroy(m_service);
return CONV_ERROR_INVALID_OPERATION;
}

err = app_control_send_terminate_request(m_service);
if (err != APP_CONTROL_ERROR_NONE)
{
app_control_destroy(m_service);
return CONV_ERROR_INVALID_OPERATION;
}

app_control_destroy(m_service);

return err;
}
*/

/* Terminate Browser (Temporary) */
int terminate_controller()
{
	int pid = aul_app_get_pid_for_uid("org.tizen.browser", tzplatform_getuid(TZ_SYS_DEFAULT_USER));

	if (pid < 0) 
	{
		_E("aul_app_get_pid_for_uid function worng...");
		return CONV_ERROR_INVALID_OPERATION;
	} 
	else 
	{	
		int ret = aul_terminate_pid_for_uid(pid, tzplatform_getuid(TZ_SYS_DEFAULT_USER));		
		if (ret < 0) 
		{
			_E("aul_terminate_pid_for_uid function worng...");
			return CONV_ERROR_INVALID_OPERATION;
		}
	}

	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::stop_request(request* request_obj)
{
	int ret = terminate_controller();
	if (ret != CONV_ERROR_NONE)
	{
		_E("terminate_controller function worng...");
		return CONV_ERROR_INVALID_OPERATION;
	}
	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::get_request(request* request_obj)
{
	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::set_request(request* request_obj)
{
	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::register_request(request* request_obj)
{
	return CONV_ERROR_NONE;
}

int send_message_to_local_node(char* message, char* node_uri)
{
	CURL *curl_handle = curl_easy_init();
	CURLcode res;

	if (curl_handle == NULL)
	{
		_E("curl_easy_init function worng...");
		return CONV_ERROR_INVALID_OPERATION;
	}

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");

	string json_message = "{\"message\" : \"" + (string)message + "\"}";
	
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_URL, node_uri);
	curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json_message.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, json_message.length());
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 1000);

	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK)
	{
		_E("Remote-Interaction Server is not exists...");
		curl_easy_cleanup(curl_handle);
		return CONV_ERROR_INVALID_OPERATION;
	}

	curl_easy_cleanup(curl_handle);

	return CONV_ERROR_NONE;
}

static int app_change_event_listen_cb(int pid, int status, void *data)
{
	if (status == APP_FOCUS)
	{
		char appid[255];

		int ret = aul_app_get_appid_bypid_for_uid(pid, appid, sizeof(appid), tzplatform_getuid(TZ_SYS_DEFAULT_USER));

		if (ret != 0)
		{
			_E("aul_app_get_appid_bypid_for_uid function worng...");
			return CONV_ERROR_INVALID_OPERATION;
		}

		ret = send_message_to_local_node(appid, "http://127.0.0.1:8001/app_change_checker");

		if (ret != CONV_ERROR_NONE)
		{
			_E("send_message_to_local_node function worng...");
			return CONV_ERROR_INVALID_OPERATION;
		}
	}
	return CONV_ERROR_NONE;
}

int app_change_event_listen()
{
	int ret = aul_listen_app_status_signal(app_change_event_listen_cb, NULL);

	if (ret != 0)
	{
		_E("aul_listen_app_status_signal function worng...");
		return CONV_ERROR_INVALID_OPERATION;
	}

	return CONV_ERROR_NONE;
}

void app_install_and_uninstall_event_listen_cb(const char *type, const char *package, package_manager_event_type_e event_type, package_manager_event_state_e event_state, int progress, package_manager_error_e error, void *user_data)
{
	if (event_state == PACKAGE_MANAGER_EVENT_STATE_COMPLETED)
	{
		int ret = send_message_to_local_node("true", "http://127.0.0.1:8001/packaging_checker");
		if (ret != CONV_ERROR_NONE)
		{
			_E("send_message_to_local_node function worng...");
		}
	}
}

int app_install_and_uninstall_event_listen()
{
	package_manager_h manager;

	int ret = package_manager_create(&manager);
	if (ret != PACKAGE_MANAGER_ERROR_NONE)
	{
		_E("package_manager_create function worng...");
		return CONV_ERROR_INVALID_OPERATION;
	}

	ret = package_manager_set_event_cb(manager, app_install_and_uninstall_event_listen_cb, NULL);
	if (ret != PACKAGE_MANAGER_ERROR_NONE)
	{
		_E("package_manager_set_event_cb function worng...");
		package_manager_destroy(manager);
		return CONV_ERROR_INVALID_OPERATION;
	}

	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::get_service_info_for_discovery(json* json_obj)
{
	json_obj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_TYPE, CONV_SERVICE_REMOTE_INTERACTION);

	connection_h connection;
	IF_FAIL_RETURN_TAG(connection_create(&connection) == CONNECTION_ERROR_NONE, CONV_ERROR_NOT_SUPPORTED, _E, "connection error");

	char* address = NULL;
	int ret = connection_get_ip_address(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &address);
	if (ret != CONNECTION_ERROR_NONE)
	{
		_E("connection_get_ip_address IPV4 function worng");
		if (connection_get_ip_address(connection, CONNECTION_ADDRESS_FAMILY_IPV6, &address) != CONNECTION_ERROR_NONE)
		{
			_E("connection_get_ip_address IPV6 function worng");
			connection_destroy(connection);
			return CONV_ERROR_NOT_SUPPORTED;
		}
	}

	if (address == NULL || strlen(address) < 1)
	{
		_E("address is worng");
		connection_destroy(connection);
		return CONV_ERROR_NOT_SUPPORTED;
	} 
	else 
	{
		char uri[1000];
		sprintf(uri, "http://%s:8001/remote_interaction", address);

		ret = send_message_to_local_node("isExistNode", uri);
		if (ret != CONV_ERROR_NONE) {	
			_E("send_message_to_local_node function worng");
			return CONV_ERROR_NOT_SUPPORTED;
		}
		
		ret = app_change_event_listen();
		if (ret != CONV_ERROR_NONE) {
			_E("app_change_event_listen function worng");
			return CONV_ERROR_NOT_SUPPORTED;
		}
		
		ret = app_install_and_uninstall_event_listen();
		if (ret != CONV_ERROR_NONE) {
			_E("app_install_and_uninstall_event_listen function worng");
			return CONV_ERROR_NOT_SUPPORTED;
		}

		json info;
		info.set(NULL, CONV_SERVICE_ID, uri);

		// info.set(NULL, CONV_JSON_SERVICE_DATA_VERSION, "1.0");
		// info.set(NULL, CONV_JSON_SERVICE_DATA_TYPE, "TYPE");		
		json_obj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_INFO, info);		

		g_free(address);
	}
	
	connection_destroy(connection);

	return CONV_ERROR_NONE;
}

int conv::remote_interaction_service_provider::send_response(json payload, request* request_obj)
{
	return CONV_ERROR_NONE;
}