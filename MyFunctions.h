#include <cstddef>
#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H 1

#include <TROOT.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <cmath>
#include <fstream>
#include <iomanip>

#include <TCanvas.h>
#include <TCutG.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TMath.h>
#include <TPDF.h>
#include <TPaveStats.h>
#include <TPostScript.h>
#include <TSpline.h>
#include <TStyle.h>
#include <TTree.h>
// //
#define DEBUG 1
#undef DEBUG
#define DEBUGMSG 1
#undef DEBUGMSG
#define SLOWFILES 1
// #undef SLOWFILES

typedef struct {
  //    float corr;
  double bsl;
  double rms;
  double ftime;
  double stime;
  double maxtime;
  double maxtimeS;
  double time50;
  double ampl;
  double sampl;
  double intg;
  double charge;
  double chargeIon;
  double chi2;
  double t1;
  double t2;
  double t3;
  double a; /// fit function f = a*x + b
  double b;
  double ttrig;
} DPARAM;

typedef struct {
  //    float corr;
  double bsl;
  double rms;
  int stime;
  int ftime;
  int maxtime;
  double tot;
  double ampl;
  double charge;
  double risecharge;
  double t10;
  double tb10;
  double t90;
  double ttrig;
  double width;
  double sampl;
  double fampl;
  double bslch;
} IPARAM;

typedef struct {
  //    float corr;
  int npeaks;
  double *tot;
  double *ampl;
  double *charge;
  double *risecharge;
  double *t10;
  double *tb10;
  double *t90;
  double *ttrig;
  double *width;
  double *sampl;
  double *fampl;
  double *bslch;
} PPARAM;

typedef struct {
  //    float corr;
  int detNo;
  int runNo;
  double ampl;
  double sampl;
  double charge;
  double scharge;
  double rate;
  double srate;
  double grate;
  double sgrate;
  double tot;
  double stot;
  double risetime;
  double srisetime;
  double width;
  double swidth;
  double chovampl;
  double schovampl;
} RUNPAR;

const double rootConv = 2871763200.;
const double unixConv = 2082844800.;

const std::string RED("\033[0;31m");
const std::string RED_U_GBKG("\033[4;31;42m");
const std::string GREEN("\033[0;32m");
const std::string YELLOW("\033[0;33m");
const std::string BLUE("\033[0;34m");
const std::string MAGENTA("\033[0;35m");
const std::string CYAN("\033[0;36m");
const std::string INVERSE_ON("\033[7m");
const std::string INVERSE_OFF("\033[27m");
const std::string RESET_COLOR("\033[0m");
const std::string endlr("\n\033[0m");

const int MAX_N_FILES = 11000;

const char *WORKDIR = "/Users/alialtingun/Lecroy_Binary_Reader";
const char *CODEDIR = "/Users/alialtingun/Lecroy_Binary_Reader";
const char *PLOTDIR = "/Users/alialtingun/Lecroy_Binary_Reader/plots";
const char *BASEDIRNAME =
    "/Users/alialtingun/Lecroy_Binary_Reader/Data/dataTrees";
const char *OUTDIRNAME =
    "/Users/alialtingun/Lecroy_Binary_Reader/Data/processedTrees";
const char *RTYPE = "PRODUCTION";
const int MINRUN = 0;
const int MAXRUN = 99;
const int TOTRUNS = MAXRUN - MINRUN + 1;

const double Threshold = 0.0073;

const int N_INTEGRATION_POINTS = 10;

/// for the bin2tree
const char *DATA_PATH_NAME = "/Users/alialtingun/Lecroy_Binary_Reader/Data/Raw";
const char *WORK_DIR_NAME =
    "/Users/alialtingun/Lecroy_Binary_Reader/Data/dataTrees";
const char *RUN_TYPE = "PRODUCTIONraw";
const int RUNMIN = 0;
const int RUNMAX = 99;
const int RUNMIN2 = 100;
const int RUNMAX2 = 9999;
const int RUNMIN3 = 10000;
const int RUNMAX3 = 999999;
const std::size_t MAXVECSIZE =
    25E6; /// 25 million entries, which is 100 GB of data for 4 channels with
          /// double precision. Adjust as needed.
const int FRAMESIZE =
    2E7; /// 20 million samples per event, which is 160 MB per event for 4
         /// channels with double precision. Adjust as needed.

inline void replaceEOL(char *fnames) {
  int nchar = 0;
  while (fnames[nchar] != '\n') {
    nchar++;
  }
  fnames[nchar] = '\0';
}

inline void replaceEOLT(char *fnames) {
  int nchar = 0;
  while (fnames[nchar] != '\n') {
    nchar++;
  }
  fnames[nchar] = '\t';
  //     fnames[nchar+1]='\0';
}

#endif
