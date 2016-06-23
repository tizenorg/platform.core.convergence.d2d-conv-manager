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

#include "Error.h"

Error::Error()
{
}

Error::Error(long code, string name, string message)
{
	this->code = code;
	this->name = name;
	this->message = message;
}

Error Error::create(string message)
{
	return create(-1, "error", message);
}

Error Error::create(long code, string name, string message)
{
	if ((name == "") || (message == "")) {
		name = "error";
	}

	Error *error = new Error(code, name, message);
	return *error;
}
const char *Error::get_error_message()
{
	return message.c_str();
}
