#!/bin/bash

# compile the code
make clean
make seq-rb
make mt-rb
make dist-rb
make hybrid-rb

# file for outputs
output2="output2.txt"
rm $output2

num_iters=30
gridsize1=8080
END=3

echo "grid size: 22400" >> $output2
# run each test for three times
echo "seq-rb" >> $output2
for i in {seq 1 $END}
do
    ./seq-rb $gridsize1 $num_iters >> $output2
done

echo "mt-rb 2 threads" >> $output2
for i in {seq 1 $END}
do
    ./mt-rb $gridsize1 $num_iters 2 >> $output2
done

echo "mt-rb 4 threads" >> $output2
for i in {seq 1 $END}
do
    ./mt-rb $gridsize1 $num_iters 4 >> $output2
done

echo "mt-rb 8 threads" >> $output2
for i in {seq 1 $END}
do
    ./mt-rb $gridsize1 $num_iters 8 >> $output2
done

echo "dist-rb 2 ranks" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./dist-rb $gridsize1 $num_iters >> $output2
done

echo "dist-rb 4 ranks" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./dist-rb $gridsize1 $num_iters >> $output2
done

echo "dist-rb 8 ranks" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./dist-rb $gridsize1 $num_iters >> $output2
done

echo "hybrid-rb 2 ranks 2 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 2 >> $output2
done

echo "hybrid-rb 2 ranks 4 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 4 >> $output2
done

echo "hybrid-rb 2 ranks 8 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 2 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 8 >> $output2
done

echo "hybrid-rb 4 ranks 2 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 2 >> $output2
done

echo "hybrid-rb 4 ranks 4 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 4 >> $output2
done

echo "hybrid-rb 4 ranks 8 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 4 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 8 >> $output2
done

echo "hybrid-rb 8 ranks 2 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 2 >> $output2
done

echo "hybrid-rb 8 ranks 4 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 4 >> $output2
done

echo "hybrid-rb 8 ranks 8 threads" >> $output2
for i in {seq 1 $END}
do
    mpirun -np 8 --hostfile hostfile ./hybrid-rb $gridsize1 $num_iters 8 >> $output2
done
