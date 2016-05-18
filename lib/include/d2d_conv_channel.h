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

#ifndef __TIZEN_D2D_CONV_CHANNEL_H__
#define __TIZEN_D2D_CONV_CHANNEL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file d2d_conv_channel.h
 */

/**
 * @addtogroup CAPI_D2D_CONVERGENCE_MANAGER_CHANNEL_MODULE
 * @{
 */

/**
 * @brief		Convergence channel handle to define channel information.
 * @since_tizen 3.0
 */
typedef struct _conv_channel_handle_s* conv_channel_h;

/**
 * @brief		Creates D2D convergence manager channel handle.
 * @since_tizen 3.0
 * @remarks		The @a handle must be released using conv_channel_destroy().
 *
 * @param[out] handle    Handle to be initialized
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @post conv_channel_destroy()
 */
int conv_channel_create(conv_channel_h* handle);

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
 * @pre	conv_channel_create()
 */
int conv_channel_destroy(conv_channel_h handle);

/**
 * @brief		Gets a string from channel.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The channel handle
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
 * @see		conv_channel_set_string()
 */
int conv_channel_get_string(conv_channel_h handle, const char* key, char** value);

/**
 * @brief		Sets a string to channel.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The channel handle
 * @param[in]	key			The key of attribute to set
 * @param[in]	value		The string value to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_channel_get_string()
 */
int conv_channel_set_string(conv_channel_h handle, const char* key, const char* value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_CHANNEL_H__ */
