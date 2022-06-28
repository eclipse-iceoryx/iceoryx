#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/error.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/platform/error_storage.hpp"

#include <iostream>

namespace eh
{
// implementation of the test platform handling
// TODO: ensure FATAL errors do not continue in tests

// if the mechanism to check for an error with exceptions or some error stack works
// (the latter should always work), there is no reason to implement a more costly runtime dispatch
// for the handling

// only available on test platform
ErrorStorage& errors()
{
    static ErrorStorage s;
    return s;
}

template <class Level>
void handle(const SourceLocation& location, Level level)
{
    (void)location;
    (void)level;
    errors().add(GenericError());
    throw GenericError();
}

template <class Level, class Error>
void handle(const SourceLocation& location, Level level, const Error& error)
{
    (void)location;
    (void)level;
    errors().add(GenericError::from_error(error));
    throw Error(error);
}

template <class Level>
void handle(const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    (void)location;
    (void)level;
    errors().add(GenericError(module, code));
    throw GenericError(module, code);
}

void preterminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh