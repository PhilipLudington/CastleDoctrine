perl -ane 'print "set label \"$F[0]\" at ($F[1]+30),($F[2]+30)\n"' bypassCost.txt > gnuplotBypassCostLabels.txt

gnuplot bypassCostCurve.gnuplot