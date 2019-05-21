#!/bin/sh

#rfkill unblock all

getWifiNetworks() {
    iwlist wlan0 scan 2>&1 | egrep "ESSID|Signal level" | tr '\n' ' ' | sed 's/\s*Quality/\nQuality/g;s/\s*ESSID/ ESSID/g'

    echo "\n"

}

WIFI=$(getWifiNetworks)

HEADERS="Content-Type: text/html; charset=ISO-8859-1"
BODY="{${WIFI}}"
#echo $BODY

#HTTP="HTTP/1.1 200 OK\r\n${HEADERS}\r\n\r\n${BODY}\r\n"
HTTP="${BODY}\n"

# Emit the HTTP response
echo -en $HTTP

