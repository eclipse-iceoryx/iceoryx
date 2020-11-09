# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FAQ.md
	DESTINATION share/doc/iceoryx
	COMPONENT dev)

if(TOML_CONFIG)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/iceoryx/roudi_config_example.toml
		DESTINATION etc/
		COMPONENT dev)
endif(TOML_CONFIG)
