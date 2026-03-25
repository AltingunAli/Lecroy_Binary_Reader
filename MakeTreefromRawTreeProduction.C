#include "MyFunctions.C"
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TGaxis.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TMath.h>
#include <TPaveText.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTimeStamp.h>
#include <TTree.h>
#include <cstddef>
#include <vector>

// void searcheandreplace(char *fnames)
// {
//     char tmp[1000];
//     int nchar=0;
//     do {
//       tmp[nchar]=fnames[nchar];
//       nchar++;
//     } while (fnames[nchar-1]!='\0');
//
//     for (int i=0; i<nchar-1; i++)
//     {
//        if (fnames[i]==' ')
//        {
//           fnames[i]='\\';
//           for (int j=i+1;j<nchar+1;j++)
// 	    fnames[j]=tmp[j-1];
// 	std::cout<<fnames<<endl;
// 	  searcheandreplace(&fnames[i+2]);
//        }
//
//     }
// }
// // void replaceEOL(char *fnames)
// // {
// //     int nchar=0;
// //     while (fnames[nchar]!='\n')
// //     {
// //       nchar++;
// //     }
// //     fnames[nchar]='\0';
// //
// // }
// // void replaceEOLT(char *fnames)
// // {
// //     int nchar=0;
// //     while (fnames[nchar]!='\n')
// //     {
// //       nchar++;
// //     }
// //     fnames[nchar]='\t';
// // //     fnames[nchar+1]='\0';
// //
// // }

// double Summa(int npoints,double *X,double *Y,double mean, double sigma)
// {
//    double sum=0.;
//    for(int i=0;i<npoints;i++)
//    {
//        if (X[i]>mean-3*sigma)
//        {
// 	 sum+=Y[i];
// //          std::cout<<"["<<i<<" | "<<X[i]<<"] - ";
//        }
//        if (X[i]>mean+4*sigma)
// 	 break;
//    }
//    return (sum);
// }

// ###########################################################################################################//
//  Finding the number of consecutive zeros in an array, which can be useful for
//  pulse analysis and noise characterization. This function counts how many
//  times two consecutive entries in the data array are zero, ßwhich can
//  indicate periods of inactivity or baseline in a signal.
int FindZeros(int n, double *data) {
  int n0 = 0;
  for (int i = 1; i < n; i++) {
    if (data[i] == 0.0)
      if (data[i - 1] == 0)
        n0++;
    //      std::cout<<"["<<data[i]<<"/"<<(data[i]==0)<<"] ";
  }
  return n0;
}
// ###########################################################################################################//

// //
// ###########################################################################################################//
// int month2int_littleendian(const char *month) {
//   const char hash[] = {3, 12, 8, 2, 1, 11, 7, 5, 0, 10, 4, 9, 6};
//   return hash[(*(const int32_t *)month & ~0x20202020 + 146732) % 13];
// }
// //
// ###########################################################################################################//

// ###########################################################################################################//
// Main function to create a ROOT tree from raw data files for a given detector
// and run number
int MakeTreefromRawTreeProduction(
    int detNo = 0, int runNo = 1, int trigger = 0,
    string filetype =
        "") /// put trigger = ChannelNo in case of external trigger
{

  // Load the MyFunctions.C macro, which contains utility functions for data
  // analysis
  gROOT->LoadMacro("MyFunctions.C");

  // There are 4 channels in the oscilloscope, numbered from 0 to 3
  if (trigger > 4) {
    std::cout << "Wrong trigger channel : " << trigger << std::endl;
    return -15;
  }

  // For conversion from V to mV
  const double mV = 1000.;

  int mask[] = {0, 0};

  // Default mesh and drift voltages
  int vm = 475;
  int vd = 850;

  // Drift gap amd mylar thickness
  float bthick, dgap;
  int angle = 0;

  // Arrays for commands, filenames, and directories
  char command[5000];
  char fname[4][5000];
  char fname2[5000], ofilename[5000], ifilename[5000];
  char fnametmp[2000];
  vector<string> fnames[4];
  char rtype[1000];
  strcpy(rtype, RTYPE);
  char basedirname[1000];
  char outdirname[1000];

  // Set the base and output directory names
  snprintf(basedirname, sizeof(basedirname), "%s", BASEDIRNAME);
  snprintf(outdirname, sizeof(outdirname), "%s", OUTDIRNAME);

  // ###################################################################################################################//
  //  Determine the width of the detector number in the folder name based on the
  //  detector number
  int width = 3;
  if (detNo >= MINRUN && detNo <= MAXRUN) {
    char afile[500];
    snprintf(afile, sizeof(afile), "%s/tmpfile.tmp",
             basedirname);          // set the §mporary file name
    FILE *ftmp = fopen(afile, "w"); // create the temporary file
    if (ftmp == NULL) {
      std::cout << afile << " can not be created. Probably the directory '"
                << basedirname << "' does not exit. Exiting..." << std::endl;
      exit(-12);
    }
    fclose(ftmp);
    // cd to the raw data directory
    snprintf(command, sizeof(command),
             "cd %s\nls -d S%03d-%02d-*%s_%s*.root > %s 2>/dev/null",
             basedirname, detNo, runNo, filetype.c_str(), rtype, afile);

    int tst = system(command);
    //    std::cout<<command<<endl<<"returned: "<<tst<<endl;
    if (tst != 0) {
      std::cout << command << std::endl << "returned: " << tst << std::endl;
      std::cout << "Probably the tree of the run " << runNo
                << " of detector No S" << detNo
                << " was not found in directory " << basedirname << std::endl
                << "Exiting..." << std::endl;
      return tst;
    }
    ftmp = fopen(afile, "r");
    if (fgets(fnametmp, 200, ftmp) == NULL) {
      std::cout << "Failed to read the filename for detector S" << detNo
                << " run " << runNo << " at " << basedirname << std::endl
                << "Exiting..." << std::endl;
      return -2;
    }

    //  Write the data to the log file
    int rtmp, dtmp;
    char ftypetmp[500];
    strcpy(ftypetmp, "");
    int stst = sscanf(fnametmp, "S%03d-%02d-%d-%d_%30[^ /,\n\t]", &dtmp, &rtmp,
                      &vm, &vd, ftypetmp);

    //--------------------------------------------------------------------------//
    // For the format SXXX-YY-Vm-Vd-bthick-dgap-filetype use:
    //       int stst =
    //       sscanf(fnametmp,"S%03d-%d-%d-%3f-%3f%30s",&rtmp,&vm,&vd,&bthick,&dgap,ftypetmp);
    //--------------------------------------------------------------------------//
    // std::cout << "___________________________________________________\n"
    //           << std::endl;
    std::cout << "Arguments read = " << stst << std::endl;
    filetype.assign(ftypetmp);
    std::cout << "Detector = " << dtmp << " (S" << std::setfill('0')
              << std::setw(width) << detNo << ")" << std::endl;
    std::cout << "Run = " << rtmp << std::endl;
    std::cout << "Vm (Mesh) = " << vm << " V" << std::endl;
    std::cout << "Vd (Drift) = " << vd << " V" << std::endl;

    //--------------------------------------------------------------------------//
    // In case the folder name contains the mylar thickness and drift gap
    // information, it can be extracted as follows:
    //       std::cout<<"Mylar thickness = "<<bthick<<std::endl
    //       std::cout<<"Drift gap = "<<dgap<<std::endl;
    //--------------------------------------------------------------------------//

    if (stst > 4)
      std::cout << "filetype = " << filetype << std::endl;
    fclose(ftmp);
    snprintf(command, sizeof(command), "rm %s\n", afile);
    tst = system(command);
  } else {
    std::cout << "Available detectors: " << MINRUN << " - " << MAXRUN
              << std::endl;
    return (-2);
  } // end of "detNo >= MINRUN && detNo <= MAXRUN"
  // ###################################################################################################################//

  // ###################################################################################################################//
  // Create the output root file
  const char *ftype = filetype.c_str(); /// add here any directory supplement
  char ofname[1000];
  strcpy(ofname, fnametmp);
  char *pch;
  pch = strstr(ofname, "raw_tree.root");
  //   snprintf(ofname, sizeof(ofname), "%s_tree.root", pch);
  strcpy(pch, "_tree.root");
#ifdef DEBUG
  strcpy(pch, "_DEBUGtree.root");
#endif
  std::cout << "output filename = " << ofname << std::endl;
  // ###################################################################################################################//

  // ###################################################################################################################//
  // Create some char arrays for histogram titles
  char mesh[1000];
  snprintf(mesh, sizeof(mesh), "%d", vm);
  char drift[1000];
  snprintf(drift, sizeof(drift), "%d", vd);
  int drft;
  sscanf(drift, "%d", &drft);

  const int MaxFiles = MAX_N_FILES;
  const int MAXSEG = 100000; // for time stamp
  // const int FRAMESIZE = 500000;
  // const int FRAMESIZE = 15000000;

  // double *ptime; // pulse time
  // ptime = new double[FRAMESIZE];
  std::vector<double> ptime; // pulse time
  ptime.reserve(FRAMESIZE);

  std::cout << "********************\n\n\n Now starting processing " << fnametmp
            << "..." << std::endl
            << std::endl;

  replaceEOL(fnametmp);
  snprintf(ifilename, sizeof(ifilename), "%s/%s", basedirname, fnametmp);
  TFile *infile = new TFile(ifilename);
  if (!infile->IsOpen()) {
    std::cout << "Failed to open \"" << ifilename
              << "\"\ncorresponding to the run " << detNo << "\nExiting..."
              << std::endl;
    return (-1);
  }

  int spoints[] = {0, 0, 0, 0};
  int evNo = 0;
  double Dt[4] = {0., 0.};
  double t0[] = {0., 0., 0., 0.};
  double gain[] = {0., 0., 0., 0.};
  double offset[] = {0., 0., 0., 0.};
  double rmax[] = {0., 0., 0., 0.};
  double rmin[] = {0., 0., 0., 0.};
  double range[] = {0., 0., 0., 0.};
  double t0o;
  double dt, odt;
  unsigned long long int epoch;
  unsigned long long int nn;
  int itrigger; // time of the trigger
  char date[4][50];
  double *amplC[4];
  int active[] = {0, 0, 0, 0};
  //   for (int i=0; i<2; i++)

  // tree name and title
  char treename[200];
  snprintf(treename, sizeof(treename), "TreeWithRawData");
  char treetitle[200];
  snprintf(treetitle, sizeof(treetitle), "Raw data tree");

  // Get the input tree
  TTree *intree = (TTree *)infile->Get(treename);
  TBranch *branch;

  // get the tree branches
  int inpoints;
  branch = intree->GetBranch("npoints");
  branch->SetAddress(&inpoints);
  int eventNo;
  branch = intree->GetBranch("eventNo");
  branch->SetAddress(&eventNo);

  branch = intree->GetBranch("t0");
  branch->SetAddress(t0);

  branch = intree->GetBranch("gain");
  branch->SetAddress(gain);
  branch = intree->GetBranch("offset");
  branch->SetAddress(offset);
  branch = intree->GetBranch("rmax");
  branch->SetAddress(rmax);
  branch = intree->GetBranch("rmin");
  branch->SetAddress(rmin);

  double idt;
  branch = intree->GetBranch("dt");
  branch->SetAddress(&idt);

  unsigned long long int iepoch = 0;
  branch = intree->GetBranch("epoch");
  branch->SetAddress(&iepoch);
  unsigned long long int inn;
  branch = intree->GetBranch("nn");
  branch->SetAddress(&inn);
  char idate[50];
  branch = intree->GetBranch("date");
  branch->SetAddress(&idate);

  // Lopp over the 4 channels to get their branches
  for (int i = 0; i < 4; i++) {
    TString channel = TString::Itoa(i + 1, 10);
    TString bname = "npoints" + channel;
    TString btype = bname + "/I";
    branch = intree->GetBranch(bname);
    if (branch == NULL) {
      continue;
    }
    active[i] = 1;
    branch->SetAddress(&spoints[i]);
    bname = "amplC" + channel;
    std::cout << bname << std::endl; // print the channel name
    btype = bname + "[npoints" + channel + "]/D";
    branch = intree->GetBranch(bname);
    amplC[i] = new double[FRAMESIZE];
    branch->SetAddress(&amplC[i][0]);
  }

  // ################################################################################//
  // Set the cache size and load the tree baskets
  intree->SetCacheSize(200000000);
  long int bufsize = intree->GetCacheSize();
  // std::cout << "Cache size = " << bufsize << endl;
  //   intree->InitializeBranchLists(true);
  intree->SetBasketSize("*", 32000);
  intree->AddBranchToCache("*");
  intree->LoadBaskets();
  bufsize = intree->GetCacheSize();
  // std::cout << "Cache size = " << bufsize << endl;
  // ################################################################################//

  // Number of events in the input tree
  const int nevents = intree->GetEntries();

  int actch = -1; // active channel
  int framesize = 0;
  double bslmax = 1.;
  double bslmin = -1.;
  const int ndivy = 8; // number of division along y, max 8, min 2
  for (int i = 0; i < 4; i++) {
    double max = 0.;
    double min = 0.;
    // if the channel is active and there is no external trigger
    if ((active[i]) && (trigger != i + 1)) {
      actch = i;

      // ################################################################################//
      // //  Before the crash line, add:
      // std::cout << "DEBUG: nevents = " << nevents << std::endl;
      // std::cout << "DEBUG: Getting entry " << nevents / 2 << std::endl;
      // std::cout << "DEBUG: intree pointer = " << intree << std::endl;
      // std::cout << "DEBUG: intree->IsOpen() = " << infile->IsOpen()
      //           << std::endl;
      // std::cout << "DEBUG: intree->GetEntries() = " << intree->GetEntries()
      //           << std::endl;
      // ################################################################################//

      intree->GetEntry(nevents / 2);

      // // Replace the crash line with:
      // std::cout << "Attempting to read entry " << nevents / 2 << "..."
      //           << std::endl;
      // try {
      //   intree->GetEntry(nevents / 2);
      //   std::cout << "Successfully read entry " << nevents / 2 << std::endl;
      // } catch (const std::exception &e) {
      //   std::cout << "Exception caught: " << e.what() << std::endl;
      // } catch (...) {
      //   std::cout << "Unknown exception caught!" << std::endl;
      // }

      // ################################################################################//

      framesize = spoints[i];
      std::cout << "\n\nFramesize = " << framesize << " points" << std::endl;
      std::cout << "Gain = " << gain[i] << " V" << std::endl;
      std::cout << "Offset = " << offset[i] << " V" << std::endl;
      std::cout << "Range Max = " << rmax[i] * mV << " mV";
      std::cout << "\t\tRange Min = " << rmin[i] * mV << "\t";
      range[i] = rmax[i] - rmin[i];
      std::cout << "Full range = " << range[i] * mV << " mV  -->  "
                << range[i] / ndivy * mV << " mV / div  --> "
                << range[i] / 256 * mV << " mV / bit" << std::endl
                << std::endl;
      bslmax = rmax[i];
      // bslmin = rmin[i] + range[i] * (6. / 8.);
      bslmin = rmin[i] + range[i] * ((8 - ndivy) / 8.); // set the min baseline
      //       std::cout<<"Corresponding to "<<endl<<endl<<endl;
      break; /// only the first active channel is used!!!!!!
    }
  }

  // Time step in ns
  dt = idt * 1e9;
  if (actch < 0) {
    std::cout
        << "No active channel found, or the only active channel is declared "
           "as external trigger !!! Exiting..."
        << std::endl;
    return -12;
  }

  long double meanoffset = (rmax[actch] + rmin[actch]) / 2.;
  std::cout << "Mean offset = " << meanoffset * mV << " mV" << std::endl
            << std::endl;
  long double vpb = range[actch] / 256.;
  //   drft=651;
  char dirname[1000];
  int runb = 0;

  snprintf(dirname, sizeof(dirname), "%s/S%03d-%02d-%d-%d", basedirname, detNo,
           runNo, vm, vd);

  if (runb)
    snprintf(dirname, sizeof(dirname), "%sb", dirname);

  if (strlen(ftype) > 1)
    snprintf(dirname, sizeof(dirname), "%s-%s", dirname, ftype);

  std::cout << "\n\n\nSearching for files at directory: " << dirname
            << std::endl
            << std::endl;

  char rfname[200];
  char detid[20];
  snprintf(detid, sizeof(detid), "S%03d", detNo);
  char runid[20];
  snprintf(runid, sizeof(runid), "%02d", runNo);
  char cangle[20];
  snprintf(cangle, sizeof(cangle), "%d", angle);

  TString rtypes(detid);
  rtypes += "-";
  rtypes += runid;
  rtypes += "-";
  rtypes += mesh;
  rtypes += "-";
  rtypes += drift;
  rtypes += "-";
  //   rtypes+=cangle;
  //   rtypes+="-";
  rtypes += rtype;

  char catt[30];

  if (runb)
    rtypes += "_b";

  TString ctypes(detid);
  ctypes += "-";
  ctypes += runid;
  ctypes += "-";
  ctypes += mesh;
  ctypes += "-";
  ctypes += drift;
  ctypes += "-";
  //   ctypes+=cangle;
  //   ctypes+="-";
  ctypes += ftype;

  snprintf(ofilename, sizeof(ofilename), "%s/%s", outdirname, ofname);
  TFile *ofile = new TFile(ofilename, "RECREATE");

  double binscale = 1.;
  if (framesize < 1000)
    binscale = 1.0;
  int nbins = 256 / 8 * ndivy - 1;

  char htitle[100];

  TH1F *hbsl, *hbsl0;
  TH1D *hbslall, *hbsl0all;

  snprintf(fname2, sizeof(fname2), "Signal_bsl");
  hbsl = new TH1F(fname2, fname2, nbins, bslmin, bslmax);
  hbsl->GetXaxis()->SetTitle("[V]");
  snprintf(fname2, sizeof(fname2), "Signal_bsl_tot");
  snprintf(htitle, sizeof(htitle), "Signal baseline from all events");
  hbslall = new TH1D(fname2, htitle, nbins, bslmin, bslmax);
  hbslall->GetXaxis()->SetTitle("[V]");

  snprintf(fname2, sizeof(fname2), "Subtracted_bsl");
  hbsl0 = new TH1F(fname2, fname2, (int)nbins * binscale, -range[actch] / 8.,
                   range[actch] / 8.);
  hbsl0->GetXaxis()->SetTitle("[V]");
  snprintf(fname2, sizeof(fname2), "Subtracted_bsl_tot");
  hbsl0all = new TH1D(fname2, fname2, (int)nbins * binscale, -range[actch] / 8.,
                      range[actch] / 8.);
  hbsl0all->GetXaxis()->SetTitle("[V]");

  double *amplCo = NULL;
  for (int i = 0; i < 2; i++)
    amplCo = new double[FRAMESIZE];

  double *amplSum; // in this case (lina4) is just ampl - baseline
  amplSum = new double[FRAMESIZE];

  double *tbase = new double[FRAMESIZE];
  for (int i = 0; i < FRAMESIZE; i++)
    tbase[i] = i * 4.;

  double *EVTIME = new double[nevents];
  double *BSL = new double[nevents];
  double *RMS = new double[nevents];
  double *sBSL = new double[nevents];
  double *sRMS = new double[nevents];

  int maxpoints = 0;

  TTree *outtree;

  snprintf(treename, sizeof(treename), "RawDataTree");
  snprintf(treetitle, sizeof(treetitle), "Raw data tree");
  outtree = new TTree(treename, treetitle);

  outtree->Branch("eventNo", &evNo, "eventNo/I");
  outtree->Branch("dt", &odt, "dt/D");
  outtree->Branch("epoch", &epoch, "epoch/l");
  outtree->Branch("nn", &nn, "nn/l");
  // tree->Branch("date", &date, "date/C");
  outtree->Branch("t0", &t0o, "t0/D");
  outtree->Branch("itrigger", &itrigger, "itrigger/I");
  double ttrig;
  outtree->Branch("ttrig", &ttrig, "ttrig/D");
  int fitstatus1 = 0, fitstatus2 = 0;
  outtree->Branch("fitstatus1", &fitstatus1, "fitstatus1/I");
  outtree->Branch("fitstatus2", &fitstatus2, "fitstatus2/I");
  double frangemax, frangemin;
  outtree->Branch("rmax", &frangemax, "rmax/D");
  outtree->Branch("rmin", &frangemin, "rmin/D");

  outtree->Branch("sumpoints", &maxpoints, "sumpoints/I");
  outtree->Branch("amplSum", amplSum, "amplSum[sumpoints]/D");
  //   outtree->Branch("amplSum2", amplSum[1], "amplSum2[sumpoints]/D");
  double bslsum = 0.;
  double rmssum = 0.1;
  outtree->Branch("bslSum", &bslsum, "bslSum/D");
  outtree->Branch("rmsSum", &rmssum, "rmsSum/D");

  std::cout << "\n\n Tree is ready!" << std::endl;

#ifdef DEBUG
  //    TCanvas *cbsl = new TCanvas("baseline"+rtypes,"baseline
  //    "+rtypes,1600*4/4,1200*3/4);
  TCanvas *cbsl = new TCanvas("baseline" + rtypes, "baseline " + rtypes,
                              1600 * 4 / 4, 1200 * 2 / 4);
  cbsl->Divide(2, 1);
#endif

  TTimeStamp *tstamp[2][MAXSEG];
  double tshift[2] = {0., 0.};
  int dd, mm, yy, hour, min, sec;
  char month[20];
  int nseg;

  double bsl[2] = {0., 0.};
  double rms[2] = {0., 0.};

  snprintf(fname2, sizeof(fname2), "gausC");
  TF1 *fbsl = new TF1(fname2, "gaus", -1., 1.);
  fbsl->SetNpx(2000);
  snprintf(fname2, sizeof(fname2), "gaus0");
  TF1 *fbsl0 = new TF1(fname2, "gaus", -1., 1.);
  fbsl0->SetNpx(2000);

  snprintf(fname2, sizeof(fname2), "gausCtot");
  TF1 *fbslall = new TF1(fname2, "gaus", -1., 1.);
  fbslall->SetNpx(2000);
  snprintf(fname2, sizeof(fname2), "gaus0tot");
  TF1 *fbsl0all = new TF1(fname2, "gaus", -1., 1.);
  fbsl0all->SetNpx(2000);

  gStyle->SetOptFit(11);
  gStyle->SetOptStat(1100);

  int ngpoints = 0;
  int eventS = 0;
  int eventF = nevents;
  // #ifdef DEBUG
  //     eventS = 16438;
  //     std::cout<<"Give event No to debug: ";
  //     cin>>eventS;
  //     eventF = eventS;
  // #endif

  intree->GetEntry(0);
  intree->GetEntry(nevents - 1);
  std::cout << "Loading tree baskets in virtual memory" << std::endl;
  int nbaskets = intree->LoadBaskets(4000000000);
  std::cout << "Loaded " << nbaskets << " baskets " << std::endl;

  double tdt = -1.;
  for (int nEv = eventS; nEv < nevents && nEv < eventF + 1; nEv += 1) {
#ifdef DEBUG
    //     eventS = 16438;
    std::cout << "Give event No to debug: ";
    cin >> nEv;
    if (nEv < 0)
      return (-1);
//     eventF = eventS;
#endif
    //     std::cout<<"ad sfsda fsda fsda af "<<nEv <<endl;
    intree->GetEntry(nEv);
    odt = idt * 1e+9;
    evNo = eventNo;
    frangemax = rmax[actch];
    frangemin = rmin[actch];

    long double tmpx;
    double tmpy;

    ///  repeat the ci loop in order to read the data. Read data for each
    ///  segment of each channel. Fill the tree every time all 4 channels of the
    ///  same segment is read. Note that ALL 4 FILES REMAIN OPEN and the pointer
    ///  is at the end at each segment!!!

    /// loop over all channels of the same segment !!!

    if (tdt != idt) {
      for (int k = 0; k < inpoints + 10; k++) /// time array
        //  ptime[k] = k * idt;                   // tmpx*1e9;
        ptime.push_back(k * idt); // tmpx*1e9;
      dt = idt * 1e9;             // ns
      tdt = idt;
    }
    epoch = iepoch;
    nn = inn;
    double epochF = (1. * iepoch + inn * 1e-9);

    itrigger = -100;

    hbsl->Reset("");

    if (trigger > 0) {
      int trgch = trigger - 1;
      for (int k = 0; k < spoints[trgch] - 2; k++) /// processing trigger (ci=0)
      {
        if (amplC[trgch][k] > 1.) {
          itrigger = k;
          break;
        }
      }
    }
    ttrig = itrigger * dt;

    long double sumsig = 0.;
    long double diff = 0.;
    for (int k = 0; k < spoints[actch];
         k++) /// read the segment data points (ci=1)
    {
      amplCo[k] = amplC[actch][k]; /// storing original waveform
      hbsl->Fill(amplCo[k]);       /// event baselin calculation
      sumsig += (amplCo[k] - meanoffset);
#ifdef DEBUGMSG
      diff = (amplCo[k] - meanoffset) + diff;
      std::cout << k << " = " << setw(15) << amplCo[k] << " --> "
                << setprecision(10) << setw(15) << (amplCo[k] - meanoffset)
                << " mV  --> sum = " << sumsig << "  diff = " << diff
                << std::endl;
      diff = (amplCo[k] - meanoffset);
#endif
    }

#ifdef DEBUG
#ifdef DEBUGMSG
    std::cout << "Sumsig = " << sumsig << " --> " << sumsig / 502
              << "   gain = " << gain[actch]
              << " --> gain/2 = " << gain[actch] / 2. << std::endl;
    std::cout << " V/bit = " << vpb << " --> V/bit/2 = " << vpb / 2.
              << std::endl;
#endif
    cbsl->cd(1);
    TGraph *gr = new TGraph(spoints[actch], tbase, amplCo);
    gr->Draw("a*l");
//       return -13;
#endif
#ifdef DEBUG
    cbsl->cd(2);
    gStyle->SetOptStat(1111110);
    hbsl->Draw();
    cbsl->Modified();
    cbsl->Update();
    std::cout << "WTF?" << std::endl;
#endif

    /// check if histo is empty. In that case either signal completely out of
    /// range or problematic event. For this reason we check the sumsig. If it
    /// is 0 or very close to 0, reject completely the event!!!!

    int hentries = hbsl->Integral();
    if (hentries == 0) {
      std::cout
          << "_____________________________________________________________"
          << std::endl;
      std::cout << "Event " << evNo
                << " has no point in the range of the baseline histo with "
                << hbsl->GetEntries() << " entries!" << std::endl;
      std::cout << "There are " << hbsl->GetBinContent(0) << " underflow and "
                << hbsl->GetBinContent(nbins + 1) << " points!" << std::endl;
      std::cout << "Sumsig for " << spoints[actch] << " is " << sumsig * mV
                << " mV with a gain of " << vpb * mV << " mV/bit" << std::endl;
      if (fabs(sumsig) < vpb / 10.) {
        std::cout << "The event is REJECTED!!!" << std::endl;
        std::cout
            << "-------------------------------------------------------------"
            << std::endl;
        continue;
      } else {
        int ovfl = hbsl->GetBinContent(nbins + 1);
        int unfl = hbsl->GetBinContent(0);
        int hnp = spoints[actch] / 2;
        if (ovfl >= hnp - 1 && unfl >= hnp - 1 && fabs(ovfl - unfl) <= 1) {
          std::cout
              << "It seems that there might be one point within the range of "
                 "the scope that makes the frame inegral non-zero."
              << std::endl;
          std::cout
              << "However, this point is out of the bsl histo range, and the "
                 "overrange / underange points are shared equally."
              << std::endl;
          std::cout << "The event is REJECTED!!!" << std::endl;
          std::cout
              << "-------------------------------------------------------------"
              << std::endl;
          continue;
        } else {
          std::cout
              << "The event is registered, thought the bsl histo integral is "
                 "zero!"
              << std::endl;
          std::cout
              << "-------------------------------------------------------------"
              << std::endl;
        }
      }
    } else if (hentries <= 20) {
      std::cout
          << "_____________________________________________________________"
          << std::endl;
      std::cout
          << "Event " << evNo
          << " has very few points point in the range of the baseline histo "
             "with "
          << hbsl->GetEntries() << " entries!" << std::endl;
      std::cout << "There are " << hbsl->GetBinContent(0) << " underflow and "
                << hbsl->GetBinContent(nbins + 1) << " points!" << std::endl;
      std::cout << "Sumsig for " << spoints[actch] << " is " << sumsig * mV
                << " mV with a gain of " << vpb * mV << " mV/bit" << std::endl;
    }

    t0o = t0[actch];

    if (1) // || (framesize<10000 && nEv<500) )
    {
      FilterHisto(hbsl, 0.03);

      int maxb = hbsl->GetMaximumBin();
      double maxAmp = hbsl->GetBinContent(maxb);
      double meant = hbsl->GetBinLowEdge(maxb);
      double rmst = hbsl->GetRMS();
      double var = hbsl->GetBinWidth(maxb);

      if (rmst > 0.005)
        rmst = 0.005;

      fbsl->SetParLimits(0, 0.5 * maxAmp, 2. * maxAmp);
      fbsl->SetParLimits(1, meant - var, meant + 2 * var);
      fbsl->SetParLimits(2, vpb / 2., 10. * vpb);
      fbsl->SetParameter(0, maxAmp);
      fbsl->SetParameter(2, rmst);
      fbsl->SetParameter(1, meant + var / 2.);
      // // 	cbsl->cd(1);

      snprintf(fname2, sizeof(fname2), "event %d baseline", evNo);
      hbsl->SetTitle(fname2);
      fitstatus1 =
          hbsl->Fit(fbsl, "BQN", "", meant - 3 * rmst, meant + 3 * rmst);

      bsl[1] = fbsl->GetParameter(1);
      rms[1] = fbsl->GetParameter(2);

      EVTIME[ngpoints] = epochF;
      BSL[ngpoints] = fbsl->GetParameter(1) * 1000.;
      RMS[ngpoints] = fbsl->GetParameter(2) * 1000.;
      sBSL[ngpoints] = fbsl->GetParError(1) * 1000.;
      sRMS[ngpoints] = fbsl->GetParError(2) * 1000.;
      ngpoints++;
      // // 	cbsl->Modified();
      // // 	cbsl->Update();

      hbslall->Add(hbsl);

      bsl[0] = 0.0;
      rms[0] = 0.0;
    }

    maxpoints = inpoints;

    if (maxpoints <= 0)
      continue;

    int tst = SubtractBaseline(maxpoints, amplCo, amplSum,
                               bsl[1]); // amplCo corresponds to signal (initial
                                        // C2) and is place in amplSum[0]

    // //       hbsl0->Reset("");  /// this is the subtracted event baseline
    // !!!!
    // //
    // //       for (int k=0; k<maxpoints; k++ ) /// read the segment data
    // points
    // //       {
    // // 	if (fabs(amplSum[k])<0.005)
    // // 	hbsl0->Fill(amplSum[k]);
    // //       }
    // //
    // //       /// calculate baseline
    // //       int maxbt = hbsl0->GetMaximumBin();
    // //       double baxby = hbsl0->GetBinContent(maxbt);
    // //       double meantt = hbsl0->GetBinLowEdge(maxbt);
    // //       double vart = hbsl0->GetBinWidth(maxbt);
    // //       double rmstt = hbsl0->GetRMS();
    // //       // 	  std::cout<<"meant C"<<ci+1<<" = "<<meant<<endl;
    // //
    // //       if (rmstt<0.005) /// don't accumulate wierd events in total
    // baseline histo;
    // //       hbsl0all->Add(hbsl0);

    /// fit and draw the basline-subtructed signal new baseline (must be ~0)

    // //       fbsl0->SetParameter(0,baxby+vart/2.);
    // //       fbsl0->SetParameter(2,rmstt);
    // // //       fbsl[ci]->SetParameter(2,0.002);
    // //       fbsl0->SetParameter(1,meantt);
    // //
    // //       fbsl0->SetParLimits(0,0.5*baxby,2.*baxby);
    // //       fbsl0->SetParLimits(1,meantt-vart,meantt+2*vart);
    // //       fbsl0->SetParLimits(2,vpb/2.,10.*vpb);
    // //
    // //       cbsl->cd(2);
    // //       snprintf(fname2, sizeof(fname2), "bsl-corrected event %d
    // baseline", evNo );
    // //       hbsl0->SetTitle(fname2);
    // //       fitstatus2 =
    // hbsl0->Fit(fbsl0,"BQ","",meantt-0.0075,meantt+0.0075);
    // //       bslsum=fbsl0->GetParameter(1);
    // //       rmssum=fbsl0->GetParameter(2);
    // //       hbsl0->Draw("E0");

    if (fitstatus1 != 0) //|| fitstatus2 || 0)
    {
      std::cout << "evNo = " << evNo << "\t  f1 = " << fitstatus1
                << " f2 = " << fitstatus2;
      std::cout << " bsl = " << bsl[1] << " rms = " << rms[1];
      std::cout << " bsl0 = " << bslsum << " rms0 = " << rmssum << std::endl;
      std::cout << " SumSig = " << sumsig
                << " histogram Integral = " << hentries << std::endl;
      // //         cbsl->Modified();
      // //         cbsl->Update();
      // 	return (fitstatus1);
    }

//       hbsl0->GetXaxis()->SetRangeUser(floor(bslsum*100.+0.5)/100-0.02,floor(bslsum*100.+0.5)/100+0.02);
#ifdef DEBUG
    cbsl->Modified();
    cbsl->Update();
    continue;
    return -4;
#endif
    /// fit and draw the acumulated baseline of the signals
    if ((nEv == eventF - 1) || (framesize < 10000 && nEv % 50 == 0)) {
      // //         cbsl->cd(3);
      double maxbt0 = hbslall->GetMaximumBin();
      double maxv = hbslall->GetBinContent(maxbt0);
      double meantt0 = hbslall->GetBinLowEdge(maxbt0);
      double rmstt0 = hbslall->GetRMS();

      fbslall->SetParameter(0, maxv);
      fbslall->SetParameter(1, meantt0);
      fbslall->SetParameter(2, rmstt0);
      fbslall->SetParLimits(0, 0.5 * maxv, 1.5 * maxv);
      fbslall->SetParLimits(1, bsl[1] - 0.01, bsl[1] + 0.01);
      fbslall->SetParLimits(2, 0.0005, 0.03);

      hbslall->Fit(fbslall, "BQN", "", bsl[1] - 0.01, bsl[1] + 0.01);
      // // 	hbslall->Draw("E0");
      bsl[1] = fbslall->GetParameter(1);
      // // 	cbsl->Modified();
      // // 	cbsl->Update();

      // // 	if((nEv==eventF-1) || evNo % 500 == 0)
      // // 	{
      // // 	  cbsl->cd(4);
      // // 	  fbsl0all->SetParameter(2,0.002);
      // // 	  fbsl0all->SetParameter(1,0.000);
      // // 	  double maxamp = hbsl0all->GetMaximum();
      // // 	  fbsl0all->SetParameter(0,maxamp);
      // // 	  fbsl0all->SetParLimits(0,0.5*maxamp,1.5*maxamp);
      // // 	  fbsl0all->SetParLimits(1,-0.005,0.005);
      // // 	  fbsl0all->SetParLimits(2,0.0002,0.005);
      // //
      // // 	  hbsl0all->Fit(fbsl0all,"BQ","",-0.0075, 0.0075);
      // //     //       hbsl0all->GetXaxis()->SetRangeUser(-0.01,0.01);
      // // 	  hbsl0all->Draw("E0");
      // // 	  cbsl->Modified();
      // // 	  cbsl->Update();
      // // 	}
    }

    if (nEv == 20) {
      outtree->OptimizeBaskets(100000000, 1.1, "d");
    }
    outtree->Fill();

    if (framesize > 20000) {
      if ((nEv + 1) % 50 == 0)
        std::cout << "Processed " << nEv + 1 << " events (evNo = " << evNo
                  << ") out of " << nevents << std::endl;

      if (nevents % 200 == 0)
        ofile->Write("", TObject::kOverwrite);
    } else {
      if ((nEv + 1) % 1000 == 0) {
        std::cout << "Processed " << nEv + 1 << " events (evNo = " << evNo
                  << ") out of " << nevents << std::endl;
        TTimeStamp *tstamp = new TTimeStamp();
        tstamp->Set();
        std::cout << "\t\tprocessing time : " << tstamp->AsString("l")
                  << std::endl;
        // 	  if ((nEv+1) == 100)
        // 	  {
        // 	    outtree->OptimizeBaskets();
        // 	    intree->OptimizeBaskets();
        // 	  }
      }

      if (nevents % 1000 == 0)
        ofile->Write("", TObject::kOverwrite);
    }
  }

  TCanvas *ebsl =
      new TCanvas("baselineEvolution" + rtypes, "Baseline Evolution " + rtypes);
  TGraphErrors *grBSL = new TGraphErrors(ngpoints, EVTIME, BSL, 0, sBSL);
  grBSL->SetTitle("BASELINE " + rtypes);
  grBSL->SetLineColor(4);
  grBSL->Draw("AP");
  TH1F *h1 = grBSL->GetHistogram();
  h1->SetBins(100000, EVTIME[0] - 0.05 * (EVTIME[ngpoints - 1] - EVTIME[0]),
              EVTIME[ngpoints - 1] + 0.05 * (EVTIME[ngpoints - 1] - EVTIME[0]));
  grBSL->GetXaxis()->SetTimeDisplay(1);
  grBSL->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d/%m/%y}");
  grBSL->GetXaxis()->SetLabelOffset(0.04);
  grBSL->GetYaxis()->SetTitle("[mV]");
  //   grBSL->SetMinimum(0);
  grBSL->Write("baselineEvolution");

  TCanvas *erms = new TCanvas("baselineRMSEvolution" + rtypes,
                              "Baseline RMS Evolution " + rtypes);
  TGraphErrors *grRMS = new TGraphErrors(ngpoints, EVTIME, RMS, 0, sRMS);
  grRMS->SetTitle("RMS " + rtypes);
  grRMS->SetLineColor(2);
  grRMS->Draw("AP");
  TH1F *h2 = grRMS->GetHistogram();
  h2->SetBins(100000, EVTIME[0] - 0.05 * (EVTIME[ngpoints - 1] - EVTIME[0]),
              EVTIME[ngpoints - 1] + 0.05 * (EVTIME[ngpoints - 1] - EVTIME[0]));
  grRMS->GetXaxis()->SetTimeDisplay(1);
  grRMS->GetXaxis()->SetLabelOffset(0.04);
  grRMS->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d/%m/%y}");
  grRMS->GetYaxis()->SetTitle("[mV]");
  //   grRMS->SetMinimum(grRMS->GetMinimum()*0.7);
  //   grRMS->SetMaximum(grRMS->GetMaximum()*1.3);
  grRMS->Write("rslRMSEvolution");

  TCanvas *ebslrms = new TCanvas("BSL+RMS_Evolution" + rtypes,
                                 "Baseline & RMS Evolution " + rtypes);
  TGraphErrors *grBSLRMS = new TGraphErrors(ngpoints, EVTIME, BSL, 0, RMS);
  grBSLRMS->SetTitle("BSLRMS " + rtypes);
  grBSLRMS->SetLineColor(4);
  grBSLRMS->Draw("AP");
  TH1F *h3 = grBSLRMS->GetHistogram();
  h3->SetBins(100000, EVTIME[0] - 0.05 * (EVTIME[ngpoints - 1] - EVTIME[0]),
              EVTIME[ngpoints - 1] + 0.05 * (EVTIME[ngpoints - 1] - EVTIME[0]));
  //   TH1D *h1 = grBSL->GetHistogram();
  grBSLRMS->GetXaxis()->SetTimeDisplay(1);
  grBSLRMS->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d/%m/%y}");
  grBSLRMS->GetXaxis()->SetLabelOffset(0.04);
  grBSLRMS->GetYaxis()->SetTitle("[mV]");
  //   grBSLRMS->SetMinimum(0);
  grBSLRMS->Write("BSLandRMSevolution");

  //   gStyle->SetOptFit(110);
  TCanvas *ctotbsl = new TCanvas("Pedestals" + rtypes, "Pedestals " + rtypes,
                                 1600 * 4 / 4, 1200 * 3 / 4);
  ctotbsl->Divide(2, 2);
  //   ctotbsl->UseCurrentStyle();
  ctotbsl->cd(1);
  hbslall->Draw();
  ctotbsl->cd(2);
  grBSL->Draw("AP");
  ctotbsl->cd(3);
  grRMS->Draw("AP");
  ctotbsl->cd(4);
  grBSLRMS->Draw("AP");
  ctotbsl->Modified();
  ctotbsl->Update();
  ctotbsl->Write();

  char tmpdir[500];
  snprintf(tmpdir, sizeof(tmpdir), "%s", gSystem->pwd());
  std::cout << "Actual directory: " << tmpdir << std::endl;
  char plotdirname[500];
  snprintf(plotdirname, sizeof(plotdirname), "%s/S%03d/", PLOTDIR,
           detNo); //,abs(threshold));

  gSystem->mkdir(plotdirname, kTRUE);
  gSystem->ChangeDirectory(plotdirname);
  ctotbsl->SaveAs(".pdf");
  ctotbsl->SaveAs(".png");
  gSystem->ChangeDirectory("./../..");
  gSystem->ChangeDirectory(tmpdir);

  std::cout << "End of file processing.\n"
            << evNo << " events were found" << std::endl;
  ofile->Write("", TObject::kOverwrite);

  return 0;
}
// enf of MakeTreefromRawTreeProduction
// ###########################################################################################################//
