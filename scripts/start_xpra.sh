#!/bin/bash
xpra start :80
DISPLAY=:80 ./../release/RS_Vision ../Settings.json
