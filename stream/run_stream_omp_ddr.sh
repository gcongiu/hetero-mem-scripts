#!/bin/bash
#SBATCH --job-name=stream_omp_ddr
#SBATCH -o stream_omp_ddr.%j.%N.out
#SBATCH -e stream_omp_ddr.%j.%N.error
#SBATCH -N 1
#SBATCH -p knlall
#SBATCH -A RADIX
#SBATCH -C knl,quad,flat
#SBATCH --ntasks-per-node=64
#SBATCH --time=30:00
#SBATCH --mail-user=gcongiu@anl.gov

for num_threads in 1 2 4 8 16 32 64; do
export OMP_NUM_THREADS=$num_threads

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((32768*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((65536*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((131072*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((262144*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((524288*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((1048576*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((2097152*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done

icc stream_omp.c -o stream_omp_ddr -g -O3 -fopenmp -mcmodel large -shared-intel -DSTREAM_ARRAY_SIZE=$((4194304*num_threads)) 
for((i=0;i<10;i++)); do
./stream_omp_ddr | egrep "(Array size|Copy|Triad|Add|Scale|Function|Number of Threads)"
done
done
