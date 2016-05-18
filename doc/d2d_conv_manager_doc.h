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

#ifndef __TIZEN_D2D_CONV_MANAGER_DOC_H__
#define __TIZEN_D2D_CONV_MANAGER_DOC_H__

/**
 * @ingroup CAPI_CONVERGENCE_FRAMEWORK
 *
 * @defgroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK Convergence Manager
 *
 * @brief D2D Convergence Manager provides APIs to manage device-to-device convergence service.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_MODULE_HEADER Required Header
 *  \#include <d2d_conv_manager.h>
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK_OVERVIEW Overview
 * Convergence Manager provides functions to discover services provided by near-by devices and to execute the services remotely.
 * The services includes the communication between applications of remote devices, application control to the remote device, or user interaction method to the remote devices. \n
 *
 * Using Convergence Manager APIs, a feature to control the remote devices can be implemented, which is a basic function for the Internet of Things (IoT).
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK_FEATURE Related Features
 * This API is related with the following features:\n
 * - %http://tizen.org/feature/convergence.d2d\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/getting-started/native-application/understanding-tizen-programming/application-filtering"><b>Feature List</b>.</a>
 *
 */

/**
 * @ingroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK
 * @defgroup CAPI_D2D_CONVERGENCE_MANAGER_CHANNEL_MODULE Channel
 *
 * @brief D2D Convergence Manager Channel APIs to manage channel information which is service-specific.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_CHANNEL_MODULE_HEADER Required Header
 *  \#include <d2d_conv_manager.h>
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_CHANNEL_MODULE_OVERVIEW Overview
 * Channel is a logically separated communication channel for each service, and thus the channel should be defined for each service.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_CHANNEL_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 * - %http://tizen.org/feature/convergence.d2d\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/getting-started/native-application/understanding-tizen-programming/application-filtering"><b>Feature List</b>.</a>
 *
 */

/**
 * @ingroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK
 * @defgroup CAPI_D2D_CONVERGENCE_MANAGER_DEVICE_MODULE Device
 *
 * @brief D2D Convergence Manager Device APIs to access device information.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_DEVICE_MODULE_HEADER Required Header
 *  \#include <d2d_conv_manager.h>
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_DEVICE_MODULE_OVERVIEW Overview
 * Device is a representative of a separate hardware device. Device handle includes an identification (ID), a name, a type, and a service list.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_DEVICE_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 * - %http://tizen.org/feature/convergence.d2d\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/getting-started/native-application/understanding-tizen-programming/application-filtering"><b>Feature List</b>.</a>
 *
 */

/**
 * @ingroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK
 * @defgroup CAPI_D2D_CONVERGENCE_MANAGER_PAYLOAD_MODULE Payload
 *
 * @brief D2D Convergence Manager Payload APIs to write payload to send data or retrieve values from payload.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_PAYLOAD_MODULE_HEADER Required Header
 *  \#include <d2d_conv_manager.h>
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_PAYLOAD_MODULE_OVERVIEW Overview
 * Payload is a set of data to transmit to or receive from the other devices, in order to perform the service. The fields of payload are defined based on the services.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_PAYLOAD_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 * - %http://tizen.org/feature/convergence.d2d\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/getting-started/native-application/understanding-tizen-programming/application-filtering"><b>Feature List</b>.</a>
 *
 */

/**
 * @ingroup CAPI_D2D_CONVERGENCE_MANAGER_FRAMEWORK
 * @defgroup CAPI_D2D_CONVERGENCE_MANAGER_SERVICE_MODULE Service
 *
 * @brief D2D Convergence Manager Service APIs to connect and communicate to remote/local service.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_SERVICE_MODULE_HEADER Required Header
 *  \#include <d2d_conv_manager.h>
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_SERVICE_MODULE_OVERVIEW Overview
 * Service is a core functioning implementation entity to process the requests of the users. Currently, three serivces are provided:\n
 *
 * - App Communication Service: provides functions to launch applications and to communicate with applications of remote devices.\n
 *
 * - App Control Service: provides Tizen application control feature between remote devices.\n
 *
 * - Remote Interaction: provides user interaction to remote devices.\n
 *
 * For more details, please refer to API guide page.
 *
 * @section CAPI_D2D_CONVERGENCE_MANAGER_SERVICE_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 * - %http://tizen.org/feature/convergence.d2d\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/getting-started/native-application/understanding-tizen-programming/application-filtering"><b>Feature List</b>.</a>
 *
 */

#endif /* __TIZEN_D2D_CONV_MANAGER_DOC_H__ */
