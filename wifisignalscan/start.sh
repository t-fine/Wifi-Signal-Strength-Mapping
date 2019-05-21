#!/bin/sh

socat TCP4-LISTEN:34567,fork EXEC:./service.sh
