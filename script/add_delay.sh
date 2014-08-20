#!/bin/bash

tc qdisc add dev lo root netem delay 50ms 5ms 
