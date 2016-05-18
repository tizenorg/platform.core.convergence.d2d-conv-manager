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

#ifndef __CONV_LOG_H__
#define __CONV_LOG_H__

#define TIZEN_DEBUG_ENABLE
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "D2D-CONV-MANAGER"

#include <dlog.h>
#include <tizen_error.h>

#define EXTAPI __attribute__ ((visibility("default")))
#define EMPTY_STRING ""

/* Internal Error Codes
    * Please define the necessary error codes here.
	 * Note that, these error codes must be aligned with the error enums defined in all API headers.
	  */


/* Logging and Error Handling */
#define _I SLOGI
#define _D SLOGD
#define _W SLOGW
#define _E SLOGE
#define _SI SECURE_SLOGI
#define _SD SECURE_SLOGD
#define _SW SECURE_LOGW
#define _SE SECURE_SLOGE

#define RED(X)		"\033[0;31m" X "\033[0m"
#define GREEN(X)	"\033[0;32m" X "\033[0m"
#define YELLOW(X)	"\033[0;33m" X "\033[0m"
#define BLUE(X)		"\033[0;34m" X "\033[0m"
#define PURPLE(X)	"\033[0;35m" X "\033[0m"
#define CYAN(X)		"\033[0;36m" X "\033[0m"

#define STR_EQ(X, Y) (g_strcmp0((X), (Y)) == 0)

#define IF_FAIL_RETURN_TAG(cond, ret, tag, fmt, arg...) \
		do { if (!(cond)) { tag(fmt, ##arg); return ret; } } while (0)

#define IF_FAIL_RETURN(cond, ret) \
		do { if (!(cond)) { return ret; } } while (0)

#define IF_FAIL_VOID_TAG(cond, tag, fmt, arg...) \
		do { if (!(cond)) { tag(fmt, ##arg); return; } } while (0)

#define IF_FAIL_VOID(cond) \
		do { if (!(cond)) { return; } } while (0)

#define IF_FAIL_TAG(cond, tag, fmt, arg...) \
		do { if (!(cond)) { tag(fmt, ##arg); } } while (0)

#define IF_FAIL_CATCH_TAG(cond, tag, fmt, arg...) \
		do { if (!(cond)) { tag(fmt, ##arg); goto CATCH; } } while (0)

#define IF_FAIL_CATCH(cond) \
		do { if (!(cond)) { goto CATCH; } } while (0)

#define IS_FAILED(X) ((X) != ERR_NONE)

#define ASSERT_ALLOC(X)		IF_FAIL_RETURN_TAG(X, CONV_ERROR_OUT_OF_MEMORY, _E, "Memory allocation failed")
#define ASSERT_NOT_NULL(X)	IF_FAIL_RETURN_TAG(X, CONV_ERROR_INVALID_PARAMETER, _E, "Parameter null")

#define HANDLE_GERROR(Err) \
		do { if ((Err)) { _E("GError: %s", Err->message); g_error_free(Err); Err = NULL; } } while (0)


#endif
