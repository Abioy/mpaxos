#!/bin/bash

tc qdisc add dev lo root netem delay 100ms 40ms 
