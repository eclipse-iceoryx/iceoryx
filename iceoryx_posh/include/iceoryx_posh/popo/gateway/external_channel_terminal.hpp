#ifndef IOX_POSH_EXTERNAL_CHANNEL_TERMINAL_HPP
#define IOX_POSH_EXTERNAL_CHANNEL_TERMINAL_HPP

#include "iceoryx_utils/cxx/string.hpp"

class ExternalChannelTerminal{
public:
    using IdString = iox::cxx::string<100>;
    ExternalChannelTerminal& create(const IdString serviceId, const IdString instanceId, const IdString eventId);
};

#endif // IOX_POSH_EXTERNAL_CHANNEL_TERMINAL_HPP
