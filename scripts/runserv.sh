#!/bin/bash
./setexposure.sh 30
exec ../build/RS_Vision ../Settings.json | nc -lp 5805
