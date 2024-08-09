# this dockerfile is only for test purposes since this Program needs to access GPIO and is
# therefore not made to run inside a container

# this is a docker specalized ubuntu
FROM phusion/baseimage:noble-1.0.0

RUN apt update
RUN apt install apt-file -y
RUN apt update
RUN apt install libasio-dev protobuf-compiler cmake wget unzip g++ git sudo -y

WORKDIR /home/ubuntu

RUN wget https://github.com/krpc/krpc/releases/download/v0.5.4/krpc-cpp-0.5.4.zip -O krpc-download
RUN unzip krpc-download

RUN mv /home/ubuntu/krpc-cpp-0.5.4 /home/ubuntu/krpc
RUN rm -rf /home/ubuntu/krpc-download
WORKDIR /home/ubuntu/krpc

RUN cmake .
RUN make
RUN make install
RUN ldconfig

WORKDIR /home/ubuntu

RUN mkdir ksp-controller

COPY ./src /home/ubuntu/ksp-controller/src

WORKDIR /home/ubuntu/ksp-controller

RUN g++ -o ksp-controller src/main.cpp -std=c++11 -lkrpc -lprotobuf -O3 -D"KSP_CONTROLLER_DEV_MODE=true"
ENTRYPOINT ["./ksp-controller"]
