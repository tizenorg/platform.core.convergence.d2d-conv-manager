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

#ifndef __TIZEN_D2D_CONV_MANAGER_H__
#define __TIZEN_D2D_CONV_MANAGER_H__

#include <d2d_conv_error.h>
#include <d2d_conv_channel.h>
#include <d2d_conv_payload.h>
#include <d2d_conv_service.h>
#include <d2d_conv_device.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @addtogroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK
 * @{
 */

/**
 * @brief		Enumeration for discovery results.
 * @since_tizen 3.0
 */
typedef enum {
	CONV_DISCOVERY_RESULT_ERROR = -1,	/**< Discovery Error */
	CONV_DISCOVERY_RESULT_SUCCESS = 0,	/**< Discovery Success */
	CONV_DISCOVERY_RESULT_FINISHED	/**< Discovery finished */
} conv_discovery_result_e;

/**
 * @brief		Convergence manager handle for discovering D2D services.
 * @since_tizen 3.0
 */
typedef struct _conv_handle_s*						conv_h;

/**
 * @brief Called when a nearby device which provides D2D services has been discovered.
 * @details Specifies the type of function passed to conv_discovery_start() \n
 * 			The @a device_handle value is only valid in the callback function and in case result is @c CONV_DISCOVERY_RESULT_SUCCESS.\n
 * 			If you want to use it outside of the callback, you need to use a clone which can be obtained with conv_service_clone()\n
 * 			The following results can be delivered. \n
 * 			#CONV_DISCOVERY_RESULT_ERROR, \n
 * 			#CONV_DISCOVERY_RESULT_SUCCESS, \n
 * 			#CONV_DISCOVERY_RESULT_FINISHED
 *
 * @since_tizen 3.0
 *
 * @param[in] device The device discovered
 * @param[in] result The result value for discovery
 * @param[in] user_data The user data to pass to the function
 *
 * @see conv_discovery_start()
 * @see conv_discovery_stop()
 */
typedef void(* conv_discovery_cb)(conv_device_h device_handle, conv_discovery_result_e result, void* user_data);

/**
 * @brief		Creates a D2D convergence manager handle.
 * @since_tizen 3.0
 * @remarks		The @a handle must be released using conv_destroy().
 *
 * @param[out] handle    Handle to be initialized
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 *
 * @post conv_destroy()
 */
int conv_create(conv_h* handle);

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
 * @pre	conv_create()
 */
int conv_destroy(conv_h handle);

/**
 * @brief		Starts discovery near-by devices.
 * @details		Discovers device-to-device services supported devices.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 *
 * @remarks		The @a callback is called when each near-by device has been found.
 *
 * @param[in]	handle      Handle for controlling discovery requests
 * @param[in]	timeout_seconds	Seconds for discovery timeout. 0 will use default timeout value
 * @param[in]	callback		Callback function to receive discovered device information. This can not be @c NULL
 * @param[in]	user_data		The user data to pass to the callback function
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @pre conv_create()
 * @post conv_discovery_stop()
 */
int conv_discovery_start(conv_h handle, const int timeout_seconds, conv_discovery_cb callback, void* user_data);

/**
 * @brief		Stops discovery near-by devices.
 * @details		Stops discovery for device-to-device services supported devices.
 * @since_tizen 3.0
 *
 * @param[in]	handle      Handle for controlling discovery requests
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @pre conv_discovery_start()
 */
int conv_discovery_stop(conv_h handle);

#ifdef __cplusplus
}
#endif/* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_MANAGER_H__ */
