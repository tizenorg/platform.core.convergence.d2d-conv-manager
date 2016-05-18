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

#ifndef __TIZEN_D2D_CONV_DEVICE_H__
#define __TIZEN_D2D_CONV_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file d2d_conv_device.h
 */

/**
 * @addtogroup CAPI_D2D_CONVERGENCE_MANAGER_DEVICE_MODULE
 * @{
 */

/**
 * @brief	The attribute key "device id"
 * @since_tizen 3.0
 */
#define CONV_DEVICE_ID "device_id"

/**
 * @brief	The attribute key "device name"
 * @since_tizen 3.0
 */
#define CONV_DEVICE_NAME "device_name"

/**
 * @brief	The attribute key "device type"
 * @since_tizen 3.0
 */
#define CONV_DEVICE_TYPE "device_type"

/**
 * @brief		Convergence device handle for accessing device information and device-to-device services.
 * @since_tizen 3.0
 */
typedef struct _conv_device_handle_s* conv_device_h;

/**
 * @brief Called for each service available on the device.
 * @details Specifies the type of function passed to conv_device_foreach_service() \n
 * 			The @a service_handle value is only valid in the callback function.\n
 * 			If you want to use it outside of the callback, you need to use a clone which can be obtained with conv_service_clone()
 *
 *
 * @since_tizen 3.0
 *
 * @param[in] service_handle The service handle to access discovered service information
 * @param[in] user_data The user data passed to the foreach function
 *
 * @pre conv_device_foreach_service() calls this callback
 * @see conv_device_foreach_service()
 */
typedef void(* conv_service_foreach_cb)(conv_service_h service_handle, void* user_data);

/**
 * @brief		Clones D2D convergence manager device handle from discovered device handle.
 * @since_tizen 3.0
 * @remarks		The @a handle must be released using conv_device_destroy().
 *
 * @param[in] original_handle    Source handle
 * @param[out] target_handle    Clone handle
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @post conv_device_destroy()
 */
int conv_device_clone(conv_device_h original_handle, conv_device_h* target_handle);

/**
 * @brief		Releases the resources occupied by the device handle.
 * @details		This releases the memory allocated for the @a handle.
 *
 * @since_tizen 3.0
 * @remarks		This must only called for cloned device handle by conv_device_clone().
 *
 * @param[in]  handle   Handle to be released
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @pre	conv_device_clone()
 */
int conv_device_destroy(conv_device_h handle);

/**
 * @brief		Gets a string from device property.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The device handle
 * @param[in]	key			The key of property to get
 * @param[out]	value		The result value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_NO_DATA				No Data
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 * @see		conv_discovery_start()
 */
int conv_device_get_property_string(conv_device_h handle, const char* key, char** value);

/**
 * @brief Gets all service handles available on the device by invoking the callback function.
 * @details conv_service_foreach_cb() will be called for each service.
 *
 * @since_tizen 3.0
 *
 * @param[in] handle The device handle contains service list
 * @param[in] cb The callback function to get each service handle
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #CONV_ERROR_NONE  Successful
 * @retval #CONV_ERROR_NOT_SUPPORTED  Not supported
 * @retval #CONV_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post conv_service_foreach_cb() will be called for each item.
 *
 * @see conv_service_foreach_cb()
 */
int conv_device_foreach_service(conv_device_h handle, conv_service_foreach_cb cb, void* user_data);

#ifdef __cplusplus
}
#endif/* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_DEVICE_H__ */

