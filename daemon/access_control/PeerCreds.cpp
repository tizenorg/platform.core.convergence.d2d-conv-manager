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
#include "Types.h"
#include "PeerCreds.h"
#include <cynara-creds-gdbus.h>
#include <cynara-session.h>
#include <app_manager.h>
#include <package_manager.h>

conv::Credentials::Credentials(char *pkgId, char *cli, char *sess, char *usr) :
	packageId(pkgId),
	client(cli),
	session(sess),
	user(usr)
{
}

conv::Credentials::~Credentials()
{
	g_free(packageId);
	g_free(client);
	g_free(session);
	g_free(user);
}

bool conv::peer_creds::get(GDBusConnection *connection, const char *unique_name, conv::Credentials **creds)
{
	pid_t pid = 0;
	char *appId = NULL;
	char *packageId = NULL;
	gchar *client = NULL;
	char *session = NULL;
	gchar *user = NULL;
	int err;

	err = cynara_creds_gdbus_get_pid(connection, unique_name, &pid);
	IF_FAIL_RETURN_TAG(err == CYNARA_API_SUCCESS, false, _E, "Peer credentialing failed");

	app_manager_get_app_id(pid, &appId);
	package_manager_get_package_id_by_app_id(appId, &packageId);
	_D("AppId: %s, PackageId: %s", appId, packageId);

	err = cynara_creds_gdbus_get_client(connection, unique_name, CLIENT_METHOD_DEFAULT, &client);
	IF_FAIL_CATCH_TAG(err == CYNARA_API_SUCCESS, _E, "Peer credentialing failed");

	session = cynara_session_from_pid(pid);
	IF_FAIL_CATCH_TAG(session, _E, "Peer credentialing failed");

	err = cynara_creds_gdbus_get_user(connection, unique_name, USER_METHOD_DEFAULT, &user);
	IF_FAIL_CATCH_TAG(err == CYNARA_API_SUCCESS, _E, "Peer credentialing failed");

	*creds = new(std::nothrow) Credentials(packageId, client, session, user);
	IF_FAIL_CATCH_TAG(*creds, _E, "Memory allocation failed");

	g_free(appId);
	return true;

CATCH:
	g_free(appId);
	g_free(packageId);
	g_free(client);
	g_free(session);
	g_free(user);
	return false;
}
