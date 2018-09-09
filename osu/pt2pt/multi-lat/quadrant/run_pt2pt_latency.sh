export LD_LIBRARY_PATH=$HOME/Installed/lib
export OSUBENCHDIR=$HOME/benchmarks/libexec/osu-micro-benchmarks/mpi
export OSU_MEMBIND_POLICY="BIND"
export OSU_MEMBIND_RULE="STRICT"
export OSU_MEMBIND_PROC="MIGRATE"

for nprocs in 64; do
echo "s/r/f/c/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done
###################################################################

export OSU_SBUF_MEMBIND_TYPE="MCDRAM" # enable user buffer binding to MCDRAM

echo "S/r/f/c/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done

unset OSU_SBUF_MEMBIND_TYPE
###################################################################

export OSU_RBUF_MEMBIND_TYPE="MCDRAM"

echo "s/R/f/c/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done

unset OSU_RBUF_MEMBIND_TYPE
###################################################################

export OSU_SBUF_MEMBIND_TYPE="MCDRAM"
export OSU_RBUF_MEMBIND_TYPE="MCDRAM"

echo "S/R/f/c/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done
###################################################################

export MPIR_CVAR_SHMEM_FBOX_BIND_TYPE="MCDRAM"

echo "S/R/F/c/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done

unset MPIR_CVAR_SHMEM_FBOX_BIND_TYPE
###################################################################

export MPIR_CVAR_SHMEM_CELLS_BIND_TYPE="MCDRAM"

echo "S/R/f/C/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done
###################################################################

export MPIR_CVAR_SHMEM_FBOX_BIND_TYPE="MCDRAM"

echo "S/R/F/C/cb"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done

unset MPIR_CVAR_SHMEM_FBOX_BIND_TYPE
unset MPIR_CVAR_SHMEM_CELLS_BIND_TYPE
unset OSU_SBUF_MEMBIND_TYPE
unset OSU_RBUF_MEMBIND_TYPE
###################################################################

export MPIR_CVAR_SHMEM_COPYBUF_BIND_TYPE="MCDRAM"

echo "s/r/f/c/CB"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done
###################################################################

export OSU_SBUF_MEMBIND_TYPE="MCDRAM"

echo "S/r/f/c/CB"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done

unset OSU_SBUF_MEMBIND_TYPE
###################################################################

export OSU_RBUF_MEMBIND_TYPE="MCDRAM"

echo "S/R/f/c/CB"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done
###################################################################

export OSU_SBUF_MEMBIND_TYPE="MCDRAM"

echo "S/R/f/c/CB"
echo "Number of procs = $nprocs"
for((i=0;i<10;i++)); do
mpirun --bind-to=core -n $nprocs $OSUBENCHDIR/pt2pt/osu_multi_lat ;
done

unset MPIR_CVAR_SHMEM_COPYBUF_BIND_TYPE
unset OSU_SBUF_MEMBIND_TYPE
unset OSU_RBUF_MEMBIND_TYPE
done
