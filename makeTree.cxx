#include <fstream>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <filesystem>  // C++17 filesystem

#include "MyFunctions.h"

using namespace std;
namespace fs = std::filesystem;  // fs shortcut

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
  if (detid < MINRUN || detid > MAXRUN) {
    cout << "detector " << detid << " out of bounts (" << MINRUN << "-"
         << MAXRUN << ")" << endl;
    return -4;
  }
  if (detid == 0) {
    cout << "Detector number can not be 0. It should be between " << MINRUN
         << " and " << MAXRUN << endl;
    return -5;
  }

  char fname[100];
  snprintf(fname, 100, "%s/logs/logS%03d-%02d.txt", WORKDIR, detid, runid);

  // Create logs/ if needed - does NOTHING if it already exists
  fs::path log_dir(WORKDIR);
  log_dir /= "logs";
  std::error_code error_code; // non-throwing version
  bool created = fs::create_directories(log_dir, error_code);

  if (error_code) {
    std::cout << "Failed to create logs/: " << error_code.message()
              << std::endl;
    return -6;
  }

  // std::cout<< "Created boolean: " << created << std::endl;

  FILE *ftmp = fopen(fname, "w");

  if (ftmp == nullptr) {
    std::cout << "log  is not created!" << std::endl;
    return -6;
  }

  fclose(ftmp);

  char command[2000];
  snprintf(command, 2000,
           "root -b -x '%s/MakeTreefromRawTreeProduction.C+(%d,%d)' >> "
           "%s/logs/logS%03d-%02d.txt &\n",
           WORKDIR, detid, runid, WORKDIR, detid, runid);
  cout << "Executing:\n" << command << endl;
  system(command);

  return (0);
}