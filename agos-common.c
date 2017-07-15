
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <argp.h>

#define C 1
#define D 0

char* filename;
double benefit;
double payoff_matrix[2][2];
double beta;
int Lambda;
int init_c;
double mutation_rate;

struct arguments {
  char* input;
  double benefit;
  double beta;
  double mutation_rate;
  int Lambda;
  int init_c;
  unsigned int seed;
};

static struct argp_option options[] = {
  {"input", 'i', "filename", 0, "Input file (stdin if no option)"},
  {"benefit", 'B', "benefit", 0, "Game benefit"},
  {"beta", 'b', "beta", 0, "Selection pressure"},
  {"m_rate", 'm', "prob", 0, "Muration rate"},
  {"Lambda", 't', "time", 0, "Simulation time"},
  {"init_c", 'c', "num_c", 0, "The initial number of cooperation"},
  {"seed", 's', "SEED", 0, "Random seed"},
  {0}
};

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
  struct arguments* arguments = state->input;
  switch(key) {
  case 'i': /* input */
    arguments->input = arg;
    break;
  case 'B': /* benefit */
    arguments->benefit = atof(arg);
    break;
  case 'b': /* beta */
    arguments->beta = atof(arg);
    break;
  case 'm': /* mutation_rate */
    arguments->mutation_rate = atof(arg);
    break;
  case 't': /* Lambda */
    arguments->Lambda = atoi(arg);
    break;
  case 'c': /* init_c */
    arguments->init_c = atoi(arg);
    break;
  case 's': /* seed */
    arguments->seed = atoi(arg);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

const char* program_version = "agos-common 1.0";
const char* bug_address = "";
static const char args_doc[] = "";
static const char doc[] = "";
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]) {
  // initialize parameters, variables, result
  int N, arraysize;
  FILE* fp;
  struct arguments args = {NULL, -1, -1, -1, -1, -1, 0};
  argp_parse(&argp, argc, argv, 0, 0, &args);

  // initialize params
  if(args.input == NULL) {
    fp = stdin;
  } else {
    fp = fopen(filename, "rb");
    if(fp == NULL) {
      fprintf(stdout, "graph file \"%s\" not found\n", filename);
      exit(-1);
    }
  }
  srand(args.seed);

  benefit = args.benefit;
  beta = args.beta;
  mutation_rate = args.mutation_rate;
  Lambda = args.Lambda;
  init_c = args.init_c;
  payoff_matrix[C][D] = 1. - benefit; // S
  payoff_matrix[D][C] = benefit;      // T
  payoff_matrix[C][C] = 1.;           // R
  payoff_matrix[D][D] = 0.;           // P

  // loading data
  assert(fread(&N, sizeof(int), 1, fp) == 1);
  assert(fread(&arraysize, sizeof(int), 1, fp) == 1);
  //int matrix[arraysize][arraysize];
  int neighbors[arraysize][N-1];
  int degrees[arraysize];
  /*
  assert(fread(&matrix, sizeof(int), arraysize*arraysize, fp)
         == arraysize*arraysize);
  */
  assert(fread(&neighbors, sizeof(int), arraysize*(N-1), fp)
         == arraysize*(N-1));
  assert(fread(&degrees, sizeof(int), arraysize, fp) == arraysize);
  fclose(fp);

  int strategies[arraysize];
  int u, v;
  for(u = 0; u < init_c; u++)
    strategies[u] = C;
  for(u = init_c; u < arraysize; u++)
    strategies[u] = D;
  for(u = 0; u < N - 1; u++) {
    v = u + rand() / (RAND_MAX / (N - u) + 1);
    int t = strategies[u];
    strategies[u] = strategies[v];
    strategies[v] = t;
  }

  double payoffs[arraysize];
  int j;
#ifndef TIME_EVOLUTION
  double Gsum[N+1];
  int Gnum[N+1];
  for(j = 0; j <= N; j++) {
    Gsum[j] = 0.;
    Gnum[j] = 0;
  }
#endif
  // run simulation
  int vi, su, sv, step;
  double pu, pv;
  double prob, rval;
#ifdef TIME_EVOLUTION
  double f;
#else
  double Tu, T[2];
#endif
  for(step = 0; step < Lambda; step++) {
    // calculate payoffs
    for(u = 0; u < N; u++) {
      pu = 0.;
      su = strategies[u];
      for(vi = 0; vi < degrees[u]; vi++) {
        v = neighbors[u][vi];
        sv = strategies[v];
        pu += payoff_matrix[su][sv];
      }
      payoffs[u] = pu;
#ifdef USE_AVERAGE_PAYOFF
      payoffs[u] /= degrees[u];
#endif
    }
#ifndef TIME_EVOLUTION
    // calculate G(j, t)
    T[D] = T[C] = 0.;
    for(u = 0; u < N; u++) {
      Tu = 0.;
      su = strategies[u];
      for(vi = 0; vi < degrees[u]; vi++) {
        v = neighbors[u][vi];
        sv = strategies[v];
        if(su == sv) continue;
        pu = payoffs[u];
        pv = payoffs[v];
        Tu += 1. / (1. + exp(-beta * (pv - pu)));
      }
      Tu /= degrees[u];
#ifdef MEAN_FIELD
      Tu = (1-mutation_rate)*Tu + mutation_rate*(1 - Tu);
#endif
      T[su] += Tu;
    }
    j = 0;
    for(u = 0; u < N; u++) j += strategies[u];
    Gsum[j] += (T[D] - T[C]) / N;
    Gnum[j] += 1;
#else
    if(step % N == 0) {
      j = 0;
      for(u = 0; u < N; u++) j += strategies[u];
      f = (double)j / N;
      fwrite(&f, sizeof(double), 1, stdout);
    }
#endif
    // update one strategy
    u = rand() % N;
    vi = rand() % degrees[u];
    v = neighbors[u][vi];
    su = strategies[u];
    sv = strategies[v];
    pu = payoffs[u];
    pv = payoffs[v];
    prob = 1. / (1. + exp(-beta * (pv - pu)));
    rval = (double) rand() / RAND_MAX;
    if(rval < prob) strategies[u] = sv;
    // mutation
    rval = (double) rand() / RAND_MAX;
    if(rval < mutation_rate) {
      u = rand() % N;
      strategies[u] = 1 - strategies[u];
    }
  }

  // print result
#ifndef TIME_EVOLUTION
  //for(j = 0; j <= N; j++) printf("%f\n", Gsum[j] / Gnum[j]);
  //for(j = 0; j <= N; j++) printf("%4d %6d % 1.6f\n", j, Gnum[j], Gsum[j]);
  fwrite(Gnum, sizeof(int), N+1, stdout);
  fwrite(Gsum, sizeof(double), N+1, stdout);
#else
  j = 0;
  for(u = 0; u < N; u++) j += strategies[u];
  f = (double)j / N;
  fwrite(&f, sizeof(double), 1, stdout);
#endif

  return 0;
}

