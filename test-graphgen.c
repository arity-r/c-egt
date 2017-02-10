
#include <stdio.h>

int main(int argc, char* argv[]) {
  int V, blocksize;
  FILE* fp = fopen("test.graph", "rb");
  fread(&V, sizeof(int), 1, fp);
  fread(&blocksize, sizeof(int), 1, fp);

  int matrix[blocksize][blocksize];
  int neighbors[blocksize][V-1];
  int degrees[blocksize];
  fread(&matrix, sizeof(int), blocksize*blocksize, fp);
  fread(&neighbors, sizeof(int), blocksize*(V-1), fp);
  fread(&degrees, sizeof(int), blocksize, fp);
  fclose(fp);

  int u, vi;
  for(u = 0; u < V; u++)
    for(vi = 0; vi < degrees[u]; vi++)
      printf("%3d %3d\n", u, neighbors[u][vi]);

  return 0;
}
