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

#ifndef __TIZEN_D2D_CONV_SERVICE_H__
#define __TIZEN_D2D_CONV_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file d2d_conv_service.h
 */

/**
 * @addtogroup CAPI_D2D_CONVERGENCE_MANAGER_SERVICE_MODULE
 * @{
 */

/**
 * @brief	The attribute key "service id"
 * @since_tizen 3.0
 */
#define CONV_SERVICE_ID "service_id"

/**
 * @brief	The attribute key "service version"
 * @since_tizen 3.0
 */
#define CONV_SERVICE_VERSION "service_version"

/**
 * @brief		Enumeration for service types.
 * @since_tizen 3.0
 */
typedef enum {
	CONV_SERVICE_NONE = -1,		/**< Undefined service */
	CONV_SERVICE_APP_TO_APP_COMMUNICATION = 0,	/**< App-to-app communication service */
	CONV_SERVICE_REMOTE_APP_CONTROL,	/**< Remote app-control service */
	CONV_SERVICE_REMOTE_INTERACTION /**< Remote Interaction service */
} conv_service_e;

/**
 * @brief		Enumeration for connection states.
 * @since_tizen 3.0
 */
typedef enum {
	CONV_SERVICE_CONNECTION_STATE_NONE = -1,		/**< Undefined state */
	CONV_SERVICE_CONNECTION_STATE_CONNECTED = 0,		/**< Connected state */
	CONV_SERVICE_CONNECTION_STATE_NOT_CONNECTED,		/**< Not connected state */
	CONV_SERVICE_CONNECTION_STATE_CONNECTING		/**< Connecting state */
} conv_service_connection_state_e;

/**
 * @brief		Convergence service handle for connecting and communicating D2D services.
 * @since_tizen 3.0
 */
typedef struct _conv_service_handle_s* conv_service_h;

/**
 * @brief Called when the connection state with the remote service is changed.
 * @details Specifies the type of function passed to conv_service_connect() \n
 * 			The following error codes can be delivered. \n
 * 			#CONV_ERROR_NONE, \n
 * 			#CONV_ERROR_INVALID_OPERATION, \n
 * 			#CONV_ERROR_INVALID_PARAMETER
 *
 * @since_tizen 3.0
 *
 * @param[in] service_handle The service handle tried to connect to
 * @param[in] error The error value
 * @param[in] result The payload handle contains result data
 * @param[in] user_data The user data to pass to the foreach function
 *
 * @see conv_service_connect()
 */
typedef void(* conv_service_connected_cb)(conv_service_h service_handle, conv_error_e error, conv_payload_h result, void* user_data);

/**
 * @brief Called when the service is started or stopped, and when data is received or published.
 * @details Specifies the type of function passed to conv_service_set_listener_cb() \n
 * 			The following error codes can be delivered. \n
 * 			#CONV_ERROR_NONE, \n
 * 			#CONV_ERROR_INVALID_OPERATION, \n
 * 			#CONV_ERROR_INVALID_PARAMETER
 *
 * @since_tizen 3.0
 *
 * @param[in] service_handle The service handle called from
 * @param[in] channel_handle The channel handle contains channel identifiers
 * @param[in] error The error value
 * @param[in] result The payload handle contains result data
 * @param[in] user_data The user data to pass to the foreach function
 *
 * @pre conv_service_set_listener_cb()
 * @post conv_service_unset_listener_cb()
 * @see conv_service_start()
 * @see conv_service_read()
 * @see conv_service_publish()
 * @see conv_service_stop()
 */
typedef void(* conv_service_listener_cb)(conv_service_h service_handle, conv_channel_h channel_handle, conv_error_e error, conv_payload_h result, void* user_data);

/**
 * @brief		Creates D2D convergence manager service handle.
 * @since_tizen 3.0
 * @remarks		The @a handle must be released using conv_service_destroy().
 *
 * @param[out] handle    Handle to be initialized
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_OUT_OF_MEMORY			Out of memory
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @post conv_service_destroy()
 */
int conv_service_create(conv_service_h* handle);

/**
 * @brief		Clones D2D convergence manager service handle from discovered service handle.
 * @since_tizen 3.0
 * @remarks		The @a handle must be released using conv_service_destroy().
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
 * @post conv_service_destroy()
 */
int conv_service_clone(conv_service_h original_handle, conv_service_h* target_handle);

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
 * @pre	conv_service_create()
 * @pre	conv_service_clone()
 */
int conv_service_destroy(conv_service_h handle);

/**
 * @brief		Gets a string from service property.
 * @since_tizen 3.0
 * @remarks		The @a value must be released using free().
 *
 * @param[in]	handle		The service handle
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
 * @see		conv_service_set_property_string()
 */
int conv_service_get_property_string(conv_service_h handle, const char* key, char** value);

/**
 * @brief		Sets a string to service property.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The service handle
 * @param[in]	key			The key of property to set
 * @param[in]	value		The string value to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_service_get_property_string()
 */
int conv_service_set_property_string(conv_service_h handle, const char* key, const char* value);

/**
 * @brief		Gets connection state from service.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/network.get
 *
 * @param[in]	handle		The service handle
 * @param[out]	state		The connection state value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 */
int conv_service_get_connection_state(conv_service_h handle, conv_service_connection_state_e* state);

/**
 * @brief		Gets service type from service handle.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The service handle
 * @param[out]	value		The service type value
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_service_set_type()
 */
int conv_service_get_type(conv_service_h handle, conv_service_e* value);

/**
 * @brief		Sets service type to service handle.
 * @since_tizen 3.0
 *
 * @param[in]	handle		The service handle
 * @param[in]	value		The service type value to set
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @see		conv_service_get_type()
 */
int conv_service_set_type(conv_service_h handle, conv_service_e value);

/**
 * @brief		Connects to remote service asynchronously.
 * @details		Makes physical connections such as WiFi-Direct or Bluetooth to remote service.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 *
 * @remarks		The @a callback is called when the connection state is changed.
 *
 * @param[in]	handle      Service handle to connect
 * @param[in]	callback		Callback function to receive connection result. This can not be @c NULL
 * @param[in]	user_data		The user data to pass to the callback function
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @post conv_service_disconnect()
 * @post conv_service_connected_cb() will be invoked.
 */
int conv_service_connect(conv_service_h handle, conv_service_connected_cb callback, void* user_data);

/**
 * @brief		Disconnects connected service asynchronously.
 * @details		Disconnects physical connection with remote service.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 *
 * @param[in]	handle      Service handle to disconnect
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @pre conv_service_connect()
 * @post conv_service_connected_cb() will be invoked.
 */
int conv_service_disconnect(conv_service_h handle);

/**
 * @brief		Starts and initiates remote service asynchronously.
 * @details		Invokes initialize and send start message to remote service to initialize.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 * @privilege %http://tizen.org/privilege/d2d.datasharing
 *
 * @param[in]	handle      Service handle to manage
 * @param[in]	channel	Channel handle to specify logical session in one service
 * @param[in]	payload		Payload handle to contain additional data for start request
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @post conv_service_stop()
 * @post conv_service_listener_cb() will be invoked.
 */
int conv_service_start(conv_service_h handle, conv_channel_h channel, conv_payload_h payload);

/**
 * @brief		Reads from remote service asynchronously.
 * @details		Retrieves data from remote service or regarding to remote service.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 *
 * @param[in]	handle      Service handle to manage
 * @param[in]	channel	Channel handle to specify logical session in one service
 * @param[in]	payload		Payload handle to contain additional data for read request
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @pre conv_service_start()
 * @post conv_service_listener_cb() will be invoked.
 * @see conv_service_publish()
 */
int conv_service_read(conv_service_h handle, conv_channel_h channel, conv_payload_h payload);

/**
 * @brief		Publishes message to remote service asynchronously.
 * @details		Publishes payload to remote service to send data or emit custom events.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 * @privilege %http://tizen.org/privilege/d2d.datasharing
 *
 * @param[in]	handle      Service handle to manage
 * @param[in]	channel	Channel handle to specify logical session in one service
 * @param[in]	payload		Payload handle to contain additional data for publish
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @pre conv_service_start()
 * @post conv_service_listener_cb() will be invoked.
 * @see conv_service_read()
 */
int conv_service_publish(conv_service_h handle, conv_channel_h channel, conv_payload_h payload);

/**
 * @brief		Stops remote service asynchronously.
 * @details		Stops and release internally allocated resources for remote service.
 * @since_tizen 3.0
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 * @privilege %http://tizen.org/privilege/bluetooth
 *
 * @param[in]	handle      Service handle to manage
 * @param[in]	channel	Channel handle to specify logical session in one service
 * @param[in]	payload		Payload handle to contain additional data for stop request
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 * @retval		#CONV_ERROR_PERMISSION_DENIED		Permission denied
 *
 * @pre conv_service_start()
 * @post conv_service_listener_cb() will be invoked.
 */
int conv_service_stop(conv_service_h handle, conv_channel_h channel, conv_payload_h payload);

/**
 * @brief		Registers listener to receive message from remote service.
 * @details		Listens any messages from remote service.
 * @since_tizen 3.0
 *
 * @remarks		The @a callback is called any event occurred or any message is received from remote server.
 *
 * @param[in]	handle      Service handle to listen
 * @param[in]	callback		Callback function to receive connection result. This can not be @c NULL
 * @param[in]	user_data		The user data to pass to the callback function
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @post conv_service_unset_listener_cb()
 */
int conv_service_set_listener_cb(conv_service_h handle, conv_service_listener_cb callback, void* user_data);

/**
 * @brief		Unregisters listener for remote service.
 * @since_tizen 3.0
 *
 * @param[in]	handle      Service handle to unregister
 *
 * @return		0 on success, otherwise a negative error value
 * @retval		#CONV_ERROR_NONE					Successful
 * @retval		#CONV_ERROR_INVALID_PARAMETER		Invalid parameter
 * @retval		#CONV_ERROR_INVALID_OPERATION		Operation failed
 * @retval		#CONV_ERROR_NOT_SUPPORTED		Not supported
 *
 * @pre conv_service_set_listener_cb()
 */
int conv_service_unset_listener_cb(conv_service_h handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @}
*/

#endif /* __TIZEN_D2D_CONV_SERVICE_H__ */
