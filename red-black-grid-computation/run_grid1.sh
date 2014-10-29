#!/bin/bash

# compile the code
make clean
make seq-rb
make mt-rb
make dist-rb
make hybrid-rb

# file for outputs
output1="output1.txt"
rm $output1

num_iters=30
gridsize1=2800
END=3

echo "grid size: 2800" >> $output1
# run each test for three times
echo "seq-rb" >> $output1
for i in {seq 1 $END}
do
    ./seq-rb $gridsize1 $num_iters >> $output1
done

echo "mt-rb 2 threads" >> $output1
for i in {seq 1 $END}
do
    ./mt-rb $gridsize1 $num_iters 2 >> $output1
done

echo "mt-rb 4 threads" >> $output1
for i in {seq 1 $END}
do
    ./mt-rb $gridsize1 $num_iters 4 >> $output1
done

echo "mt-rb 8 threads" >> $output1
for i in {seq 1 $END}
do
    ./mt-rb $gridsize1 $num_iters 8 >> $output1
done

echo "dist-rb 2 ranks" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./dist-rb $gridsize1 $num_iters >> $output1
done

echo "dist-rb 4 ranks" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./dist-rb $gridsize1 $num_iters >> $output1
done

echo "dist-rb 8 ranks" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./dist-rb $gridsize1 $num_iters >> $output1
done

echo "hybrid-rb 2 ranks 2 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 2 >> $output1
done

echo "hybrid-rb 2 ranks 4 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 4 >> $output1
done

echo "hybrid-rb 2 ranks 8 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 8 >> $output1
done

echo "hybrid-rb 4 ranks 2 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 2 >> $output1
done

echo "hybrid-rb 4 ranks 4 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 4 >> $output1
done

echo "hybrid-rb 4 ranks 8 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 8 >> $output1
done

echo "hybrid-rb 8 ranks 2 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 2 >> $output1
done

echo "hybrid-rb 8 ranks 4 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 4 >> $output1
done

echo "hybrid-rb 8 ranks 8 threads" >> $output1
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 8 >> $output1
done
