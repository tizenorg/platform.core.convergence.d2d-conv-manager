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

#include "Util.h"
#include <bluetooth.h>
#include <bluetooth_internal.h>
#include <vconf.h>
#include "Log.h"

#include <glib.h>
#include <pthread.h>
#include "Service.h"

using namespace std;

std::string conv::util::getBtMacAddress()
{
	static std::string __macAddress;
	if(__macAddress.empty()) {
		bt_initialize();
		char* macAddress;
		bt_adapter_enable();
		int ret = bt_adapter_get_address(&macAddress);
		IF_FAIL_RETURN_TAG(ret == 0, NULL, _E, "bluetooth get mac address failed : %d", ret);
		_D("bluetooth get mac address : %s", macAddress);
		__macAddress = macAddress;
		free(macAddress);
		bt_deinitialize();
	}
	_D("mac address:%s", __macAddress.c_str());
	return __macAddress;
}


std::string conv::util::getDeviceName()
{
	static std::string __deviceName;
	if(__deviceName.empty()) {
		char* deviceName = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
		if (deviceName == NULL) {
			__deviceName = "Tizen";
		} else {
			__deviceName = deviceName;
		}
		_D("device_name: %s", __deviceName.c_str());
	}

	return __deviceName;
}

static char __make_p2p_mac(char c)
{
	char convertC = c;
	if ((convertC >= 'A') && (convertC <= 'F')) {
		convertC = ((((convertC - 'A') + 10) | 0x02) - 10) + 'A';
	} else if ((convertC >= '0') && (convertC <= '9')) {
		convertC = ((convertC - '0') | 0x02);
		if (convertC < 10)
			convertC = convertC + '0';
		else
			convertC = 'A' + (convertC - 10);
	} else {
		_E("wrong byte for mac!");
	}
	return convertC;
}

std::string conv::util::getP2pMacAddress()
{
	static std::string __p2pMacAddress;
	if(__p2pMacAddress.empty()) {
		char p2p_mac[MAC_ADDR_STR_LEN];
		memset(p2p_mac, 0x0, MAC_ADDR_STR_LEN);

		char* temp_addr = vconf_get_str(VCONFKEY_WIFI_BSSID_ADDRESS);
		if (temp_addr == NULL) {
			_E("vconf_get_str Failed for %s", VCONFKEY_WIFI_BSSID_ADDRESS);
		} else {
			memcpy(p2p_mac, temp_addr, MAC_ADDR_STR_LEN-1);
			p2p_mac[1] = __make_p2p_mac(p2p_mac[1]);
			_I("P2P mac is %s", p2p_mac);
			free(temp_addr);

			__p2pMacAddress = p2p_mac;
		}
	}
	_D("p2p mac address:%s", __p2pMacAddress.c_str());
	return __p2pMacAddress;
}

static gboolean __misc_timer_worker(gpointer ud)
{
	_D("timer_work..");
    gpointer *tdata = (gpointer*)ud;
    if (tdata[0]) {
        ((conv::util::timer_function)tdata[0])(tdata[1]);
    }
    return TRUE;
}

void* conv::util::miscStartTimer(timer_function function, unsigned int interval, void *data)
{
    guint id = 0;
    GSource *src = NULL;
    gpointer *tdata = NULL;

	_D("misc_start_timer with interval[%d]", interval);
    src = g_timeout_source_new(interval*1000);

    tdata = g_new0(gpointer, 2);

    tdata[0] = (void*)function;
    tdata[1] = data;

    g_source_set_callback(src, __misc_timer_worker, tdata, g_free);
    id = g_source_attach(src, NULL);
    g_source_unref(src);

	_D("Done with id[%d] in misc_start_timer", id);
    return (void*)id;
}

void conv::util::miscStopTimer(void *timer)
{
    guint id = (guint) timer;
	_D("Requested Stop Timer[%d]", id);
    if (id) {
        if (!g_source_remove(id)) {
            _E("g_source_remove is fail (timer)");
        }
    }
}

std::string conv::util::getDeviceId()
{
	static std::string __deviceId;
	if(__deviceId.empty()) {
		__deviceId = Service::getUniqueId("127.0.0.1:8001");

		if(__deviceId.empty())
#ifdef _TV_
			__deviceId = getBtMacAddress();
#else
			__deviceId = getP2pMacAddress();
#endif
	}
	_D("device id : %s", __deviceId.c_str());

	return __deviceId;
}

bool conv::util::isServiceActivated(int serviceValue)
{
	int currentState;
	int error = vconf_get_int(VCONFKEY_SETAPPL_D2D_CONVERGENCE_SERVICE, &currentState);

	if (error != 0) {
		_D("vconf_get_int failed %d", error);
		return false;
	}

	if ((serviceValue & currentState) > 0) {
		return true;
	} else {
		return false;
	}
}
