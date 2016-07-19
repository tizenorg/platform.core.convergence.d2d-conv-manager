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

#ifndef __TIZEN_D2D_CONV_INTERNAL_H__
#define __TIZEN_D2D_CONV_INTERNAL_H__

#include <d2d_conv_error.h>
#include <d2d_conv_channel.h>
#include <d2d_conv_payload.h>
#include <d2d_conv_service.h>
#include <d2d_conv_device.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file d2d_conv_internal.h
 */

/**
 * @brief	The internal value to set app-to-app communication service
 * @since_tizen 3.0
 */
#define CONV_INTERNAL_SERVICE_APP_TO_APP_COMMUNICATION 0x0001

/**
 * @brief	The internal value to set remote app-control service
 * @since_tizen 3.0
 */
#define CONV_INTERNAL_SERVICE_REMOTE_APP_CONTROL 0x0002

/**
 * @brief		Exports json string from channel.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The channel handle
 * @param[out]	value		The result value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 */
int conv_channel_internal_export_to_string(conv_channel_h handle, char** value);

/**
 * @brief		Exports json string from payload.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The payload handle
 * @param[out]	value		The result value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 */
int conv_payload_internal_export_to_string(conv_payload_h handle, char** value);

/**
 * @brief		Set D2D convergence activation state
 * @since_tizen 3.0
 *
 * @param[in]	activation_state		The activation state to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 */
int conv_internal_set_activation_state(int activation_state);

/**
 * @brief		Get D2D convergence activation state
 * @since_tizen 3.0
 *
 * @param[out]	activation_state		The current activation state
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 */
int conv_internal_get_activation_state(int* activation_state);

/**
 * @brief		Set D2D convergence activation state of services
 * @since_tizen 3.0
 *
 * @param[in]	activation_state		The activation state to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 */
int conv_internal_set_service_activation_state(int activation_state);

/**
 * @brief		Get D2D convergence activation state of services
 * @since_tizen 3.0
 *
 * @param[in]	activation_state		The activation state to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 */
int conv_internal_get_service_activation_state(int* activation_state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_INTERNAL_H__ */
