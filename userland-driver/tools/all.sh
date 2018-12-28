


./parser.py -y bps -x msg -v cpu \
	output/bench_walk-seq_msg-*_batch-1_cpu-*.txt	\
	> dat/read_bps_walk-seq_msg_vs_cpu.dat

./parser.py -y bps -x msg -v batch \
	output/bench_walk-seq_msg-*_batch-*_cpu-1.txt	\
	> dat/read_bps_walk-seq_msg_vs_batch.dat

./parser.py -y bps -x msg -v cpu \
	output/bench_walk-random_msg-*_batch-1_cpu-*.txt	\
	> dat/read_bps_walk-random_msg_vs_cpu.dat

./parser.py -y bps -x msg -v batch \
	output/bench_walk-random_msg-*_batch-*_cpu-1.txt	\
	> dat/read_bps_walk-random_msg_vs_batch.dat

gnuplot -e "walk='seq'; direct='read';" plot-bps-msg-vs-cpu.plt
gnuplot -e "walk='random'; direct='read';" plot-bps-msg-vs-cpu.plt

gnuplot -e "walk='seq'; direct='read';" plot-bps-msg-vs-batch.plt
gnuplot -e "walk='random'; direct='read';" plot-bps-msg-vs-batch.plt


./parser.py -y bps -x msg -v cpu \
	output/write_walk-seq_msg-*_batch-1_cpu-*.txt	\
	> dat/write_bps_walk-seq_msg_vs_cpu.dat

./parser.py -y bps -x msg -v batch \
	output/write_walk-seq_msg-*_batch-*_cpu-1.txt	\
	> dat/write_bps_walk-seq_msg_vs_batch.dat

./parser.py -y bps -x msg -v cpu \
	output/write_walk-random_msg-*_batch-1_cpu-*.txt	\
	> dat/write_bps_walk-random_msg_vs_cpu.dat

./parser.py -y bps -x msg -v batch \
	output/write_walk-random_msg-*_batch-*_cpu-1.txt	\
	> dat/write_bps_walk-random_msg_vs_batch.dat

gnuplot -e "walk='seq'; direct='write';" plot-bps-msg-vs-cpu.plt
gnuplot -e "walk='random'; direct='write';" plot-bps-msg-vs-cpu.plt

gnuplot -e "walk='seq'; direct='write';" plot-bps-msg-vs-batch.plt
gnuplot -e "walk='random'; direct='write';" plot-bps-msg-vs-batch.plt



./parser.py -y iops -x msg -v cpu \
	output/bench_walk-seq_msg-*_batch-1_cpu-*.txt	\
	> dat/read_iops_walk-seq_msg_vs_cpu.dat

./parser.py -y iops -x msg -v batch \
	output/bench_walk-seq_msg-*_batch-*_cpu-1.txt	\
	> dat/read_iops_walk-seq_msg_vs_batch.dat

./parser.py -y iops -x msg -v cpu \
	output/bench_walk-random_msg-*_batch-1_cpu-*.txt	\
	> dat/read_iops_walk-random_msg_vs_cpu.dat

./parser.py -y iops -x msg -v batch \
	output/bench_walk-random_msg-*_batch-*_cpu-1.txt	\
	> dat/read_iops_walk-random_msg_vs_batch.dat

gnuplot -e "walk='seq'; direct='read';" plot-iops-msg-vs-cpu.plt
gnuplot -e "walk='random'; direct='read';" plot-iops-msg-vs-cpu.plt

gnuplot -e "walk='seq'; direct='read';" plot-iops-msg-vs-batch.plt
gnuplot -e "walk='random'; direct='read';" plot-iops-msg-vs-batch.plt


./parser.py -y iops -x msg -v cpu \
	output/write_walk-seq_msg-*_batch-1_cpu-*.txt	\
	> dat/write_iops_walk-seq_msg_vs_cpu.dat

./parser.py -y iops -x msg -v batch \
	output/write_walk-seq_msg-*_batch-*_cpu-1.txt	\
	> dat/write_iops_walk-seq_msg_vs_batch.dat

./parser.py -y iops -x msg -v cpu \
	output/write_walk-random_msg-*_batch-1_cpu-*.txt	\
	> dat/write_iops_walk-random_msg_vs_cpu.dat

./parser.py -y iops -x msg -v batch \
	output/write_walk-random_msg-*_batch-*_cpu-1.txt	\
	> dat/write_iops_walk-random_msg_vs_batch.dat

gnuplot -e "walk='seq'; direct='write';" plot-iops-msg-vs-cpu.plt
gnuplot -e "walk='random'; direct='write';" plot-iops-msg-vs-cpu.plt

gnuplot -e "walk='seq'; direct='write';" plot-iops-msg-vs-batch.plt
gnuplot -e "walk='random'; direct='write';" plot-iops-msg-vs-batch.plt


