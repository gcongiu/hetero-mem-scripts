/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * See COPYRIGHT in top-level directory.
 */

/*
 * 2D stencil code using a nonblocking send/receive with manual packing/unpacking.
 *
 * 2D regular grid is divided into px * py blocks of grid points (px * py = # of processes.)
 * In every iteration, each process calls nonblocking operations to exchange a halo with
 * neighbors. Grid points in a halo are packed and unpacked before and after communications.
 */

#include "stencil_par.h"
#include <hwloc.h>
#include <assert.h>
#include <sys/mman.h>

/* row-major order */
#define ind(i,j) ((j)*(bx+2)+(i))

static int initialized = 0;
static hwloc_topology_t topology;
static hwloc_nodeset_t mcdram_nodes;
static int bufsize, halo_x_size, halo_y_size;

long long ind_f(int i, int j, int bx)
{
    return ind(i, j);
}

void init_mcdram();
void fini_mcdram();
int  alloc_mcdram_mem(void **buf, size_t size);
void free_mcdram_mem(void *buf, size_t size);

void setup(int rank, int proc, int argc, char **argv,
           int *n_ptr, int *energy_ptr, int *niters_ptr, int *px_ptr, int *py_ptr, int *final_flag);

void init_sources(int bx, int by, int offx, int offy, int n,
                  const int nsources, int sources[][2], int *locnsources_ptr, int locsources[][2]);

void alloc_bufs(int bx, int by,
                double **aold_ptr, double **anew_ptr,
                double **sbufnorth_ptr, double **sbufsouth_ptr,
                double **sbufeast_ptr, double **sbufwest_ptr,
                double **rbufnorth_ptr, double **rbufsouth_ptr,
                double **rbufeast_ptr, double **rbufwest_ptr);

void pack_data(int bx, int by, double *aold,
               double *sbufnorth, double *sbufsouth, double *sbfueast, double *sbufwest);

void unpack_data(int bx, int by, double *aold,
                 double *rbufnorth, double *rbufsouth, double *rbufeast, double *rbufwest);

void update_grid(int bx, int by, double *aold, double *anew, double *heat_ptr);

void free_bufs(double *aold, double *anew,
               double *sbufnorth, double *sbufsouth,
               double *sbufeast, double *sbufwest,
               double *rbufnorth, double *rbufsouth, double *rbufeast, double *rbufwest);

int main(int argc, char **argv)
{
    int rank, size;
    int n, energy, niters, px, py;

    int rx, ry;
    int north, south, west, east;
    int bx, by, offx, offy;

    /* three heat sources */
    const int nsources = 3;
    int sources[nsources][2];
    int locnsources;            /* number of sources in my area */
    int locsources[nsources][2];        /* sources local to my rank */

    double t1, t2;

    int iter, i;

    double *sbufnorth, *sbufsouth, *sbufeast, *sbufwest;
    double *rbufnorth, *rbufsouth, *rbufeast, *rbufwest;
    double *aold, *anew, *tmp;

    double heat, rheat;

    int final_flag;

    /* initialize MPI envrionment */
    init_mcdram();
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* argument checking and setting */
    setup(rank, size, argc, argv, &n, &energy, &niters, &px, &py, &final_flag);

    if (final_flag == 1) {
        MPI_Finalize();
        exit(0);
    }

    /* determine my coordinates (x,y) -- rank=x*a+y in the 2d processor array */
    rx = rank % px;
    ry = rank / px;

    /* determine my four neighbors */
    north = (ry - 1) * px + rx;
    if (ry - 1 < 0)
        north = MPI_PROC_NULL;
    south = (ry + 1) * px + rx;
    if (ry + 1 >= py)
        south = MPI_PROC_NULL;
    west = ry * px + rx - 1;
    if (rx - 1 < 0)
        west = MPI_PROC_NULL;
    east = ry * px + rx + 1;
    if (rx + 1 >= px)
        east = MPI_PROC_NULL;

    /* decompose the domain */
    bx = n / px;        /* block size in x */
    by = n / py;        /* block size in y */
    offx = rx * bx;     /* offset in x */
    offy = ry * by;     /* offset in y */

    //printf("%i (%i,%i) - w: %i, e: %i, n: %i, s: %i\n", rank, ry,rx,west,east,north,south);

    /* initialize three heat sources */
    init_sources(bx, by, offx, offy, n, nsources, sources, &locnsources, locsources);

    /* allocate working arrays & communication buffers */
    alloc_bufs(bx, by, &aold, &anew,
               &sbufnorth, &sbufsouth, &sbufeast, &sbufwest,
               &rbufnorth, &rbufsouth, &rbufeast, &rbufwest);

    MPI_Request reqs[8];

    t1 = MPI_Wtime();   /* take time */

    for (iter = 0; iter < niters; ++iter) {
        /* refresh heat sources */
        //for (i = 0; i < locnsources; ++i) {
        //    aold[ind(locsources[i][0], locsources[i][1])] += energy;    /* heat source */
        //}

        /* pack data */
        pack_data(bx, by, aold, sbufnorth, sbufsouth, sbufeast, sbufwest);

        /* exchange data with neighbors */
        MPI_Isend(sbufnorth, bx*sizeof(double), MPI_PACKED, north, 9, MPI_COMM_WORLD, &reqs[0]);
        MPI_Isend(sbufsouth, bx*sizeof(double), MPI_PACKED, south, 9, MPI_COMM_WORLD, &reqs[1]);
        MPI_Isend(sbufeast,  by*sizeof(double), MPI_PACKED, east,  9, MPI_COMM_WORLD, &reqs[2]);
        MPI_Isend(sbufwest,  by*sizeof(double), MPI_PACKED, west,  9, MPI_COMM_WORLD, &reqs[3]);

        MPI_Irecv(rbufnorth, bx*sizeof(double), MPI_PACKED, north, 9, MPI_COMM_WORLD, &reqs[4]);
        MPI_Irecv(rbufsouth, bx*sizeof(double), MPI_PACKED, south, 9, MPI_COMM_WORLD, &reqs[5]);
        MPI_Irecv(rbufeast,  by*sizeof(double), MPI_PACKED, east,  9, MPI_COMM_WORLD, &reqs[6]);
        MPI_Irecv(rbufwest,  by*sizeof(double), MPI_PACKED, west,  9, MPI_COMM_WORLD, &reqs[7]);

        MPI_Waitall(8, reqs, MPI_STATUSES_IGNORE);

        /* unpack data */
        unpack_data(bx, by, aold, rbufnorth, rbufsouth, rbufeast, rbufwest);

        /* update grid points */
        //update_grid(bx, by, aold, anew, &heat);

        /* swap working arrays */
        tmp = anew;
        anew = aold;
        aold = tmp;
    }

    t2 = MPI_Wtime();

    /* get final heat in the system */
    MPI_Allreduce(&heat, &rheat, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    if (rank == 0) {
        char cmd[64];
        fprintf(stderr, "%-*d%*.*f\n", 10, n, 10, 6, t2 - t1); 
        sprintf(cmd, "numastat -p %d", getpid());
        system(cmd);
    }


    /* free working arrays and communication buffers */
    free_bufs(aold, anew, sbufnorth, sbufsouth, sbufeast, sbufwest,
              rbufnorth, rbufsouth, rbufeast, rbufwest);

    MPI_Finalize();
    fini_mcdram();
    return 0;
}

void setup(int rank, int proc, int argc, char **argv,
           int *n_ptr, int *energy_ptr, int *niters_ptr, int *px_ptr, int *py_ptr, int *final_flag)
{
    int n, energy, niters, px, py;

    (*final_flag) = 0;

    if (argc < 6) {
        if (!rank)
            printf("usage: stencil_mpi <n> <energy> <niters> <px> <py>\n");
        (*final_flag) = 1;
        return;
    }

    n = atoi(argv[1]);  /* nxn grid */
    energy = atoi(argv[2]);     /* energy to be injected per iteration */
    niters = atoi(argv[3]);     /* number of iterations */
    px = atoi(argv[4]); /* 1st dim processes */
    py = atoi(argv[5]); /* 2nd dim processes */

    if (px * py != proc)
        MPI_Abort(MPI_COMM_WORLD, 1);   /* abort if px or py are wrong */
    if (n % px != 0)
        MPI_Abort(MPI_COMM_WORLD, 2);   /* abort px needs to divide n */
    if (n % py != 0)
        MPI_Abort(MPI_COMM_WORLD, 3);   /* abort py needs to divide n */

    (*n_ptr) = n;
    (*energy_ptr) = energy;
    (*niters_ptr) = niters;
    (*px_ptr) = px;
    (*py_ptr) = py;
}

void init_sources(int bx, int by, int offx, int offy, int n,
                  const int nsources, int sources[][2], int *locnsources_ptr, int locsources[][2])
{
    int i, locnsources = 0;

    sources[0][0] = n / 2;
    sources[0][1] = n / 2;
    sources[1][0] = n / 3;
    sources[1][1] = n / 3;
    sources[2][0] = n * 4 / 5;
    sources[2][1] = n * 8 / 9;

    for (i = 0; i < nsources; ++i) {    /* determine which sources are in my patch */
        int locx = sources[i][0] - offx;
        int locy = sources[i][1] - offy;
        if (locx >= 0 && locx < bx && locy >= 0 && locy < by) {
            locsources[locnsources][0] = locx + 1;      /* offset by halo zone */
            locsources[locnsources][1] = locy + 1;      /* offset by halo zone */
            locnsources++;
        }
    }

    (*locnsources_ptr) = locnsources;
}

void alloc_bufs(int bx, int by, double **aold_ptr, double **anew_ptr,
                double **sbufnorth_ptr, double **sbufsouth_ptr,
                double **sbufeast_ptr, double **sbufwest_ptr,
                double **rbufnorth_ptr, double **rbufsouth_ptr,
                double **rbufeast_ptr, double **rbufwest_ptr)
{
    double *aold, *anew;
    double *sbufnorth, *sbufsouth, *sbufeast, *sbufwest;
    double *rbufnorth, *rbufsouth, *rbufeast, *rbufwest;
    int ret = 0;
    char *stencil_sbuf_membind_type = NULL;
    char *stencil_rbuf_membind_type = NULL;

    stencil_sbuf_membind_type = getenv("STENCIL_SBUF_MEMBIND_TYPE");
    stencil_rbuf_membind_type = getenv("STENCIL_RBUF_MEMBIND_TYPE");

    if (stencil_sbuf_membind_type && strcmp(stencil_sbuf_membind_type, "MCDRAM") == 0) {
        /* allocate communication buffers */
        ret |= alloc_mcdram_mem((void **)&sbufnorth, bx * sizeof(double)); /* send buffers */
        ret |= alloc_mcdram_mem((void **)&sbufsouth, bx * sizeof(double)); /* send buffers */
        ret |= alloc_mcdram_mem((void **)&sbufeast,  by * sizeof(double)); /* send buffers */
        ret |= alloc_mcdram_mem((void **)&sbufwest,  by * sizeof(double)); /* send buffers */
    } else {
        /* allocate communication buffers */
        sbufnorth = (double *) malloc(bx * sizeof(double)); /* send buffers */
        sbufsouth = (double *) malloc(bx * sizeof(double));
        sbufeast  = (double *) malloc(by * sizeof(double));
        sbufwest  = (double *) malloc(by * sizeof(double));
    }
 
    if (stencil_rbuf_membind_type && strcmp(stencil_rbuf_membind_type, "MCDRAM") == 0) {
        ret |= alloc_mcdram_mem((void **)&rbufnorth, bx * sizeof(double)); /* send buffers */
        ret |= alloc_mcdram_mem((void **)&rbufsouth, bx * sizeof(double)); /* send buffers */
        ret |= alloc_mcdram_mem((void **)&rbufeast,  by * sizeof(double)); /* send buffers */
        ret |= alloc_mcdram_mem((void **)&rbufwest,  by * sizeof(double)); /* send buffers */
    } else {
        rbufnorth = (double *) malloc(bx * sizeof(double)); /* receive buffers */
        rbufsouth = (double *) malloc(bx * sizeof(double));
        rbufeast  = (double *) malloc(by * sizeof(double));
        rbufwest  = (double *) malloc(by * sizeof(double));
    }

    if (ret) {
       fprintf(stdout, "Error allocating memory in MCDRAM %s\n", strerror(errno));
    }

    anew = (double *) malloc((bx + 2) * (by + 2) * sizeof(double));     /* 1-wide halo zones! */
    aold = (double *) malloc((bx + 2) * (by + 2) * sizeof(double));     /* 1-wide halo zones! */
    
    memset(aold, 0, (bx + 2) * (by + 2) * sizeof(double));
    memset(anew, 0, (bx + 2) * (by + 2) * sizeof(double));

    memset(sbufnorth, 0, bx * sizeof(double));
    memset(sbufsouth, 0, bx * sizeof(double));
    memset(sbufeast,  0, by * sizeof(double));
    memset(sbufwest,  0, by * sizeof(double));
    memset(rbufnorth, 0, bx * sizeof(double));
    memset(rbufsouth, 0, bx * sizeof(double));
    memset(rbufeast,  0, by * sizeof(double));
    memset(rbufwest,  0, by * sizeof(double));

    bufsize = (bx + 2) * (by + 2) * sizeof(double);
    halo_x_size = (bx * sizeof(double));
    halo_y_size = (by * sizeof(double));

    (*aold_ptr) = aold;
    (*anew_ptr) = anew;
    (*sbufnorth_ptr) = sbufnorth;
    (*sbufsouth_ptr) = sbufsouth;
    (*sbufeast_ptr)  = sbufeast;
    (*sbufwest_ptr)  = sbufwest;
    (*rbufnorth_ptr) = rbufnorth;
    (*rbufsouth_ptr) = rbufsouth;
    (*rbufeast_ptr)  = rbufeast;
    (*rbufwest_ptr)  = rbufwest;
}

void free_bufs(double *aold, double *anew,
               double *sbufnorth, double *sbufsouth,
               double *sbufeast, double *sbufwest,
               double *rbufnorth, double *rbufsouth, double *rbufeast, double *rbufwest)
{
    char *stencil_sbuf_membind_type;
    char *stencil_rbuf_membind_type;

    free(aold);
    free(anew);
    
    stencil_sbuf_membind_type = getenv("STENCIL_SBUF_MEMBIND_TYPE");
    stencil_rbuf_membind_type = getenv("STENCIL_RBUF_MEMBIND_TYPE");

    if (stencil_sbuf_membind_type && strcmp(stencil_sbuf_membind_type, "MCDRAM") == 0) {
        munmap(sbufnorth, halo_x_size);
        munmap(sbufsouth, halo_x_size);
        munmap(sbufeast,  halo_y_size);
        munmap(sbufwest,  halo_y_size);
    } else {
        free(sbufnorth);
        free(sbufsouth);
        free(sbufeast);
        free(sbufwest);
    }

    if (stencil_rbuf_membind_type && strcmp(stencil_rbuf_membind_type, "MCDRAM") == 0) {        
        munmap(rbufnorth, halo_x_size);
        munmap(rbufsouth, halo_x_size);
        munmap(rbufeast,  halo_y_size);
        munmap(rbufwest,  halo_y_size);
    } else {    
        free(rbufnorth);
        free(rbufsouth);
        free(rbufeast);
        free(rbufwest);
    }
}

void pack_data(int bx, int by, double *aold,
               double *sbufnorth, double *sbufsouth, double *sbufeast, double *sbufwest)
{
    MPI_Datatype north, south, west, east;
    
    MPI_Type_contiguous(bx, MPI_DOUBLE, &north);
    MPI_Type_contiguous(bx, MPI_DOUBLE, &south);
    MPI_Type_vector(by, 1, bx + 2, MPI_DOUBLE, &east);
    MPI_Type_vector(by, 1, bx + 2, MPI_DOUBLE, &west);
    MPI_Type_commit(&north);
    MPI_Type_commit(&south);
    MPI_Type_commit(&east);
    MPI_Type_commit(&west);

    /* pack data */
    int position = 0;
    MPI_Pack(&aold[ind(1, 1)],  1, north, sbufnorth, bx * sizeof(double), &position, MPI_COMM_WORLD);
    position = 0;
    MPI_Pack(&aold[ind(1, by)], 1, south, sbufsouth, bx * sizeof(double), &position, MPI_COMM_WORLD);
    position = 0;
    MPI_Pack(&aold[ind(1, 1)],  1, west,  sbufwest,  by * sizeof(double), &position, MPI_COMM_WORLD);
    position = 0;
    MPI_Pack(&aold[ind(bx, 1)], 1, east,  sbufeast,  by * sizeof(double), &position, MPI_COMM_WORLD);

    MPI_Type_free(&north);
    MPI_Type_free(&south);
    MPI_Type_free(&east);
    MPI_Type_free(&west);
}

void unpack_data(int bx, int by, double *aold,
                 double *rbufnorth, double *rbufsouth, double *rbufeast, double *rbufwest)
{
    MPI_Datatype north, south, west, east;
    
    MPI_Type_contiguous(bx, MPI_DOUBLE, &north);
    MPI_Type_contiguous(bx, MPI_DOUBLE, &south);
    MPI_Type_vector(by, 1, bx + 2, MPI_DOUBLE, &east);
    MPI_Type_vector(by, 1, bx + 2, MPI_DOUBLE, &west);
    MPI_Type_commit(&north);
    MPI_Type_commit(&south);
    MPI_Type_commit(&east);
    MPI_Type_commit(&west);

    int position = 0;
    MPI_Unpack(rbufnorth, bx * sizeof(double), &position, &aold[ind(1, 0)],    1, north, MPI_COMM_WORLD);
    position = 0;
    MPI_Unpack(rbufsouth, bx * sizeof(double), &position, &aold[ind(1, by+1)], 1, south, MPI_COMM_WORLD);
    position = 0;
    MPI_Unpack(rbufwest,  by * sizeof(double), &position, &aold[ind(0, 1)],    1, west,  MPI_COMM_WORLD);
    position = 0;
    MPI_Unpack(rbufeast,  by * sizeof(double), &position, &aold[ind(bx+1, 1)], 1, east,  MPI_COMM_WORLD);

    MPI_Type_free(&north);
    MPI_Type_free(&south);
    MPI_Type_free(&east);
    MPI_Type_free(&west);
}

void update_grid(int bx, int by, double *aold, double *anew, double *heat_ptr)
{
    int i, j;
    double heat = 0.0;

    for (i = 1; i < bx + 1; ++i) {
        for (j = 1; j < by + 1; ++j) {
            anew[ind(i, j)] =
                anew[ind(i, j)] / 2.0 + (aold[ind(i - 1, j)] + aold[ind(i + 1, j)] +
                                         aold[ind(i, j - 1)] + aold[ind(i, j + 1)]) / 4.0 / 2.0;
            heat += anew[ind(i, j)];
        }
    }

    (*heat_ptr) = heat;
}

void init_mcdram() {
    int i;
    int numa_nodes;
    char *str;
    hwloc_obj_t obj;

    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);

    numa_nodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NUMANODE);
    mcdram_nodes = hwloc_bitmap_alloc();

    /* detect all mcdram nodes */
    for (i = 0; i < numa_nodes; i++) {
        obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NUMANODE, i);
        if (obj->subtype && !strcmp(obj->subtype, "MCDRAM")) {
            hwloc_bitmap_or(mcdram_nodes, obj->nodeset, mcdram_nodes);
        }
    }

    //hwloc_bitmap_asprintf(&str, mcdram_nodes);
    //fprintf(stdout, "%s\n", str);

    initialized = 1;
}

void fini_mcdram() {
    hwloc_bitmap_free(mcdram_nodes);
    hwloc_topology_destroy(topology);
    initialized = 0;
}

int alloc_mcdram_mem(void **buf, size_t size) {
    char *bind_policy = NULL;
    char *bind_rule = NULL;
    char *bind_proc = NULL;
    hwloc_membind_policy_t membind_policy;
    int membind_flags = HWLOC_MEMBIND_BYNODESET;

    assert(initialized);

    bind_policy = getenv("STENCIL_MEMBIND_POLICY");
    bind_rule   = getenv("STENCIL_MEMBIND_RULE");
    bind_proc   = getenv("STENCIL_MEMBIND_PROC");

    if (bind_policy && strcmp(bind_policy, "BIND") == 0) {
        membind_policy = HWLOC_MEMBIND_BIND;
    } else if (bind_policy && strcmp(bind_policy, "FIRSTTOUCH") == 0) {
        membind_policy = HWLOC_MEMBIND_FIRSTTOUCH;
    } else if (bind_policy && strcmp(bind_policy, "INTERLEAVE") == 0) {
        membind_policy = HWLOC_MEMBIND_INTERLEAVE;
    } else if (bind_policy && strcmp(bind_policy, "NEXTTOUCH") == 0) {
        membind_policy = HWLOC_MEMBIND_NEXTTOUCH;
    } else if (bind_policy && strcmp(bind_policy, "MIXED") == 0) {
        membind_policy = HWLOC_MEMBIND_MIXED;
    } else {
        membind_policy = HWLOC_MEMBIND_DEFAULT;
    }

    if (bind_rule && strcmp(bind_rule, "STRICT") == 0) {
        membind_flags |= HWLOC_MEMBIND_STRICT;
    }

    if (bind_proc && strcmp(bind_proc, "PROCESS") == 0) {
        membind_flags |= HWLOC_MEMBIND_PROCESS;
    } else if (bind_proc && strcmp(bind_proc, "THREAD") == 0) {
        membind_flags |= HWLOC_MEMBIND_THREAD;
    } else if (bind_proc && strcmp(bind_proc, "MIGRATE") == 0) {
        membind_flags |= HWLOC_MEMBIND_MIGRATE;
    } else {
        membind_flags |= HWLOC_MEMBIND_NOCPUBIND;
    }

    *buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    return hwloc_set_area_membind(topology, *buf, size, mcdram_nodes, membind_policy, membind_flags);
}

void free_mcdram_mem(void *buf, size_t size) {
    assert(initialized);
    munmap(buf, size);
}
