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

#include "discovery_mgr_impl.h"

conv::discovery_manager_impl* conv::discovery_provider_base::_discovery_manager = NULL;

int conv::discovery_provider_base::set_manager(discovery_manager_impl* discovery_manager)
{
	_discovery_manager = discovery_manager;
	return CONV_ERROR_NONE;
}