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

#include "Search.h"
#include "Error.h"
#include "Service.h"
#include "Debug.h"
#include "mDNSSearchProvider.h"
#include "MSFDSearchProvider.h"
#include "Result.h"

int Search::SERVICE_CHECK_TIMEOUT = 5000;
bool Search::clearProviders;
pthread_t Search::threads[NUM_OF_THREADS];
mDNSSearchProvider Search::provider_mdns;
MSFDSearchProvider Search::provider_msfd;
Search *Search::instance = NULL;
bool Search::starting = false;
bool Search::stopping = false;
int Search::search_ref_count = 0;
list<Search*> Search::search_list;
list<Service> Search::services;
int Search::onStartNotified;
bool Search::pt_update_start = false;

int Search::numRunning;

void Search::onStart()
{
	MSF_DBG("Search::onStart()");

	// ....... ==0 ??
	if (onStartNotified > 0) {
		onStartNotified = 0;
		starting = false; //not sure why ankit
		list<Search*>::iterator itr;
		for (itr = search_list.begin(); itr != search_list.end(); itr++) {
			if ((*itr)->searchListener != NULL &&  (*itr)->searching_now) {
				(*itr)->searchListener->onStart();
			}
		}
	}
}

void Search::onStop()
{
	MSF_DBG("Search::onStop()");
	list<Search*>::iterator itr;
	for (itr = search_list.begin(); itr != search_list.end(); itr++) {
		if ((*itr)->searchListener != NULL && (*itr)->searching_now) {
			(*itr)->searchListener->onStop();
		}
	}
}

void Search::onFound(Service service)
{
	MSF_DBG("Search::onFound()");

	list<Search*>::iterator itr;

	MSF_DBG("Search::onFound() list size = %d", services.size());

	for (itr = search_list.begin(); itr != search_list.end(); itr++) {
		if ((*itr)->searchListener != NULL && (*itr)->searching_now) {
			(*itr)->searchListener->onFound(service);
		}
	}
}

void Search::onLost(Service service)
{
	MSF_DBG("Search::onLost()");

	list<Search*>::iterator itr;

	for (itr = search_list.begin(); itr != search_list.end(); itr++) {
		if ((*itr)->searchListener != NULL && (*itr)->searching_now) {
			(*itr)->searchListener->onLost(service);
		}
	}
}

void  SearchListener :: onStart()
{
}

void  SearchListener :: onStop()
{
}

void  SearchListener :: onFound(Service service)
{
}

void SearchListener :: onLost(Service service)
{
}

Search::Search()
{
	searching_now = false;
	searchListener = NULL;

	search_list.push_back(this);
}

Search::~Search()
{
	search_list.remove(this);
}

void Search::setSearchListener(SearchListener *obj)
{
	searchListener = obj;
}

void Search::releaseSearchListener()
{
	if (searchListener) {
		searchListener = NULL;
	}
}

Service Search::getServiceById(string id)
{
	std::list<Service>::iterator iterator;
	for (iterator = services.begin(); iterator != services.end(); ++iterator) {
		if ((iterator->getId()) == id) {
			return(*iterator);
		}
	}
	return Service("", "", "", "", "");
}

bool Search::start()
{
	MSF_DBG("Search::start()");
	if (searching_now == false) {
		if (search_ref_count == 0) {
			search_ref_count++;

			searching_now = true;


			startDiscovery();
		} else {
			search_ref_count++;

			searching_now = true;

			if (searchListener != NULL) {
				searchListener->onStart();
			}
		}
	}

	return true;
}

bool Search::stop()
{
	MSF_DBG("search stop");
	if (searching_now == true) {
		search_ref_count--;
		searching_now = false;

		if (search_ref_count == 0) {
			stopDiscovery();
		} else {
			if (searchListener != NULL)
				searchListener->onStop();
		}
	}

	return true;
}

list<Service> Search::getServices()
{
	//return SearchProvider::getServices();
	return services;
}

void *Search::pt_startMDNS(void *arg)
{
	return NULL;
}

void *Search::pt_startMSFD(void *arg)
{
	provider_msfd.start();
	return NULL;
}

void *Search::pt_update_alivemap(void *arg)
{
	pt_update_start = true;

	while (pt_update_start) {
		sleep(5);
		SearchProvider::reapServices();
	}

	return NULL;
}

void Search::startDiscovery()
{
	MSF_DBG("Search::startDiscovery()");
	starting = true;
	// If there are no search providers, then default to using MDNS.
	if (providers.empty()) {
		providers.push_back(MSFDSearchProvider::create(this));
	}

	services.clear();

	onStartNotified = numRunning = providers.size();

	int ret = -1;

	provider_mdns.start();

	ret = pthread_create(&threads[MSFD_THREAD_NUMBER], NULL, pt_startMSFD, NULL);
	if (ret == -1)
		MSF_DBG("MSFD thread create failed");

	ret = pthread_create(&threads[UPDATE_THREAD_NUMBER], NULL, pt_update_alivemap, NULL);
	if (ret == -1)
		MSF_DBG("update thread create failed");

	onStart();
}

void Search::stopDiscovery()
{
	MSF_DBG("Search::stopDiscovery()");
	stopping = true;
	bool stop = false;
	stop = provider_mdns.stop();
	stop = provider_msfd.stop();
	pt_update_start = false;

	if (stop)
		onStop();

	pthread_cancel(threads[MSFD_THREAD_NUMBER]);
	pthread_join(threads[MSFD_THREAD_NUMBER], NULL);

	pthread_cancel(threads[UPDATE_THREAD_NUMBER]);
	pthread_join(threads[UPDATE_THREAD_NUMBER], NULL);

	MSF_DBG("MSFD thread joined");
	MSF_DBG("update alivemap thread joined");
}

bool Search::addService(Service service)
{
	MSF_DBG("Search::addService()");
	bool match = false;
	for (std::list<Service>::iterator it = services.begin(); it != services.end(); it++) {
		if ((*it).getId() == service.getId()) {
			match = true;
			break;
		}
	}
	if (!match) {
		services.push_back(service);
		onFound(service);
	}

	return true;
}

bool Search::removeService(Service service)
{
	for (std::list<Service>::iterator it = services.begin(); it != services.end(); it++) {
		if ((*it).getId() == service.getId()) {
			services.erase(it);
			break;
		}
	}
	return true;
}

void Search::removeServiceAndNotify(Service service)
{
	removeService(service);
	onLost(service);
}

void Search::validateService(Service service)
{
}

