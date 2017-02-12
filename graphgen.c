
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <argp.h>
#include <igraph.h>

struct arguments {
  char* topology;
  int nodes;
  int degree;
  double hoswap;
  double heswap;
  char* output;
  unsigned long seed;
};

static struct argp_option options[] = {
  {"topology", 't', "topology", 0, "Network topology [rr|ba|sl]"},
  {"nodes", 'N', "N", 0, "The number of nodes"},
  {"degree", 'k', "k", 0, "Average degree"},
  {"hoswap", 'f', "f", 0, "Homogeneous swap frequency"},
  {"heswap", 'p', "p", 0, "Heterogeneous swap frequency"},
  {"output", 'o', "filename", 0, "Output file"},
  {"seed", 's', "SEED", 0, "Random seed"},
  {0}
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;
  switch(key) {
  case 't': /* topology */
    arguments->topology = arg;
    break;
  case 'N': /* nodes */
    arguments->nodes = atoi(arg);
    break;
  case 'k': /* degree */
    arguments->degree = atoi(arg);
    break;
  case 'f': /* hoswap */
    arguments->hoswap = atof(arg);
    break;
  case 'p': /* heswap */
    arguments->heswap = atof(arg);
    break;
  case 'o': /* output */
    arguments->output = arg;
    break;
  case 's': /* seed */
    arguments->seed = atol(arg);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

const char* program_version = "graphgen 1.0";
const char* bug_address = "";
static const char args_doc[] = "";
static const char doc[] = "";
static struct argp argp = {options, parse_opt, args_doc, doc};

void barabasi_albert_network(igraph_t* graph, int N, int m);
void regular_ring_graph(igraph_t* graph, int N, int k);
void square_lattice(igraph_t* graph, int N, int M);
void homogeneous_swap(igraph_t* graph, double f);
void heterogeneous_swap(igraph_t* graph, double p);

int main(int argc, char* argv[]) {
  struct arguments args = {NULL, -1, -1, -1, -1, NULL, 0};
  igraph_t graph;
  argp_parse(&argp, argc, argv, 0, 0, &args);

  if(!args.topology) {
    fprintf(stdout, "No topology\n");
    return -1;
  }
  if(args.nodes <= 0) {
    fprintf(stderr, "Invalid node number\n");
    return -1;
  }
  if(args.hoswap > 0 && args.heswap > 0) {
    fprintf(stderr, "Can't apply homogeneous and heterogeneous swap\n");
    return -1;
  }
  igraph_rng_seed(igraph_rng_default(), args.seed);

  if(strcmp(args.topology, "rr") == 0) {
    if(args.degree <= 0) {
      fprintf(stderr, "Invalid degree\n");
      return -1;
    }
    regular_ring_graph(&graph, args.nodes, args.degree);
  } else if(strcmp(args.topology, "ba") == 0) {
    if(args.degree <= 0) {
      fprintf(stderr, "Invalid degree\n");
      return -1;
    }
    barabasi_albert_network(&graph, args.nodes, args.degree / 2);
  } else if(strcmp(args.topology, "sl") == 0) {
    int n = (int) sqrt(args.nodes);
    args.degree = 4;
    square_lattice(&graph, n, n);
  } else {
    fprintf(stdout, "No topology named \"%s\"\n", args.topology);
    return -1;
  }

  if(args.hoswap > 0)
    homogeneous_swap(&graph, args.hoswap);
  if(args.heswap > 0)
    heterogeneous_swap(&graph, args.heswap);

  int k, u, v, V = igraph_vcount(&graph);
  int blocksize = ((V-1) / 32 + 1) * 32;
  int dim[2];
  dim[0] = V; dim[1] = blocksize;

  int matrix[blocksize][blocksize];
  int neighbors[blocksize][V-1];
  int degrees[blocksize];
  igraph_integer_t eid;
  for(u = 0; u < V; u++) {
    k = 0;
    for(v = 0; v < V; v++) {
      igraph_get_eid(&graph, &eid, u, v, IGRAPH_UNDIRECTED, 0);
      if(eid >= 0) {
        matrix[u][v] = 1;
        neighbors[u][k++] = v;
      } else {
        matrix[u][v] = 0;
      }
    }
    for(v = V; v < blocksize; v++)
      matrix[u][v] = 0;
    for(v = k; v < V-1; v++)
      neighbors[u][v] = -1;
    degrees[u] = k;
  }
  for(u = V; u < blocksize; u++) {
    for(v = 0; v < blocksize; v++)
      matrix[u][v] = 0;
    for(v = 0; v < V-1; v++)
      neighbors[u][v] = -1;
    degrees[u] = 0;
  }

  FILE* fp;
  if(!args.output) fp = stdout;
  else fp = fopen(args.output, "wb");
  fwrite(&dim, sizeof(int), 2, fp);
  fwrite(&matrix, sizeof(int), blocksize*blocksize, fp);
  fwrite(&neighbors, sizeof(int), blocksize*(V-1), fp);
  fwrite(&degrees, sizeof(int), blocksize, fp);
  if(args.output) fclose(fp);

  return 0;
}

void barabasi_albert_network(igraph_t* graph, int N, int m) {
  long int u, v, vi;
  igraph_vector_t edges;
  igraph_integer_t edges_size = 0, n_edges = (N-m)*m;
  igraph_vector_init(&edges, 2*n_edges);
  igraph_vector_long_t target;
  igraph_vector_long_init(&target, m);
  for(u = 0; u < m; u++)
    VECTOR(target)[u] = u;

  for(u = m; u < N; u++) {
    for(vi = 0; vi < m; vi++) {
      VECTOR(edges)[edges_size++] = u;
      VECTOR(edges)[edges_size++] = VECTOR(target)[vi];
    }

    igraph_vector_long_clear(&target);
    while(igraph_vector_long_size(&target) != m) {
      igraph_bool_t found;
      vi = igraph_rng_get_integer(igraph_rng_default(),
                                  0, edges_size-1);
      v = VECTOR(edges)[vi];
      if(!(found = igraph_vector_long_binsearch(&target, v, &vi)))
        igraph_vector_long_insert(&target, vi, v);
    }
  }

  igraph_create(graph, &edges, N, IGRAPH_UNDIRECTED);
  igraph_vector_destroy(&edges);
  igraph_vector_long_destroy(&target);
}

void regular_ring_graph(igraph_t* graph, int N, int k) {
  int u, v;
  igraph_vector_t edges;
  int edges_size = 0, n_edges = N * k / 2;
  igraph_vector_init(&edges, 2*n_edges);

  for(u = 0; u < N; u++) {
    for(v = u+1; v <= u+k/2; v++) {
      VECTOR(edges)[edges_size++] = u;
      VECTOR(edges)[edges_size++] = v % N;
    }
  }

  igraph_create(graph, &edges, N, IGRAPH_UNDIRECTED);
  igraph_vector_destroy(&edges);
}

void square_lattice(igraph_t* graph, int N, int M) {
  int i, j;
  igraph_vector_t edges;
  int edges_size = 0, n_edges = 2 * N * M;
  igraph_vector_init(&edges, 2*n_edges);

  for(j = 0; j < N; j++) {
    for(i = 0; i < M; i++) {
      VECTOR(edges)[edges_size++] = j*M + i;
      VECTOR(edges)[edges_size++] = j*M + (i+1)%M;
      VECTOR(edges)[edges_size++] = j*M + i;
      VECTOR(edges)[edges_size++] = ((j+1)%N)*M + i;
    }
  }

  igraph_create(graph, &edges, N*M, IGRAPH_UNDIRECTED);
  igraph_vector_destroy(&edges);
}

void homogeneous_swap(igraph_t *graph, double f) {
  igraph_integer_t E = igraph_ecount(graph);
  igraph_integer_t nunselected = E;
  igraph_integer_t nswaps = (igraph_integer_t) (f * E);
  igraph_integer_t sources[E], targets[E];
  igraph_integer_t eid, eid1, eid2, u, v, x, y;
  igraph_es_t es;

  if(nswaps > E) nswaps = E;
  if(nswaps % 2 == 1) nswaps--;
  // initialize vertices set
  for(eid = 0; eid < E; eid++) {
    igraph_edge(graph, eid, &u, &v);
    sources[eid] = u;
    targets[eid] = v;
  }

  while(nswaps > 0) {
    // select 2 edges
    eid1 = igraph_rng_get_integer(igraph_rng_default(), 0, nunselected-1);
    eid2 = igraph_rng_get_integer(igraph_rng_default(), 0, nunselected-1);
    u = sources[eid1]; v = targets[eid1];
    x = sources[eid2]; y = targets[eid2];

    if(u == x || v == y) continue; // same source or target

    // check parallel edge
    igraph_get_eid(graph, &eid, u, x, IGRAPH_UNDIRECTED, 0);
    if(eid >= 0) continue;
    igraph_get_eid(graph, &eid, v, y, IGRAPH_UNDIRECTED, 0);
    if(eid >= 0) continue;

    // swap by add and delete edges
    igraph_add_edge(graph, u, x);
    igraph_add_edge(graph, v, y);
    igraph_es_pairs_small(&es, IGRAPH_UNDIRECTED, u, v, x, y, -1);
    igraph_delete_edges(graph, es);

    // delete eid1-th and eid2-th elements from sources and targets
    if(eid1 > eid2) {
      eid = eid1; eid1 = eid2; eid2 = eid;
    }
    for(eid = eid1; eid < eid2 - 1; eid++) {
      sources[eid] = sources[eid+1];
      targets[eid] = targets[eid+1];
    }
    for(eid = eid2 - 1; eid < nunselected - 2; eid++) {
      sources[eid] = sources[eid+2];
      targets[eid] = targets[eid+2];
    }

    nswaps -= 2;
    nunselected -= 2;
  }
}

void heterogeneous_swap(igraph_t* graph, double p) {
  fprintf(stderr, "heterogeneous swap is not implemented\n");
  exit(-1);
}
