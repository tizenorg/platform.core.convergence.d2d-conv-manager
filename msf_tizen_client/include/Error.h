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

#ifndef __MSF_ERROR_H__
#define __MSF_ERROR_H__

#include <stdio.h>
#include<iostream>
#include<string>

using namespace std;

class Error
{
private:
	long code;
	string name;
	string message;

public:
	Error();
	Error(long code, string name, string message);
	static Error create(string message);
	static Error create(long code, string name , string message);
	const char *get_error_message();
	//write other create funcs
};

#endif
