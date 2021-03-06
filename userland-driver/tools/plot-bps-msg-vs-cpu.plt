
set terminal pdf enhanced color fontscale 1
set termoption noenhanced
set output "graph/graph-bps_".direct."_walk-".walk."_msg-vs-cpu.pdf"

set key top left
set xlabel "message size"
set ylabel "throughput (byte/sec)"

set yrange [0:]
plot	"dat/".direct."_bps_walk-".walk."_msg_vs_cpu.dat"	\
	using 0:2:xtic(1) with lp title "cpu=1",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_cpu.dat"	\
	using 0:3 with lp title "cpu=4",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_cpu.dat"	\
	using 0:4 with lp title "cpu=8",	\
	"dat/".direct."_bps_walk-".walk."_msg_vs_cpu.dat"	\
	using 0:5 with lp title "cpu=12"	\
