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

#ifndef __TIZEN_D2D_CONV_PAYLOAD_H__
#define __TIZEN_D2D_CONV_PAYLOAD_H__

#include <appfw/app_control.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file d2d_conv_payload.h
 */

/**
 * @addtogroup CAPI_D2D_CONVERGENCE_MANAGER_PAYLOAD_MODULE
 * @{
 */

/**
 * @brief		Convergence payload handle to manage payload data.
 * @since_tizen 3.0
 */
typedef struct _conv_payload_handle_s* conv_payload_h;

/**
 * @brief		Creates D2D convergence manager payload handle.
 * @since_tizen 3.0
 * @remarks		The @a handle must be released using conv_payload_destroy().
 *
 * @param[out] handle    Handle to be initialized
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @post conv_payload_destroy()
 */
int conv_payload_create(conv_payload_h* handle);

/**
 * @brief		Releases the resources occupied by a handle.
 * @details		This releases the memory allocated for the @a handle.
 *
 * @since_tizen 3.0
 *
 * @param[in]  handle   Handle to be released
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @pre	conv_payload_create()
 */
int conv_payload_destroy(conv_payload_h handle);

/**
 * @brief		Sets a string to payload.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The payload handle
 * @param[in]	key			The key of attribute to set
 * @param[in]	value		The string value to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_payload_get_string()
 */
int conv_payload_set_string(conv_payload_h handle, const char* key, const char* value);

/**
 * @brief		Gets a string from payload.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The payload handle
 * @param[in]	key			The key of attribute to get
 * @param[out]	value		The result value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 * @see		conv_payload_set_string()
 */
int conv_payload_get_string(conv_payload_h handle, const char* key, char** value);

/**
 * @brief		Sets an app-control to payload.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The payload handle
 * @param[in]	key			The key of property to set
 * @param[in]	app_control		The app-control handle to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_payload_get_app_control()
 */
int conv_payload_set_app_control(conv_payload_h handle, const char* key, app_control_h app_control);

/**
 * @brief		Gets an app-control handle from payload.
 * @since_tizen 3.0
 * @remarks		The @a app_control must be released using app_control_destroy().
 *
 * @param[in]	handle		The payload handle
 * @param[in]	key			The key of attribute to get
 * @param[out]	app_control		The app-control handle
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 * @see		conv_payload_set_app_control()
 */
int conv_payload_get_app_control(conv_payload_h handle, const char* key, app_control_h* app_control);

/**
 * @brief		Sets byte array to payload.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The payload handle
 * @param[in]	key			The key of property to set
 * @param[in]	length		The length of byte array to set
 * @param[in]	value		The byte array to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_payload_get_byte()
 */
int conv_payload_set_byte(conv_payload_h handle, const char* key,  int length, unsigned char* value);

/**
 * @brief		Gets byte array from payload.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The payload handle
 * @param[in]	key			The key of attribute to get
 * @param[out]	length		The length of byte array
 * @param[out]	value		The result value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 *
 * @see		conv_payload_set_byte()
 */
int conv_payload_get_byte(conv_payload_h handle, const char* key,  int* length, unsigned char** value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_PAYLOAD_H__ */
