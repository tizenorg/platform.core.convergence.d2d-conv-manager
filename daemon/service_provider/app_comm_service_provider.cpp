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

#include "app_comm_service_provider.h"
#include "../client_mgr_impl.h"
#include <net_connection.h>

using namespace std;

conv::app_comm_service_provider::app_comm_service_provider()
{
	_type = CONV_SERVICE_TYPE_SMARTVIEW_APP_COMMUNICATION;
	_resource_type = CONV_RESOURCE_TYPE_SMARTVIEW_APP_COMMUNICATION;
	_uri = CONV_URI_SMARTVIEW_APP_COMMUNICATION;
	_activation_state = 1;
}

conv::app_comm_service_provider::~app_comm_service_provider()
{
}

int conv::app_comm_service_provider::init()
{
	_D("app_comm_service_provider init done");
	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::release()
{
	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::load_service_info(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_activation_state == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	string client;
	conv::client* client_obj = NULL;
	int is_local = 0;

	json service;
	string name, version, type, id, uri;

	json description = request_obj->get_description();

	description.get(NULL, CONV_JSON_SERVICE, &service);

	service.get(NULL, CONV_JSON_SERVICE_DATA_NAME, &name);
	service.get(NULL, CONV_JSON_SERVICE_DATA_VERSION, &version);
	service.get(NULL, CONV_JSON_SERVICE_DATA_TYPE, &type);
	service.get(NULL, CONV_JSON_SERVICE_DATA_ID, &id);
	service.get(NULL, CONV_JSON_SERVICE_DATA_URI, &uri);

	description.get(NULL, CONV_JSON_IS_LOCAL, &is_local);

	client = request_obj->get_sender();

	_D("client id : %s", client.c_str());
	client_obj = conv::client_manager::get_client(client);
	IF_FAIL_RETURN_TAG(client_obj, CONV_ERROR_OUT_OF_MEMORY, _E, "client info alloc failed");

	app_comm_service_info *svc_info = NULL;

	if (is_local == 1) {
		service_info_base* svc_info_base = client_obj->get_service_info(_type, "LOCAL_HOST");

		if ( svc_info_base != NULL ) {
			_D("local service instance already exists");
			svc_info = reinterpret_cast<app_comm_service_info*>(svc_info_base);

			if (svc_info == NULL) {
				_D("casting failed");
				return CONV_ERROR_INVALID_OPERATION;
			}
		} else {
			_D("allocating new service instance for local service");
			svc_info = new(std::nothrow) app_comm_service_info();
			ASSERT_ALLOC(svc_info);

			svc_info->is_local = true;
			client_obj->add_service_info(_type, "LOCAL_HOST", (service_info_base*)svc_info);

			_D("MSF service is created");
		}
	} else {
		service_info_base* svc_info_base = client_obj->get_service_info(_type, id);

		if ( svc_info_base != NULL ) {
			_D("service instance already exists");
			svc_info = reinterpret_cast<app_comm_service_info*>(svc_info_base);

			if (svc_info == NULL) {
				_D("casting failed");
				return CONV_ERROR_INVALID_OPERATION;
			}
		} else {
			_D("allocating new service instance");
			svc_info = new(std::nothrow) app_comm_service_info();
			ASSERT_ALLOC(svc_info);

			_D("uri : %s", uri.c_str());
			Service::getByURI(uri, 2000, svc_info);
			IF_FAIL_RETURN_TAG(svc_info->get_service_result == true, CONV_ERROR_INVALID_OPERATION, _E, "getByURI failed");
			svc_info->is_local = false;
			client_obj->add_service_info(_type, id, (service_info_base*)svc_info);

			_D("MSF service is created");
		}
	}
	request_obj->service_info = svc_info;
	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::start_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_activation_state == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/start requested");
	app_comm_service_info *svc_info = reinterpret_cast<app_comm_service_info*>(request_obj->service_info);

	json channel;
	request_obj->get_channel_from_description(&channel);

	string uri, channel_id;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channel_id);

	application_instance *app_info = NULL;

	for (application_instance_list_t::iterator iter = svc_info->application_instance_list.begin(); iter != svc_info->application_instance_list.end(); ++iter) {
		_D("iteration");
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channel_id.compare(channel_id) ) {
			if ( (*iter)->application != NULL  ) {
				_D("already started");
				// check if it's connected and re-try if it's not
				return CONV_ERROR_INVALID_OPERATION;
			} else {
				_D("app_info exists but no application instance");
				app_info = (*iter);
				break;
			}
		}
	}

	if ( app_info == NULL ) {
		app_info = new(std::nothrow) application_instance();
		ASSERT_ALLOC(app_info);

		app_info->uri = uri;
		app_info->channel_id = channel_id;
	}

	// if it's local -> server application
	if ( svc_info->is_local ) {
		_D("COMMUNCATION_START : local channel. channel_id : %s", channel_id.c_str());

		app_info->local_service = Service::getLocal();

		Channel* application = app_info->local_service.createChannel(channel_id);

		// add listeners
		app_info->request_obj = &(svc_info->registered_request);
		application->setonConnectListener(app_info);
		application->setonClientConnectListener(app_info);
		application->setonClientDisconnectListener(app_info);
		application->setonErrorListener(app_info);
		application->setonDisconnectListener(app_info);
		application->addOnAllMessageListener(app_info);

		app_info->is_local = true;
		app_info->application = application;
		application->connect();
		_D("connect called");
		svc_info->application_instance_list.push_back(app_info);
	} else {
		_D("COMMUNCATION_START : uri : %s, channel_id : %s", uri.c_str(), channel_id.c_str());
		Application* application = new(std::nothrow) Application(&(svc_info->service_obj), uri, channel_id);
		ASSERT_ALLOC(application);

		// add listeners
		app_info->request_obj = &(svc_info->registered_request);
		application->setonConnectListener(app_info);
		application->setonClientConnectListener(app_info);
		application->setonClientDisconnectListener(app_info);
		application->setonErrorListener(app_info);
		application->setonDisconnectListener(app_info);
		((Application*)application)->setonStartAppListener(app_info);
		((Application*)application)->setonStopAppListener(app_info);
		application->addOnAllMessageListener(app_info);

		app_info->is_local = false;
		app_info->application = application;
		application->connect();

		svc_info->application_instance_list.push_back(app_info);
	}
	_D("connect requested");

	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::stop_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_activation_state == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/stop requested");
	app_comm_service_info *svc_info = reinterpret_cast<app_comm_service_info*>(request_obj->service_info);

	json channel;
	request_obj->get_channel_from_description(&channel);

	string uri, channel_id;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channel_id);

	for (application_instance_list_t::iterator iter = svc_info->application_instance_list.begin(); iter != svc_info->application_instance_list.end(); ++iter) {
		_D("%s, %s", (*iter)->uri.c_str(), (*iter)->channel_id.c_str());
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channel_id.compare(channel_id) ) {
			application_instance *app_info = *iter;

			_D("COMMUNCATION_STOP : uri : %s, channel_id : %s", uri.c_str(), channel_id.c_str());

			if ( *iter == NULL ) {
				_D("iter is NULL");
			}

			if ( svc_info->is_local ) {
				app_info->application->disconnect();
				svc_info->application_instance_list.erase(iter);
			} else {
				((Application*)app_info->application)->stop();
				svc_info->application_instance_list.erase(iter);
			}
			return CONV_ERROR_NONE;
		}
	}

	_D("no connection found");
	return CONV_ERROR_NONE;
}


int conv::app_comm_service_provider::get_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_activation_state == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/get requested");
	app_comm_service_info *svc_info = reinterpret_cast<app_comm_service_info*>(request_obj->service_info);

	json channel;
	request_obj->get_channel_from_description(&channel);

	string uri, channel_id;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channel_id);

	application_instance *app_info = NULL;

	for (application_instance_list_t::iterator iter = svc_info->application_instance_list.begin(); iter != svc_info->application_instance_list.end(); ++iter) {
		_D("iteration");
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channel_id.compare(channel_id) ) {
			if ( (*iter)->application != NULL ) {
				_D("app_info exists and application instance exists");
				app_info = (*iter);

				Clients* client_list = app_info->application->getclients();

				if ( svc_info->registered_request == NULL) {
					_D("No callback is registered");
					request_obj->reply(CONV_ERROR_INVALID_OPERATION);
					delete request_obj;

					return CONV_ERROR_INVALID_OPERATION;
				}

				if (client_list == NULL) {
					_D("No clients");
					request_obj->reply(CONV_ERROR_NO_DATA);
					delete request_obj;

					return CONV_ERROR_NO_DATA;
				} else {
					std::list<Client> cl = client_list->lists();

					int cs_index = 0;

					Client client_obj;
					_D("clients size = %d", client_list->size());

					json result;

					for (auto cs_itr = cl.begin(); cs_itr != cl.end(); cs_itr++) {
						cs_index++;
						json client;

						client.set(NULL, CONV_JSON_CLIENT_ID, (*cs_itr).getId());
						client.set(NULL, CONV_JSON_IS_HOST, (*cs_itr).isHost());
						client.set(NULL, CONV_JSON_CONNECT_TIME, (*cs_itr).getConnectTime());
						Channel* cha = NULL;
						cha = (*cs_itr).getChannel();

						if (cha != NULL)
							client.set(NULL, CONV_JSON_CHANNEL_URI, cha->getChannelUri(NULL).c_str());

						result.array_append(NULL, CONV_JSON_CLIENT_LIST, client);
					}
					send_response(result, svc_info->registered_request);

					return CONV_ERROR_NONE;
				}
			}
		}
	}

	_D("service is not started");

	return CONV_ERROR_INVALID_OPERATION;
}

int conv::app_comm_service_provider::send_response(json payload, request* request_obj)
{
	_D(RED("publishing_response"));
	json result;

	json description = request_obj->get_description();

	payload.set(NULL, CONV_JSON_RESULT_TYPE, "getClient");

	result.set(NULL, CONV_JSON_DESCRIPTION, description);
	result.set(NULL, CONV_JSON_PAYLOAD, payload);
	request_obj->publish(CONV_ERROR_NONE, result);

	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::set_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_activation_state == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/set requested");
	app_comm_service_info *svc_info = reinterpret_cast<app_comm_service_info*>(request_obj->service_info);

	json channel;
	request_obj->get_channel_from_description(&channel);

	string uri, channel_id;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channel_id);

	for (application_instance_list_t::iterator iter = svc_info->application_instance_list.begin(); iter != svc_info->application_instance_list.end(); ++iter) {
		_D("iteration");
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channel_id.compare(channel_id) ) {
			if ( (*iter)->application != NULL ) {
				_D("publishing payload");

				json payload;

				request_obj->get_payload_from_description(&payload);

				char* message = new(std::nothrow) char[strlen(payload.str().c_str())+1];
				ASSERT_ALLOC(message);

				strncpy(message, payload.str().c_str(), strlen(payload.str().c_str())+1);

				string payload_str = payload.str();
				_D("payload : %s, size : %d", message, strlen(message));

				(*iter)->application->publish("d2d_service_message", NULL, reinterpret_cast<unsigned char*>(message), strlen(message));

				_D("publishing done");

				delete[] message;
				return CONV_ERROR_NONE;
			}
		}
	}
	_D("service is not started");

	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::register_request(request* request_obj)
{
	IF_FAIL_RETURN_TAG(_activation_state == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/recv requested");
	app_comm_service_info *svc_info = reinterpret_cast<app_comm_service_info*>(request_obj->service_info);

	switch (request_obj->get_type()) {
	case REQ_SUBSCRIBE:
		if ( svc_info->registered_request != NULL ) {
			delete svc_info->registered_request;
		}
		svc_info->registered_request = request_obj;
		request_obj->reply(CONV_ERROR_NONE);
		_D("subscribe requested");
		break;
	case REQ_UNSUBSCRIBE:
		svc_info->registered_request = NULL;
		request_obj->reply(CONV_ERROR_NONE);
		delete request_obj;
		break;
	default:
		request_obj->reply(CONV_ERROR_INVALID_OPERATION);
		delete request_obj;
		return CONV_ERROR_INVALID_OPERATION;
		break;
	}
	return CONV_ERROR_NONE;
}

int conv::app_comm_service_provider::get_service_info_for_discovery(json* json_obj)
{
	json_obj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_TYPE, CONV_SERVICE_APP_TO_APP_COMMUNICATION);

	// set data for service handle
	connection_h connection;
	IF_FAIL_RETURN_TAG(connection_create(&connection) == CONNECTION_ERROR_NONE, CONV_ERROR_NOT_SUPPORTED, _E, "connection error");

	char* address = NULL;
	int ret = connection_get_ip_address(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &address);

	if(ret != CONNECTION_ERROR_NONE) {
		_E("connection error");
		if (connection_get_ip_address(connection, CONNECTION_ADDRESS_FAMILY_IPV6, &address) != CONNECTION_ERROR_NONE) {
			_E("connection error");
			connection_destroy(connection);
			return CONV_ERROR_NOT_SUPPORTED;
		}
	}

	if ( address == NULL || strlen(address) < 1 ) {
		_E("connection error");
		connection_destroy(connection);
		return CONV_ERROR_NOT_SUPPORTED;
	} else {
		char uri[200];
		Service local_service = Service::getLocal();

		if (local_service.getUri().empty()) {
			g_free(address);
			return CONV_ERROR_NOT_SUPPORTED;
		}

		snprintf(uri, sizeof(uri), "http://%s:8001/api/v2/", address);

		json info;

		info.set(NULL, CONV_JSON_SERVICE_DATA_URI, uri);
		info.set(NULL, CONV_JSON_SERVICE_DATA_VERSION, local_service.getVersion());
		info.set(NULL, CONV_JSON_SERVICE_DATA_TYPE, local_service.getType());
		info.set(NULL, CONV_JSON_SERVICE_DATA_ID, local_service.getId());
		info.set(NULL, CONV_JSON_SERVICE_DATA_NAME, local_service.getName());

		json_obj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_INFO, info);
		g_free(address);
	}

	connection_destroy(connection);
	return CONV_ERROR_NONE;
}
