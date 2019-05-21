DOCKER_NAME := $(ARCH)_$(WIFISIGNALSCAN_NAME)

IMAGE_NAME := $(DOCKER_HUB_ID)/$(DOCKER_NAME):$(WIFISIGNALSCAN_VERSION)

default: all

all: build run

build:
	docker build -f Dockerfile.$(ARCH) -t $(IMAGE_NAME) .

cli:
	docker run -it --privileged --net=host --volume /usr/sbin/rfkill:/usr/sbin/rfkill --volume /sbin/ifconfig:/sbin/ifconfig --volume /sbin/iwconfig:/sbin/iwconfig --volume /sbin/iwlist:/sbin/iwlist --volume /lib/arm-linux-gnueabihf/libiw.so.30:/lib/arm-linux-gnueabihf/libiw.so.30 --device /proc/net/wireless:/proc/net/wireless /bin/bash

run: 
	-docker rm -f $(DOCKER_NAME) 2> /dev/null || :
	docker run --rm -d --privileged --net=host --volume /usr/sbin/rfkill:/usr/sbin/rfkill --volume /sbin/ifconfig:/sbin/ifconfig --volume /sbin/iwconfig:/sbin/iwconfig --volume /sbin/iwlist:/sbin/iwlist --volume /lib/arm-linux-gnueabihf/libiw.so.30:/lib/arm-linux-gnueabihf/libiw.so.30 --device /proc/net/wireless:/proc/net/wireless $(IMAGE_NAME)

docker-push:
	docker push $(IMAGE_NAME)

check:
	docker logs -f $(DOCKER_NAME) 
