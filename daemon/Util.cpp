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
	static std::string g_mac_address;
	if(g_mac_address.empty()) {
		bt_initialize();
		char* mac_address;
		bt_adapter_enable();
		int ret = bt_adapter_get_address(&mac_address);
		IF_FAIL_RETURN_TAG(ret == 0, NULL, _E, "bluetooth get mac address failed : %d", ret);
		_D("bluetooth get mac address : %s", mac_address);
		g_mac_address = mac_address;
		free(mac_address);
		bt_deinitialize();
	}
	_D("mac address:%s", g_mac_address.c_str());
	return g_mac_address;
}


std::string conv::util::getDeviceName()
{
	static std::string g_device_name;
	if(g_device_name.empty()) {
		char* device_name = vconf_get_str(VCONFKEY_SETAPPL_DEVICE_NAME_STR);
		if (device_name == NULL) {
			g_device_name = "Tizen";
		} else {
			g_device_name = device_name;
		}
		_D("device_name: %s", g_device_name.c_str());
	}

	return g_device_name;
}

static char __make_p2p_mac(char c)
{
	char convert_c = c;
	if ((convert_c >= 'A') && (convert_c <= 'F')) {
		convert_c = ((((convert_c - 'A') + 10) | 0x02) - 10) + 'A';
	} else if ((convert_c >= '0') && (convert_c <= '9')) {
		convert_c = ((convert_c - '0') | 0x02);
		if (convert_c < 10)
			convert_c = convert_c + '0';
		else
			convert_c = 'A' + (convert_c - 10);
	} else {
		_E("wrong byte for mac!");
	}
	return convert_c;
}

std::string conv::util::getP2pMacAddress()
{
	static std::string g_p2p_mac_address;
	if(g_p2p_mac_address.empty()) {
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

			g_p2p_mac_address = p2p_mac;
		}
	}
	_D("p2p mac address:%s", g_p2p_mac_address.c_str());
	return g_p2p_mac_address;
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
	static std::string g_device_id;
	if(g_device_id.empty()) {
		g_device_id = Service::getUniqueId("127.0.0.1:8001");

		if(g_device_id.empty())
#ifdef _TV_
			g_device_id = getBtMacAddress();
#else
			g_device_id = getP2pMacAddress();
#endif
	}
	_D("device id : %s", g_device_id.c_str());

	return g_device_id;
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
