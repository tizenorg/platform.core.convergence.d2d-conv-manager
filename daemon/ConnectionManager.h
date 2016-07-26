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

#ifndef __CONNECTION_MANAGER_IMPL_H__
#define __CONNECTION_MANAGER_IMPL_H__

#include <iotcon.h>
#include <glib.h>
#include <vector>
#include "IManager.h"
#include "Request.h"

namespace conv {
	class ConnectionManager : public IManager  {
		public:
			ConnectionManager();
			~ConnectionManager();

			int init();
			int release();
			int handleRequest(Request* requestObj);
	};

	namespace connection_manager {
		void setInstance(ConnectionManager* mgr);
		int handleRequest(Request* requestObj);
	}
}

#endif /* __CONNECTION_MANAGER_IMPL_H__ */
