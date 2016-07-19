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

#ifndef __CONV_COMMON_H__
#define __CONV_COMMON_H__

#include <stdlib.h>
#include <tzplatform_config.h>
#include "log.h"
#include "d2d_conv_manager.h"

#define CONV_DATA "ConvHeader"
#define CONV_OPTION "ConvOption"
#define CONV_RESPONSE "ConvResponse"

#define CONV_IOTCON_FILEPATH tzplatform_mkpath(TZ_USER_SHARE, "d2d-conv-manager/d2d-conv-manager-iotcon-server.dat")

/* DBus */
#define DBUS_DEST		"org.tizen.d2dconv"
#define DBUS_PATH		"/org/tizen/d2dconv"
#define DBUS_IFACE		"org.tizen.d2dconv"
#define DBUS_TIMEOUT	3000000

#define METHOD_REQUEST	"Request"
#define METHOD_RESPOND	"Respond"
#define SIGNAL_LAUNCHED	"Launched"

#define ARG_REQTYPE		"type"
#define ARG_COOKIE		"cookie"
#define ARG_REQID		"req_id"
#define ARG_SUBJECT		"subject"
#define ARG_INPUT		"input"

#define ARG_RESULT_ERR	"r_err"
#define ARG_RESULT_ADD	"r_add"
#define ARG_OUTPUT		"output"

#define CONV_METHOD_REQUEST "CONV_Request"
#define CONV_METHOD_RESPOND "CONV_Respond"
#define CONV_ARG_KEY_RESULT "CONV_Result"

#define CONV_SUBJECT_DISCOVERY_START	"conv/discovery/start"
#define CONV_SUBJECT_DISCOVERY_STOP		"conv/discovery/stop"

#define CONV_SUBJECT_COMMUNICATION_RECV	"conv/communication/recv"
#define CONV_SUBJECT_COMMUNICATION_START	"conv/communication/start"
#define CONV_SUBJECT_COMMUNICATION_STOP	"conv/communication/stop"
#define CONV_SUBJECT_COMMUNICATION_GET	"conv/communication/get"
#define CONV_SUBJECT_COMMUNICATION_SET	"conv/communication/set"

#define CONV_SUBJECT_COMMUNICATION_WRITE	"conv/communication/write"
#define CONV_SUBJECT_COMMUNICATION_READ	"conv/communication/read"

#define CONV_SUBJECT_CONNECTION_START	"conv/connection/start"
#define CONV_SUBJECT_CONNECTION_STOP	"conv/connection/stop"

#define CONV_SERVICE_TYPE_SMARTVIEW_APP_COMMUNICATION "smartview"
#define CONV_SERVICE_TYPE_REMOTE_APP_CONTROL "remote_appcontrol"

#define CONV_COMM_TYPE_SMARTVIEW "comm/smartview"
#define CONV_COMM_TYPE_IOTCON "comm/iotcon"

#define CONV_RESOURCE_TYPE_SMARTVIEW_APP_COMMUNICATION "x.org.tizen.app-communication"
#define CONV_RESOURCE_TYPE_REMOTE_APP_CONTROL "x.org.tizen.remote-app-control"
#define CONV_RESOURCE_TYPE_TIZEN_D2D_SERVICE	"x.org.tizen.d2d-service"

#define CONV_URI_SMARTVIEW_APP_COMMUNICATION "/tizen/app-communication"
#define CONV_URI_REMOTE_APP_CONTROL "/tizen/remote-app-control"
#define CONV_URI_TIZEN_D2D_SERVICE "/tizen/d2d-service"

#define CONV_PRIVILEGE_INTERNET			"internet"
#define CONV_PRIVILEGE_BLUETOOTH		"bluetooth"
#define CONV_PRIVILEGE_NETWORK_GET		"network.get"
#define CONV_PRIVILEGE_D2D_DATA_SHARING	"d2d.datasharing"

enum request_type {
	REQ_SUBSCRIBE = 1,
	REQ_UNSUBSCRIBE,
	REQ_READ,
	REQ_READ_SYNC,
	REQ_WRITE,
	REQ_SUPPORT,
};

#define CONV_JSON_SERVICE_PATH			"service"
#define CONV_JSON_SERVICE_DATA			"data"
#define CONV_JSON_SERVICE_TYPE			"type"

#define CONV_JSON_DESCRIPTION			"description"
#define CONV_JSON_PAYLOAD				"payload"
#define CONV_JSON_SERVICE				"service"
#define CONV_JSON_CHANNEL				"channel"
#define CONV_JSON_IS_LOCAL				"is_local"
#define CONV_JSON_DEVICE				"device"
#define CONV_JSON_TYPE					"type"
#define CONV_JSON_DISCOVERY_SERVICE_TYPE "service_type"
#define CONV_JSON_DISCOVERY_SERVICE_INFO "service_info"
#define CONV_JSON_REQ_ID				"req_id"

#define CONV_JSON_SERVICE_DATA_PATH		NULL
#define CONV_JSON_SERVICE_DATA_NAME		"name"
#define CONV_JSON_SERVICE_DATA_VERSION	CONV_SERVICE_VERSION
#define CONV_JSON_SERVICE_DATA_TYPE		"type"
#define CONV_JSON_SERVICE_DATA_ID		"id"
#define CONV_JSON_SERVICE_DATA_URI		CONV_SERVICE_ID

#define CONV_TYPE_APP_TO_APP_COMMUNICATION	0
#define CONV_TYPE_REMOVE_APP_CONTROL		1

#define CONV_DISCOVERY_MIN_VALUE			5
#define CONV_DISCOVERY_MAX_VALUE			60

#define CONV_DISCOVERY_ERROR				-1
#define CONV_DISCOVERY_SUCCESS				0
#define CONV_DISCOVERY_FINISHED				1
#define CONV_DISCOVERY_DEVICE_LOST			2

#define CONV_JSON_DEVICE_ID				CONV_DEVICE_ID
#define CONV_JSON_DEVICE_NAME			CONV_DEVICE_NAME
#define CONV_JSON_DEVICE_TYPE			CONV_DEVICE_TYPE
#define CONV_JSON_DEVICE_ADDRESS		"host_address"

// smartview app to app communication service
#define CONV_JSON_CHANNEL_ID			"channel_id"
#define CONV_JSON_URI					"uri"
#define CONV_JSON_RESULT_TYPE			"result_type"
#define CONV_JSON_MESSAGE				"message"
#define CONV_JSON_CLIENT				"client"
#define CONV_JSON_PAYLOAD_SIZE			"payload_size"
#define CONV_JSON_EVENT					"event"
#define CONV_JSON_FROM					"from"
#define CONV_JSON_IS_HOST				"isHost"
#define CONV_JSON_CONNECT_TIME			"connectTime"
#define CONV_JSON_CLIENT_ID				"clientId"
#define CONV_JSON_CHANNEL_URI			"channelUri"
#define CONV_JSON_CLIENT_LIST			"client_list"
#define CONV_JSON_ERROR_MESSAGE			"error_message"

// remote app control service
#define CONV_JSON_APP_CONTROL			"app_control"
#define CONV_JSON_REPLY					"reply"

#define CONV_SETTING_VALUE_SERVICE_APP_TO_APP_COMMUNICATION 0x0001
#define CONV_SETTING_VALUE_SERVICE_REMOTE_APP_CONTROL 0x0002

#endif
