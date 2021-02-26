# Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
# Copyright (c) 2019 by Cornel Izbasa. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0


# ======================================================================================================================== #

FROM ubuntu:bionic as builder
ARG REVISION
ARG B_ICEORYX_BUILD
ENV ICEORYX_BUILD=$B_ICEORYX_BUILD

# Dependencies
RUN apt-get update && apt-get install -y \
        cmake \
        openjdk-11-jdk \
        maven \
        libacl1-dev \
        libncurses5-dev \
        pkg-config \
        git \
        wget

## Install the latest CMake to take advantage of parallel builds
RUN wget --no-check-certificate -P /root/ https://github.com/Kitware/CMake/releases/download/v3.16.3/cmake-3.16.3-Linux-x86_64.tar.gz && \
  cd /root/ && \
  tar xf ./cmake-3.16.3-Linux-x86_64.tar.gz
ENV PATH="/root/cmake-3.16.3-Linux-x86_64/bin:${PATH}"

# Configuration
## Maven Proxy - required for usage from behind a corporate proxy. Corporate proxy should be bound to the docker network bridge at 172.17.0.1.
#RUN mkdir -p /root/.m2
#RUN echo '\n\
#<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" \n\
  #xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" \n\
  #xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 \n\
                  #http://maven.apache.org/xsd/settings-1.0.0.xsd"> \n\
  #<proxies> \n\
    #<proxy> \n\
      #<active>true</active> \n\
      #<protocol>http</protocol> \n\
      #<host>172.17.0.1</host> \n\
      #<port>3128</port> \n\
      #<nonProxyHosts>127.0.0.1</nonProxyHosts> \n\
    #</proxy> \n\
  #</proxies> \n\
#</settings> \n'\
#>> /root/.m2/settings.xml

ADD . /iceoryx
WORKDIR /iceoryx

RUN ./tools/iceoryx_build_test.sh build-all

# ======================================================================================================================== #

FROM ubuntu:bionic as roudi

RUN apt-get update && apt-get install -y \
        screen \
        fish 

COPY --from=builder /iceoryx/build/install/prefix/lib/lib* /usr/lib/x86_64-linux-gnu/
COPY --from=builder /iceoryx/build/dependencies/install/lib/lib* /usr/lib/x86_64-linux-gnu/
COPY --from=builder /iceoryx/build/install/prefix/bin/* /usr/bin/
COPY --from=builder /iceoryx/build/install/prefix/etc/* /etc/
COPY --from=builder /iceoryx/tools/docker/.screenrc /root

ENTRYPOINT ["/usr/bin/iox-roudi"]
