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

#include "Result.h"
#include "ApplicationInfo.h"
#include "Service.h"
#include "Client.h"
#include "Device.h"
#include "Error.h"

Result_Base::Result_Base()
{
}

Result_Base::Result_Base(void *user_cb, void *data, result_base_e subject)
{
	appinfo_result_cb = NULL;
	client_result_cb = NULL;
	device_result_cb = NULL;
	bool_result_cb = NULL;
	service_result_cb = NULL;
	error_result_cb = NULL;

	switch (subject) {
		case RESULT_BASE_APP_INFO:
			appinfo_result_cb = reinterpret_cast<_appinfo_result_cb>(user_cb);
			break;
		case RESULT_BASE_CLIENT:
			client_result_cb = reinterpret_cast<_client_result_cb>(user_cb);
			break;
		case RESULT_BASE_DEVICE:
			device_result_cb = reinterpret_cast<_device_result_cb>(user_cb);
			break;
		case RESULT_BASE_BOOL:
			bool_result_cb = reinterpret_cast<_bool_result_cb>(user_cb);
			break;
		case RESULT_BASE_SERVICE:
			service_result_cb = reinterpret_cast<_service_result_cb>(user_cb);
			break;
		case RESULT_BASE_ERROR:
			error_result_cb = reinterpret_cast<_error_result_cb>(user_cb);
			break;
	}

	user_data = data;
}

Result_Base::~Result_Base()
{
}


void Result_Base::onSuccess(ApplicationInfo arg)
{
	if (appinfo_result_cb)
		appinfo_result_cb(arg, user_data);
}

void Result_Base::onSuccess(Client arg)
{
	if (client_result_cb)
	client_result_cb(arg, user_data);
}

void Result_Base::onSuccess(Device arg)
{
	if (device_result_cb)
	device_result_cb(arg, user_data);
}

void Result_Base::onSuccess(bool arg)
{
	if (bool_result_cb)
	bool_result_cb(arg, user_data);
}

void Result_Base::onSuccess(Service arg)
{
	if (service_result_cb)
	service_result_cb(arg, user_data);
}

void Result_Base::onError(Error er)
{
	if (error_result_cb)
		error_result_cb(er, user_data);
}
