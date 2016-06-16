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

#include <glib.h>
#include <gio/gio.h>
#include <map>

#include "internal_types.h"
#include "dbus_client.h"

static GDBusConnection *dbus_connection = NULL;
static GDBusNodeInfo *dbus_node_info = NULL;

typedef std::map<std::string, subject_response_cb> response_cb_map_t;
static response_cb_map_t *response_cb_map = NULL;

static const gchar introspection_xml[] =
	"<node>"
	"	<interface name='" DBUS_IFACE "'>"
	"		<method name='" METHOD_RESPOND "'>"
	"			<arg type='i' name='" ARG_REQID "' direction='in'/>"
	"			<arg type='s' name='" ARG_SUBJECT "' direction='in'/>"
	"			<arg type='i' name='" ARG_RESULT_ERR "' direction='in'/>"
	"			<arg type='s' name='" ARG_OUTPUT "' direction='in'/>"
	"		</method>"
	"	</interface>"
	"</node>";

//LCOV_EXCL_START
static int get_req_id()
{
	static int req_id = 0;

	if (++req_id < 0) {
		req_id = 1;
	}

	return req_id;
}

static void handle_response(const gchar *sender, GVariant *param, GDBusMethodInvocation *invocation)
{
	gint req_id = 0;
	const gchar *subject = NULL;
	gint error = 0;
	const gchar *data = NULL;

	g_variant_get(param, "(i&si&s)", &req_id, &subject, &error, &data);
	_D("[Response] ReqId: %d, Subject: %s, Error: %d", req_id, subject, error);

	response_cb_map_t::iterator it = response_cb_map->find(subject);
	IF_FAIL_VOID_TAG(it!= response_cb_map->end(), _E, "Unknown subject'%s'", subject);
	it->second(subject, req_id,  error, data);

	g_dbus_method_invocation_return_value(invocation, NULL);
}

static void handle_method_call(GDBusConnection *conn, const gchar *sender,
		const gchar *obj_path, const gchar *iface, const gchar *method_name,
		GVariant *param, GDBusMethodInvocation *invocation, gpointer user_data)
{
	_D("handle_method_call");
	IF_FAIL_VOID_TAG(STR_EQ(obj_path, DBUS_PATH), _W, "Invalid path: %s", obj_path);
	IF_FAIL_VOID_TAG(STR_EQ(iface, DBUS_IFACE), _W, "Invalid interface: %s", obj_path);

	_D("handle_method_call...sender[%s] obj_path[%s] iface[%s] method_name[%s]",
				sender, obj_path, iface, method_name);

	if (STR_EQ(method_name, METHOD_RESPOND)) {
		handle_response(sender, param, invocation);
	} else {
		_W("Invalid method: %s", method_name);
	}
}
//LCOV_EXCL_STOP

bool conv::dbus_client::init()
{
	_D("dbus_client init with dbus_connection %d response_cb_map:%x", dbus_connection, response_cb_map);
	if (dbus_connection) {
		return true;
	}

	response_cb_map = new(std::nothrow) response_cb_map_t;
	_D("dbus_client init with response_cb_map:%x", response_cb_map);

	GError *gerr = NULL;

	dbus_node_info = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	IF_FAIL_RETURN_TAG(dbus_node_info != NULL, false, _E, "Initialization failed");

	gchar *addr = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SESSION, NULL, &gerr);
	HANDLE_GERROR(gerr);
	IF_FAIL_RETURN_TAG(addr != NULL, false, _E, "Getting address failed");
	_SD("Address: %s", addr);

	dbus_connection = g_dbus_connection_new_for_address_sync(addr,
			(GDBusConnectionFlags)(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
			NULL, NULL, &gerr);
	g_free(addr);
	HANDLE_GERROR(gerr);
	IF_FAIL_RETURN_TAG(dbus_connection != NULL, false, _E, "Connection failed");

	GDBusInterfaceVTable vtable;
	vtable.method_call = handle_method_call;
	vtable.get_property = NULL;
	vtable.set_property = NULL;

	guint reg_id = g_dbus_connection_register_object(dbus_connection, DBUS_PATH,
			dbus_node_info->interfaces[0], &vtable, NULL, NULL, &gerr);
	HANDLE_GERROR(gerr);
	IF_FAIL_RETURN_TAG(reg_id > 0, false, _E, "Object registration failed");

	_I("Dbus connection established: %s", g_dbus_connection_get_unique_name(dbus_connection));
	return true;
}

void conv::dbus_client::release()
{
	if (dbus_connection) {
		g_dbus_connection_flush_sync(dbus_connection, NULL, NULL);
		g_dbus_connection_close_sync(dbus_connection, NULL, NULL);
		g_object_unref(dbus_connection);
		dbus_connection = NULL;
	}

	if (dbus_node_info) {
		g_dbus_node_info_unref(dbus_node_info);
		dbus_node_info = NULL;
	}
}

int conv::dbus_client::request(
		int type, int* req_id, const char* subject, const char* input,
		std::string* req_result, std::string* data_read)
{
	_D("Requesting: %d, %s", type, subject);
	IF_FAIL_RETURN_TAG(init(), CONV_ERROR_INVALID_OPERATION, _E, "Connection failed");
	IF_FAIL_RETURN_TAG(req_id != NULL, CONV_ERROR_INVALID_PARAMETER, _E, "Invalid parameter");

	if (subject == NULL) {
		subject = EMPTY_STRING; //LCOV_EXCL_LINE
	}

	if (input == NULL) {
		input = EMPTY_JSON_OBJECT;
	}
	*req_id = get_req_id();

	// second param is security cookie which is deprecated in 3.0
	GVariant *param = g_variant_new("(isiss)", type, "", *req_id, subject, input);
	IF_FAIL_RETURN_TAG(param, CONV_ERROR_OUT_OF_MEMORY, _E, "Memory allocation failed");

	GError *err = NULL;
	GVariant *response = g_dbus_connection_call_sync(dbus_connection, DBUS_DEST, DBUS_PATH, DBUS_IFACE,
			METHOD_REQUEST, param, NULL, G_DBUS_CALL_FLAGS_NONE, DBUS_TIMEOUT, NULL, &err);

	HANDLE_GERROR(err);
	IF_FAIL_RETURN_TAG(response, CONV_ERROR_INVALID_OPERATION, _E, "Method call failed");

	gint _error = CONV_ERROR_INVALID_OPERATION;
	const gchar *_req_result = NULL;
	const gchar *_data_read = NULL;

	g_variant_get(response, "(i&s&s)", &_error, &_req_result, &_data_read);
	if (req_result) {
		*req_result = _req_result; //LCOV_EXCL_LINE
	}
	if (data_read) {
		*data_read = _data_read; //LCOV_EXCL_LINE
	}

	g_variant_unref(response);

	return _error;
}

//LCOV_EXCL_START
int conv::dbus_client::request_with_no_reply(
		int type, int* req_id, const char* subject, const char* input)
{
	_D("Requesting: %d, %d, %s", type, req_id, subject);
	IF_FAIL_RETURN_TAG(init(), CONV_ERROR_INVALID_OPERATION, _E, "Connection failed");
	IF_FAIL_RETURN_TAG(req_id != NULL, CONV_ERROR_INVALID_PARAMETER, _E, "Invalid parameter");

	if (subject == NULL) {
		subject = EMPTY_STRING;
	}

	if (input == NULL) {
		input = EMPTY_JSON_OBJECT;
	}

	*req_id = get_req_id();

	// second param is security cookie which is deprecated in 3.0
	GVariant *param = g_variant_new("(isiss)", type, "", req_id, subject, input);
	IF_FAIL_RETURN_TAG(param, CONV_ERROR_OUT_OF_MEMORY, _E, "Memory allocation failed");

	GError *err = NULL;
	g_dbus_connection_call(dbus_connection, DBUS_DEST, DBUS_PATH, DBUS_IFACE,
			METHOD_REQUEST, param, NULL, G_DBUS_CALL_FLAGS_NONE, DBUS_TIMEOUT, NULL, NULL, &err);

	if (err) {
		HANDLE_GERROR(err);
		return CONV_ERROR_INVALID_OPERATION;
	}

	return CONV_ERROR_NONE;
}
//LCOV_EXCL_STOP

int conv::dbus_client::register_callback(const char* subject, subject_response_cb callback)
{
	IF_FAIL_RETURN_TAG(subject && callback, CONV_ERROR_INVALID_PARAMETER, _E, "Invalid parameter");
	IF_FAIL_RETURN_TAG(init(), CONV_ERROR_INVALID_OPERATION, _E, "Connection failed");

	_I("Registering callback for subject '%s'", subject);
	_I("response_cb_map : %x", response_cb_map);

	response_cb_map->insert(std::pair<std::string, subject_response_cb> (subject, callback));

	_D("registering done..");

	return true;
}
