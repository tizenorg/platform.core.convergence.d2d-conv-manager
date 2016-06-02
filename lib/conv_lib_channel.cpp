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
#include "d2d_conv_internal.h"
#include "conv_lib_json.h"
#include "internal_types.h"
#include "dbus_client.h"
#include "conv_lib_util.h"

EXTAPI int conv_channel_create(conv_channel_h* handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);

	*handle = new(std::nothrow) _conv_channel_handle();
	ASSERT_ALLOC(*handle);

	return CONV_ERROR_NONE;
}

EXTAPI int conv_channel_destroy(conv_channel_h handle)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	delete handle;

	return CONV_ERROR_NONE;
}

EXTAPI int conv_channel_set_string(conv_channel_h handle, const char* key, const char* value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	handle->jchannel.set(NULL, key, value);

	return CONV_ERROR_NONE;
}


EXTAPI int conv_channel_get_string(conv_channel_h handle, const char* key, char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(key);
	ASSERT_NOT_NULL(value);

	std::list<std::string> key_list;
	handle->jchannel.get_keys(&key_list);
	IF_FAIL_RETURN_TAG(key_list.size() > 0, CONV_ERROR_NO_DATA, _E, "No data");

	// Check Invalid record key
	std::string str;
	IF_FAIL_RETURN_TAG(handle->jchannel.get(NULL, key, &str), CONV_ERROR_INVALID_PARAMETER, _E, "Value load failed");

	*value = g_strdup(str.c_str());
	ASSERT_ALLOC(*value);

	return CONV_ERROR_NONE;
}

// internal API
EXTAPI int conv_channel_internal_export_to_string(conv_channel_h handle, char** value)
{
	IF_FAIL_RETURN_TAG(conv::util::is_feature_supported(), CONV_ERROR_NOT_SUPPORTED, _E, "Not supported");
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL(value);

	*value = g_strdup(handle->jchannel.str().c_str());
	ASSERT_ALLOC(*value);

	return CONV_ERROR_NONE;
}
