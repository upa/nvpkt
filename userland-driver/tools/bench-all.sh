#!/bin/bash


OUTDIR=output
BENCH_NVME=./bench-nvme
NVMEDEV=b3:00.0

msg_sizes="64 256 512 1024 4096 1048576 2097152"
batch_sizes="1 4 8 16 32"
cpu_nums="1 4 8 12"
walk_modes="seq random same"
duration=30

for walk in $walk_modes; do
for msg in $msg_sizes; do
for batch in $batch_sizes; do
for cpu in $cpu_nums; do

	cmd="$BENCH_NVME -m write -p $NVMEDEV -t $duration \
		-w $walk -s $msg -b $batch -n $cpu"
	out=$OUTDIR/write_walk-${walk}_msg-${msg}_batch-${batch}_cpu-${cpu}.txt
	echo "===== Start ======"
	echo $cmd
	echo $out
	$cmd > $out
	echo "==== Finished ===="
	echo
	sleep 5
done
done
done
done
