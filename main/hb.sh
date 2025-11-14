#!/bin/bash
while true; do
    mosquitto_pub -h 10.210.129.196 -p 1883 -t huerta/heartbeat -m "ping"
    sleep 5
done
