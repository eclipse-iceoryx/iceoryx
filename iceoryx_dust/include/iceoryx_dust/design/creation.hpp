// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_DUST_DESIGN_CREATION_HPP
#define IOX_DUST_DESIGN_CREATION_HPP

// NOLINTJUSTIFICATION iox-#1036 will be replaced by builder pattern
// NOLINTBEGIN
#include "iox/expected.hpp"

namespace DesignPattern
{
/// @brief This pattern can be used if you write an abstraction where you have
///        to throw an exception in the constructor when you for instance would
///        like to manage a resource and the constructor was unable to acquire
///        that resource.
///        In this case you inherit from 'Creation' and your class has three more
///        static factory methods - 'create', 'placementCreate' and 'verify'.
///        'create' forwards all arguments to the underlying class constructor and
///        if the construction was successful an expected containing the type is
///        returned, otherwise an error value which describes the error.
///        Additionally, this class is providing two protected member variables
///        'm_isInitialized' and 'm_errorValue'. The user always has to set
///        'm_isInitialized' to true when the object construction was successful
///        otherwise one sets it to false and write the corresponding error cause
///        in the provided 'm_errorValue' variable which is then returned to
///        the user.
/// @code
///   enum class MyResourceAbstractionError {
///     ResourceNotAvailable,
///     BlaBlubError
///   };
///   class MyResourceAbstraction : public Creation<MyResourceAbstraction, MyResourceAbstractionError> {
///     public:
///       // some public methods
///       MyResourceAbstraction & operator=(MyResourceAbstraction && rhs) noexcept {
///         if ( this != &rhs ) {
///           // always call the creation move assignment operator when you have a user defined
///           // move operation
///           CreationPattern_t::operator=(std::move(rhs));
///
///           // user move code
///         }
///         return *this;
///       }
///
///       // the creation pattern is the only one which should be allowed to construct
///       // the class, therefore it has to be friend of that class
///       friend class Creation<MyResourceAbstraction, MyResourceAbstractionError>;
///     private:
///       MyResourceAbstraction(int a) {
///         if ( a > 0) {
///           // we are able to initialize the class an set m_isInitialized to true
///           m_isInitialized = true;
///         } else {
///           // we are unable to construct the class therefore we have to set
///           // m_isInitialized to false and store the error code in the
///           // provided m_errorValue member
///           m_errorValue = MyResourceAbstractionError::ResourceNotAvailable;
///           m_isInitialized = false;
///         }
///       }
///   }
///
///   // if the system resource is movable
///   auto resource = MyResourceAbstraction::Create(123);
///   if ( resource.has_error() && resource.error() == MyResourceAbstractionError::ResourceNotAvailable )
///     // perform error handling
///   else
///     // perform some work
///
///   // if the system resource is not movable
///   MyResourceAbstraction * resource = malloc(sizeof(MyResourceAbstraction));
///   auto result = MyResourceAbstraction::placementCreate(resource, 123);
///   if ( result.has_error() )
///     // perform error handling
///   else
///     resource->DoStuff();
///
///   delete resource;
/// @endcode
/// @tparam DerivedClass the class which inherits from the creation pattern
/// @tparam ErrorType the error type which is going to be used when an error occurs
template <typename DerivedClass, typename ErrorType>
class Creation
{
  public:
    using CreationPattern_t = Creation<DerivedClass, ErrorType>;
    using result_t = iox::expected<DerivedClass, ErrorType>;
    using errorType_t = ErrorType;

    /// @brief factory method which guarantees that either a working object is produced
    ///         or an error value describing the error during construction
    /// @tparam Targs the argument types which will be forwarded to the ctor
    /// @param[in] args the argument values which will be forwarded to the ctor
    /// @return returns an expected which either contains the object in a valid
    ///         constructed state or an error value stating why the construction failed.
    template <typename... Targs>
    static result_t create(Targs&&... args) noexcept;

    /// @brief verifies if a class was created successfully
    /// @param[in] newObject rvalue of the object which should be verified
    /// @return returns an expected which either contains the object in a valid
    ///         constructed state or an error value stating why it was in an invalid state.
    static result_t verify(DerivedClass&& newObject) noexcept;

    /// @brief factory method which guarantees that either a working object is produced
    ///         or an error value describing the error during construction
    /// @tparam Targs the argument types which will be forwarded to the ctor
    /// @param[in] memory a piece of memory where the object is created into with placement new
    /// @param[in] args the argument values which will be forwarded to the ctor
    /// @return returns an expected which either contains the object in a valid
    ///         constructed state or an error value stating why the construction failed.
    template <typename... Targs>
    static iox::expected<void, ErrorType> placementCreate(void* const memory, Targs&&... args) noexcept;

    Creation() noexcept = default;
    Creation(Creation&& rhs) noexcept;

    Creation& operator=(Creation&& rhs) noexcept;
    Creation(const Creation& rhs) noexcept = default;
    Creation& operator=(const Creation& rhs) noexcept = default;

    /// @brief returns true if the object was constructed successfully, otherwise false
    bool isInitialized() const noexcept;

  protected:
    bool m_isInitialized{false};
    ErrorType m_errorValue;
};

} // namespace DesignPattern

#include "iceoryx_dust/internal/design/creation.inl"
// NOLINTEND

#endif // IOX_DUST_DESIGN_CREATION_HPP
