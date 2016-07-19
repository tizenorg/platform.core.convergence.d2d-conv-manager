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
#include "Service.h"
#include "Search.h"
#include "SearchProvider.h"
#include "Debug.h"

map<string, ttl_info> SearchProvider::aliveMap;

ttl_info::ttl_info(long ttl, int service_type):msfd_ttl(0), mdns_ttl(0)
{
	if (service_type == MSFD)
		msfd_ttl = ttl;
	else if (service_type == MDNS)
		mdns_ttl = ttl;

}

ttl_info::ttl_info():msfd_ttl(0), mdns_ttl(0)
{

}

long ttl_info:: get_ttl(int service_type)
{
	if (service_type == MSFD)
		return msfd_ttl;
	else if (service_type == MDNS)
		return mdns_ttl;
	else
		return -1;

}

void ttl_info::update_ttl(long ttl, int service_type)
{
	if (service_type == MSFD) {
		msfd_ttl = ttl;
	} else if (service_type == MDNS) {
		mdns_ttl = ttl;
	}
}

bool ttl_info::is_expired()
{
	long now = time(0);

	MSF_DBG("msfd_remain_ttl = %d mdns_remain_ttl = %d", msfd_ttl - now, mdns_ttl - now);

	if (msfd_ttl < now && mdns_ttl < now)
		return true;
	else
		return false;
}

SearchProvider::SearchProvider()
{
	searching = false;
}

SearchProvider::SearchProvider(Search *searchListen)
{
	searchListener = searchListen;
	searching = false;
}

//list<Service> SearchProvider::getServices()
//{
//	return services;
//}

void SearchProvider::setServices(list<Service> listofservices)
{
	clearServices();
	if (listofservices.size() != 0) {
		std::list<Service>::const_iterator iterator;
		for (iterator = listofservices.begin(); iterator != listofservices.end(); ++iterator) {
			Search::services.push_back(*iterator);
		}
	}
}



void SearchProvider::clearServices()
{
	Search::services.clear();
}

Service SearchProvider::getServiceById(string id)
{
	std::list<Service>::iterator iterator;
	for (iterator = Search::services.begin(); iterator != Search::services.end(); ++iterator) {
		if (((*iterator).getId()) == id) {
			return(*iterator);
		}
	}
	return Service("", "", "", "", "");
}

Service SearchProvider::getServiceByIp(string ip)
{
	std::list<Service>::iterator iterator;
	for (iterator = Search::services.begin(); iterator != Search::services.end(); ++iterator) {

		string url = (*iterator).getUri();
		std::string::size_type pos = url.find(ip);
		if (pos != string::npos) {
			return(*iterator);
		}
	}
	return Service("", "", "", "", "");
}

bool SearchProvider::isSearching()
{
	if (this->searching)
		return true;
	else
		return false;
}

void SearchProvider::push_in_alivemap(long ttl, string id, int service_type)
{
	MSF_DBG("push_in_alivemap : ttl = %d, id = %s, service_type = %d", ttl, id.c_str(), service_type);

	if (id.empty()) {
		return;
	}

	if (aliveMap.find(id) == aliveMap.end()) {
		ttl_info info;
		info.update_ttl(time(0) + ttl, service_type);
		aliveMap[id] = info;
	} else {
		updateAlive(ttl, id, service_type);
	}
}

void SearchProvider::updateAlive(long ttl, string id, int service_type)
{
	MSF_DBG("updateAlive : ttl = %d, id = %s, service_type = %s", ttl, id.c_str(), (service_type == 1)? "MSFD" : "MDNS");

	if (id.empty()) {
		MSF_DBG("updateAlive id is empty");
		return;
	}

	if (aliveMap.find(id) == aliveMap.end()) {
	} else {
		long _ttl=time(0) + ttl;

		ttl_info info = aliveMap[id];

		info.update_ttl(_ttl, service_type);
		aliveMap[id]=info;
	}
}

void SearchProvider::reapServices()
{
	map<string,ttl_info>::iterator it;
	for(it=aliveMap.begin();it!=aliveMap.end();++it) {
		ttl_info info=it->second;
		if ( info.is_expired()) {
			Service service=getServiceByIp(it->first);
			MSF_DBG("reapServices - Remove service : [%s]", service.getId().c_str());
			aliveMap.erase(it->first);
			Search::removeServiceAndNotify(service);
		}
	}
}

std::string SearchProvider::getIP(std::string url)
{
	std::string::size_type pos1  = url.find("192");
	std::string::size_type pos2  = url.find(":", 6);

	std::string::size_type pos = pos2 - pos1;

	std::string sub = url.substr(pos1, pos);

	return sub;
}
