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

#ifndef __APP_COMMUNICATION_SERVICE_APPLICATION_H__
#define __APP_COMMUNICATION_SERVICE_APPLICATION_H__

#include <glib.h>
#include <vector>
#include <vconf.h>
#include "../IServiceProvider.h"
#include "Application.h"

namespace conv {
	class AppCommServiceApplication : public OnConnectListener, public OnDisconnectListener, public OnClientConnectListener, public OnClientDisconnectListener,
									public OnMessageListener, public OnErrorListener, public OnStartAppListener, public OnStopAppListener, public OnPublishListener {
		public:
			virtual ~AppCommServiceApplication();

			conv::Request** requestObj;
			Channel* application;
			string uri;
			string channelId;
			Service localService;
			bool isLocal;

			void onStart(bool start_result);
			void onStop(bool start_result);
			void onConnect(Client client);
			void onDisconnect(Client client);
			void onClientConnect(Client client);
			void onClientDisconnect(Client client);
			void onMessage(Message message);
			void onError(Error error);
			void onPublished(bool publish_result, void* user_data);
			void publishResponse(int error, string result_type, Client *client);
	};
}
#endif /* __APP_COMMUNICATION_SERVICE_APPLICATION_H__ */
