# this is a docker specalized ubuntu
FROM phusion/baseimage:noble-1.0.2

WORKDIR /home/ubuntu

RUN mkdir build
WORKDIR /home/ubuntu/build

RUN apt update
RUN apt install apt-file -y
RUN apt update
RUN apt install cmake git g++ --fix-missing -y

COPY ./src /home/ubuntu/src
COPY ./CMakeLists.txt /home/ubuntu/CMakeLists.txt

RUN cmake -DCMAKE_BUILD_TYPE=DEBUG ..
RUN cmake --build .

ENTRYPOINT ["./ksp-controller"]
