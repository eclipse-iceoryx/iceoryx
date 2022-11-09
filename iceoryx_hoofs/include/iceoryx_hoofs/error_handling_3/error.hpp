#pragma once

#include <stdint.h>

namespace eh3
{
using error_code_t = uint32_t;
using module_id_t = uint32_t;

// we expect an error to have
// 1. error_code_t code()
// 2. module_id_t module()
// 3. const char* name()

// 0 is reserved
constexpr module_id_t INVALID_MODULE = 0;

// primary template is the identity, only for lvalues (change?)
template <typename Error>
auto& toError(Error& error)
{
    return error;
}

// generic comparison, has interface requirements on error types E1 and E2 without
// inheritance
template <typename E1, typename E2>
bool equals(const E1& a, const E2& b)
{
    return a.code() == b.code() && a.module() == b.module();
}

/*
// not useful since argument dependent lookup will generally not find it
template <typename E1, typename E2>
bool operator==(const E1& a, const E2& b)
{
    return equals(a, b);
}

template <typename E1, typename E2>
bool operator!=(const E1& a, const E2& b)
{
    return !(a == b);
}
*/

} // namespace eh3
