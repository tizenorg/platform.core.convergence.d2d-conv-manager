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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <glib.h>
#include <glib-object.h>
#include "d2d_conv_manager.h"

#define CONV_JSON_DESCRIPTION		"description"
#define CONV_JSON_PAYLOAD		"payload"
#define CONV_JSON_SERVICE		"service"
#define CONV_JSON_CHANNEL		"channel"
#define CONV_JSON_IS_LOCAL		"is_local"
#define CONV_JSON_DEVICE		"device"
#define CONV_JSON_TYPE	"type"

#define CONV_JSON_SERVICE_PATH		"service"
#define CONV_JSON_SERVICE_DATA		"data"
#define CONV_JSON_SERVICE_TYPE		"type"

#define CONV_JSON_SERVICE_DATA_PATH		NULL
#define CONV_JSON_SERVICE_DATA_NAME		"name"
#define CONV_JSON_SERVICE_DATA_VERSION	"version"
#define CONV_JSON_SERVICE_DATA_TYPE		"type"
#define CONV_JSON_SERVICE_DATA_ID		"id"
#define CONV_JSON_SERVICE_DATA_URI		CONV_SERVICE_ID

#define CONV_JSON_DEVICE_ID			"device_id"
#define CONV_JSON_DEVICE_NAME		"device_name"
#define CONV_JSON_DEVICE_ADDRESS	"host_address"

struct arg_data {
	int type;
	//..
};

static GMainLoop *loop;

void simple_test_conv_service_cb(conv_service_h handle, conv_channel_h channel, int error, conv_payload_h result, void* user_data)
{
	printf("callback for service..\n");
	char* test;

	conv_payload_get_string(result, "result_type", &test);

	printf("result type : %s\n", test);

	if (!strcmp(test, "getClient")) {
		conv_service_h service_handle = (conv_service_h)user_data;

		printf("conv_service_stop called..\n");
		conv_service_stop(service_handle, channel, NULL);
		printf("conv_service_destroy called..\n");
		conv_service_destroy(service_handle);

		g_main_loop_quit(loop);
	}
}

void service_remote_app_control_foreach_cb(conv_service_h handle, void* user_data)
{
	printf("==============================\n");
	printf("service_foreach_cb called");

	if (handle != NULL) {
		char *version = NULL, *type = NULL, *uri = NULL;
		conv_service_e serv_type;

		conv_service_get_type(handle, &serv_type);

		printf("service type[%d]\n", serv_type);

		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_VERSION, &version);
		if (version != NULL) printf("service version[%s]", version);
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_TYPE, &type);
		if (type != NULL) printf(" type[%s]", type);
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_URI, &uri);
		if (uri != NULL) printf(" uri[%s]\n", uri);

		if (serv_type == CONV_SERVICE_REMOTE_APP_CONTROL) {

			printf("app control test start\n");
			conv_service_start(handle, NULL, NULL);
			printf("service start\n");
			conv_payload_h payload_handle;

			conv_payload_create(&payload_handle);

			app_control_h app_control = NULL;
			app_control_create(&app_control);
			app_control_set_app_id(app_control, "org.tizen.browser");
			app_control_set_operation(app_control, APP_CONTROL_OPERATION_MAIN);

			printf("conv_payload_add_string called..\n");
			conv_payload_set_app_control(payload_handle, "app_control", app_control);
			conv_payload_set_string(payload_handle, "reply", "0");
			printf("conv_service_publish called..\n");
			conv_service_publish(handle, NULL, payload_handle);

			sleep(3);

			printf("conv_service_stop called..\n");
			conv_service_stop(handle, NULL, NULL);

			printf("conv_payload_destroy called..\n");
			conv_payload_destroy(payload_handle);

		}
		printf("next\n");
	}

}

void service_app_communication_foreach_cb(conv_service_h handle, void* user_data)
{

	printf("==============================\n");
	printf("service_foreach_cb called");

	if (handle != NULL) {
		char *version = NULL, *type = NULL, *uri = NULL;
		conv_service_e serv_type;

		conv_service_get_type(handle, &serv_type);

		printf("service type[%d]\n", serv_type);

		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_VERSION, &version);
		if (version != NULL) printf("service version[%s]", version);
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_TYPE, &type);
		if (type != NULL) printf(" type[%s]", type);
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_URI, &uri);
		if (uri != NULL) printf(" uri[%s]\n", uri);

		if (serv_type == CONV_SERVICE_APP_TO_APP_COMMUNICATION) {
			conv_channel_h channel_handle;
			conv_payload_h payload_handle;

			printf("conv_channel_create called..\n");
			conv_channel_create(&channel_handle);

			printf("conv_payload_create called..\n");
			conv_payload_create(&payload_handle);

			printf("conv_channel_add called..\n");
			conv_channel_set_string(channel_handle, "uri", "http://www.tizen.org");
			conv_channel_set_string(channel_handle, "channel_id", "test_id");

			printf("set service cb..\n");
			conv_service_set_listener_cb(handle, simple_test_conv_service_cb, (void*)handle);

			printf("conv_service_start called..\n");
			conv_service_start(handle, channel_handle, NULL);

			sleep(5);

			printf("conv_payload_add_string called..\n");
			conv_payload_set_string(payload_handle, "event", "send_message");
			conv_payload_set_string(payload_handle, "data", "hello. This is d2d conv manager test");

			printf("conv_service_publish called..\n");
			conv_service_publish(handle, channel_handle, payload_handle);

			printf("conv_service_publish called..\n");
			conv_service_publish(handle, channel_handle, payload_handle);

			sleep(3);


			printf("conv_service_stop called..\n");
			conv_service_stop(handle, channel_handle, NULL);
/*
			printf("conv_service_get called..\n");
			conv_service_get(handle, channel_handle, NULL, NULL);
*/
			printf("conv_payload_destroy called..\n");
			conv_payload_destroy(payload_handle);

			printf("conv_channel_destroy called..\n");
			conv_channel_destroy(channel_handle);

		}
		printf("next\n");


	}

}

void service_foreach_cb(conv_service_h handle, void* user_data)
{
	printf("==============================\n");
	printf("service_foreach_cb called");

	if (handle != NULL) {
		char *version = NULL, *type = NULL, *uri = NULL;
		conv_service_e serv_type;
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_VERSION, &version);
		if (version != NULL) printf("service version[%s]", version);
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_TYPE, &type);
		if (type != NULL) printf(" type[%s]", type);
		conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_URI, &uri);
		if (uri != NULL) printf(" uri[%s]\n", uri);

		conv_service_get_type(handle, &serv_type);

		printf("service type[%d]\n", serv_type);
	}

}

void device_conv_remote_app_control_discovery_cb(conv_device_h device_h, int result, void* user_data)
{
	printf("Discovered!!!\n");
	char* device_id = NULL, *device_name = NULL;

	int ret;
	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_ID, &device_id);
	if (ret != CONV_ERROR_NONE || device_id == NULL) {
		printf("device id is null %s\n", device_id);
		return;
	}

	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_NAME, &device_name);
	if (ret != CONV_ERROR_NONE || device_name == NULL) {
		printf("device name is null %s\n", device_name);
		return;
	}

	printf("the discovered device's info : id[%s] name[%s]\n", device_id, device_name);
	ret = conv_device_foreach_service(device_h, service_remote_app_control_foreach_cb, user_data);
	if (ret != CONV_ERROR_NONE)
		printf("Error in conv_device_foreach_service\n");
}

void device_conv_app_communication_discovery_cb(conv_device_h device_h, int result, void* user_data)
{
	printf("Discovered!!!\n");
	char* device_id = NULL, *device_name = NULL;

	int ret;
	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_ID, &device_id);
	if (ret != CONV_ERROR_NONE || device_id == NULL) {
		printf("device id is null %s\n", device_id);
		return;
	}

	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_NAME, &device_name);
	if (ret != CONV_ERROR_NONE || device_name == NULL) {
		printf("device name is null %s\n", device_name);
		return;
	}

	printf("the discovered device's info : id[%s] name[%s]\n", device_id, device_name);
	ret = conv_device_foreach_service(device_h, service_app_communication_foreach_cb, user_data);
	if (ret != CONV_ERROR_NONE)
		printf("Error in conv_device_foreach_service\n");
}

void device_conv_discovery_cb(conv_device_h device_h, int result, void* user_data)
{
	printf("Discovered!!!\n");
	char* device_id = NULL, *device_name = NULL, *device_address;

	int ret;
	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_ID, &device_id);
	if (ret != CONV_ERROR_NONE || device_id == NULL) {
		printf("device id is null %s\n", device_id);
		return;
	}

	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_NAME, &device_name);
	if (ret != CONV_ERROR_NONE || device_name == NULL) {
		printf("device name is null %s\n", device_name);
		return;
	}

	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_ADDRESS, &device_address);
	if (ret != CONV_ERROR_NONE || device_name == NULL) {
		printf("device name is null %s\n", device_name);
		return;
	}

	printf("the discovered device's info : id[%s] name[%s] address[%s]\n", device_id, device_name, device_address);
	ret = conv_device_foreach_service(device_h, service_foreach_cb, user_data);
	if (ret != CONV_ERROR_NONE)
		printf("Error in conv_device_foreach_service\n");
}

void simple_test_conv_discovery_cb(conv_device_h device_h, int result, void* user_data)
{
	conv_channel_h channel_handle;
	conv_payload_h payload_handle;
	conv_service_h service_handle;

	printf("callback for discovery..\n");

	service_handle = (conv_service_h)user_data;

	printf("conv_channel_create called..\n");
	conv_channel_create(&channel_handle);

	printf("conv_payload_create called..\n");
	conv_payload_create(&payload_handle);

	printf("conv_channel_add called..\n");
	conv_channel_set_string(channel_handle, "uri", "http://www.tizen.org");
	conv_channel_set_string(channel_handle, "channel_id", "test_id");

	printf("set service cb..\n");
	conv_service_set_listener_cb(service_handle, simple_test_conv_service_cb, NULL);

	printf("conv_service_start called..\n");
	conv_service_start(service_handle, channel_handle, NULL);

	sleep(10);

}

void simple_test_conv_remote_app_control_start_stop_discovery()
{
	conv_h handle;

	printf("conv_create called..\n");
	conv_create(&handle);

	printf("conv_discovery_start called..");
	conv_discovery_start(handle, 10, device_conv_remote_app_control_discovery_cb, NULL);
}

void simple_test_conv_app_communication_start_stop_discovery()
{
	conv_h handle;

	printf("conv_create called..\n");
	conv_create(&handle);

	printf("conv_discovery_start called..");
	conv_discovery_start(handle, 10, device_conv_app_communication_discovery_cb, NULL);
}

// Simple Test.. Start/Stop Discovery
void simple_test_conv_start_stop_discovery()
{
	conv_h handle;

	printf("conv_create called..\n");
	conv_create(&handle);

/*
	printf("set discovery cb..\n");
	conv_set_discovery_cb(handle, device_conv_discovery_cb, NULL);
*/

	printf("conv_discovery_start called..\n");
	conv_discovery_start(handle, 10, device_conv_discovery_cb, NULL);

#if 0
	sleep(20);

	printf("conv_discovery_stop called..\n");
	conv_discovery_stop(handle);

	printf("conv_destroy called..\n");
	conv_destroy(handle);
#endif

}

void simple_test_remote_app_control_service_start_stop(char* address)
{
	if (address == NULL) {
		printf("no address entered\n");
		return;
	}

	conv_service_h service_handle;
	conv_payload_h payload_handle;

	printf("conv_service_create called..\n");
	conv_service_create(&service_handle);

	conv_service_set_property_string(service_handle, "id", "84:A4:66:BC:C9:86");
	conv_service_set_property_string(service_handle, CONV_SERVICE_ID, address);
	conv_service_set_property_string(service_handle, "name", "Tizen TV");
	conv_service_set_type(service_handle, CONV_SERVICE_REMOTE_APP_CONTROL);

	printf("conv_payload_create called..\n");
	conv_payload_create(&payload_handle);

	printf("set service cb..\n");
	conv_service_set_listener_cb(service_handle, simple_test_conv_service_cb, (void*)service_handle);

	printf("conv_service_start called..\n");
	conv_service_start(service_handle, NULL, NULL);

	sleep(5);

	app_control_h app_control = NULL;
	app_control_create(&app_control);
	app_control_set_app_id(app_control, "org.tizen.browser");
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_MAIN);

	printf("conv_payload_add_string called..\n");
	conv_payload_set_app_control(payload_handle, "app_control", app_control);
	conv_payload_set_string(payload_handle, "reply", 0);
	printf("conv_service_publish called..\n");
	conv_service_publish(service_handle, NULL, payload_handle);

	sleep(3);

	printf("conv_service_stop called..\n");
	conv_service_stop(service_handle, NULL, NULL);

	printf("conv_payload_destroy called..\n");
	conv_payload_destroy(payload_handle);

}

void service_remote_interaction_foreach_cb(conv_service_h handle, void* user_data)
{
	printf("\n============================================\n");
	printf("service_foreach_cb called. \n");

	if (handle != NULL) {

		char *version = NULL, *type = NULL, *uri = NULL;
		conv_service_e serv_type;

		conv_service_get_type(handle, &serv_type);

		printf("service type[%d] : ", serv_type);

		if (serv_type == CONV_SERVICE_REMOTE_INTERACTION) {

			printf("REMOTE_INTERACTION_SERVICE\n");

			conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_VERSION, &version);
			if (version != NULL) printf("service version[%s]", version);

			conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_TYPE, &type);
			if (type != NULL) printf(" type[%s]", type);

			conv_service_get_property_string(handle, CONV_JSON_SERVICE_DATA_URI, &uri);
			if (uri != NULL) printf(" uri[%s]\n", uri);

			conv_channel_h channel_handle;
			conv_channel_create(&channel_handle);
			conv_service_start(handle, channel_handle, NULL);
			sleep(15);
			conv_service_stop(handle, channel_handle, NULL);
			conv_channel_destroy(channel_handle);
		}
	}
}

void device_conv_remote_interaction_discovery_cb(conv_device_h device_h, int result, void* user_data)
{
	printf("Discovered!!!\n");
	char* device_id = NULL, *device_name = NULL, *device_address;

	int ret;
	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_ID, &device_id);
	if (ret != CONV_ERROR_NONE || device_id == NULL) {
		printf("device id is null %s\n", device_id);
	}

	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_NAME, &device_name);
	if (ret != CONV_ERROR_NONE || device_name == NULL) {
		printf("device name is null %s\n", device_name);
	}

	ret = conv_device_get_property_string(device_h, CONV_JSON_DEVICE_ADDRESS, &device_address);
	if (ret != CONV_ERROR_NONE || device_name == NULL) {
		printf("device name is null %s\n", device_name);
	}

	ret = conv_device_foreach_service(device_h, service_remote_interaction_foreach_cb, user_data);
	if (ret != CONV_ERROR_NONE) {
		printf("Error in conv_device_foreach_service\n");
	}
}

void simple_test_conv_remote_interaction_start_stop_discovery()
{
	conv_h handle;

	printf("conv_service_create called..\n");
	conv_create(&handle);

	printf("conv_service_start called..\n");
	conv_discovery_start(handle, 10, device_conv_remote_interaction_discovery_cb, NULL);
}

int main(int argc, char** argv)
{
	loop = g_main_loop_new(NULL, FALSE);

	if (argc > 1) {
		int type = atoi(argv[1]);
		struct arg_data* args = g_slice_alloc(sizeof(*args));
		args->type = type;

		switch (type) {
		case 0:
			simple_test_conv_start_stop_discovery();
			break;
		case 1:
			simple_test_conv_app_communication_start_stop_discovery();
			break;
		case 2:
			simple_test_conv_remote_app_control_start_stop_discovery();
			break;
		case 3:
			simple_test_conv_remote_interaction_start_stop_discovery();
			break;
		default:
			break;
		};
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);


	return 0;
}
