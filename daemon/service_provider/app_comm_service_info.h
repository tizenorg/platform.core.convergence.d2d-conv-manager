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

#ifndef __APP_COMMUNICATION_SERVICE_INFO_H__
#define __APP_COMMUNICATION_SERVICE_INFO_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include "../service_info_base.h"
#include "../request.h"
#include "Service.h"
#include "Clients.h"
#include "Application.h"
#include "Result.h"

namespace conv {
	class application_instance : public OnConnectListener, public OnDisconnectListener, public OnClientConnectListener, public OnClientDisconnectListener,
									public OnMessageListener, public OnErrorListener, public OnStartAppListener, public OnStopAppListener {
		public:
			virtual ~application_instance()
			{
				if (application != NULL)
					delete application;
			}

			conv::request** request_obj;
			Channel* application;
			string uri;
			string channel_id;
			Service local_service;
			bool is_local;

			void onStart(bool start_result)
			{
				_D("onStart Called");

				if ((*request_obj) != NULL) {
					_D(RED("publishing_response"));
					json result;
					json payload;
					json description;

					payload.set(NULL, CONV_JSON_RESULT_TYPE, "onStart");

					description = (*request_obj)->get_description();

					description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
					description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channel_id);

					result.set(NULL, CONV_JSON_DESCRIPTION, description);
					result.set(NULL, CONV_JSON_PAYLOAD, payload);
					if (start_result)
						(*request_obj)->publish(CONV_ERROR_NONE, result);
					else
						(*request_obj)->publish(CONV_ERROR_INVALID_OPERATION, result);
				}
			}

			void onStop(bool start_result)
			{
				_D("onStop Called");

				if (!is_local && application != NULL) {
					application->disconnect();
				}

				if ((*request_obj) != NULL) {
					_D(RED("publishing_response"));
					json result;
					json payload;
					json description;

					payload.set(NULL, CONV_JSON_RESULT_TYPE, "onStop");

					description = (*request_obj)->get_description();

					description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
					description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channel_id);

					result.set(NULL, CONV_JSON_DESCRIPTION, description);
					result.set(NULL, CONV_JSON_PAYLOAD, payload);
					if (start_result)
						(*request_obj)->publish(CONV_ERROR_NONE, result);
					else
						(*request_obj)->publish(CONV_ERROR_INVALID_OPERATION, result);
				}
			}

			void onConnect(Client client)
			{
				_D("onConnect Called");
				publish_response(CONV_ERROR_NONE, "onConnect", &client);

				if (!is_local && application != NULL) {
					((Application*)application)->start();
					_D("Application start requested");
				}
			}

			void onDisconnect(Client client)
			{
				_D("onDisconnect Called");
				publish_response(CONV_ERROR_NONE, "onDisconnect", &client);
			}

			void onClientConnect(Client client)
			{
				_D("onClientConnect Called");
				publish_response(CONV_ERROR_NONE, "onClientConnect", &client);
			}

			void onClientDisconnect(Client client)
			{
				_D("onClientDisconnect Called");
				publish_response(CONV_ERROR_NONE, "onClientDisconnect", &client);
			}

			void onMessage(Message message)
			{
				_D("onMessage Called");

				if ((*request_obj) != NULL) {
					_D(RED("publishing_response"));
					json result;
					json message_json;
					json description;

					_D("size %d", message.m_payload_size);
					_D("payload %s", message.m_payload);

					message_json.set(NULL, CONV_JSON_EVENT, message.m_event);
					message_json.set(NULL, CONV_JSON_MESSAGE, message.m_data);
					message_json.set(NULL, CONV_JSON_FROM, message.m_from);
					string payload_str(reinterpret_cast<const char*>(message.m_payload), message.m_payload_size);

					json payload = payload_str;

					payload.set(NULL, CONV_JSON_PAYLOAD_SIZE, message.m_payload_size);
					payload.set(NULL, CONV_JSON_RESULT_TYPE, "onMessage");
					payload.set(NULL, CONV_JSON_MESSAGE, message_json);

					description = (*request_obj)->get_description();

					description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
					description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channel_id);

					result.set(NULL, CONV_JSON_DESCRIPTION, description);
					result.set(NULL, CONV_JSON_PAYLOAD, payload);
					(*request_obj)->publish(CONV_ERROR_NONE, result);
				}
			}

			void onError(Error error)
			{
				_D("onError Called");

				if ((*request_obj) != NULL) {
					_D(RED("publishing_response"));
					json result;
					json payload;
					json description;

					payload.set(NULL, CONV_JSON_RESULT_TYPE, "onError");
					payload.set(NULL, CONV_JSON_ERROR_MESSAGE, error.get_error_message());

					description = (*request_obj)->get_description();

					description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
					description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channel_id);

					result.set(NULL, CONV_JSON_DESCRIPTION, description);
					result.set(NULL, CONV_JSON_PAYLOAD, payload);
					(*request_obj)->publish(CONV_ERROR_INVALID_OPERATION, result);
				}
			}

			void publish_response(int error, string result_type, Client *client)
			{
				bool isHost = client->isHost();
				int connecttime = client->getConnectTime();

				if ((*request_obj) != NULL) {
					_D(RED("publishing_response"));
					json result;
					json payload;
					json client_json;
					json description;

					client_json.set(NULL, CONV_JSON_IS_HOST, isHost);
					client_json.set(NULL, CONV_JSON_CONNECT_TIME, connecttime);
					client_json.set(NULL, CONV_JSON_CLIENT_ID, client->getId());

					payload.set(NULL, CONV_JSON_RESULT_TYPE, result_type);
					payload.set(NULL, CONV_JSON_CLIENT, client_json);

					description = (*request_obj)->get_description();

					description.set(CONV_JSON_CHANNEL, CONV_JSON_URI, uri);
					description.set(CONV_JSON_CHANNEL, CONV_JSON_CHANNEL_ID, channel_id);

					result.set(NULL, CONV_JSON_DESCRIPTION, description);
					result.set(NULL, CONV_JSON_PAYLOAD, payload);
					(*request_obj)->publish(error, result);
				}
			}
	};

	typedef vector<application_instance*> application_instance_list_t;

	// service information to handle app-to-app service with specific device 'id'
	class app_comm_service_info : public service_info_base {
		public:
			~app_comm_service_info()
			{
				if (service_obj != NULL) {
					delete service_obj;
				}

				if (registered_request != NULL) {
					delete registered_request;
				}

				for (application_instance_list_t::iterator iter = application_instance_list.begin(); iter != application_instance_list.end(); ++iter) {
					application_instance *app_info = *iter;
					delete app_info;
					application_instance_list.erase(iter);
				}
			}
			std::string id;
			Service* service_obj;
			bool is_local;
			conv::request* registered_request;

			application_instance_list_t application_instance_list;
	};

}
#endif /* __APP_COMMUNICATION_SERVICE_INFO_H__ */
