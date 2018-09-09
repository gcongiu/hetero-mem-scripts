#!/bin/bash

export LD_LIBRARY_PATH=$HOME/Installed/lib
export OSUBENCHDIR=$HOME/benchmarks/libexec/osu-micro-benchmarks/mpi
export OSU_MEMBIND_POLICY="BIND"
export OSU_MEMBIND_RULE="STRICT"
export OSU_MEMBIND_PROC="MIGRATE"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_POLICY="BIND"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_RULE="STRICT"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_PROC="MIGRATE"

for nprocs in 64; do # run for different number of processes

export OSU_SBUF_MEMBIND_TYPE="MCDRAM"
export MPIR_CVAR_CH3_SHMEM_WIN_BIND_TYPE="MCDRAM"

echo "O/T"
echo "Number of procs = $nprocs"
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/one-sided/osu_put_mbw ;
unset OSU_SBUF_MEMBIND_TYPE
unset MPIR_CVAR_CH3_SHMEM_WIN_BIND_TYPE
###################################################################

done
