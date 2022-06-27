#pragma once

// 1) actually these would be from different modules, but
// they can be used together in e.g. tests or if one module
// is used by the other
// 2) there is only one active platform that determines the
//    reaction

#include "codes_A.hpp"
#include "codes_B.hpp"

// we can in principle have codes from multiple module in a configuration
// all these codes can be used in a type-safe way in the compilation unit