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

#include <stdlib.h>
#include <new>
#include <gio/gio.h>


#include "d2d_conv_manager.h"
#include "server.h"
#include "dbus_server_impl.h"
#include "discovery_mgr_impl.h"
#include "connection_mgr_impl.h"
#include "client_mgr_impl.h"
#include "service_mgr_impl.h"
#include "request_handler.h"

using namespace std;

static GMainLoop *mainloop = NULL;
static gboolean started = FALSE;
static conv::dbus_server_impl *dbus_handle = NULL;
static conv::discovery_manager_impl *discovery_mgr = NULL;
static conv::connection_manager_impl *connection_mgr = NULL;
static conv::client_manager_impl *client_mgr = NULL;
static conv::service_manager_impl *service_mgr = NULL;
static conv::request_handler *request_mgr = NULL;

void conv::initialize()
{
	int result;

	if (started) {
		_D("flowd is started already");
		return;
	}

	mainloop = g_main_loop_new(NULL, FALSE);

	_I("Init Discovery Manager");
	discovery_mgr = new(std::nothrow) conv::discovery_manager_impl();
	IF_FAIL_CATCH_TAG(discovery_mgr, _E, "Memory allocation failed");
	discovery_manager::set_instance(discovery_mgr);
	result = discovery_mgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Connection Manager");
	connection_mgr = new(std::nothrow) conv::connection_manager_impl();
	IF_FAIL_CATCH_TAG(connection_mgr, _E, "Memory allocation failed");
	connection_manager::set_instance(connection_mgr);
	result = connection_mgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Client Manager");
	client_mgr = new(std::nothrow) conv::client_manager_impl();
	IF_FAIL_CATCH_TAG(client_mgr, _E, "Memory allocation failed");
	client_manager::set_instance(client_mgr);
	result = client_mgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Service Manager");
	service_mgr = new(std::nothrow) conv::service_manager_impl();
	IF_FAIL_CATCH_TAG(service_mgr, _E, "Memory allocation failed");
	service_manager::set_instance(service_mgr);
	result = service_mgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Request Manager");
	request_mgr = new(std::nothrow) request_handler();
	IF_FAIL_CATCH_TAG(request_mgr, _E, "Memory allocation failed");
	result = request_mgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Dbus Server");
	dbus_handle = new(std::nothrow) conv::dbus_server_impl();
	IF_FAIL_CATCH_TAG(discovery_mgr, _E, "Memory allocation failed");
	dbus_server::set_instance(dbus_handle);
	result = dbus_handle->init();
	IF_FAIL_CATCH_TAG(result == true, _E, "Initialization Failed");

	_I("Start main loop");
	started = TRUE;

	g_main_loop_run(mainloop);

	return;

CATCH:
	_E(RED("Launching Failed"));

	g_main_loop_quit(mainloop);
}

void conv::release()
{
	_I(CYAN("Terminating flow-daemon"));
	_I("Release discovery manager");
	if (discovery_mgr)
		discovery_mgr->release();

	_I("Release connection manager");
	if (connection_mgr)
		connection_mgr->release();

	_I("Release dbus server");
	if (dbus_handle)
		dbus_handle->release();

	_I("Release client manager");
	if (client_mgr)
		client_mgr->release();

	_I("Release service manager");
	if (service_mgr)
		service_mgr->release();

	_I("Release request handler");
	if (request_mgr)
		request_mgr->release();

	g_main_loop_unref(mainloop);

	delete discovery_mgr;
	delete connection_mgr;
	delete dbus_handle;
	delete client_mgr;
	delete request_mgr;
	delete service_mgr;
	started = FALSE;
}


void conv::send_request(request* request_obj)
{
	_D("send_request request_obj:%x request_mg:%x", request_obj, request_mgr);
	if (request_mgr) {
		request_mgr->handle_request(request_obj);
	} else if (request_obj) { //jhp27.park temp code..
		request_obj->reply(CONV_ERROR_NONE);
	}

	_D("request handling done");
}

static void signal_handler(int signo)
{
	_I("SIGNAL %d received", signo);

	// Stop the main loop
	g_main_loop_quit(mainloop);
}

int main(int argc, char **argv)
{
	static struct sigaction signal_action;
	signal_action.sa_handler = signal_handler;
	sigemptyset(&signal_action.sa_mask);

	sigaction(SIGINT, &signal_action, NULL);
	sigaction(SIGHUP, &signal_action, NULL);
	sigaction(SIGTERM, &signal_action, NULL);
	sigaction(SIGQUIT, &signal_action, NULL);
	sigaction(SIGABRT, &signal_action, NULL);

#if !defined(GLIB_VERSION_2_36)
	g_type_init();
#endif

	conv::initialize();
	conv::release();

	return EXIT_SUCCESS;
}
