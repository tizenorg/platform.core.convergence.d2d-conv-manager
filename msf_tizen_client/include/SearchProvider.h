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

#ifndef __MSF_SEARCH_PROVIDER_H__
#define __MSF_SEARCH_PROVIDER_H__

#include<iostream>
#include<list>
#include<string>
#include <map>

//#include "Service.h"
//#include "Search.h"

#define TTL 10
#define MSFD 1
#define MDNS 2

using namespace std;

class Service;
class Search;

class ttl_info
{

private:
	long msfd_ttl;
	long mdns_ttl;

public:
	ttl_info(long ttl, int service_type);
	ttl_info();
	int update_ttl(long ttl, int service_type);
	long get_ttl(int service_type);
	bool is_expired();
};

class SearchProvider
{
private:
	Search *searchListener;
	static list<Service> services;

public:
	SearchProvider();
	SearchProvider(Search *);
	virtual ~SearchProvider(){}
	virtual void start() {}
	virtual bool stop() { return true; }
	static list<Service> getServices();
	bool isSearching();
	void setSearchListener(Search *obj);
	std::string getIP(std::string url);
	void addService(Service service);
	void push_in_alivemap(long ttl, string id , int service_type);
	void updateAlive(long ttl, string id , int service_type);
	static void reapServices();

protected:
	bool searching;
	void setServices(list<Service>);
	static void removeService(Service service);
	static void removeServiceAndNotify(Service service);
	void clearServices();
	Service getServiceById(string id);
	static Service getServiceByIp(string ip);
	static map<string, ttl_info> aliveMap;

};

#endif
