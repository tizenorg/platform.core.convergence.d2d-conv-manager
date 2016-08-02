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

#include <signal.h>
#include <glib.h>
#include <gio/gio.h>
#include <app_manager.h>

#include "Server.h"
#include "access_control/PeerCreds.h"
#include "DbusServer.h"

#define DEFAULT_APP_ID	"Default"

static bool connAcquired = false;
static bool nameAcquired = false;
static conv::DbusServer *__instance = NULL;
static GDBusConnection *dbusConnection = NULL;
static guint dbusOwnerId = 0;
static GDBusNodeInfo *dbusNodeInfo = NULL;

static const gchar introspectionXml[] =
	"<node>"
	"	<interface name='" DBUS_IFACE "'>"
	"		<method name='" METHOD_REQUEST "'>"
	"			<arg type='i' name='" ARG_REQTYPE "' direction='in'/>"
	"			<arg type='s' name='" ARG_COOKIE "' direction='in'/>"
	"			<arg type='i' name='" ARG_REQID "' direction='in'/>"
	"			<arg type='s' name='" ARG_SUBJECT "' direction='in'/>"
	"			<arg type='s' name='" ARG_INPUT "' direction='in'/>"
	"			<arg type='i' name='" ARG_RESULT_ERR "' direction='out'/>"
	"			<arg type='s' name='" ARG_RESULT_ADD "' direction='out'/>"
	"			<arg type='s' name='" ARG_OUTPUT "' direction='out'/>"
	"		</method>"
	"		<method name='" METHOD_CHK_PRIV_NETWORK_GET "'>"
	"			<arg type='i' name='" ARG_RESULT_ERR "' direction='out'/>"
	"		</method>"
	"	</interface>"
	"</node>";

static const char* __req_type_to_str(int reqType)
{
	switch (reqType) {
		case REQ_SUBSCRIBE:
			return "Subscribe";
		case REQ_UNSUBSCRIBE:
			return "Unsubscribe";
		case REQ_READ:
			return "Read";
		case REQ_READ_SYNC:
			return "Read (Sync)";
		case REQ_WRITE:
			return "Write";
		default:
			return NULL;
	}
}

static void __handle_request(GDBusConnection* conn, const char *sender, GVariant *param, GDBusMethodInvocation *invocation)
{
	gint req_type = 0;
	const gchar *cookie = NULL;
	gint req_id = 0;
	const gchar *subject = NULL;
	const gchar *input = NULL;

	g_variant_get(param, "(i&si&s&s)", &req_type, &cookie, &req_id, &subject, &input);
	IF_FAIL_VOID_TAG(req_type > 0 && req_id > 0 && cookie && subject && input, _E, "Invalid request");

	_SD("Cookie: %s", cookie);
	_I("[%s] ReqId: %d, Subject: %s", __req_type_to_str(req_type), req_id, subject);
	_SI("Input: %s", input);

	conv::Credentials *creds = NULL;

	if (!conv::peer_creds::get(conn, sender, &creds)) {
		_E("Peer credentialing failed");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
		return;
	}

	conv::Request *recvRequest = NULL;
	try{
		recvRequest = new conv::Request(req_type, DEFAULT_APP_ID, req_id, subject, input, sender, creds, invocation);
	} catch (std::bad_alloc& ba) {
		_E("Memory Allocation Failed..");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
		delete creds;
		return;
	} catch (int e) {
		_E("Caught %d", e);
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
		delete creds;
		return;
	}

	if (!recvRequest) {
		_E("Memory allocation failed");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
		delete creds;
		return;
	}

	conv::sendRequest(recvRequest);
}

static void __handle_method_call(GDBusConnection *conn, const gchar *sender,
		const gchar *obj_path, const gchar *iface, const gchar *method_name,
		GVariant *param, GDBusMethodInvocation *invocation, gpointer user_data)
{
	IF_FAIL_VOID_TAG(STR_EQ(obj_path, DBUS_PATH), _W, "Invalid path: %s", obj_path);
	IF_FAIL_VOID_TAG(STR_EQ(iface, DBUS_IFACE), _W, "Invalid interface: %s", obj_path);

	if (STR_EQ(method_name, METHOD_REQUEST))
		__handle_request(conn, sender, param, invocation);
	else if (STR_EQ(method_name, METHOD_CHK_PRIV_NETWORK_GET))
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", CONV_ERROR_NONE));
	else
		_W("Invalid method: %s", method_name);

	_D("end of __handle_method_call");
}

static void __on_bus_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	GError *error = NULL;

	GDBusInterfaceVTable vtable;
	vtable.method_call = __handle_method_call;
	vtable.get_property = NULL;
	vtable.set_property = NULL;

	guint reg_id = g_dbus_connection_register_object(conn, DBUS_PATH,
			dbusNodeInfo->interfaces[0], &vtable, NULL, NULL, &error);

	if (reg_id <= 0) {
		_E("Failed to acquire dbus gerror(%s)", error->message);
		raise(SIGTERM);
	}

	connAcquired = true;
	dbusConnection = conn;

	_I("Dbus connection acquired");
}

static void __on_name_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	nameAcquired = true;
	_SI("Dbus name acquired: %s", name);
}

static void __on_name_lost(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	_E("Dbus name lost");
	raise(SIGTERM);
}

conv::DbusServer::DbusServer()
{
}

conv::DbusServer::~DbusServer()
{
	release();
}

bool conv::DbusServer::init()
{
	GError *error = NULL;

	_I("init with dbusNodeInfo : %x", dbusNodeInfo);

	dbusNodeInfo = g_dbus_node_info_new_for_xml(introspectionXml, &error);
	if (dbusNodeInfo == NULL) {
		_E("g_dbusNodeInfo_new_for_xml() Fail(%s)", error->message);
		g_error_free(error);
		return false;
	}

	dbusOwnerId = g_bus_own_name(G_BUS_TYPE_SESSION, DBUS_DEST, G_BUS_NAME_OWNER_FLAGS_NONE,
			__on_bus_acquired, __on_name_acquired, __on_name_lost, NULL, NULL);

	if (dbusOwnerId == 0) {
		_E("dbusOwnerId Fail()");
		return false;
	}

	__instance = this;

	return true;
}

void conv::DbusServer::release()
{
	if (dbusConnection)
		g_dbus_connection_flush_sync(dbusConnection, NULL, NULL);

	if (dbusOwnerId > 0) {
		g_bus_unown_name(dbusOwnerId);
		dbusOwnerId = 0;
	}

	if (dbusConnection) {
		g_dbus_connection_close_sync(dbusConnection, NULL, NULL);
		g_object_unref(dbusConnection);
		dbusConnection = NULL;
	}

	if (dbusNodeInfo) {
		g_dbus_node_info_unref(dbusNodeInfo);
		dbusNodeInfo = NULL;
	}
}

void conv::DbusServer::publish(const char* dest, int reqId, const char* subject, int error, const char* data)
{
	IF_FAIL_VOID_TAG(dest && subject && data, _E, "Parameter null");

	_SI("Publish: %s, %d, %s, %#x, %s", dest, reqId, subject, error, data);

	GVariant *param = g_variant_new("(isis)", reqId, subject, error, data);
	IF_FAIL_VOID_TAG(param, _E, "Memory allocation failed");

	_D("before g_dbusConnection_call..");
	GError *err = NULL;
	g_dbus_connection_call(dbusConnection, dest, DBUS_PATH, DBUS_IFACE,
			METHOD_RESPOND, param, NULL, G_DBUS_CALL_FLAGS_NONE, DBUS_TIMEOUT, NULL, NULL, &err);

	if (err != NULL) {
		_D("dbusConnection_call Error msg : %s", err->message);
		HANDLE_GERROR(err);
	}
}

static void __handle_call_result(GObject *source, GAsyncResult *res, gpointer user_data)
{
	_I("Call %u done", *static_cast<unsigned int*>(user_data));

	GDBusConnection *conn = G_DBUS_CONNECTION(source);
	GError *error = NULL;
	g_dbus_connection_call_finish(conn, res, &error);
	HANDLE_GERROR(error);
}

void conv::DbusServer::call(const char *dest, const char *obj, const char *iface, const char *method, GVariant *param)
{
	IF_FAIL_VOID_TAG(dest && obj && iface && method, _E, "Parameter null");

	static unsigned int callCount = 0;
	++callCount;

	_SI("Call %u: %s, %s, %s.%s", callCount, dest, obj, iface, method);

	g_dbus_connection_call(dbusConnection, dest, obj, iface, method, param, NULL,
			G_DBUS_CALL_FLAGS_NONE, DBUS_TIMEOUT, NULL, __handle_call_result, &callCount);
}

void conv::dbus_server::publish(const char* dest, int reqId, const char* subject, int error, const char* data)
{
	__instance->publish(dest, reqId, subject, error, data);
}

void conv::dbus_server::call(const char *dest, const char *obj, const char *iface, const char *method, GVariant *param)
{
	__instance->call(dest, obj, iface, method, param);
}

void conv::dbus_server::setInstance(conv::IDbusServer* svr)
{
	__instance = static_cast<DbusServer*>(svr);
}
