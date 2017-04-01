cd ../release
#make -s
#./RS_Vision ./recordings
gnuplot -p ../scripts/plotsettings.plot > /tmp/vision.png
feh /tmp/vision.png
