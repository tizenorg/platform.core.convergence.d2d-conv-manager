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

#include "server.h"
#include "access_control/peer_creds.h"
#include "dbus_server_impl.h"

#define DEFAULT_APP_ID	"Default"

static bool conn_acquired = false;
static bool name_acquired = false;
static conv::dbus_server_impl *_instance = NULL;
static GDBusConnection *dbus_connection = NULL;
static guint dbus_owner_id = 0;
static GDBusNodeInfo *dbus_node_info = NULL;

static const gchar introspection_xml[] =
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
	"	</interface>"
	"</node>";

static const char* req_type_to_str(int req_type)
{
	switch (req_type) {
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

static void handle_request(GDBusConnection* conn, const char *sender, GVariant *param, GDBusMethodInvocation *invocation)
{
	gint req_type = 0;
	const gchar *cookie = NULL;
	gint req_id = 0;
	const gchar *subject = NULL;
	const gchar *input = NULL;

	g_variant_get(param, "(i&si&s&s)", &req_type, &cookie, &req_id, &subject, &input);
	IF_FAIL_VOID_TAG(req_type > 0 && req_id > 0 && cookie && subject && input, _E, "Invalid request");

	_SD("Cookie: %s", cookie);
	_I("[%s] ReqId: %d, Subject: %s", req_type_to_str(req_type), req_id, subject);
	_SI("Input: %s", input);

	conv::credentials *creds = NULL;
	// temporaily removed - privilege checker
/*
	if (!peer_creds::get(conn, sender, &creds)) {
		_E("Peer credentialing failed");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
		return;
	}
*/
	conv::request *recv_request = NULL;
	try{
		recv_request = new conv::request(req_type, DEFAULT_APP_ID, req_id, subject, input, sender, creds, invocation);
//		recv_request = new conv::request(req_type, app_id, req_id, subject, input, sender, NULL, invocation);
	} catch (std::bad_alloc& ba) {
		_E("Memory Allocation Failed..");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
//		delete creds;
		return;
	} catch (int e) {
		_E("Caught %d", e);
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
//		delete creds;
		return;
	}

	if (!recv_request) {
		_E("Memory allocation failed");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(iss)", CONV_ERROR_INVALID_OPERATION, EMPTY_JSON_OBJECT, EMPTY_JSON_OBJECT));
		return;
	}

	conv::send_request(recv_request);
}

static void handle_method_call(GDBusConnection *conn, const gchar *sender,
		const gchar *obj_path, const gchar *iface, const gchar *method_name,
		GVariant *param, GDBusMethodInvocation *invocation, gpointer user_data)
{
	IF_FAIL_VOID_TAG(STR_EQ(obj_path, DBUS_PATH), _W, "Invalid path: %s", obj_path);
	IF_FAIL_VOID_TAG(STR_EQ(iface, DBUS_IFACE), _W, "Invalid interface: %s", obj_path);

	if (STR_EQ(method_name, METHOD_REQUEST)) {
		handle_request(conn, sender, param, invocation);
	} else {
		_W("Invalid method: %s", method_name);
	}

	_D("end of handle_method_call");
}

static void on_bus_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	GError *error = NULL;

	GDBusInterfaceVTable vtable;
	vtable.method_call = handle_method_call;
	vtable.get_property = NULL;
	vtable.set_property = NULL;

	guint reg_id = g_dbus_connection_register_object(conn, DBUS_PATH,
			dbus_node_info->interfaces[0], &vtable, NULL, NULL, &error);

	if (reg_id <= 0) {
		_E("Failed to acquire dbus gerror(%s)", error->message);
		raise(SIGTERM);
	}

	conn_acquired = true;
	dbus_connection = conn;

	_I("Dbus connection acquired");
}

static void on_name_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	name_acquired = true;
	_SI("Dbus name acquired: %s", name);
}

static void on_name_lost(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
	_E("Dbus name lost");
	raise(SIGTERM);
}

conv::dbus_server_impl::dbus_server_impl()
{
}

conv::dbus_server_impl::~dbus_server_impl()
{
	release();
}

bool conv::dbus_server_impl::init()
{
	GError *error = NULL;

	_I("init with dbus_node_info : %x", dbus_node_info);

	dbus_node_info = g_dbus_node_info_new_for_xml(introspection_xml, &error);
	if (dbus_node_info == NULL) {
		_E("g_dbus_node_info_new_for_xml() Fail(%s)", error->message);
		g_error_free(error);
		return false;
	}

	dbus_owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, DBUS_DEST, G_BUS_NAME_OWNER_FLAGS_NONE,
			on_bus_acquired, on_name_acquired, on_name_lost, NULL, NULL);

	if (dbus_owner_id == 0) {
		_E("dbus_owner_id Fail()");
		return false;
	}

	_instance = this;

	return true;
}

void conv::dbus_server_impl::release()
{
	if (dbus_connection) {
		g_dbus_connection_flush_sync(dbus_connection, NULL, NULL);
	}

	if (dbus_owner_id > 0) {
		g_bus_unown_name(dbus_owner_id);
		dbus_owner_id = 0;
	}

	if (dbus_connection) {
		g_dbus_connection_close_sync(dbus_connection, NULL, NULL);
		g_object_unref(dbus_connection);
		dbus_connection = NULL;
	}

	if (dbus_node_info) {
		g_dbus_node_info_unref(dbus_node_info);
		dbus_node_info = NULL;
	}
}

void conv::dbus_server_impl::publish(const char* dest, int req_id, const char* subject, int error, const char* data)
{
	IF_FAIL_VOID_TAG(dest && subject && data, _E, "Parameter null");

	_SI("Publish: %s, %d, %s, %#x, %s", dest, req_id, subject, error, data);

	GVariant *param = g_variant_new("(isis)", req_id, subject, error, data);
	IF_FAIL_VOID_TAG(param, _E, "Memory allocation failed");

	_D("before g_dbus_connection_call..");
	GError *err = NULL;
	g_dbus_connection_call(dbus_connection, dest, DBUS_PATH, DBUS_IFACE,
			METHOD_RESPOND, param, NULL, G_DBUS_CALL_FLAGS_NONE, DBUS_TIMEOUT, NULL, NULL, &err);

	if (err != NULL)
	{
		_D("dbus_connection_call Error msg : %s", err->message);

		HANDLE_GERROR(err);
	}
	else
	{
		_D("err is NULL");
	}
}

static void handle_call_result(GObject *source, GAsyncResult *res, gpointer user_data)
{
	_I("Call %u done", *static_cast<unsigned int*>(user_data));

	GDBusConnection *conn = G_DBUS_CONNECTION(source);
	GError *error = NULL;
	g_dbus_connection_call_finish(conn, res, &error);
	HANDLE_GERROR(error);
}

void conv::dbus_server_impl::call(const char *dest, const char *obj, const char *iface, const char *method, GVariant *param)
{
	IF_FAIL_VOID_TAG(dest && obj && iface && method, _E, "Parameter null");

	static unsigned int call_count = 0;
	++call_count;

	_SI("Call %u: %s, %s, %s.%s", call_count, dest, obj, iface, method);

	g_dbus_connection_call(dbus_connection, dest, obj, iface, method, param, NULL,
			G_DBUS_CALL_FLAGS_NONE, DBUS_TIMEOUT, NULL, handle_call_result, &call_count);
}

void conv::dbus_server::publish(const char* dest, int req_id, const char* subject, int error, const char* data)
{
	_instance->publish(dest, req_id, subject, error, data);
}

void conv::dbus_server::call(const char *dest, const char *obj, const char *iface, const char *method, GVariant *param)
{
	_instance->call(dest, obj, iface, method, param);
}

void conv::dbus_server::set_instance(conv::dbus_server_iface* svr)
{
	_instance = static_cast<dbus_server_impl*>(svr);
}
