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

#ifndef __MSF_MDNS_SEARCH_PROVIDER_H__
#define __MSF_MDNS_SEARCH_PROVIDER_H__

using namespace std;

#include"SearchProvider.h"
#include <nsd/dns-sd.h>
#include "Service.h"
#include "Result.h"

class mDNSSearchProvider : public SearchProvider
{
private:
	Result_Base *service_cb;

public:
	mDNSSearchProvider();
	~mDNSSearchProvider();
	void addService(Service service);
	void updateAlive(long ttl, string id, int service_type);
	mDNSSearchProvider(Search *sListener);
	void start();
	bool stop();
	static SearchProvider create();
	static SearchProvider create(Search *);
	Result_Base* get_service_cb();
	void reapServices();
};

class MDNSServiceCallback : public Result_Base
{

	private:

		mDNSSearchProvider* provider;

	public:

		MDNSServiceCallback(mDNSSearchProvider *provider) : provider(provider){ }

		void onSuccess(Service service);
		void onError(Error);
};

#endif
