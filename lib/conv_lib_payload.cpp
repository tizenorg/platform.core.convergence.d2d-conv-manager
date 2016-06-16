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
#include <app_control_internal.h>
#include <bundle.h>
#include <bundle_internal.h>

#include "common.h"
#include "d2d_conv_manager.h"
#include "d2d_conv_internal.h"
#include "conv_lib_json.h"
#include "internal_types.h"
#include "dbus_client.h"
#include "conv_lib_util.h"

EXTAPI int conv_payload_create(conv_payload_h* handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	*handle = new(std::nothrow) _conv_payload_handle();
	ASSERT_ALLOC(*handle);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_destroy(conv_payload_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	delete handle;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_set_string(conv_payload_h handle, const char* key, const char* value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	handle->jpayload.set(NULL, key, value);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_get_string(conv_payload_h handle, const char* key, char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	std::list<std::string> key_list;
	handle->jpayload.get_keys(&key_list);
	IF_FAIL_RETURN_TAG(key_list.size() > 0, CONV_ERROR_NO_DATA, _E, "No data");

	// Check Invalid record key
	std::string str;
	IF_FAIL_RETURN_TAG(handle->jpayload.get(NULL, key, &str), CONV_ERROR_INVALID_PARAMETER, _E, "Value load failed");

	*value = g_strdup(str.c_str());
	ASSERT_ALLOC(*value);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_set_app_control(conv_payload_h handle, const char* key, app_control_h app_control)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(app_control);

	bundle* appctl_bundle = NULL;
	int error = app_control_to_bundle(app_control, &appctl_bundle);
	IF_FAIL_RETURN_TAG(error == APP_CONTROL_ERROR_NONE, CONV_ERROR_INVALID_PARAMETER, _E, "App_control to bundle failed");

	bundle_raw* appctl_raw;
	int raw_length;
	error = bundle_encode(appctl_bundle, &appctl_raw, &raw_length);
	IF_FAIL_RETURN_TAG(error == BUNDLE_ERROR_NONE, CONV_ERROR_INVALID_PARAMETER, _E, "Bundle encode failed");

	std::string appctl_str = reinterpret_cast<const char*>(appctl_raw);
	handle->jpayload.set(NULL, key, appctl_str);

	bundle_free(appctl_bundle);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_get_app_control(conv_payload_h handle, const char* key, app_control_h* app_control)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(app_control);
	int ret;

	std::list<std::string> key_list;
	handle->jpayload.get_keys(&key_list);
	IF_FAIL_RETURN_TAG(key_list.size() > 0, CONV_ERROR_NO_DATA, _E, "No data");

	// Check Invalid record key
	std::string appctl_str;
	IF_FAIL_RETURN_TAG(handle->jpayload.get(NULL, key, &appctl_str), CONV_ERROR_INVALID_PARAMETER, _E, "Value load failed");

	char* str = static_cast<char*>(malloc(appctl_str.length()));

	IF_FAIL_RETURN_TAG(str, CONV_ERROR_INVALID_PARAMETER, _E, "Memory allocation failed");

	appctl_str.copy(str, appctl_str.length(), 0);
	bundle_raw* encoded = reinterpret_cast<unsigned char*>(str);
	bundle* appctl_bundle = bundle_decode(encoded, appctl_str.length());

	app_control_create(app_control);
	ret = app_control_import_from_bundle(*app_control, appctl_bundle);

	bundle_free(appctl_bundle);

	IF_FAIL_RETURN_TAG(ret == APP_CONTROL_ERROR_NONE, CONV_ERROR_INVALID_PARAMETER, _E, "app_control load failed");

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_set_byte(conv_payload_h handle, const char* key, int length, unsigned char* value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	std::string str_value(reinterpret_cast<const char*>(value), length);

	handle->jpayload.set(NULL, key, str_value);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_payload_get_byte(conv_payload_h handle, const char* key, int* length, unsigned char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	std::list<std::string> key_list;
	handle->jpayload.get_keys(&key_list);
	IF_FAIL_RETURN_TAG(key_list.size() > 0, CONV_ERROR_NO_DATA, _E, "No data");

	// Check Invalid record key
	std::string str;
	IF_FAIL_RETURN_TAG(handle->jpayload.get(NULL, key, &str), CONV_ERROR_INVALID_PARAMETER, _E, "Value load failed");

	*value = reinterpret_cast<unsigned char*>(g_strdup(str.c_str()));
	*length = str.size();

	ASSERT_ALLOC(*value);

	return CONV_ERROR_NONE;
}

//LCOV_EXCL_START
// internal API
EXTAPI int conv_payload_internal_export_to_string(conv_payload_h handle, char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(value);

	*value = g_strdup(handle->jpayload.str().c_str());
	ASSERT_ALLOC(*value);

	return CONV_ERROR_NONE;
}
//LCOV_EXCL_STOP

