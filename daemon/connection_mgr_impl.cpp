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

#include "connection_mgr_impl.h"
#include "access_control/privilege.h"

static conv::connection_manager_impl *_instance;
static conv::request* cur_Req;

using namespace std;

conv::connection_manager_impl::connection_manager_impl()
{
}

conv::connection_manager_impl::~connection_manager_impl()
{
}

int conv::connection_manager_impl::init()
{
	return CONV_ERROR_NONE;
}

int conv::connection_manager_impl::release()
{
	return CONV_ERROR_NONE;
}

void conv::connection_manager::set_instance(conv::connection_manager_impl* mgr)
{
	_instance = mgr;
}

int conv::connection_manager_impl::handle_request(request* request_obj)
{
	_D("handle_request called");
	int error = CONV_ERROR_INVALID_OPERATION;

	if ( !conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_INTERNET) ||
			!conv::privilege_manager::is_allowed(request_obj->get_creds(), CONV_PRIVILEGE_BLUETOOTH) )
	{
		_E("permission denied");
		request_obj->reply(CONV_ERROR_PERMISSION_DENIED);
		delete request_obj;
		return CONV_ERROR_PERMISSION_DENIED;
	}

	switch (request_obj->get_type()) {
		case REQ_SUBSCRIBE:
		{
			cur_Req = request_obj;
			error = CONV_ERROR_NONE;
		}
		break;
		case REQ_UNSUBSCRIBE:
		{
			cur_Req = NULL;
			error = CONV_ERROR_NONE;
		}
		break;
		case REQ_WRITE:
		{
			json cb_json, jservice, description, tmp_payload;
			string did;
			description = request_obj->get_description();
			description.get(NULL, CONV_JSON_SERVICE, &jservice);
			jservice.get(CONV_JSON_SERVICE_DATA_PATH, CONV_JSON_SERVICE_DATA_ID, &did);
			// TODO:
			//make callback data and invoke
			cb_json.set(NULL, CONV_JSON_DESCRIPTION, request_obj->get_description());
			cb_json.set(NULL, CONV_JSON_PAYLOAD, tmp_payload);
			if (cur_Req != NULL) {
				cur_Req->publish(CONV_ERROR_NONE, cb_json);
			}
			error = CONV_ERROR_NONE;
		}
		break;
		default:
		{
			_E("Invalid type of request");
			error = CONV_ERROR_INVALID_OPERATION;
		}
		break;
	}

	request_obj->reply(error);
	delete request_obj;
	return CONV_ERROR_NONE;
}

int conv::connection_manager::handle_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_instance, CONV_ERROR_INVALID_PARAMETER, _E, "Not initialized");
	_instance->handle_request(request_obj);

	return CONV_ERROR_NONE;
}

