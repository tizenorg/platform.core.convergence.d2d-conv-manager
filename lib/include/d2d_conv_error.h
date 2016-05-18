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

#ifndef __TIZEN_D2D_CONV_ERROR_H__
#define __TIZEN_D2D_CONV_ERROR_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file d2d_conv_error.h
 */

/**
 * @addtogroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK
 * @{
 */

/**
 * @brief		Enumeration for errors.
 * @since_tizen 3.0
 */
typedef enum {
	CONV_ERROR_NONE = TIZEN_ERROR_NONE,		/**< Successful */
	CONV_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,	/**< Invalid parameter */
	CONV_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,	/**< Invalid operation */
	CONV_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,	/**< Memory allocation failed */
	CONV_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,	/**< Permission denied */
	CONV_ERROR_NOT_SUPPORTED = TIZEN_ERROR_NOT_SUPPORTED,	/**< Not supported */
	CONV_ERROR_NO_DATA = TIZEN_ERROR_NO_DATA	/**< No Data */
} conv_error_e;

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_ERROR_H__ */
