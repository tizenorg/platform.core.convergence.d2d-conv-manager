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

#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "MSFDSearchProvider.h"
#include "Service.h"
#include "Debug.h"
#include "Error.h"
#include "Result.h"
#include "Channel.h"
#include "Search.h"
#define MSGBUFSIZE 1000
#define MULTICAST_PORT 8001
#define MULTICAST_GROUP "224.0.0.7"

string MSFDSearchProvider::KEY_TYPE_STATE = "type";
string MSFDSearchProvider::KEY_TTL = "ttl";
string MSFDSearchProvider::KEY_SID = "sid";
string MSFDSearchProvider::KEY_DATA = "data";
string MSFDSearchProvider::KEY_VERSION_1 = "v1";
string MSFDSearchProvider::KEY_VERSION_2 = "v2";
string MSFDSearchProvider::KEY_URI = "uri";
string MSFDSearchProvider::TYPE_DISCOVER = "discover";
string MSFDSearchProvider::STATE_UP = "up";
string MSFDSearchProvider::STATE_DOWN = "down";
string MSFDSearchProvider::STATE_ALIVE = "alive";
int MSFDSearchProvider::SERVICE_CHECK_TIMEOUT = 5000;

int MSFDSearchProvider::flag = 0;
long MSFDSearchProvider::ttl = 0;

class ResultMSFDServiceCallback : public Result_Base
{
	public:
		MSFDSearchProvider* MSFDSearchProvider_pointer = NULL;
		long ttl;
		string ip_id;
		int provider_type;

		void onSuccess(Service svc)
		{
			MSF_DBG("MSFD Service Callback onSucces()");
			MSFDSearchProvider_pointer->push_in_alivemap(ttl, ip_id, provider_type);
			Search::addService(svc);
		}

		void onError(Error)
		{
			MSF_DBG("MSFD Service Callback onError()");
		}
};


MSFDSearchProvider::MSFDSearchProvider()
{
	SearchProvider();
}

MSFDSearchProvider::MSFDSearchProvider(Search *sListener):SearchProvider(sListener)
{
}

MSFDSearchProvider::~MSFDSearchProvider()
{
	if (fd != 0) {
		close(fd);
		fd = 0;
	}
}

SearchProvider MSFDSearchProvider::create()
{
	return (SearchProvider)MSFDSearchProvider();
}

SearchProvider MSFDSearchProvider::create(Search *searchListener)
{
	return (SearchProvider)MSFDSearchProvider(searchListener);
}

void MSFDSearchProvider::start()
{
	MSF_DBG("MSFD start()");
	if (searching) {
		stop();
	}
	clearServices();

	createMSFD();
}

void MSFDSearchProvider::createMSFD()
{
	struct sockaddr_in addr;
	struct sockaddr_in msf_server_addr;
	int  nbytes;
	unsigned int addrlen;
	struct ip_mreq mreq;
	char msgbuf[MSGBUFSIZE];
	fd = 0;
	u_int yes = 1;          /*** MODIFICATION TO ORIGINAL */

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		MSF_DBG("MSFD socket faile");
		return;
	}

	/**** MODIFICATION TO ORIGINAL */
	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		MSF_DBG("MSFD reusing ADDR failed");
	}
	/*** END OF MODIFICATION TO ORIGINAL */

	/* set up destination address */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
	addr.sin_port = htons(MULTICAST_PORT);

	//dlog_print(DLOG_INFO, "MSF", "MSFD try bind socket");
	/* bind to receive address */
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		MSF_DBG("MSFD bind failed");
	}
	//dlog_print(DLOG_INFO, "MSF", "MSFD try bind socket success");

	/* use setsockopt() to request that the kernel join a multicast group */
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt");
		MSF_DBG("MSFD setsockopt failed");
	}

	struct timeval tv = {2, 0};

	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt");
		MSF_DBG("MSFD setsockopt(SOL_SOCKET) failed");
	}

	while (1) {
		addrlen = sizeof(msf_server_addr);
		if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &msf_server_addr, &addrlen)) < 0) {
			if (fd < 1)
				return;
		} else {
			msgbuf[nbytes] = '\0';
			processReceivedMsg(msgbuf, nbytes);
		}
	}
}

void MSFDSearchProvider::processReceivedMsg(char *buf, int buflen)
{
	if (buf == NULL)
		return;

	if (buflen < 10) {
		return;
	}

	MSF_DBG("buf = %s", buf);

	if (!json_parse(buf, buflen)) {
		return;
	}

	string ip;
	int port = 0;

	Channel::get_ip_port_from_uri(url, &ip, &port);
	map<string,ttl_info>::iterator itr = aliveMap.find(ip);

	if (state == TYPE_DISCOVER) {
		return;
	} else if (state == STATE_ALIVE || state == STATE_UP) {

		if (itr == aliveMap.end()) {
			ResultMSFDServiceCallback *rService = new ResultMSFDServiceCallback();
			rService->MSFDSearchProvider_pointer = this;
			rService->ttl = ttl;
			rService->provider_type = MSFD;
			rService->ip_id = ip;
			Service::getByURI(url, SERVICE_CHECK_TIMEOUT, rService);
		} else {
			updateAlive(ttl, ip, MSFD);
		}
	} else if (state == STATE_DOWN) {
		if (itr != aliveMap.end()) {
			aliveMap.erase(ip);
		}

		Service serv =	getServiceById(id);

		if (serv.getId().length() != 0) {
			Search::removeServiceAndNotify(serv);
		}
	}
}

void MSFDSearchProvider::foreach_json_object(JsonObject *object, const gchar *key, JsonNode *node, gpointer user_data)
{
	MSFDSearchProvider *p = static_cast<MSFDSearchProvider*>(user_data);

	if (json_node_get_node_type(node) == JSON_NODE_VALUE) {
		if (!strncmp(key , MSFDSearchProvider::KEY_TTL.c_str(), 2)) {
			p->ttl = json_node_get_int(node);
		} else if (!strncmp(key , MSFDSearchProvider::KEY_TYPE_STATE.c_str(), 2)) {
			p->state = json_node_get_string(node);
		} else if (!strncmp(key, MSFDSearchProvider::KEY_SID.c_str(), 4)) {
			p->id = json_node_get_string(node);
		} else if (!strncmp(key, MSFDSearchProvider::KEY_URI.c_str(), 7)) {
			if (flag == 1) {
				p->url = json_node_get_string(node);
				flag = 0;
			}
		}

	} else if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
		if (!strncmp(key , KEY_VERSION_2.c_str(), 7)) {
			flag = 1;
		}
		json_object_foreach_member(json_node_get_object(node), foreach_json_object, user_data);
	}
}

bool MSFDSearchProvider::json_parse(const char *in, int length)
{
	JsonParser *parser = json_parser_new();

	if (json_parser_load_from_data(parser, in, length, NULL)) {
		JsonNode *node = NULL;
		node = json_parser_get_root(parser);

		if (node == NULL) {
			MSF_DBG("json_parsing error");
			return false;
		}

		if (json_node_get_node_type(node) == JSON_NODE_OBJECT) {
			json_object_foreach_member(json_node_get_object(node), foreach_json_object, this);
		} else {
			MSF_DBG("json_parsing error");
			return false;
		}

	} else {
		MSF_DBG("json_parsing error");
		return false;
	}

	return true;
}

bool MSFDSearchProvider::stop()
{
	MSF_DBG("MSFD stop()");
	shutdown(fd, SHUT_RDWR);
	close(fd);
	fd = 0;
	return true;
}

