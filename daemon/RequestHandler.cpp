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

#include "RequestHandler.h"
#include "DiscoveryManager.h"
#include "ServiceManager.h"
#include "ConnectionManager.h"

using namespace std;

conv::RequestHandler::RequestHandler()
{
}

conv::RequestHandler::~RequestHandler()
{
}

int conv::RequestHandler::init()
{
	return CONV_ERROR_NONE;
}

int conv::RequestHandler::release()
{
	return CONV_ERROR_NONE;
}

int conv::RequestHandler::handleRequest(request* requestObj)
{
	_D("handleRequest called");
	int result = CONV_ERROR_INVALID_OPERATION;

	switch (requestObj->get_type()) {
	case REQ_SUBSCRIBE:
		_D("Requested type : REQ_SUBSCRIBE..");
		if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_COMMUNICATION_RECV) )
			result = service_manager::handleRequest(requestObj);
		else if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_CONNECTION_START) )
			result = connection_manager::handleRequest(requestObj);
		break;
	case REQ_UNSUBSCRIBE:
		if (!strcmp(requestObj->get_subject(), CONV_SUBJECT_COMMUNICATION_RECV) )
			result = service_manager::handleRequest(requestObj);
		else if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_CONNECTION_START) )
			result = connection_manager::handleRequest(requestObj);
		break;
	case REQ_READ:
		break;
	case REQ_READ_SYNC:
		break;
	case REQ_WRITE:
		_D("Requested type : REQ_WRITE..");
		if (!strcmp(requestObj->get_subject(), CONV_SUBJECT_DISCOVERY_START) || !strcmp(requestObj->get_subject(), CONV_SUBJECT_DISCOVERY_STOP))
			return discovery_manager::handleRequest (requestObj);
		else if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_COMMUNICATION_START) || !strcmp(requestObj->get_subject(), CONV_SUBJECT_COMMUNICATION_STOP)
				|| !strcmp(requestObj->get_subject(), CONV_SUBJECT_COMMUNICATION_SET) || !strcmp(requestObj->get_subject(), CONV_SUBJECT_COMMUNICATION_GET))
			result = service_manager::handleRequest(requestObj);
		else if ( !strcmp(requestObj->get_subject(), CONV_SUBJECT_CONNECTION_START) || !strcmp(requestObj->get_subject(), CONV_SUBJECT_CONNECTION_STOP) )
			result = connection_manager::handleRequest(requestObj);
		break;
	case REQ_SUPPORT:
		break;
	default:
		_E("Invalid type of request");
		requestObj->reply(result);
		delete requestObj;
	}

	return result;
}
