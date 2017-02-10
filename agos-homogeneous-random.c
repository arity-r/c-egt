
#include <stdio.h>
#include <math.h>
#include <igraph.h>

#define C 1
#define D 0

#define N 1000
#define k 4
#define Lambda 100000
#define beta 10.0

double benefit;
double payoff_matrix[2][2];
int init_c;

void init_params(int argc, char* argv[]);
void generate_network(igraph_t* graph);
void double_edge_swap(igraph_t *graph, int nswap, int ntrials);
void homogeneous_swap(igraph_t *graph, double f);

int main(int argc, char* argv[]) {
  // initialize parameters, variables, result
  int j, u;
  init_params(argc, argv);

  igraph_t graph;
  generate_network(&graph);

  igraph_vector_t degrees;
  igraph_vector_init(&degrees, N);
  igraph_degree(&graph, &degrees,
                igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);

  igraph_vector_t neighbors[N];
  for(u = 0; u < N; u++) {
    igraph_vector_init(&neighbors[u], igraph_vector_e(&degrees, u));
    igraph_neighbors(&graph, &neighbors[u], u, IGRAPH_ALL);
  }

  igraph_vector_bool_t strategies;
  igraph_vector_bool_init(&strategies, N);
  for(u = 0; u < init_c; u++)
    igraph_vector_bool_set(&strategies, u, C);
  for(u = init_c; u < N; u++)
    igraph_vector_bool_set(&strategies, u, D);
  igraph_vector_bool_shuffle(&strategies);

  igraph_vector_t payoffs;
  igraph_vector_init(&payoffs, N);

  double Gsum[N+1];
  int Gnum[N+1];
  for(j = 0; j <= N; j++) {
    Gsum[j] = 0.;
    Gnum[j] = 0;
  }

  // run simulation
  int vi, v, su, sv, step;
  double pu, pv, Tu, T[2];
  double prob, rval;
  for(step = 0; step < Lambda; step++) {
    // calculate payoffs
    for(u = 0; u < N; u++) {
      pu = 0.;
      su = igraph_vector_bool_e(&strategies, u);
      for(vi = 0; vi < igraph_vector_e(&degrees, u); vi++) {
        v = igraph_vector_e(&neighbors[u], vi);
        sv = igraph_vector_bool_e(&strategies, v);
        pu += payoff_matrix[su][sv];
      }
      igraph_vector_set(&payoffs, u, pu);
    }
    // calculate G(j, t)
    T[D] = T[C] = 0.;
    for(u = 0; u < N; u++) {
      Tu = 0.;
      su = igraph_vector_bool_e(&strategies, u);
      for(vi = 0; vi < igraph_vector_e(&degrees, u); vi++) {
        v = igraph_vector_e(&neighbors[u], vi);
        sv = igraph_vector_bool_e(&strategies, v);
        if(su == sv) continue;
        pu = igraph_vector_e(&payoffs, u);
        pv = igraph_vector_e(&payoffs, v);
        Tu += 1. / (1. + exp(-beta * (pv - pu)));
      }
      T[su] += Tu / igraph_vector_e(&degrees, u);
    }
    j = igraph_vector_bool_sum(&strategies);
    Gsum[j] += (T[D] - T[C]) / N;
    Gnum[j] += 1;
    // update one strategy
    u = igraph_rng_get_integer(igraph_rng_default(), 0, N-1);
    vi = igraph_rng_get_integer(igraph_rng_default(),
                                0, igraph_vector_e(&degrees, u)-1);
    v = igraph_vector_e(&neighbors[u], vi);
    su = igraph_vector_bool_e(&strategies, u);
    sv = igraph_vector_bool_e(&strategies, v);
    pu = igraph_vector_e(&payoffs, u);
    pv = igraph_vector_e(&payoffs, v);
    prob = 1. / (1. + exp(-beta * (pv - pu)));
    rval = igraph_rng_get_unif01(igraph_rng_default());
    if(rval < prob) {
      igraph_vector_bool_set(&strategies, u, sv);
    }
    // TODO mutation
  }

  // print result
  //for(j = 0; j <= N; j++) printf("%f\n", Gsum[j] / Gnum[j]);
  //for(j = 0; j <= N; j++) printf("%d\t%f\n", Gnum[j], Gsum[j]);
  fwrite(Gnum, sizeof(int), N+1, stdout);
  fwrite(Gsum, sizeof(double), N+1, stdout);

  // destroy
  igraph_vector_destroy(&payoffs);
  igraph_vector_bool_destroy(&strategies);
  for(u = 0; u < N; u++)
    igraph_vector_destroy(&neighbors[u]);
  igraph_vector_destroy(&degrees);
  igraph_destroy(&graph);
  return 0;
}

void init_params(int argc, char* argv[]) {
  if(argc < 4) {
    fprintf(stderr, "%s benefit init_c seed\n", argv[0]);
    exit(-1);
  }
  benefit = atof(argv[1]);
  payoff_matrix[C][D] = 1. - benefit; // S
  payoff_matrix[D][C] = benefit;      // T
  payoff_matrix[C][C] = 1.;           // R
  payoff_matrix[D][D] = 0.;           // P
  init_c = atoi(argv[2]);
  igraph_rng_seed(igraph_rng_default(), atoi(argv[3]));
}

void regular_ring_network(igraph_t* graph) {
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
  /*
    igraph_vector_t dimvector;
    igraph_vector_init(&dimvector, 1);

    VECTOR(dimvector)[0] = N;
    igraph_lattice(graph, &dimvector, k/2,
    IGRAPH_UNDIRECTED, 1, 1);
    igraph_vector_destroy(&dimvector);
  */
}

void generate_network(igraph_t* graph) {
  regular_ring_network(graph);
  homogeneous_swap(graph, 0.9);
  /*
  double_edge_swap(graph,
                   2*igraph_ecount(graph)*9/10,
                   2*igraph_ecount(graph)*10);
  */
}

void double_edge_swap(igraph_t *graph, int nswap, int ntrials) {
  int swapcount = 0, trials = 0;
  igraph_integer_t eid1, eid2, u, v, x, y;
  igraph_es_t es;

  while(swapcount < nswap && trials < ntrials) {
    trials++;
    // select 2 edges
    eid1 = igraph_rng_get_integer(igraph_rng_default(),
                                  0, igraph_ecount(graph)-1);
    eid2 = igraph_rng_get_integer(igraph_rng_default(),
                                  0, igraph_ecount(graph)-1);
    igraph_edge(graph, eid1, &u, &v);
    igraph_edge(graph, eid2, &x, &y);

    if(u == x || v == y) continue; // same source or target

    // check parallel edge
    igraph_get_eid(graph, &eid1, u, x, IGRAPH_UNDIRECTED, 0);
    igraph_get_eid(graph, &eid2, v, y, IGRAPH_UNDIRECTED, 0);
    if(eid1 >= 0 || eid2 >= 0) continue;

    // swap by add and delete edges
    igraph_add_edge(graph, u, x);
    igraph_add_edge(graph, v, y);
    igraph_es_pairs_small(&es, IGRAPH_UNDIRECTED, u, v, x, y, -1);
    igraph_delete_edges(graph, es);
    swapcount++;
  }
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
