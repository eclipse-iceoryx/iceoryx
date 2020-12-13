// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#pragma once

/// major todo: the function_ref.hpp has some persistency problem
/// storing a free function pointer and calling it later will not work properly
/// in some cases it seems
/// further investigation is needed whether this can be repaired
/// use this alternative for the time being
/// expectation: a function_ref is valid as long as the referenced
/// function exists (e.g. whole program runtime for a free function)


#include <type_traits>
#include <utility>

namespace iox
{
namespace cxx
{
template <typename ReturnType, typename... Args>
using signature = ReturnType(Args...);

template <typename T>
class function_ref;

template <typename ReturnType, typename... Args>
class function_ref<signature<ReturnType, Args...>>
{
    using obj_t = void*;
    using func_t = ReturnType (*)(Args...);
    using invocation_t = ReturnType (*)(func_t, obj_t, Args...);

  public:
    function_ref() = default;

    // from functor (includes lambda)
    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                 /*&& std::is_invocable_r<ReturnType, Functor, Args...>::value*/,
                                                 void>::type>
    function_ref(const Functor& functor)
    {
        assign((obj_t)&functor, functor_call<Functor>);
    }

    // from function pointer
    function_ref(ReturnType (*function)(Args...))
    {
        // note: we cannot cast function to void safely, we need to keep it as a function pointer
        assign(function, function_call);
    }

    template <typename T>
    function_ref(T& object, ReturnType (T::*method)(Args...))
    {
        // assign(object, method);
    }

    // from method, not possible with ctor (it seems, needs further research)
    template <typename T, ReturnType (T::*Func)(Args...)>
    static function_ref create(T& obj)
    {
        return function_ref((obj_t)&obj, method_call<T, Func>);
    }

    template <typename T, ReturnType (T::*Func)(Args...) const>
    static function_ref create(T& obj)
    {
        return function_ref((obj_t)&obj, const_method_call<T, Func>);
    }

    template <ReturnType (*Func)(Args...)>
    static function_ref create()
    {
        return function_ref(function_call_template<Func>);
    }

    function_ref(const function_ref&) = default;
    function_ref& operator=(const function_ref&) = default;
    function_ref(function_ref&&) = default;
    function_ref& operator=(function_ref&&) = default;

    ReturnType operator()(Args... args)
    {
        return (*m_invocation)(m_func, m_obj, std::forward<Args>(args)...);
    }

    operator bool()
    {
        return m_invocation != nullptr;
    }

    // note: equal operator makes not much sense in general if a callable is stored so we do not provide one

  private:
    // note: we cannot cast between void* and function pointers or member function pointers in a safe way...
    // therefore we need an erased type for function pointers (func_t) and for callable objects (obj_t)
    // we furthermore construct a helper invocation that given m_func and m_obj together with user provided arguments
    // will handle the actual call of the target function the wrapper stores

    func_t m_func; // the target function (from a function pointer)
    obj_t m_obj;   // the object to call the function on or on which to call operator()
    invocation_t m_invocation{
        nullptr}; // the wrapping invocation for the current case (functor, free function or member function)

    void assign(func_t func, invocation_t invocation)
    {
        m_func = func;
        m_obj = nullptr;
        m_invocation = invocation;
    }

    void assign(obj_t obj, invocation_t invocation)
    {
        m_func = nullptr;
        m_obj = obj;
        m_invocation = invocation;
    }

    void assign(func_t func, obj_t obj, invocation_t invocation)
    {
        m_func = func;
        m_obj = obj;
        m_invocation = invocation;
    }

    void assign(invocation_t invocation)
    {
        m_func = nullptr;
        m_obj = nullptr;
        m_invocation = invocation;
    }

    function_ref(obj_t obj, invocation_t invocation)
    {
        assign(obj, invocation);
    }

    function_ref(invocation_t invocation)
    {
        assign(invocation);
    }

    template <typename Functor>
    static ReturnType functor_call(func_t, obj_t obj, Args... args)
    {
        Functor* p = reinterpret_cast<Functor*>(obj);
        return (*p)(args...);
    }

    static ReturnType function_call(func_t func, obj_t, Args... args)
    {
        return func(args...);
    }

    template <ReturnType (*Func)(Args...)>
    static ReturnType function_call_template(func_t, obj_t, Args... args)
    {
        return (Func)(args...);
    }

    template <typename T, ReturnType (T::*Func)(Args...)>
    static ReturnType method_call(func_t, obj_t obj, Args... args)
    {
        T* p = reinterpret_cast<T*>(obj);
        return (p->*Func)(args...);
    }

    template <typename T, ReturnType (T::*Func)(Args...) const>
    static ReturnType const_method_call(func_t, obj_t obj, Args... args)
    {
        const T* p = reinterpret_cast<T*>(obj);
        return (p->*Func)(args...);
    }
};

} // namespace cxx
} // namespace iox