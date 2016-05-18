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

#include "request_handler.h"
#include "discovery_mgr_impl.h"
#include "service_mgr_impl.h"
#include "connection_mgr_impl.h"

using namespace std;

conv::request_handler::request_handler()
{
}

conv::request_handler::~request_handler()
{
}

int conv::request_handler::init()
{
	return CONV_ERROR_NONE;
}

int conv::request_handler::release()
{
	return CONV_ERROR_NONE;
}

int conv::request_handler::handle_request(request* request_obj)
{
	_D("handle_request called");
	int result = CONV_ERROR_INVALID_OPERATION;

	switch (request_obj->get_type()) {
		case REQ_SUBSCRIBE:
			_D("Requested type : REQ_SUBSCRIBE..");
			if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_RECV) )
			{
				result = service_manager::handle_request(request_obj);
			}
			else if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_CONNECTION_START) )
			{
				result = connection_manager::handle_request(request_obj);
			}
			break;
		case REQ_UNSUBSCRIBE:
			if (!strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_RECV) )
			{
				result = service_manager::handle_request(request_obj);
			}
			else if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_CONNECTION_START) )
			{
				result = connection_manager::handle_request(request_obj);
			}
			break;
		case REQ_READ:
			break;
		case REQ_READ_SYNC:
			break;
		case REQ_WRITE:
			_D("Requested type : REQ_WRITE..");
			if (!strcmp (request_obj->get_subject(), CONV_SUBJECT_DISCOVERY_START) || !strcmp (request_obj->get_subject(), CONV_SUBJECT_DISCOVERY_STOP))
			{
				return discovery_manager::handle_request (request_obj);
			}
			else if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_START) || !strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_STOP)
					|| !strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_SET) || !strcmp(request_obj->get_subject(), CONV_SUBJECT_COMMUNICATION_GET))
			{
				result = service_manager::handle_request(request_obj);
			}
			else if ( !strcmp(request_obj->get_subject(), CONV_SUBJECT_CONNECTION_START) || !strcmp(request_obj->get_subject(), CONV_SUBJECT_CONNECTION_STOP) )
			{
				result = connection_manager::handle_request(request_obj);
			}
			break;
		case REQ_SUPPORT:
			break;
		default:
			_E("Invalid type of request");
			request_obj->reply(result);
			delete request_obj;
	}

	return result;
}
