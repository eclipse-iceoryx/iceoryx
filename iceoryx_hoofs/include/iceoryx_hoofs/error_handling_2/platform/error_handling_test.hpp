#pragma once

#include "iceoryx_hoofs/error_handling_2/error_code.hpp"
#include "iceoryx_hoofs/error_handling_2/error_stream.hpp"
#include "iceoryx_hoofs/error_handling_2/location.hpp"

#include "iceoryx_hoofs/error_handling_2/error.hpp"

#include "iceoryx_hoofs/error_handling_2/platform/error_levels.hpp"
#include "iceoryx_hoofs/error_handling_2/platform/error_reporting.hpp"
#include "iceoryx_hoofs/error_handling_2/platform/error_storage.hpp"

#include <iostream>

namespace eh
{
// implementation of the test platform handling

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
void handle(ErrorStream& stream, const SourceLocation& location, Level level)
{
    report(location, level);
    std::cout << stream.str();
    errors().add(GenericError());
    throw GenericError();
}

template <class Level, class Error>
void handle(ErrorStream& stream, const SourceLocation& location, Level level, const Error& error)
{
    report(location, level, error);
    std::cout << stream.str();
    errors().add(GenericError::from_error(error));
    throw Error(error);
}

template <class Level>
void handle(ErrorStream& stream, const SourceLocation& location, Level level, error_code_t code, module_id_t module)
{
    report(location, level, code, module);
    std::cout << stream.str();
    errors().add(GenericError(module, code));
    throw GenericError(module, code);
}

void terminate()
{
    std::cout << "TERMINATE" << std::endl;
}
} // namespace eh