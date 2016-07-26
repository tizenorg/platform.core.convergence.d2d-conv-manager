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

#ifndef __CONV_JSON_H__
#define __CONV_JSON_H__

#include <sys/types.h>
#include <glib.h>
#include <string>
#include <list>
#include "Types.h"

#define _J(cmt, jobj) \
do { \
	_SD("%s: %s", (cmt), jobj.str().c_str()); \
} while (0)

#define EMPTY_JSON_OBJECT	"{}"
#define DEFAULT_PRECISION	3

namespace conv {
	class Json {
	private:
		typedef struct _JsonNode json_node_t;
		json_node_t *json_node;

		void parse(const char* s);
		void release();

		/* For Json vs Json comparison */
		bool getMemberList(json_node_t* node, std::list<std::string>& list);
		bool nodeEquals(json_node_t* lhs, json_node_t* rhs);
		bool valueEquals(json_node_t* lhs, json_node_t* rhs);
		bool objectEquals(json_node_t* lhs, json_node_t* rhs);
		bool arrayEquals(json_node_t* lhs, json_node_t* rhs);

	public:
		Json();
		Json(const char* s);
		Json(const std::string& s);

		/* This Json(const Json& j) only copies the reference to the underlying Json node.
		 * Therefore, changes applied to a Json object affect the other.
		 * If you need to create a 'real' copy of a Json, which can be manipulated separately,
		 * utilize the str() function, e.g., ctx::Json copy(original.str());
		 */
		Json(const Json& j);

		~Json();

		Json& operator=(const char* s);
		Json& operator=(const std::string& s);

		/* This operator=(const Json& j) only copies the reference to the underlying Json node.
		 * Therefore, changes applied to a Json object affect the other.
		 * If you need to create a 'real' copy of a Json, which can be manipulated separately,
		 * utilize the str() function, e.g., ctx::Json copy = original.str();
		 */
		Json& operator=(const Json& j);

		bool operator==(const Json& rhs);
		bool operator!=(const Json& rhs);

		char* dupCstr();
		std::string str();

		bool getKeys(std::list<std::string>* list);

		bool set(const char* path, const char* key, Json& val);
		bool set(const char* path, const char* key, int val);
		bool set(const char* path, const char* key, int64_t val);
		bool set(const char* path, const char* key, double val, int prec = DEFAULT_PRECISION);
		bool set(const char* path, const char* key, std::string val);
		bool set(const char* path, const char* key, GVariant *val);

		bool get(const char* path, const char* key, Json* val);
		bool get(const char* path, const char* key, int* val);
		bool get(const char* path, const char* key, int64_t* val);
		bool get(const char* path, const char* key, double* val);
		bool get(const char* path, const char* key, std::string* val);
		bool get(const char* path, const char* key, GVariant **val);

		int getArraySize(const char* path, const char* key);

		bool appendArray(const char* path, const char* key, Json& val);
		bool appendArray(const char* path, const char* key, int val);
		bool appendArray(const char* path, const char* key, int64_t val);
		bool appendArray(const char* path, const char* key, double val, int prec = DEFAULT_PRECISION);
		bool appendArray(const char* path, const char* key, std::string val);

		bool setArrayAt(const char* path, const char* key, int index, Json& val);
		bool setArrayAt(const char* path, const char* key, int index, int val);
		bool setArrayAt(const char* path, const char* key, int index, int64_t val);
		bool setArrayAt(const char* path, const char* key, int index, double val, int prec = DEFAULT_PRECISION);
		bool setArrayAt(const char* path, const char* key, int index, std::string val);

		bool getArrayElem(const char* path, const char* key, int index, Json* val);
		bool getArrayElem(const char* path, const char* key, int index, int* val);
		bool getArrayElem(const char* path, const char* key, int index, int64_t* val);
		bool getArrayElem(const char* path, const char* key, int index, double* val);
		bool getArrayElem(const char* path, const char* key, int index, std::string* val);
	};
}

#endif // __CONV_JSON_H__

