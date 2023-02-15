#ifndef IOX_HOOFS_ERROR_REPORTING_LOCATION_HPP
#define IOX_HOOFS_ERROR_REPORTING_LOCATION_HPP

namespace iox
{
namespace err
{
struct SourceLocation
{
    const char* file{nullptr};
    int line{0};
    const char* function{nullptr};
};

} // namespace err
} // namespace iox

#define CURRENT_SOURCE_LOCATION                                                                                        \
    iox::err::SourceLocation                                                                                           \
    {                                                                                                                  \
        __FILE__, __LINE__, __func__                                                                                   \
    }

#endif
