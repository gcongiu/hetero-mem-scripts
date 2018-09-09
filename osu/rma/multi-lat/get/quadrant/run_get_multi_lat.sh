#!/bin/bash

export LD_LIBRARY_PATH=$HOME/Installed/lib
export OSUBENCHDIR=$HOME/benchmarks/libexec/osu-micro-benchmarks/mpi
export OSU_MEMBIND_POLICY="BIND"
export OSU_MEMBIND_RULE="STRICT"
export OSU_MEMBIND_PROC="MIGRATE"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_POLICY="BIND"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_RULE="STRICT"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_PROC="MIGRATE"

for nprocs in 2 4 8 16 32 64; do # run for different number of processes

echo "o/t"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun -bind-to=core -n $nprocs $OSUBENCHDIR/one-sided/osu_get_multi_lat ;
done
###################################################################

export OSU_RBUF_MEMBIND_TYPE="MCDRAM" # enable user buffer binding to MCDRAM

echo "O/t"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun -bind-to=core -n $nprocs $OSUBENCHDIR/one-sided/osu_get_multi_lat ;
done

unset OSU_RBUF_MEMBIND_TYPE
###################################################################

export MPIR_CVAR_CH3_SHMEM_WIN_BIND_TYPE="MCDRAM"

echo "o/T"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun -bind-to=core -n $nprocs $OSUBENCHDIR/one-sided/osu_get_multi_lat ;
done

unset OSU_RBUF_MEMBIND_TYPE
###################################################################

export OSU_RBUF_MEMBIND_TYPE="MCDRAM"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_TYPE="MCDRAM"

echo "O/T"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun -bind-to=core -n $nprocs $OSUBENCHDIR/one-sided/osu_get_multi_lat ;
done
unset OSU_RBUF_MEMBIND_TYPE
unset MPIR_CVAR_CH3_SHMEM_WIN_BIND_TYPE
###################################################################

done
