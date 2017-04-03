set datafile sep ","
set grid
set nokey
set size ratio -1

# start value for H
#h1 = 117/360.0
h1 = 0.0
# end value for H
h2 = 227/360.0
set palette model HSV functions (1-gray)*(h2-h1)+h1,1,1

set terminal pngcairo size 1400,1400 enhanced font "FantasqueSansMono Nerd Font,20"
plot 	"/tmp/center_display.csv" 	using 1:2:3 w lines lw 5 lc palette, \
		"/tmp/right_display.csv" 	using 1:2:3 w lines lw 5 lc palette, \
		"/tmp/left_display.csv" 	using 1:2:3 w lines lw 5 lc palette, \
		"/tmp/points_display.csv" 	lc rgb "#0000EE" pt 7 ps 3

#plot 	"/tmp/center_display.csv" 	with lines lw 5 lt rgb "#000000", \
#		"/tmp/right_display.csv" 	using 1:2:3 w lines lw 5 lc palette, \
#		"/tmp/left_display.csv" 	using 1:2:3 w lines lw 5 lc palette, \
#		"/tmp/points_display.csv" 	lc rgb "#0000EE" pt 7 ps 3
