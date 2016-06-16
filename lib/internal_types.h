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

#ifndef __CONV_INTERNAL_TYPES_H__
#define __CONV_INTERNAL_TYPES_H__

#include <string>
#include <list>
#include "conv_lib_json.h"

typedef struct _conv_handle_s {
	std::list<int> request_ids;
} _conv_handle;

typedef struct service_callback_info_s {
	conv_service_listener_cb cb;
	void* user_data;
	conv_service_h handle;
	json description;
} _conv_service_callback_info;

//LCOV_EXCL_START
typedef struct service_connect_callback_info_s {
	conv_service_connected_cb cb;
	void* user_data;
	conv_service_h handle;
	json description;
} _conv_service_connect_callback_info;
//LCOV_EXCL_STOP

typedef struct _conv_channel_handle_s {
	json jchannel;
} _conv_channel_handle;

typedef struct _conv_payload_handle_s {
	json jpayload;
} _conv_payload_handle;

typedef struct _conv_service_handle_s {
	json jservice;
	conv_service_e service_type;
	bool is_local;
	conv_service_connection_state_e connection_state;
	json jdevice;
	_conv_service_callback_info* callback;
} _conv_service_handle;

//LCOV_EXCL_START
typedef struct _conv_device_handle_s {
	json jbody;
} _conv_device_handle;
//LCOV_EXCL_STOP

#endif
