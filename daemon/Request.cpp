/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <glib.h>
#include "Types.h"
#include "Request.h"
#include "DbusServer.h"

conv::Request::Request(int type, const char* client, int reqId, const char* subj, const char* desc)
	: __type(type)
	, __reqId(reqId)
	, __client(client)
	, __subject(subj)
	, __description(desc)
{
}

conv::Request::Request(int type,
		const char *client, int reqId, const char *subj, const char *desc,
		const char *sender, Credentials *creds, GDBusMethodInvocation *inv)
	: __type(type)
	, __reqId(reqId)
	, __client(client)
	, __subject(subj)
	, __description(desc)
	, __creds(creds)
	, __sender(sender)
	, __invocation(inv)
{
}

conv::Request::~Request()
{
	if ( __creds != NULL ) {
		delete __creds;
	}
}

int conv::Request::getType()
{
	return __type;
}

int conv::Request::getId()
{
	return __reqId;
}

const char* conv::Request::getClient()
{
	return __client.c_str();
}

const char* conv::Request::getSender()
{
	return __sender.c_str();
}

const char* conv::Request::getSubject()
{
	return __subject.c_str();
}

conv::Json& conv::Request::getDescription()
{
	return __description;
}

conv::Credentials* conv::Request::getCreds()
{
	return __creds;
}

bool conv::Request::reply(int error)
{
	IF_FAIL_RETURN(__invocation, true);

	_I("Reply %#x", error);

	g_dbus_method_invocation_return_value(__invocation, g_variant_new("(iss)", error, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
	__invocation = NULL;
	_D("Reply done");
	return true;
}

bool conv::Request::reply(int error, Json& requestResult)
{
	IF_FAIL_RETURN(__invocation, true);
	IF_FAIL_RETURN(__type != REQ_READ_SYNC, true);

	char *result = requestResult.dupCstr();
	IF_FAIL_RETURN_TAG(result, false, _E, "Memory allocation failed");

	_I("Reply %#x", error);
	_SD("Result: %s", result);

	g_dbus_method_invocation_return_value(__invocation, g_variant_new("(iss)", error, result, EMPTY_JSON_OBJECT));
	__invocation = NULL;

	g_free(result);
	return true;
}

bool conv::Request::reply(int error, Json& requestResult, Json& readData)
{
	if (__invocation == NULL) {
		return publish(error, readData);
	}

	char *result = NULL;
	char *data = NULL;

	result = requestResult.dupCstr();
	IF_FAIL_CATCH_TAG(result, _E, "Memory allocation failed");

	data = readData.dupCstr();
	IF_FAIL_CATCH_TAG(data, _E, "Memory allocation failed");

	_I("Reply %#x", error);
	_SD("Result: %s", result);
	_SD("Data: %s", data);

	g_dbus_method_invocation_return_value(__invocation, g_variant_new("(iss)", error, result, data));
	__invocation = NULL;

	g_free(result);
	g_free(data);
	return true;

CATCH:
	g_free(result);
	g_free(data);
	return false;
}

bool conv::Request::publish(int error, conv::Json& data)
{
	char *dataStr = data.dupCstr();
	IF_FAIL_RETURN_TAG(dataStr, false, _E, "Memory allocation failed");

	_D("info : sendor %s", __sender.c_str());
	_D("info : reqId %d", __reqId);
	_D("info : subject %s", __subject.c_str());
	_D("info : error %d", error);
	_D("info : data %s", dataStr);
	_D("info : description %s", __description.str().c_str());
	_D("publish info : sender[%s] reqId[%d] subject[%s] error[%d] data[%s]"
			, __sender.c_str(), __reqId, __subject.c_str(), error, dataStr);

	conv::dbus_server::publish(__sender.c_str(), __reqId, __subject.c_str(), error, dataStr);
	g_free(dataStr);

	return true;
}

bool conv::Request::getChannelFromDescription(conv::Json* target)
{
	return __description.get(NULL, CONV_JSON_CHANNEL, target);
}

bool conv::Request::getPayloadFromDescription(conv::Json* target)
{
	return __description.get(NULL, CONV_JSON_PAYLOAD, target);
}

conv::IServiceInfo* conv::Request::getServiceInfo()
{
	return __serviceInfo;
}

conv::ICommunicationInfo* conv::Request::getCommunicationInfo()
{
	return __communicationInfo;
}

void conv::Request::setServiceInfo(conv::IServiceInfo* serviceInfo)
{
	__serviceInfo = serviceInfo;
}

void conv::Request::setCommunicationInfo(conv::ICommunicationInfo* communicationInfo)
{
	__communicationInfo = communicationInfo;
}
