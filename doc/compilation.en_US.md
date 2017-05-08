# Compilation

This document assumes the readers have basic experiences of operating a Linux box.
And also the entire repo is assumed to be downloaded at `$SDK_ROOT`.

## Compilation in 3 steps

We can build a tarball of our SDK with the help of [docker](https://www.docker.com/) in 3 steps:
1. build a base image: under `$SDK_ROOT`, run
   ```sh
   $ cd docker/centos7; sudo docker build -t cpp_sdk .
   ```
1. run the image with SDK directory attached: run the following command by replacing `$SDK_ROOT` by your absolute path of `$SDK_ROOT`:
   ```sh
   $ sudo docker run -it -v $SDK_ROOT:/opt/cpp_sdk/ cpp_sdk /bin/bash
   ```
1. compilation: run in the docker container created by the previous step:
   ```sh
   # scons -j3
   ```
After these 3 steps, you will find a tarball file under `$SDK_ROOT/build/release/`.
As far as this document is written, the tarball file is named by `aliyun-tablestore-cpp98-sdk-4.3.0-centos7.3.1611-x86_64.tar.gz`.
You see, its name will change with respect to its version, the compilation operating system and CPU architecture.
This tarball contains necessary header files and library files.
Copy them to your projects or install them on your machines, then our SDK is ready to use.

## Detailed explaination of the 3 steps

### Yes, we only support centos-7 right now.

We plan to support a lot of operating systems as well as a lot of Linux distributions.
But with limited resources, we can support exactly one of them.
That's centos-7.

### Docker is not a must-have

Docker is a helpful way to manage dependencies, but it is not a must-have.
You can manually install dependencies on a centos-7 box.

Let us take a look at docker/centos7/Dockerfile (as far as this document is written):
```dockerfile
FROM centos:7
RUN yum update --assumeyes
COPY scons-2.5.1-1.noarch.rpm /opt/
RUN rpm -Uvh /opt/scons-2.5.1-1.noarch.rpm
RUN yum install --assumeyes protobuf protobuf-devel gcc gcc-c++ libcurl-devel openssl-devel libuuid-devel
WORKDIR /opt/cpp_sdk/
```
You can see, we depends on `protobuf`, `gcc`, `libcurl`, `openssl`, 'libuuid` and `scons`.
Most of them, except `scons`, can be installed by `yum`.
And `scons`, which is not found in `yum`, must be installed by a downloaded rpm ball.

### By default, it is release mode

In release mode, it is compiled with `-pthread`, `-fPIC`, `-g` and `-O2`.

Besides release mode, there is also debug mode.
The difference between these two modes is: debug mode use `-O0` instead of `-O2`.
To compile in debug mode, you have to invoke `scons` with `mode=debug`.
```sh
# scons mode=debug -j3
```
