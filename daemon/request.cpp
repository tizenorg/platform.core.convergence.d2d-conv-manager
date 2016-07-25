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
#include "request.h"
#include "DbusServer.h"

conv::request::request(int type, const char* client, int req_id, const char* subj, const char* desc)
	: _type(type)
	, _req_id(req_id)
	, _client(client)
	, _subject(subj)
	, _description(desc)
{
}

conv::request::request(int type,
		const char *client, int req_id, const char *subj, const char *desc,
		const char *sender, Credentials *creds, GDBusMethodInvocation *inv)
	:  _type(type)
	, _req_id(req_id)
	, _client(client)
	, _subject(subj)
	, _description(desc)
	, __creds(creds)
	, __sender(sender)
	, __invocation(inv)
{
}

conv::request::~request()
{
	if ( __creds != NULL ) {
		delete __creds;
	}
}

int conv::request::get_type()
{
	return _type;
}

int conv::request::get_id()
{
	return _req_id;
}

const char* conv::request::get_client()
{
	return _client.c_str();
}

const char* conv::request::get_sender()
{
	return __sender.c_str();
}

const char* conv::request::get_subject()
{
	return _subject.c_str();
}

json& conv::request::get_description()
{
	return _description;
}

conv::Credentials* conv::request::get_creds()
{
	return __creds;
}

bool conv::request::reply(int error)
{
	IF_FAIL_RETURN(__invocation, true);

	_I("Reply %#x", error);

	g_dbus_method_invocation_return_value(__invocation, g_variant_new("(iss)", error, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
	__invocation = NULL;
	_D("Reply done");
	return true;
}

bool conv::request::reply(int error, json& request_result)
{
	IF_FAIL_RETURN(__invocation, true);
	IF_FAIL_RETURN(_type != REQ_READ_SYNC, true);

	char *result = request_result.dup_cstr();
	IF_FAIL_RETURN_TAG(result, false, _E, "Memory allocation failed");

	_I("Reply %#x", error);
	_SD("Result: %s", result);

	g_dbus_method_invocation_return_value(__invocation, g_variant_new("(iss)", error, result, EMPTY_JSON_OBJECT));
	__invocation = NULL;

	g_free(result);
	return true;
}

bool conv::request::reply(int error, json& request_result, json& data_read)
{
	if (__invocation == NULL) {
		return publish(error, data_read);
	}

	char *result = NULL;
	char *data = NULL;

	result = request_result.dup_cstr();
	IF_FAIL_CATCH_TAG(result, _E, "Memory allocation failed");

	data = data_read.dup_cstr();
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

bool conv::request::publish(int error, json& data)
{
	char *data_str = data.dup_cstr();
	IF_FAIL_RETURN_TAG(data_str, false, _E, "Memory allocation failed");

	_D("info : sendor %s", __sender.c_str());
	_D("info : req_id %d", _req_id);
	_D("info : subject %s", _subject.c_str());
	_D("info : error %d", error);
	_D("info : data %s", data_str);
	_D("info : description %s", _description.str().c_str());
	_D("publish info : sender[%s] req_id[%d] subject[%s] error[%d] data[%s]"
			, __sender.c_str(), _req_id, _subject.c_str(), error, data_str);

	conv::dbus_server::publish(__sender.c_str(), _req_id, _subject.c_str(), error, data_str);
	g_free(data_str);

	return true;
}

bool conv::request::get_channel_from_description(json* target)
{
	return _description.get(NULL, CONV_JSON_CHANNEL, target);
}

bool conv::request::get_payload_from_description(json* target)
{
	return _description.get(NULL, CONV_JSON_PAYLOAD, target);
}
