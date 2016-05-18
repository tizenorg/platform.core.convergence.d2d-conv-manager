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

#include "client_mgr_impl.h"

static conv::client_manager_impl *_instance;

using namespace std;

conv::client_manager_impl::client_manager_impl()
{
}

conv::client_manager_impl::~client_manager_impl()
{
}

int conv::client_manager_impl::init()
{
	return CONV_ERROR_NONE;
}

int conv::client_manager_impl::release()
{
	for (client_list_t::iterator it = client_list.begin(); it != client_list.end(); ++it) {
		delete *it;
	}

	client_list.clear();

	return CONV_ERROR_NONE;
}

void conv::client_manager::set_instance(conv::client_manager_impl* mgr)
{
	_instance = mgr;
}


int conv::client_manager_impl::handle_request(request* request_obj)
{
	_D("handle_request called");

	return CONV_ERROR_NONE;
}

conv::client* conv::client_manager::get_client(std::string client_id)
{
	IF_FAIL_RETURN_TAG(_instance, NULL, _E, "Not initialized");

	return _instance->get_client(client_id);
}

conv::client* conv::client_manager_impl::get_client(std::string client_id)
{
	for (client_list_t::iterator it = client_list.begin(); it != client_list.end(); ++it) {
		if ( !(*it)->get_id().compare(client_id) )
		{
			// already exists
			_D("client exists");
			return *it;
		}
	}

	_D("new client obj created");
	client* client_obj = new conv::client(client_id, NULL);

	client_list.push_back(client_obj);
	return client_obj;
}
