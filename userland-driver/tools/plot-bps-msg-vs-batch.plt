
set terminal pdf enhanced color fontscale 1
set termoption noenhanced
set output "graph/graph-bps_".direct."_walk-".walk."_msg-vs-batch.pdf"

set key top left
set xlabel "message size"
set ylabel "throughput (byte/sec)"

set yrange [0:]
plot	"dat/".direct."_bps_walk-".walk."_msg_vs_batch.dat"	\
	using 0:2:xtic(1) with lp title "batch=1",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_batch.dat"	\
	using 0:3 with lp title "batch=4",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_batch.dat"	\
	using 0:4 with lp title "batch=8",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_batch.dat"	\
	using 0:5 with lp title "batch=16",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_batch.dat"	\
	using 0:6 with lp title "batch=32"	\

