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

list<Service> SearchProvider::services;
map<string, ttl_info> SearchProvider::aliveMap;

ttl_info::ttl_info(long ttl, int service_type):msfd_ttl(0), mdns_ttl(0)
{
	if(service_type == MSFD)
		msfd_ttl = ttl;
	else if(service_type == MDNS)
		mdns_ttl = ttl;

}

ttl_info::ttl_info():msfd_ttl(0), mdns_ttl(0)
{

}

long ttl_info:: get_ttl(int service_type)
{
	if(service_type == MSFD)
		return msfd_ttl;
	else if(service_type == MDNS)
		return mdns_ttl;
	else
		return -1;

}

int ttl_info::update_ttl(long ttl, int service_type)
{
	if(service_type == MSFD) {
		msfd_ttl = ttl;
		MSF_DBG("MSFD ttl update : %d", ttl);
	} else if(service_type == MDNS) {
		mdns_ttl = ttl;
		MSF_DBG("mDNS ttl update : %d", ttl);
	}
}


bool ttl_info::is_expired()
{
	long now = time(0);

	MSF_DBG("Expired => msfd : %d, mdns : %d", msfd_ttl < now, mdns_ttl < now);
	if(msfd_ttl < now && mdns_ttl < now)
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

list<Service> SearchProvider::getServices()
{
	return services;
}

void SearchProvider::setServices(list<Service> listofservices)
{
	clearServices();
	if (listofservices.size() != 0) {
		std::list<Service>::const_iterator iterator;
		for (iterator = listofservices.begin(); iterator != listofservices.end(); ++iterator) {
			services.push_back(*iterator);
		}
	}
}

void SearchProvider::setSearchListener(Search *obj)
{
	searchListener = obj;
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
}

void SearchProvider::addService(Service service)
{
	dlog_print(DLOG_INFO, "MSF", "SearchProvider::addService()");
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	bool found = false;

	std::list<Service>::iterator iterator;

	bool match = false;

	for (iterator = services.begin(); iterator != services.end(); ++iterator) {
		if (iterator->getId() == service.getId()) {
			match = true;
			break;
		}
	}

	if (!match) {
		services.push_back(service);
		found = true;
		MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);

		if (found) {
			Search::st_onFound(service);
		}
	} else {
	}
}

void SearchProvider::removeService(Service service)
{
	if (&service == NULL) {
		return;
	}

	std::list<Service>::iterator it = services.begin();
	while (it != services.end()) {
		if (((*it).getId()) == (service.getId())) {
			services.erase(it++);
		} else {
			it++;
		}
	}
}

void SearchProvider::removeServiceAndNotify(Service service)
{
	removeService(service);
	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n", __FUNCTION__, __LINE__, __FILE__);
	Search::st_onLost(service);
}

void SearchProvider::clearServices()
{
	services.clear();
}

Service SearchProvider::getServiceById(string id)
{
	//	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n",__FUNCTION__ ,__LINE__,__FILE__);
	std::list<Service>::iterator iterator;
	for (iterator = services.begin(); iterator != services.end(); ++iterator) {
		if (((*iterator).getId()) == id) {
			return(*iterator);
		}
	}
	return Service("", "", "", "", "");
}

Service SearchProvider::getServiceByIp(string ip)
{
	//	MSF_DBG("\n [MSF : API] Debug log Function : [%s] and line [%d] in file [%s] \n",__FUNCTION__ ,__LINE__,__FILE__);
	std::list<Service>::iterator iterator;
	for (iterator = services.begin(); iterator != services.end(); ++iterator) {

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

void SearchProvider::updateAlive(long ttl, string id, int service_type)
{
	MSF_DBG("updateAlive : ttl = %d, id = %s, service_type = %d", ttl, id.c_str(), service_type);

	if(id.empty()) {
		return;
	}
	long _ttl=time(0) + ttl;
	ttl_info info = aliveMap[id];
	MSF_DBG("mdns ttl : %d , msfd ttl : %d", info.get_ttl(MDNS), info.get_ttl(MSFD));
	info.update_ttl(_ttl, service_type);
	aliveMap[id]=info;
}

void SearchProvider::reapServices()
{
	dlog_print(DLOG_ERROR, "MSF", "reapServices");
	map<string,ttl_info>::iterator it;
	for(it=aliveMap.begin();it!=aliveMap.end();++it) {
		ttl_info info=it->second;
		dlog_print(DLOG_INFO, "MSF_API", "reapService - Service id: %s", it->first.c_str());
		if( info.is_expired()) {
			Service service=getServiceByIp(it->first);
			dlog_print(DLOG_ERROR, "MSF", "reapServices - Remove service : [%s]", service.getId().c_str());
			aliveMap.erase(it->first);
			removeServiceAndNotify(service);
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
