#include <fstream>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "MyFunctions.h"

using namespace std;

int main(int argc, char **argv) {
  int detid = 0;
  int runid = 0;
  if (argc > 2) {
    int tst = sscanf(argv[1], "%d", &detid);
    int tst2 = sscanf(argv[2], "%d", &runid);
    if (tst <= 0 || tst2 <= 0) {
      cout << "syntax: makeTree <detector number> <run number>" << endl;
      return -1;
    }
  } else {
    cout << "syntax: makeTree <detector number> <run number>" << endl;
    return -2;
  }

  if (runid < MINRUN || runid > MAXRUN) {
    cout << "run " << runid << " out of bounts (" << MINRUN << "-" << MAXRUN
         << ")" << endl;
    return -3;
  }

  char fname[100];
  snprintf(fname, 100, "%s/logs/logS%03d-%02d.txt", WORKDIR, detid, runid);
  FILE *ftmp = fopen(fname, "w");
  fclose(ftmp);
  char command[2000];
  snprintf(command, 2000,
           "root -b -x '%s/AnalyseTreeProduction.C+(%d,%d)' -q >> %s &\n",
           WORKDIR, detid, runid, fname);
  cout << "executing:\n" << command << endl;
  system(command);

  return (1);
}