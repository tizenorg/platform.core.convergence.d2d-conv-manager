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
#include "common.h"
#include "conv_json.h"
#include "service_info_base.h"
#include "communication_info_base.h"
#include "access_control/peer_creds.h"

namespace conv {
	class request {
		public:
			request(int type, const char *client, int req_id, const char *subj, const char *desc);
			request(int type,
			const char *client, int req_id, const char *subj, const char *desc,
			const char *sender, credentials *creds, GDBusMethodInvocation *inv);
			~request();

			int get_type();
			int get_id();
			const char *get_client();
			const char *get_sender();
			const char *get_subject();
			json& get_description();
			credentials *get_creds();
			bool reply(int error);
			bool reply(int error, json &request_result);
			bool reply(int error, json &request_result, json &data_read);
			bool publish(int error, json &data);

			bool get_channel_from_description(json* target);
			bool get_payload_from_description(json* target);
			std::string service_type;

			std::string connection_type;
			service_info_base* service_info;
			communication_info_base* communication_info;

		protected:
			int _type;
			int _req_id;
			std::string _client;
			std::string _subject;
			json _description;
			credentials *__creds;
			std::string __sender;
			GDBusMethodInvocation *__invocation;
	};
}

#endif	/* End of __REQUEST_H__ */
