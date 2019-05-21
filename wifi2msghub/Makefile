#SYSTEM_ARCH := $(shell uname -m | sed -e 's/aarch64.*/arm64/' -e 's/x86_64.*/amd64/' -e 's/armv.*/arm/')

#JSON_C_DIR=/usr/include/install
#CFLAGS += -I$(JSON_C_DIR)/include/json-c
#LDFLAGS+= -L$(JSON_C_DIR)/lib -ljson-c

DOCKER_NAME := $(ARCH)_$(WIFI2MSGHUB_NAME)
IMAGE_NAME := $(DOCKER_HUB_ID)/$(DOCKER_NAME):$(WIFI2MSGHUB_VERSION)

all : build run


build:
	docker build -f Dockerfile.$(ARCH) -t $(IMAGE_NAME) .

run:
	-docker rm -f $(DOCKER_NAME) 2> /dev/null || :
	docker run -d --name $(DOCKER_NAME) --privileged --net=host $(IMAGE_NAME)

test1 : test1.o
	gcc -g test1.c -o test1 -lcurl -ljson-c

clean : 
	rm -f test1.o
