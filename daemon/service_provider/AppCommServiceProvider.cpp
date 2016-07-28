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

#include <net_connection.h>
#include "AppCommServiceProvider.h"
#include "../ClientManager.h"
#include "../Util.h"

using namespace std;

static void vconf_update_cb(keynode_t *node, void* user_data)
{
	conv::AppCommServiceProvider* instance = static_cast<conv::AppCommServiceProvider*>(user_data);
	IF_FAIL_VOID_TAG(instance, _E, "static_cast failed");

	instance->handleVconfUpdate(node);
}

conv::AppCommServiceProvider::AppCommServiceProvider()
{
	__type = CONV_SERVICE_TYPE_SMARTVIEW_APP_COMMUNICATION;
	__resourceType = CONV_RESOURCE_TYPE_SMARTVIEW_APP_COMMUNICATION;
	__uri = CONV_URI_SMARTVIEW_APP_COMMUNICATION;
	if (conv::util::isServiceActivated(CONV_INTERNAL_SERVICE_APP_TO_APP_COMMUNICATION))
		__activationState = 1;
	else
		__activationState = 0;

	vconf_notify_key_changed(VCONFKEY_SETAPPL_D2D_CONVERGENCE_SERVICE, vconf_update_cb, this);
}

int conv::AppCommServiceProvider::handleVconfUpdate(keynode_t *node)
{
	int current_state = vconf_keynode_get_int(node);

	if ((CONV_INTERNAL_SERVICE_APP_TO_APP_COMMUNICATION & current_state) > 0) {
		__activationState = 1;
		init();
	} else {
		__activationState = 0;
		release();
	}

	return CONV_ERROR_NONE;
}

conv::AppCommServiceProvider::~AppCommServiceProvider()
{
}

int conv::AppCommServiceProvider::init()
{
	_D("AppCommServiceProvider init done");
	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::release()
{
	_D("AppCommServiceProvider release done");
	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::loadServiceInfo(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	string client;
	conv::ClientInfo* clientObj = NULL;
	int isLocal = 0;

	Json service;
	string name, version, type, id, uri;

	Json description = requestObj->getDescription();

	description.get(NULL, CONV_JSON_SERVICE, &service);

	service.get(NULL, CONV_JSON_SERVICE_DATA_NAME, &name);
	service.get(NULL, CONV_JSON_SERVICE_DATA_VERSION, &version);
	service.get(NULL, CONV_JSON_SERVICE_DATA_TYPE, &type);
	service.get(NULL, CONV_JSON_SERVICE_DATA_ID, &id);
	service.get(NULL, CONV_JSON_SERVICE_DATA_URI, &uri);

	description.get(NULL, CONV_JSON_IS_LOCAL, &isLocal);

	client = requestObj->getSender();

	_D("client id : %s", client.c_str());
	clientObj = conv::client_manager::getClient(client);
	IF_FAIL_RETURN_TAG(clientObj, CONV_ERROR_OUT_OF_MEMORY, _E, "client info alloc failed");

	AppCommServiceInfo *svcInfo = NULL;

	if (isLocal == 1) {
		IServiceInfo* svcInfoBase = clientObj->getServiceInfo(__type, "LOCAL_HOST");

		if ( svcInfoBase != NULL ) {
			_D("local service instance already exists");
			svcInfo = reinterpret_cast<AppCommServiceInfo*>(svcInfoBase);

			if (svcInfo == NULL) {
				_D("casting failed");
				return CONV_ERROR_INVALID_OPERATION;
			}
		} else {
			_D("allocating new service instance for local service");
			svcInfo = new(std::nothrow) AppCommServiceInfo();
			ASSERT_ALLOC(svcInfo);

			svcInfo->isLocal = true;
			clientObj->addServiceInfo(__type, "LOCAL_HOST", (IServiceInfo*)svcInfo);

			_D("MSF service is created");
		}
	} else {
		IServiceInfo* svcInfoBase = clientObj->getServiceInfo(__type, id);

		if ( svcInfoBase != NULL ) {
			_D("service instance already exists");
			svcInfo = reinterpret_cast<AppCommServiceInfo*>(svcInfoBase);

			if (svcInfo == NULL) {
				_D("casting failed");
				return CONV_ERROR_INVALID_OPERATION;
			}
		} else {
			_D("allocating new service instance");
			svcInfo = new(std::nothrow) AppCommServiceInfo();
			ASSERT_ALLOC(svcInfo);

			_D("uri : %s", uri.c_str());
			Service::getByURI(uri, 2000, svcInfo);
			IF_FAIL_RETURN_TAG(svcInfo->readRequestResult == true, CONV_ERROR_INVALID_OPERATION, _E, "getByURI failed");
			svcInfo->isLocal = false;
			clientObj->addServiceInfo(__type, id, (IServiceInfo*)svcInfo);

			_D("MSF service is created");
		}
	}
	requestObj->setServiceInfo(svcInfo);
	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::startRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/start requested");
	AppCommServiceInfo *svcInfo = reinterpret_cast<AppCommServiceInfo*>(requestObj->getServiceInfo());

	Json channel;
	requestObj->getChannelFromDescription(&channel);

	string uri, channelId;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channelId);

	ApplicationInstance *appInfo = NULL;

	for (ApplicationInstanceList::iterator iter = svcInfo->applicationInstanceList.begin(); iter != svcInfo->applicationInstanceList.end(); ++iter) {
		_D("iteration");
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channelId.compare(channelId) ) {
			if ( (*iter)->application != NULL  ) {
				_D("already started");
				// check if it's connected and re-try if it's not
				return CONV_ERROR_INVALID_OPERATION;
			} else {
				_D("appInfo exists but no application instance");
				appInfo = (*iter);
				break;
			}
		}
	}

	if ( appInfo == NULL ) {
		appInfo = new(std::nothrow) ApplicationInstance();
		ASSERT_ALLOC(appInfo);

		appInfo->uri = uri;
		appInfo->channelId = channelId;
	}

	// if it's local -> server application
	if ( svcInfo->isLocal ) {
		_D("COMMUNCATION_START : local channel. channel_id : %s", channelId.c_str());

		appInfo->localService = Service::getLocal();

		Channel* application = appInfo->localService.createChannel(channelId);

		// add listeners
		appInfo->requestObj = &(svcInfo->registeredRequest);
		application->setonConnectListener(appInfo);
		application->setonClientConnectListener(appInfo);
		application->setonClientDisconnectListener(appInfo);
		application->setonErrorListener(appInfo);
		application->setonDisconnectListener(appInfo);
		application->addOnAllMessageListener(appInfo);
		application->setonPublishListener(appInfo);

		appInfo->isLocal = true;
		appInfo->application = application;
		application->connect();
		_D("connect called");
		svcInfo->applicationInstanceList.push_back(appInfo);
	} else {
		_D("COMMUNCATION_START : uri : %s, channel_id : %s", uri.c_str(), channelId.c_str());
		Application* application = new(std::nothrow) Application(&(svcInfo->serviceObj), uri, channelId);
		ASSERT_ALLOC(application);

		// add listeners
		appInfo->requestObj = &(svcInfo->registeredRequest);
		application->setonConnectListener(appInfo);
		application->setonClientConnectListener(appInfo);
		application->setonClientDisconnectListener(appInfo);
		application->setonErrorListener(appInfo);
		application->setonDisconnectListener(appInfo);
		((Application*)application)->setonStartAppListener(appInfo);
		((Application*)application)->setonStopAppListener(appInfo);
		application->addOnAllMessageListener(appInfo);
		application->setonPublishListener(appInfo);

		appInfo->isLocal = false;
		appInfo->application = application;
		application->connect();

		svcInfo->applicationInstanceList.push_back(appInfo);
	}
	_D("connect requested");

	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::stopRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/stop requested");
	AppCommServiceInfo *svcInfo = reinterpret_cast<AppCommServiceInfo*>(requestObj->getServiceInfo());

	Json channel;
	requestObj->getChannelFromDescription(&channel);

	string uri, channelId;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channelId);

	for (ApplicationInstanceList::iterator iter = svcInfo->applicationInstanceList.begin(); iter != svcInfo->applicationInstanceList.end(); ++iter) {
		_D("%s, %s", (*iter)->uri.c_str(), (*iter)->channelId.c_str());
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channelId.compare(channelId) ) {
			ApplicationInstance *appInfo = *iter;

			_D("COMMUNCATION_STOP : uri : %s, channel_id : %s", uri.c_str(), channelId.c_str());

			if ( *iter == NULL ) {
				_D("iter is NULL");
			}

			if ( svcInfo->isLocal ) {
				appInfo->application->disconnect();
				svcInfo->applicationInstanceList.erase(iter);
			} else {
				((Application*)appInfo->application)->stop();
				svcInfo->applicationInstanceList.erase(iter);
			}
			return CONV_ERROR_NONE;
		}
	}

	_D("no connection found");
	return CONV_ERROR_NONE;
}


int conv::AppCommServiceProvider::readRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/get requested");
	AppCommServiceInfo *svcInfo = reinterpret_cast<AppCommServiceInfo*>(requestObj->getServiceInfo());

	Json channel;
	requestObj->getChannelFromDescription(&channel);

	string uri, channelId;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channelId);

	ApplicationInstance *appInfo = NULL;
	Json result;

	for (ApplicationInstanceList::iterator iter = svcInfo->applicationInstanceList.begin(); iter != svcInfo->applicationInstanceList.end(); ++iter) {
		_D("iteration");
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channelId.compare(channelId) ) {
			if ( (*iter)->application != NULL ) {
				_D("appInfo exists and application instance exists");
				appInfo = (*iter);

				Clients* client_list = appInfo->application->getclients();

				if ( svcInfo->registeredRequest == NULL) {
					_D("No callback is registered");
					sendReadResponse(result, CONV_JSON_GET_CLIENTS, CONV_ERROR_INVALID_OPERATION, svcInfo->registeredRequest);

					return CONV_ERROR_INVALID_OPERATION;
				}

				if (client_list == NULL) {
					_D("No clients");
					sendReadResponse(result, CONV_JSON_GET_CLIENTS, CONV_ERROR_NO_DATA, svcInfo->registeredRequest);

					return CONV_ERROR_NO_DATA;
				} else {
					std::list<Client> cl = client_list->lists();

					int cs_index = 0;

					Client clientObj;
					_D("clients size = %d", client_list->size());
					for (auto cs_itr = cl.begin(); cs_itr != cl.end(); cs_itr++) {
						cs_index++;
						Json client;

						client.set(NULL, CONV_JSON_CLIENT_ID, (*cs_itr).getId());
						client.set(NULL, CONV_JSON_IS_HOST, (*cs_itr).isHost());
						client.set(NULL, CONV_JSON_CONNECT_TIME, (*cs_itr).getConnectTime());
						Channel* cha = NULL;
						cha = (*cs_itr).getChannel();

						if (cha != NULL)
							client.set(NULL, CONV_JSON_CHANNEL_URI, cha->getChannelUri(NULL).c_str());

						result.appendArray(NULL, CONV_JSON_CLIENT_LIST, client);
					}
					sendReadResponse(result, CONV_JSON_GET_CLIENTS, CONV_ERROR_NONE, svcInfo->registeredRequest);

					return CONV_ERROR_NONE;
				}
			}
		}
	}
	_D("service is not started");
	sendReadResponse(result, CONV_JSON_GET_CLIENTS, CONV_ERROR_INVALID_OPERATION, svcInfo->registeredRequest);

	return CONV_ERROR_INVALID_OPERATION;
}

int conv::AppCommServiceProvider::sendReadResponse(Json payload, const char* read_type, conv_error_e error, Request* requestObj)
{
	_D(RED("publishing_response"));
	IF_FAIL_RETURN_TAG(requestObj != NULL, CONV_ERROR_INVALID_OPERATION, _E, "listener_cb is not registered");

	Json result;
	Json description = requestObj->getDescription();

	payload.set(NULL, CONV_JSON_RESULT_TYPE, CONV_JSON_ON_READ);
	payload.set(NULL, CONV_JSON_READ_TYPE, read_type);

	result.set(NULL, CONV_JSON_DESCRIPTION, description);
	result.set(NULL, CONV_JSON_PAYLOAD, payload);
	requestObj->publish(error, result);

	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::publishRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/set requested");
	AppCommServiceInfo *svcInfo = reinterpret_cast<AppCommServiceInfo*>(requestObj->getServiceInfo());

	Json channel;
	requestObj->getChannelFromDescription(&channel);

	string uri, channelId;

	channel.get(NULL, CONV_JSON_URI, &uri);
	channel.get(NULL, CONV_JSON_CHANNEL_ID, &channelId);

	for (ApplicationInstanceList::iterator iter = svcInfo->applicationInstanceList.begin(); iter != svcInfo->applicationInstanceList.end(); ++iter) {
		_D("iteration");
		if ( (*iter) != NULL && !(*iter)->uri.compare(uri) && !(*iter)->channelId.compare(channelId) ) {
			if ( (*iter)->application != NULL ) {
				_D("publishing payload");

				Json payload;

				requestObj->getPayloadFromDescription(&payload);

				char* message = new(std::nothrow) char[strlen(payload.str().c_str())+1];
				ASSERT_ALLOC(message);

				strncpy(message, payload.str().c_str(), strlen(payload.str().c_str())+1);

				string payload_str = payload.str();
				_D("payload : %s, size : %d", message, strlen(message));

				(*iter)->application->publish("d2d_service_message", NULL, reinterpret_cast<unsigned char*>(message), strlen(message), NULL);

				_D("publishing done");

				delete[] message;
				return CONV_ERROR_NONE;
			}
		}
	}
	_D("service is not started");

	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::registerRequest(Request* requestObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_INVALID_OPERATION, _E, "service provider is not activated");

	_D("communcation/recv requested");
	AppCommServiceInfo *svcInfo = reinterpret_cast<AppCommServiceInfo*>(requestObj->getServiceInfo());

	switch (requestObj->getType()) {
	case REQ_SUBSCRIBE:
		if ( svcInfo->registeredRequest != NULL ) {
			delete svcInfo->registeredRequest;
		}
		svcInfo->registeredRequest = requestObj;
		requestObj->reply(CONV_ERROR_NONE);
		_D("subscribe requested");
		break;
	case REQ_UNSUBSCRIBE:
		svcInfo->registeredRequest = NULL;
		requestObj->reply(CONV_ERROR_NONE);
		delete requestObj;
		break;
	default:
		requestObj->reply(CONV_ERROR_INVALID_OPERATION);
		delete requestObj;
		return CONV_ERROR_INVALID_OPERATION;
		break;
	}
	return CONV_ERROR_NONE;
}

int conv::AppCommServiceProvider::getServiceInfoForDiscovery(Json* jsonObj)
{
	IF_FAIL_RETURN_TAG(__activationState == 1, CONV_ERROR_NOT_SUPPORTED, _E, "service provider is not activated");

	jsonObj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_TYPE, CONV_SERVICE_APP_TO_APP_COMMUNICATION);

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
		Service localService = Service::getLocal();

		if (localService.getUri().empty()) {
			g_free(address);
			return CONV_ERROR_NOT_SUPPORTED;
		}

		snprintf(uri, sizeof(uri), "http://%s:8001/api/v2/", address);

		Json info;

		info.set(NULL, CONV_JSON_SERVICE_DATA_URI, uri);
		info.set(NULL, CONV_JSON_SERVICE_DATA_VERSION, localService.getVersion());
		info.set(NULL, CONV_JSON_SERVICE_DATA_TYPE, localService.getType());
		info.set(NULL, CONV_JSON_SERVICE_DATA_ID, localService.getId());
		info.set(NULL, CONV_JSON_SERVICE_DATA_NAME, localService.getName());

		jsonObj->set(NULL, CONV_JSON_DISCOVERY_SERVICE_INFO, info);
		g_free(address);
	}

	connection_destroy(connection);
	return CONV_ERROR_NONE;
}
