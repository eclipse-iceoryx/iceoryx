#pragma once

// ***
// *** TO BE IMPLEMENTED BY CLIENT - part of any module
// ***

// Each module must define its errors and afterwards provide the reporting API it intends to use.
// This header is to be included in every file of the module that relies on error reporting.

#include "errors.hpp"

#include "iceoryx_hoofs/error_reporting/platform/error_reporting.hpp"

#include "iceoryx_hoofs/error_reporting/api.hpp"