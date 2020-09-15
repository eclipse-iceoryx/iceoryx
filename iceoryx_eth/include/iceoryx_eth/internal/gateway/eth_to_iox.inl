// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_eth_INTERNAL_GATEWAY_eth_TO_IOX_INL
#define IOX_eth_INTERNAL_GATEWAY_eth_TO_IOX_INL

#include "iceoryx_eth/eth/eth_config.hpp"
#include "iceoryx_eth/internal/log/logging.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/string.hpp"

/* TCP headers */
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#define PORT 8080
#define LONGVAL 1024 * 1024

namespace iox
{
namespace eth
{
template <typename channel_t, typename gateway_t>
inline eth2IceoryxGateway<channel_t, gateway_t>::eth2IceoryxGateway() noexcept
    : gateway_t(iox::capro::Interfaces::DDS, DISCOVERY_PERIOD, FORWARDING_PERIOD)
{
}
template <typename channel_t, typename gateway_t>
inline eth2IceoryxGateway<channel_t, gateway_t>::~eth2IceoryxGateway() noexcept
{}


template <typename channel_t, typename gateway_t>
inline void eth2IceoryxGateway<channel_t, gateway_t>::loadConfiguration(const iox::config::GatewayConfig& config) noexcept
{
    iox::LogDebug() << "[eth2IceoryxGateway] Configuring gateway...";
    for (const auto& service : config.m_configuredServices)
    {
        if (!this->findChannel(service.m_serviceDescription).has_value())
        {
            auto serviceDescription =  service.m_serviceDescription;
            iox::LogDebug() << "[eth2IceoryxGateway] Setting up channel for service: {"
                            << serviceDescription.getServiceIDString() << ", "
                            << serviceDescription.getInstanceIDString() << ", "
                            << serviceDescription.getEventIDString() << "}";
            setupChannel(serviceDescription);
        }
    }
}

// /* loading PMap config file */
template <typename channel_t, typename gateway_t>
inline void eth2IceoryxGateway<channel_t, gateway_t>::loadConfigurationEth(const std::vector<iox::popo::EthGatewayConf>& config) noexcept
{
    iox::LogDebug() << "[eth2IceoryxGateway] Configuring gateway...";
    for (const auto& service : config)
    {
        if (!this->findChannel(service.m_serviceDescription).has_value())
        {
            auto serviceDescription =  service.m_serviceDescription;
            iox::LogDebug() << "[Eth2IceoryxGateway] Setting up channel for service: {"
                            << serviceDescription.getServiceIDString() << ", "
                            << serviceDescription.getInstanceIDString() << ", "
                            << serviceDescription.getEventIDString() << "}";
            setupChannel(serviceDescription);
        }
    }
    forwardLocal();
}

template <typename channel_t, typename gateway_t>
inline void
eth2IceoryxGateway<channel_t, gateway_t>::discover([[gnu::unused]] const iox::capro::CaproMessage& msg) noexcept
{
    /// @note not implemented - requires eth discovery which is currently not implemented in the used eth stack.
}

template <typename channel_t, typename gateway_t>
inline void eth2IceoryxGateway<channel_t, gateway_t>::forward(const channel_t& channel) noexcept
{
    // auto publisher = channel.getIceoryxTerminal();
    // auto reader = channel.getExternalTerminal();

    // reader->peekNextSize().and_then([&](uint64_t size) {
    //     // reserve a chunk for the sample
    //     m_reservedChunk = publisher->allocateChunk(static_cast<uint32_t>(size));
    //     // read sample into reserved chunk
    //     auto buffer = static_cast<uint8_t*>(m_reservedChunk);
    //     reader->takeNext(buffer, size)
    //         .and_then([&]() {
    //             // publish chunk
    //             publisher->sendChunk(buffer);
    //         })
    //         .or_else([&](DataReaderError err) {
    //             LogWarn() << "[eth2IceoryxGateway] Encountered error reading from eth network: "
    //                       << iox::eth::DataReaderErrorString[static_cast<uint8_t>(err)];
    //         });
    // });

}

template <typename channel_t, typename gateway_t>
inline void eth2IceoryxGateway<channel_t, gateway_t>::forwardLocal() noexcept
{
    uint8_t code{};
    uint64_t length{};
    int server_fd, new_socket, valread, count = 0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    uint8_t data_long[LONGVAL];

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){

        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){

        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else{
        printf("bind done\n");
    }
    
    /* starts listening for incoming connections */
    while(1){

        printf("Iter : %d\n", ++count);
        if (listen(server_fd, 3) < 0)
        {
            printf("listen fail");
            exit(EXIT_FAILURE);
        }
        else{
            printf("listening...\n");
        }

        /* receive control information */
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            printf("accept fail");
            exit(EXIT_FAILURE);
        }

        int sizeread = 0;
        uint64_t total = 0;

        valread = read(new_socket, (uint8_t *)&code, sizeof(code));
        sizeread += valread;

        valread = read(new_socket, (uint64_t *)&length, sizeof(length));
        sizeread += valread;

        while(valread != 0){

            valread = read(new_socket, (uint8_t *)(data_long + total), length - total);
            total += valread;
        }
        for(uint64_t k=0; k < length; ++k)
            printf("%u, ",data_long[k]);
        printf("\n");
        printf("Header : %d bytes, Total : %ld bytes\n\n\n",sizeread, total);

        /* data_long has the serialized data */
        /* do something */
        this->forEachChannel([this,&code, &length, &data_long](channel_t channel) { 
            auto reader = channel.getExternalTerminal();
            if(reader->getUniqueCode() == code){
                /* it's a match for publisher */
                std::cout << "\nMatch for published!!\n";
                auto publisher = channel.getIceoryxTerminal();
                auto publisherData = publisher->allocateChunk(length);
                memcpy(publisherData,data_long, length);
                publisher->sendChunk(publisherData);
            }
        });

        //     forEachChannel([this](channel_t channel) { 
        //         if(channel.getexternalterminal().getuniquecode() == unicode)
        //     {  publisher = channel.geticeterminal()
        //     ptr = publisher.allocate(size)
        //     mecpy(ptrfromremote,ptr);
        //     publisher.publisher();
        //      })

        /* clear buffer for next cycle after publishing it*/
        (void) memset(data_long, 0x00, sizeof(data_long));
        code = 0u;
        length = 0u;
    }
}


// ======================================== Private ======================================== //
template <typename channel_t, typename gateway_t>
iox::cxx::expected<channel_t, iox::gw::GatewayError>
eth2IceoryxGateway<channel_t, gateway_t>::setupChannel(const iox::capro::ServiceDescription& service) noexcept
{
    return this->addChannel(service).and_then([&service](channel_t channel) {
        auto publisher = channel.getIceoryxTerminal();
        auto reader = channel.getExternalTerminal();
        publisher->offer();
        reader->setUniqueCode(service);
        reader->connect();
        iox::LogDebug() << "[eth2IceoryxGateway] Setup channel for service: {" << service.getServiceIDString() << ", "
                        << service.getInstanceIDString() << ", " << service.getEventIDString() << "}";
    });
}

} // namespace eth
} // namespace iox

#endif
