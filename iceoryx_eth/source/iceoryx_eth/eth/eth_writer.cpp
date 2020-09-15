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

#include "eth/eth_writer.hpp"
#include "eth/eth_context.hpp"
#include "internal/log/logging.hpp"

//#include <Mempool_DCPS.hpp>
#include <string>
extern "C"{
    #include "eth/ipc-shm.h"
}
#include <vector>

int32_t connect_to_server(int *cfd, struct sockaddr *serv_addr){

    if(connect(*cfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0){
        close(*cfd);
        perror("Connect failed : ");
        return -1;
    }
    return 0;
}

iox::eth::ethDataWriter::ethDataWriter(IdString serviceId, IdString instanceId, IdString eventId)
    : m_serviceId(serviceId)
    , m_instanceId(instanceId)
    , m_eventId(eventId)
{
    LogDebug() << "[ethDataWriter] Created ethDataWriter.";
}

iox::eth::ethDataWriter::~ethDataWriter()
{

}

void iox::eth::ethDataWriter::connect() noexcept
{
   std::cout << "Testing the connect" << std::endl ;  
   unique_code = 0;
}

void iox::eth::ethDataWriter::write(const uint8_t* const bytes, const uint64_t size) noexcept
{

    std::cout << "Testing the write" << std::endl ;  
    
    client_handle = socket(AF_INET, SOCK_STREAM, 0);
    if(client_handle < 0){
        /* error handling */
        std::cout << "\n Socket creation error \n" << std::endl; 
    } 
    else{
        std::cout << "\nclient handle : " << client_handle << std::endl;
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(PORT);

        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0){ 
            
            std::cout << "\nInvalid address/ Address not supported \n" << std::endl; 
            client_handle = -1;
        }
        else{
            std::cout << "\ninet_pton done \n";
            if(connect_to_server(&client_handle, (struct sockaddr *)&serv_addr) < 0){ 
                
                std::cout << "\nConnection Failed \n" << std::endl;
                client_handle = -1; 
            }
            else{
                std::cout << "Connected to server!!" << std::endl;
            }
        }
    }

    if(client_handle >= 0){
        std::cout << "client handle : " << client_handle << std::endl;

        // GatewayWrapper is the packet that will be sent over Eth, (Header + Payload)
        std::vector<uint8_t> gatewayWrapper;

        gatewayWrapper.push_back(unique_code);

        uint8_t size_array[sizeof(size)/sizeof(uint8_t)];
        (void) memcpy(size_array, &size, sizeof(size));

        for(uint8_t k = 0u; k < sizeof(size_array); ++k){
            gatewayWrapper.push_back(size_array[k]);
        }

        for(uint8_t k = 0u; k < size; ++k){
            gatewayWrapper.push_back(*(bytes + k));
        }
        printf("Size : %d, Data : ",gatewayWrapper.size());
        for(auto v : gatewayWrapper){
            printf("%u, ",v);
        }
        std::cout << std::endl;
        
        
        if(-1 != send(client_handle, (std::vector<uint8_t>*)&gatewayWrapper[0], gatewayWrapper.size() , 0 )){
            
            // std::cout << "Sent Payload: " <<gw_st.Size << " bytes, packet: " << sizeof(gw_st) << " bytes successfully!!" << std::endl;

            // ssize_t send_status = send(client_handle, (const void*) bytes , size , 0 ); 
            // if(send_status != -1){
                // ssize_t ack = read( client_handle , buffer, 1024);

            //     // if(ack != -1){
            //     //     std::cout << buffer << std::endl;
            //     // }
            //     // else{
            //     //     std::cout << "ack is -1 !!" << std::endl;        
            //     // }
            //     std::cout << "Send success!!" << std::endl;
            // }
            // else{
            //     std::cout << "Send failure!!" << std::endl;
            // } 
        }
        else{
            std::cout << "Send data failed!" << std::endl;
        }
        
        if(close(client_handle) == -1){
            perror("socket close failed : ");
        }

    }
    else{
        std::cout << "client handle is -1 !!" << std::endl;
    }

}
iox::eth::IdString iox::eth::ethDataWriter::getServiceId() const noexcept
{
    return m_serviceId;
}

iox::eth::IdString iox::eth::ethDataWriter::getInstanceId() const noexcept
{
    return m_instanceId;
}

iox::eth::IdString iox::eth::ethDataWriter::getEventId() const noexcept
{
    return m_eventId;
}

