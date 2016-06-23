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
//#include "Service.h"
//#include "Search.h"

using namespace std;

class Service;
class Search;

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

protected:
	bool searching;
	void setServices(list<Service>);
	void addService(Service service);
	void removeService(Service service);
	void removeServiceAndNotify(Service service);
	void clearServices();
	Service getServiceById(string id);
};

#endif
