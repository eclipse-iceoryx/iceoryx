// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FUZZTESTSROUDIWRAPPER_HPP
#define FUZZTESTSROUDIWRAPPER_HPP
/// @brief fuzztests_roudi_wrapper contains the main method for the RouDi Wrappers which can be used to fuzz several
/// interfaces

/// @brief	Main function of the Fuzz Wrapper
/// @param[in] amount of arguments given to the method
///	@param[in] containing the command line parameters
///	@param[out] int containing the status if the fuzzing was successfull. If the return value is -1 something went
/// wrong.
int main(int argc, char* argv[]);

#endif /*FUZZTESTSROUDIWRAPPER*/
