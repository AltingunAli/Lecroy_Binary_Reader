#include "MyFunctions.h"
#include <fstream>
#include <math.h>
#include <stdint.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <string>
#include <unistd.h>  // for getcwd
#include <string.h>
#include <vector>


// using namespace std;

// Define these constants or get them from somewhere
#define WORKDIR "/path/to/your/workdir"  // Replace with actual path
#define MINRUN 1
#define MAXRUN 9999

int main(int argc, char **argv) {
  int detid = 0;
  int runid = 0;
  
  // Default values for optional parameters
  int draw = 0;
  int saveWF = 0;
  double threshold = 200.0;
  double nTh = 300.0;
  int polarity = -1;
  int exttrig = 0;
  int nRun = 1;
  int bkg = 0;
  std::string filetype = "";
  
  // Parse required arguments
  if (argc < 3) {
    std::cout << "\nUsage: " << argv[0] << " <detector number> <run number> [options]\n" << std::endl;
    std::cout << "Required arguments:" << std::endl;
    std::cout << "  detector number    : Detector ID (e.g., 123)" << std::endl;
    std::cout << "  run number         : Run number" << std::endl;
    std::cout << "\nOptional arguments (with defaults):" << std::endl;
    std::cout << "  --draw <0/1>              : Draw plots (default: 0)" << std::endl;
    std::cout << "  --saveWF <0/1>            : Save waveforms (default: 0)" << std::endl;
    std::cout << "  --threshold <value>       : Threshold value (default: 200.0)" << std::endl;
    std::cout << "  --nTh <value>             : nTh value (default: 300.0)" << std::endl;
    std::cout << "  --polarity <-1/1>         : Polarity (default: -1)" << std::endl;
    std::cout << "  --exttrig <0/1>           : External trigger (default: 0)" << std::endl;
    std::cout << "  --nRun <value>            : Number of runs (default: 1)" << std::endl;
    std::cout << "  --bkg <0/1>               : Background subtraction (default: 0)" << std::endl;
    std::cout << "  --filetype <string>       : File type (default: \"\")" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << argv[0] << " 123 45" << std::endl;
    std::cout << "  " << argv[0] << " 123 45 --draw 1 --threshold 150.0 --polarity 1" << std::endl;
    std::cout << "  " << argv[0] << " 123 45 --saveWF 1 --filetype root" << std::endl;
    return -2;
  }
  
  int tst = sscanf(argv[1], "%d", &detid);
  int tst2 = sscanf(argv[2], "%d", &runid);
  if (tst <= 0 || tst2 <= 0) {
    std::cout << "Error: Invalid detector or run number" << std::endl;
    std::cout << "Usage: " << argv[0] << " <detector number> <run number> [options]" << std::endl;
    return -1;
  }
  
  // Parse optional arguments
  for (int i = 3; i < argc; i++) {
    if (strcmp(argv[i], "--draw") == 0 && i+1 < argc) {
      draw = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--saveWF") == 0 && i+1 < argc) {
      saveWF = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--threshold") == 0 && i+1 < argc) {
      threshold = atof(argv[++i]);
    }
    else if (strcmp(argv[i], "--nTh") == 0 && i+1 < argc) {
      nTh = atof(argv[++i]);
    }
    else if (strcmp(argv[i], "--polarity") == 0 && i+1 < argc) {
      polarity = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--exttrig") == 0 && i+1 < argc) {
      exttrig = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--nRun") == 0 && i+1 < argc) {
      nRun = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--bkg") == 0 && i+1 < argc) {
      bkg = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "--filetype") == 0 && i+1 < argc) {
      filetype = argv[++i];
    }
    else {
      std::cout << "Error: Unknown option or missing value: " << argv[i] << std::endl;
      return -4;
    }
  }

  if (runid < MINRUN || runid > MAXRUN) {
    std::cout << "Error: run " << runid << " out of bounds (" << MINRUN << "-" << MAXRUN << ")" << std::endl;
    return -3;
  }

  // Print configuration
  std::cout << "\n=== Configuration ===" << std::endl;
  std::cout << "Detector ID: " << detid << std::endl;
  std::cout << "Run ID: " << runid << std::endl;
  std::cout << "Draw: " << draw << std::endl;
  std::cout << "Save WF: " << saveWF << std::endl;
  std::cout << "Threshold: " << threshold << std::endl;
  std::cout << "nTh: " << nTh << std::endl;
  std::cout << "Polarity: " << polarity << std::endl;
  std::cout << "Ext Trig: " << exttrig << std::endl;
  std::cout << "nRun: " << nRun << std::endl;
  std::cout << "Background: " << bkg << std::endl;
  std::cout << "Filetype: " << filetype << std::endl;
  std::cout << "=====================\n" << std::endl;

  char fname[100];
  char dirpath[500];
  char command_0[2000];
  snprintf(dirpath, sizeof(dirpath), "%s/logs/analysis", WORKDIR);

  // Check if directory exists
  struct stat st;
  if (stat(dirpath, &st) != 0) {
    // Directory doesn't exist, create it
    snprintf(command_0, sizeof(command_0), "mkdir -p %s", dirpath);
    system(command_0);
  }

  snprintf(fname, sizeof(fname), "%s/logs/analysis/logS%03d-%02d.txt", WORKDIR,
           detid, runid);
  FILE *ftmp = fopen(fname, "w");
  if (ftmp) fclose(ftmp);
  
  // Build command with all parameters
  // Note: Using TString format for string parameters in ROOT
  char command[3000];
  snprintf(command, sizeof(command),
           "root -b -x '%s/AnalyseTreeProduction.C+(%d,%d,%d,%d,%f,%f,%d,%d,%d,%d,\\\"%s\\\")' -q >> %s 2>&1 &\n",
           WORKDIR, detid, runid, draw, saveWF, threshold, nTh, polarity, 
           exttrig, nRun, bkg, filetype.c_str(), fname);
  
  std::cout << "Executing command:" << std::endl;
  std::cout << command << std::endl;
  std::cout << "Log file: " << fname << std::endl;
  std::cout << "Job submitted in background (PID: " << getpid() << ")" << std::endl;
  
  int ret = system(command);
  if (ret != 0) {
    std::cout << "Warning: system() returned " << ret << std::endl;
  }

  return (1);
}