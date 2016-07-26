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

#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <string>
#include <gio/gio.h>
#include "Types.h"
#include "Json.h"
#include "IServiceInfo.h"
#include "ICommunicationInfo.h"
#include "access_control/PeerCreds.h"

namespace conv {
	class Request {
		public:
			Request(int type, const char *client, int reqId, const char *subj, const char *desc);
			Request(int type,
			const char *client, int reqId, const char *subj, const char *desc,
			const char *sender, Credentials *creds, GDBusMethodInvocation *inv);
			~Request();

			int getType();
			int getId();
			const char *getClient();
			const char *getSender();
			const char *getSubject();
			Json& getDescription();
			Credentials *getCreds();
			bool reply(int error);
			bool reply(int error, Json &requestResult);
			bool reply(int error, Json &requestResult, Json &readData);
			bool publish(int error, Json &data);

			bool getChannelFromDescription(Json* target);
			bool getPayloadFromDescription(Json* target);

			IServiceInfo* getServiceInfo();
			ICommunicationInfo* getCommunicationInfo();
			void setServiceInfo(IServiceInfo* serviceInfo);
			void setCommunicationInfo(ICommunicationInfo* communicationInfo);

		protected:
			int __type;
			int __reqId;
			std::string __client;
			std::string __subject;
			Json __description;
			Credentials *__creds;
			std::string __sender;
			GDBusMethodInvocation *__invocation;

			IServiceInfo* __serviceInfo;
			ICommunicationInfo* __communicationInfo;
	};
}

#endif	/* End of __REQUEST_H__ */
