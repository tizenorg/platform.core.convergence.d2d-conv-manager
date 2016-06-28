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

#include "common.h"
#include "conv_lib_util.h"
#include <system_info.h>

#define D2D_FEATURE "http://tizen.org/feature/convergence.d2d"

static int _feature_supported = -1;

bool conv::util::is_feature_supported()
{
	//LCOV_EXCL_START
	if (_feature_supported < 0) {
		bool feature_supported = false;
		system_info_get_platform_bool(D2D_FEATURE, &feature_supported);
		_feature_supported = feature_supported ? 1 : 0;
		_D("D2D feature enable %d", feature_supported);
	}
	return _feature_supported;
	//LCOV_EXCL_STOP
}
