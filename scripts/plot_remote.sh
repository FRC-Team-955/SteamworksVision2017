scp -P 1180 ubuntu@tegra-ubuntu.local:/tmp/*_display.csv /tmp/
gnuplot -p ../scripts/plotsettings.plot > /tmp/vision.png
feh /tmp/vision.png
