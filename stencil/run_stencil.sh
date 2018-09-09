export STENCIL_MEMBIND_POLICY="BIND"
export STENCIL_MEMBIND_RULE="STRICT"
export STENCIL_MEMBIND_PROC="MIGRATE"

echo "s/r/f/c/cb" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536; do
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8 
done
done

export STENCIL_SBUF_MEMBIND_TYPE="MCDRAM"
echo "S/r/f/c/cb" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done
unset STENCIL_SBUF_MEMBIND_TYPE

export STENCIL_RBUF_MEMBIND_TYPE="MCDRAM"
echo "s/R/f/c/cb" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done

export STENCIL_SBUF_MEMBIND_TYPE="MCDRAM"
echo "S/R/f/c/cb" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done

export MPIR_CVAR_SHMEM_FBOX_BIND_TYPE="MCDRAM"
echo "S/R/F/c/cb" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done
unset MPIR_CVAR_SHMEM_FBOX_BIND_TYPE

#export MPIR_CVAR_SHMEM_CELLS_BIND_TYPE="MCDRAM"
#echo "S/R/f/C/cb" >&2
#echo "Number of procs = 64" >&2
#for((i=0;i<10;i++)); do
#for n in 2048 4096 8192 16384 32768 65536 ; do 
#mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
#done
#done

#export MPIR_CVAR_SHMEM_FBOX_BIND_TYPE="MCDRAM"
#echo "S/R/F/C/cb" >&2
#echo "Number of procs = 64" >&2
#for((i=0;i<10;i++)); do
#for n in 2048 4096 8192 16384 32768 65536 ; do 
#mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
#done
#done
#unset MPIR_CVAR_SHMEM_FBOX_BIND_TYPE
#unset MPIR_CVAR_SHMEM_CELLS_BIND_TYPE
unset STENCIL_SBUF_MEMBIND_TYPE
unset STENCIL_RBUF_MEMBIND_TYPE

export MPIR_CVAR_SHMEM_FBOX_BIND_TYPE="MCDRAM"
echo "s/r/F/c/cb" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done
unset MPIR_CVAR_SHMEM_FBOX_BIND_TYPE

export MPIR_CVAR_SHMEM_COPYBUF_BIND_TYPE="MCDRAM"
echo "s/r/f/c/CB" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done

export STENCIL_SBUF_MEMBIND_TYPE="MCDRAM"
export STENCIL_RBUF_MEMBIND_TYPE="MCDRAM"
echo "S/R/f/c/CB" >&2
echo "Number of procs = 64" >&2
for((i=0;i<10;i++)); do
for n in 2048 4096 8192 16384 32768 65536 ; do 
mpirun -bind-to=core -n 64 ./stencil $n 10 250 8 8
done
done
unset MPIR_CVAR_SHMEM_COPYBUF_BIND_TYPE
unset STENCIL_SBUF_MEMBIND_TYPE
unset STENCIL_RBUF_MEMBIND_TYPE
