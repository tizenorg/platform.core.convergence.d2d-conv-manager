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

#include "AppCommServiceApplication.h"

conv::AppCommServiceApplication::~AppCommServiceApplication()
{
	if (application != NULL)
		delete application;
}

void conv::AppCommServiceApplication::onStart(bool start_result)
{
	_D("onStart Called");

	if ((*requestObj) != NULL) {
		_D(RED("publishing_response"));
		Json result;
		Json payload;
		Json description;

		payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_START);

		description = (*requestObj)->getDescription();

		description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
		description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channelId);

		result.set(NULL, CONV_JSON_DESCRIPTION, description);
		result.set(NULL, CONV_JSON_PAYLOAD, payload);
		if (start_result)
			(*requestObj)->publish(CONV_ERROR_NONE, result);
		else
			(*requestObj)->publish(CONV_ERROR_INVALID_OPERATION, result);
	}
}

void conv::AppCommServiceApplication::onStop(bool start_result)
{
	_D("onStop Called");

	if (!isLocal && application != NULL) {
		application->disconnect();
	}

	if ((*requestObj) != NULL) {
		_D(RED("publishing_response"));
		Json result;
		Json payload;
		Json description;

		payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_STOP);

		description = (*requestObj)->getDescription();

		description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
		description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channelId);

		result.set(NULL, CONV_JSON_DESCRIPTION, description);
		result.set(NULL, CONV_JSON_PAYLOAD, payload);
		if (start_result)
			(*requestObj)->publish(CONV_ERROR_NONE, result);
		else
			(*requestObj)->publish(CONV_ERROR_INVALID_OPERATION, result);
	}
}

void conv::AppCommServiceApplication::onConnect(Client client)
{
	_D("onConnect Called");
	publishResponse(CONV_ERROR_NONE, CONV_JSON_ON_CONNECT, &client);

	if (!isLocal && application != NULL) {
		((Application*)application)->start();
		_D("Application start requested");
	}
}

void conv::AppCommServiceApplication::onDisconnect(Client client)
{
	_D("onDisconnect Called");
	publishResponse(CONV_ERROR_NONE, CONV_JSON_ON_DISCONNECT, &client);
}

void conv::AppCommServiceApplication::onClientConnect(Client client)
{
	_D("onClientConnect Called");
	publishResponse(CONV_ERROR_NONE, CONV_JSON_ON_CLIENT_CONNECT, &client);
}

void conv::AppCommServiceApplication::onClientDisconnect(Client client)
{
	_D("onClientDisconnect Called");
	publishResponse(CONV_ERROR_NONE, CONV_JSON_ON_CLIENT_DISCONNECT, &client);
}

void conv::AppCommServiceApplication::onMessage(Message message)
{
	_D("onMessage Called");

	if ((*requestObj) != NULL) {
		_D(RED("publishing_response"));
		Json result;
		Json message_json;
		Json description;

		_D("size %d", message.m_payload_size);
		_D("payload %s", message.m_payload);

		message_json.set(NULL, CONV_JSON_EVENT, message.m_event);
		message_json.set(NULL, CONV_JSON_MESSAGE, message.m_data);
		message_json.set(NULL, CONV_JSON_FROM, message.m_from);
		string payload_str(reinterpret_cast<const char*>(message.m_payload), message.m_payload_size);

		Json payload = payload_str;

		payload.set(NULL, CONV_JSON_PAYLOAD_SIZE, message.m_payload_size);
		payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_MESSAGE);
//		payload.set(NULL, CONV_JSON_MESSAGE, message_json);
		payload.set(NULL, CONV_JSON_MESSAGE, message_json.dupCstr());
		payload.set(NULL, CONV_JSON_FROM, message.m_from);

		description = (*requestObj)->getDescription();

		description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
		description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channelId);

		result.set(NULL, CONV_JSON_DESCRIPTION, description);
		result.set(NULL, CONV_JSON_PAYLOAD, payload);
		(*requestObj)->publish(CONV_ERROR_NONE, result);
	}
}

void conv::AppCommServiceApplication::onError(Error error)
{
	_D("onError Called");

	if ((*requestObj) != NULL) {
		_D(RED("publishing_response"));
		Json result;
		Json payload;
		Json description;

		payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_ERROR);
		payload.set(NULL, CONV_JSON_ERROR_MESSAGE, error.get_error_message());

		description = (*requestObj)->getDescription();

		description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
		description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channelId);

		result.set(NULL, CONV_JSON_DESCRIPTION, description);
		result.set(NULL, CONV_JSON_PAYLOAD, payload);
		(*requestObj)->publish(CONV_ERROR_INVALID_OPERATION, result);
	}
}

void conv::AppCommServiceApplication::onPublished(bool publish_result, void* user_data)
{
	_D("onPublished Called");

	if ((*requestObj) != NULL) {
		_D(RED("publishing_response"));
		Json result;
		Json payload;
		Json description;

		payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_PUBLISH);

		description = (*requestObj)->getDescription();

		description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
		description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channelId);

		result.set(NULL, CONV_JSON_DESCRIPTION, description);
		result.set(NULL, CONV_JSON_PAYLOAD, payload);
		if (publish_result)
			(*requestObj)->publish(CONV_ERROR_NONE, result);
		else
			(*requestObj)->publish(CONV_ERROR_INVALID_OPERATION, result);
	}
}

void conv::AppCommServiceApplication::publishResponse(int error, string result_type, Client *client)
{
	if ((*requestObj) != NULL) {
		_D(RED("publishing_response"));
		Json result;
		Json payload;
		Json description;

		string isHostStr = to_string(client->isHost());
		string connectTimeStr = to_string(client->getConnectTime());

		payload.set(NULL, CONV_JSON_RESULT_TYPE, result_type);
		payload.set(NULL, CONV_JSON_CLIENT_IS_HOST, isHostStr.c_str());
		payload.set(NULL, CONV_JSON_CLIENT_CONNECT_TIME, connectTimeStr.c_str());
		payload.set(NULL, CONV_JSON_CLIENT_CLIENT_ID, client->getId());

		description = (*requestObj)->getDescription();

		description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
		description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channelId);

		result.set(NULL, CONV_JSON_DESCRIPTION, description);
		result.set(NULL, CONV_JSON_PAYLOAD, payload);
		(*requestObj)->publish(error, result);
	}
}
