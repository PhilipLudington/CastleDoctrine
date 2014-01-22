set yrange [1:4500]
set xrange [1:2000]

set xlabel "Tile Cost"

set ylabel "Bypass Cost"


load "gnuplotBypassCostLabels.txt"


plot "bypassCost.txt" using 2:3 with points lt 2 pt 5 ps 2, 10 * x with lines lt 3 t "Wall Curve", 2 * x with lines lt 4 t "Bottleneck Curve", 80 * x**.5 with lines lt 5 t "Non-linear Wall Curve", 40 * x**.5 with lines lt 6 t "Non-linear Bottleneck Curve"


pause -1