#!/bin/bash
./setexposure.sh 70
exec ../build/RS_Vision ../Settings.json | nc -lp 5805
