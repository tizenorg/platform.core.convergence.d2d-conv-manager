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
#include "Server.h"
#include "DbusServer.h"
#include "DiscoveryManager.h"
#include "ConnectionManager.h"
#include "ClientManager.h"
#include "ServiceManager.h"
#include "RequestHandler.h"

using namespace std;

static GMainLoop *mainLoop = NULL;
static gboolean started = FALSE;
static conv::DbusServer *dbusHandle = NULL;
static conv::DiscoveryManager *discoveryMgr = NULL;
static conv::ConnectionManager *connectionMgr = NULL;
static conv::ClientManager *clientMgr = NULL;
static conv::ServiceManager *serviceMgr = NULL;
static conv::RequestHandler *requestMgr = NULL;

void conv::initialize()
{
	int result;

	if (started) {
		_D("flowd is started already");
		return;
	}

	mainLoop = g_main_loop_new(NULL, FALSE);

	_I("Init Discovery Manager");
	discoveryMgr = new(std::nothrow) conv::DiscoveryManager();
	IF_FAIL_CATCH_TAG(discoveryMgr, _E, "Memory allocation failed");
	discovery_manager::set_instance(discoveryMgr);
	result = discoveryMgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Connection Manager");
	connectionMgr = new(std::nothrow) conv::ConnectionManager();
	IF_FAIL_CATCH_TAG(connectionMgr, _E, "Memory allocation failed");
	connection_manager::setInstance(connectionMgr);
	result = connectionMgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Client Manager");
	clientMgr = new(std::nothrow) conv::ClientManager();
	IF_FAIL_CATCH_TAG(clientMgr, _E, "Memory allocation failed");
	client_manager::setInstance(clientMgr);
	result = clientMgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Service Manager");
	serviceMgr = new(std::nothrow) conv::ServiceManager();
	IF_FAIL_CATCH_TAG(serviceMgr, _E, "Memory allocation failed");
	service_manager::set_instance(serviceMgr);
	result = serviceMgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Request Manager");
	requestMgr = new(std::nothrow) RequestHandler();
	IF_FAIL_CATCH_TAG(requestMgr, _E, "Memory allocation failed");
	result = requestMgr->init();
	IF_FAIL_CATCH_TAG(result == CONV_ERROR_NONE, _E, "Initialization Failed");

	_I("Init Dbus Server");
	dbusHandle = new(std::nothrow) conv::DbusServer();
	IF_FAIL_CATCH_TAG(discoveryMgr, _E, "Memory allocation failed");
	dbus_server::setInstance(dbusHandle);
	result = dbusHandle->init();
	IF_FAIL_CATCH_TAG(result == true, _E, "Initialization Failed");

	_I("Start main loop");
	started = TRUE;

	g_main_loop_run(mainLoop);

	return;

CATCH:
	_E(RED("Launching Failed"));

	g_main_loop_quit(mainLoop);
}

void conv::release()
{
	_I(CYAN("Terminating flow-daemon"));
	_I("Release discovery manager");
	if (discoveryMgr)
		discoveryMgr->release();

	_I("Release connection manager");
	if (connectionMgr)
		connectionMgr->release();

	_I("Release dbus server");
	if (dbusHandle)
		dbusHandle->release();

	_I("Release client manager");
	if (clientMgr)
		clientMgr->release();

	_I("Release service manager");
	if (serviceMgr)
		serviceMgr->release();

	_I("Release request handler");
	if (requestMgr)
		requestMgr->release();

	g_main_loop_unref(mainLoop);

	delete discoveryMgr;
	delete connectionMgr;
	delete dbusHandle;
	delete clientMgr;
	delete requestMgr;
	delete serviceMgr;
	started = FALSE;
}


void conv::sendRequest(request* requestObj)
{
	_D("send_request requestObj:%x request_mg:%x", requestObj, requestMgr);
	if (requestMgr) {
		requestMgr->handleRequest(requestObj);
	} else if (requestObj) {
		requestObj->reply(CONV_ERROR_NONE);
	}

	_D("request handling done");
}

static void signal_handler(int signo)
{
	_I("SIGNAL %d received", signo);

	// Stop the main loop
	g_main_loop_quit(mainLoop);
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
