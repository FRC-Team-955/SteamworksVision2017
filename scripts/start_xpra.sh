#!/bin/bash
xpra start :80
#./setexposure 30
DISPLAY=:80 ./../release/RS_Vision ../Settings.json
