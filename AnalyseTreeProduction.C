#include <TFile.h>
#include <TH1.h>
#include <TMath.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>

// #include <TH1F.h>
// #include <TH2F.h>
// #include <TH2D.h>
// #include <TH3F.h>
#include "MyFunctions.C"
#include <TCanvas.h>
#include <TExec.h>
#include <TF1.h>
#include <TGaxis.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TPaveStats.h>
#include <TPaveText.h>
#include <TString.h>
#include <TStyle.h>
#include <TTimeStamp.h>
#include <sstream>

#include <filesystem>

namespace fs = std::filesystem;

// #include <iomanip>

// void replaceEOL(char *fnames)
// {
//     int nchar=0;
//     while (fnames[nchar]!='\n')
//     {
//       nchar++;
//     }
//     fnames[nchar]='\0';
//
// }

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
// nTh- neutron threshold in mV
// threshold - amplitude threshold in mV
// inverse - 1 for negative pulses, 1 for positive
int AnalyseTreeProduction(int detNo = 3, int runNo = 1, int draw = 0,
                          int saveWF = 0, double threshold = 200.0,
                          double nTh = 300.0, int inverse = 1, int exttrig = 0,
                          int nRun = 1, int bkg = 0, string filetype = "") {
  gROOT->LoadMacro("MyFunctions.C");

  char particle[20];
  if (bkg == 1)
    sprintf(particle, "bkg");
  else if (nRun == 1)
    sprintf(particle, "neutron");
  else
    sprintf(particle, "gamma");

  int vm = 500;
  int vd = 750;

  float bthick = 0.125, dgap = 1.7;
  int angle = 0;
  if (threshold > 0) // always negative
    threshold *= -1;
  if (nTh > 0)
    nTh *= -1;

  nTh = fabs(nTh);
  threshold *= 1e-3;
  nTh *= 1e-3;

  if (fabs(nTh) < fabs(threshold))
    nTh = fabs(threshold);

  char dirname[1000];
  char anadirname[1000];
  char command[500];
  char fname[500];
  char ofname[500];
  char fname2[500];
  char fnames[10000][200];
  char fnames2[10000][200];

  int runb = 0;
  char basedirname[1000];

  char afile[500];
  char ftypetmp[500];
  char fnametmp[500];

  sprintf(basedirname, "%s", OUTDIRNAME);
  // // //
  if (detNo > MINRUN && detNo <= MAXRUN) {
    sprintf(afile, "%s/tmpfile.tmp", basedirname);
    FILE *ftmp = fopen(afile, "w");
    if (ftmp == NULL) {
      std::cout << afile << " can not be created. Probablly the directory '"
                << basedirname << "' does not exit. Exiting..." << std::endl;
      exit(-12);
    }
    fclose(ftmp);
    sprintf(
        command,
        "cd %s\nls -d S%03d-%02d-*%s*_PRODUCTION_tree.root > %s 2>/dev/null",
        basedirname, detNo, runNo, filetype.c_str(), afile);
    int tst = system(command);
    //    std::cout<<command<< std::endl<<"returned: "<<tst<< std::endl;
    if (tst != 0) {
      std::cout << command << std::endl << "returned: " << tst << std::endl;
      std::cout << "Probably the tree of run No " << detNo
                << " was not found in directory " << basedirname << std::endl
                << "Exiting..." << std::endl;
      return tst;
    }
    ftmp = fopen(afile, "r");
    if (fgets(fnametmp, 200, ftmp) == NULL) {
      std::cout << "Failed to read the filename for run " << detNo << " at "
                << basedirname << std::endl
                << "Exiting..." << std::endl;
      return -2;
    }
    int rtmp, dtmp;
    strcpy(ftypetmp, "");
    int stst = sscanf(fnametmp, "S%03d-%02d-%d-%d%30[^ /,\n\t]", &dtmp, &rtmp,
                      &vm, &vd, ftypetmp);
    std::cout << "arguments read = " << stst << std::endl;
    filetype.assign(ftypetmp);
    std::cout << "detector = " << dtmp << " (S" << detNo << ")" << std::endl;
    std::cout << "run = " << rtmp << std::endl;
    std::cout << "Vm = " << vm << std::endl;
    std::cout << "Vd = " << vd << std::endl;
    std::cout << "Mylar thickness = " << bthick << std::endl;
    std::cout << "Drift gap = " << dgap << std::endl;
    if (stst > 4)
      std::cout << "run type = " << filetype << std::endl;
    fclose(ftmp);
    sprintf(command, "rm %s\n", afile);
    tst = system(command);
  } else {
    std::cout << "Available detectors: " << MINRUN << " - " << MAXRUN
              << std::endl;
    return (-2);
  }

  int dv = vd - vm;

  RUNPAR *runpar = new RUNPAR;
  runpar->detNo = detNo;
  runpar->runNo = runNo;

  const char *cftype = filetype.c_str(); /// add here any directory supplement
  char ftype[1000];
  strcpy(ftype, cftype);
  char *pch;
  pch = strstr(ftype, "_PRODUCTION_tree.root");
  //   sprintf(ofname,"%s_tree.root",pch);
  *pch = '\0';
  std::cout << "fyletype = " << ftype << std::endl;

  char rtype[1000];
  strcpy(rtype, RTYPE);
  char mesh[1000];
  sprintf(mesh, "%d", vm);
  char drift[1000];
  sprintf(drift, "%d", vd);

  int drft;
  sscanf(drift, "%d", &drft);

  gStyle->SetLabelSize(0.045, "X");
  gStyle->SetLabelSize(0.045, "Y");
  gStyle->SetLabelFont(132, "X");
  gStyle->SetLabelFont(132, "Y");
  gStyle->SetTitleSize(0.045, "X");
  gStyle->SetTitleSize(0.045, "Y");
  gStyle->SetTitleFont(22, "X");
  gStyle->SetTitleFont(22, "Y");
  gStyle->SetOptStat(1001111);
  gStyle->SetNdivisions(507);

  std::cout << "********************\n\n\n Starting now...\n\n" << std::endl;

  int position = strlen(basedirname);
  sprintf(dirname, "%s/S%03d-%02d-%d-%d", basedirname, detNo, runNo, vm, vd);
  //   sprintf(anadirname,"%s/anaTrees/S%03d-%d-%d-%3.1f-%3.1f",basedirname,detNo,vm,vd,bthick,dgap);
  sprintf(anadirname, "%s/S%03d-%02d-%d-%d", OUTDIRNAME, detNo, runNo, vm, vd);

  if (runb)
    sprintf(dirname, "%sb", dirname);

  if (strlen(ftype) > 1)
    sprintf(dirname, "%s-%s", dirname, ftype);

  std::cout << "\n\n\nWorking directory: " << dirname << std::endl << std::endl;

  string linein, linein2;
  char cline[1000];
  char cline2[1000];

  char rfname[200];

  double pScale = 0.02; ///  dt in microsec - to be clarified!!!
  double dt = 4.;       // 4 ns

  char detid[20];
  sprintf(detid, "S%03d", detNo);
  char runid[20];
  sprintf(runid, "%02d", runNo);

  char cthreshold[20];
  sprintf(cthreshold, "%.1fmV", fabs(threshold * 1000.));
  char cnthreshold[20];
  sprintf(cnthreshold, "%.1fmV", fabs(nTh * 1000.));

  TString rtypes(detid);
  rtypes += "_";
  rtypes += runid;
  rtypes += "_";
  rtypes += mesh;
  rtypes += "_";
  rtypes += drift;
  rtypes += "_thAmpl";
  rtypes += cthreshold;
  rtypes += "_thNeutrons";
  rtypes += cnthreshold;

  char catt[30];

  if (runb)
    rtypes += "_b";

  TString ctypes(detid);
  ctypes += "_";
  ctypes += runid;
  ctypes += "_";
  ctypes += mesh;
  ctypes += "_";
  ctypes += drift;
  ctypes += "_thAmpl";
  ctypes += cthreshold;
  ctypes += "_thNeutrons";
  ctypes += cnthreshold;
  // ctypes+="-";
  // ctypes+=ftype;

  /// open root tree file
  //  sprintf(fname,"%s%s-Vm%s-Vd%s.root",dirname,rtype,mesh,drift);
  sprintf(fname, "%s_%s_tree.root", dirname, rtype);

  std::cout << "Input filename: " << fname << std::endl;

  sprintf(ofname, "%s_%s_%s_treeParam_aTh%.1fmV_nTh%.1fmV.root", anadirname,
          rtype, particle, fabs(threshold * 1000.), fabs(nTh * 1000.));
  std::cout << "Output filename: " << ofname << std::endl;

  TFile *ifile = new TFile(fname);

  if (!ifile->IsOpen()) {
    std::cout << "Attention! File \n"
              << fname << "\ncorresponding to the run " << runid
              << " Does not exist!!!" << std::endl;
    return (-3);
  }
  const int ARRAYSIZE = 15000000;
  const int MAXTRIG = 100000;
  IPARAM *ipar, *ipars[2], *spar;
  for (int i = 0; i < 2; i++)
    ipars[i] = new IPARAM[MAXTRIG];
  spar = new IPARAM[MAXTRIG];
  ipar = new IPARAM;

  //   int spoints = 0;
  int maxpoints = 0;
  //   int spoints2=0,nsegments=0;
  int evNo;
  double Dt = 0.;
  double t0 = 0.;
  unsigned long long int epoch;
  unsigned long long int nn;
  int itrigger; // time of the trigger
  double ttrig; // time of the trigger in seconds
  char date[4][50];
  double *amplC;
  //   for (int i=0; i<2; i++)
  amplC = new double[ARRAYSIZE];

  double *samplC;
  //   for (int i=0; i<2; i++)
  samplC = new double[ARRAYSIZE];

  double *idamplC;
  //   for (int i=0; i<2; i++)
  idamplC = new double[ARRAYSIZE];

  double *amplSum;
  amplSum = new double[ARRAYSIZE];

  double *ptime;
  ptime = new double[ARRAYSIZE];
  double *dampl, *dsampl, *idampl, *iampl, *sampl, *sampl2;
  dampl = new double[ARRAYSIZE];
  dsampl = new double[ARRAYSIZE];
  idampl = new double[ARRAYSIZE];
  iampl = new double[ARRAYSIZE];
  sampl = new double[ARRAYSIZE];
  sampl2 = new double[ARRAYSIZE];

  int ptrig[4];
  TTree *tree;
  char treename[200];
  char treetitle[200];
  TBranch *branch;
  int nfiles[4] = {0, 0, 0, 0};

  sprintf(treename, "RawDataTree");
  sprintf(treetitle, "Raw data tree");
  tree = (TTree *)ifile->Get(treename);

  branch = tree->GetBranch("eventNo");
  branch->SetAddress(&evNo);
  branch = tree->GetBranch("dt");
  branch->SetAddress(&dt);
  branch = tree->GetBranch("epoch");
  branch->SetAddress(&epoch);
  branch = tree->GetBranch("nn");
  branch->SetAddress(&nn);
  branch = tree->GetBranch("t0");
  branch->SetAddress(&t0);
  branch = tree->GetBranch("itrigger");
  branch->SetAddress(&itrigger);
  branch = tree->GetBranch("ttrig");
  branch->SetAddress(&ttrig);

  branch = tree->GetBranch("sumpoints");
  branch->SetAddress(&maxpoints);
  double bslsum = 0.;
  branch = tree->GetBranch("bslSum");
  branch->SetAddress(&bslsum);
  double rmssum = 0.;
  branch = tree->GetBranch("rmsSum");
  branch->SetAddress(&rmssum);

  double frmax = 1.;
  branch = tree->GetBranch("rmax");
  branch->SetAddress(&frmax);
  double frmin = 1.;
  branch = tree->GetBranch("rmin");
  branch->SetAddress(&frmin);

  branch = tree->GetBranch("amplSum");
  branch->SetAddress(amplSum);

  int fitstatus1, fitstatus2;
  branch = tree->GetBranch("fitstatus1");
  branch->SetAddress(&fitstatus1);
  branch = tree->GetBranch("fitstatus2");
  branch->SetAddress(&fitstatus2);

  int nevents = tree->GetEntries();
  std::cout << "Found " << nevents << " events for the tree " << std::endl;

  double bsl = 0.;

  char tmpdir[500];
  sprintf(tmpdir, "%s", gSystem->pwd());
  std::cout << "Actual directory: " << tmpdir << std::endl;

  char plotdirname[500];
  sprintf(plotdirname, "%s/S%03d/", PLOTDIR, detNo); //,abs(threshold));
  gSystem->mkdir(plotdirname, kTRUE);
  gSystem->ChangeDirectory(plotdirname);
  gSystem->ChangeDirectory(tmpdir);

  char allplotdirname[500];
  sprintf(allplotdirname, "%s/moreplots/", plotdirname); //,abs(threshold));

  gSystem->mkdir(allplotdirname, kTRUE);
  gSystem->ChangeDirectory(allplotdirname);
  ///  prepare time array
  ptime[0] = 0;
  tree->GetEntry(1);

  for (int i = 1; i < 3000000; i++) {
    ptime[i] = ptime[i - 1] + dt; // dt in ns--> ptime in ns...
  }

  std::cout << " dt = " << dt << std::endl;

  double nThCh = fabs(172.08 * nTh);
  nThCh = fabs(200.0 * nTh);
  std::cout << "Neutron thresholds:   Amplitude = " << nTh * 1000.
            << " mV ,  Charge = " << nThCh << " nC \n"
            << std::endl;

  TGraph *waveform;
  TGraph *derivative;
  TGraph *integralh;
  char cname[100];

  /// follows a simple event display
  TCanvas *ecanv, *dcanv, *icanv;
  if (draw) {
    ecanv = new TCanvas("EventDisplay", "Event display");
    dcanv = new TCanvas("DerivativeDisplay", "Derivative display");
    icanv = new TCanvas("IntegrqalDisplay", "Integral display");
  }
  int eventNo = 0;

  int clr[40] = {kRed, kBlue, kGreen, kMagenta};
  int col0 = 1;
  for (int i = 4; i < 40; i += 4) {
    clr[i] = kRed + 1 * col0;
    clr[i + 1] = kBlue + 1 * col0;
    clr[i + 2] = kGreen + 1 * col0;
    clr[i + 3] = kMagenta + 1 * col0;
    col0++;
  }

  TFile *ofile;
  if (draw)
    ofile = new TFile(ofname);
  else
    ofile = new TFile(ofname, "RECREATE");

  sprintf(treename, "ParameterTree");
  sprintf(treetitle, "Pulse Parameter tree");

  TTree *otree = new TTree(treename, treetitle);

  long double tnow = 0., tlast = 0., tlastneutrons = 0.;
  long double dtlast = 0., dtlastneutrons = 0.;
  int npeaks, npeaksNeutrons;
  int ntrigsTot = 0;
  int ntrigsTotNeutrons = 0;

  otree->Branch("eventNo", &eventNo, "eventNo/I");
  otree->Branch("evtime", &tnow, "evtime/l");
  double T10;    // = new double[20000];
  double T90;    // = new double[20000];
  double TB10;   // = new double[20000];
                 //   double Tstart;// = new double[20000];
                 //   double Tend;// = new double[20000];
  double Ampl;   // = new double[20000];
  double Charge; // = new double[20000];
  double Width;  // = new double[20000];
  double TOT;    // = new double[20000];
  double BSLch;  // = new double[20000];
  otree->Branch("npeaks", &npeaks, "npeaks/I");
  otree->Branch("t10", &T10, "t10/D");
  otree->Branch("tb10", &TB10, "tb10/D");
  otree->Branch("t90", &T90, "t90/D");
  otree->Branch("tot", &TOT, "tot/D");
  //   otree->Branch("tstart", &Tstart, "tstart/D");
  //   otree->Branch("tend", &Tend, "tend/D");
  otree->Branch("ampl", &Ampl, "ampl/D");
  otree->Branch("charge", &Charge, "charge/D");
  otree->Branch("BSLcharge", &BSLch, "BSLcharge/D");

  TH1D *hAMPL;
  TH1D *hCH;
  TH1D *hnCH;
  TH1D *hsCH;
  TH1D *hRT;
  TH1D *hPW;
  TH1D *hTOT;
  TH1D *hRate;
  TH1D *hDt;
  TH1D *hRateEvolution;
  TH1D *hRateStructure;
  TH1D *hRateStructTrigger; // time corrected by trigger position
  TH1D *hStructTrigger;     // time corrected by trigger position
  TH2D *hCHvsAMPL;
  TH2D *hAmplvsTOT;
  TH1D *hRateEvolutionCheck;
  TH2D *hAmplvsRT;

  //------------------------------
  // doubling the histos for the nThreshold (neutrons threshold)
  TH1D *hNeutronsAMPL;
  TH1D *hNeutronsCH;
  TH1D *hNeutronsRT;
  TH1D *hNeutronsPW;
  TH1D *hNeutronsTOT;
  TH1D *hNeutronsRate;
  TH1D *hNeutronsDt;
  TH1D *hNeutronsRateStructure;
  TH1D *hNeutronsRateEvolution;
  TH1D *hBkgNeutronsRateEvolution;
  TH1D *hNeutronsRateStructTrigger; // time corrected by trigger position
  TH1D *hSparkEvolution;
  //--------------------------------

  const double microsec = 1e-3;
  const double msec = 1e-6;

  int nbins = 200;
  double amplMax = 0.16;
  double amplMin = 0.;
  double chMax = 50.0;
  if (vm > 520) {
    amplMax = 0.3;
    chMax = 120.0;
  }
  if (vm == 450 && vd == 1050) {
    amplMax = 0.3;
    chMax = 200.0;
  }
  if (vm == 450 && vd == 1050) {
    amplMax = 0.3;
    chMax = 200.0;
  }

  //   double chMin = 0.;
  double pwMax = 400.;
  double rtMax = 120.;
  double totMax = 400.;
  double rbins = 20;
  double rmax = 20.;
  int dtbins = 800;
  double tmax = 1.;

  tree->GetEntry(0);
  long double epochS = 1. * epoch;
  double framesize = maxpoints * dt * microsec; // microsec
  tree->GetEntry(nevents - 1);
  tree->GetEntry(nevents - 1);
  long double epochF = (1. * epoch + nn * 1e-9) + 0.004;
  std::cout << "Epoch S = " << epochS << std::endl;
  std::cout << "Epoch F = " << epochF << std::endl;

  double exprate = nevents / (epochF - epochS);

  int expdt = 1;
  if (maxpoints < 5000) {
    expdt = ((int)(1. / exprate)) / 1 +
            1; /// Normalizing to seconds for the slow detector
    std::cout << "Expected average rate = " << exprate
              << " / sec ,  average DT = " << 1. / exprate
              << " , implemented DT = " << expdt << std::endl;

    //     return 1;
  }

  int tdiviser = 1 + (int)(nevents / (epochF - epochS) * 0.1);
  //   int tbins = (int) ((epochF-epochS)/tdiviser);

  int ntim = 10;
  double timebinwidth = ntim * expdt; /// multiples of 10 (or 20...) sec!!!
  int tbins = (int)((epochF - epochS) / (timebinwidth));

  tbins += 1;
  epochF = epochS + (tbins)*timebinwidth;

  char hname[200], htitle[200], axtitle[200];

  double period = timebinwidth / 1.0;
  //   int tbins2 = (int)((epochF-epochS)/(period));//

  const double mV = 1000.; /// set mV=1. to make everything in volts!

  double rstep = (frmax - frmin) / 256.;
  std::cout << "Rmin = " << frmin << "  , Rmax = " << frmax
            << "  step = " << rstep * mV << " mV" << std::endl;
  if (fabs(threshold) < 3 * rstep) {
    std::cout << "setting new threshold from " << threshold * mV
              << " mV   to    ";
    threshold = -floor(10000 * 3 * rstep) / 10000.;
    std::cout << threshold * mV << " mV" << std::endl;
  }

  amplMax = 256 * (rstep);

  tree->GetEntry(nevents - 1);
  int longpulse = 1;

  nbins = 256;
  if (nevents < 25000)
    nbins /= 2;
  if (nevents > 200000)
    nbins *= 2;

  if (maxpoints < 5000) {
    tmax = 1. / 20.;
    longpulse = 0;
    rmax = 500;
    rbins = 100;
  } else {
    tmax = 1. / 200.;
    rmax = 2000;
    rbins = 100;
  }

  if (nRun == 0) {
    rmax = 50;
    rbins = 50;
  }
  if (bkg) {
    rmax = 20;
    rbins = 20;
    dtbins = 1000;
    tmax = 3.;
  }

  if (exttrig) {
    rmax = 20;
    rbins = 20;
    tmax = 1. / 5000.;
  }

  if (1) /// for Fast detector callibratyion
  {
    rmax = 15;
    rbins = 15;
    dtbins = 1000;
    tmax = 300.;
  }

  if (maxpoints < 5000) {
    tmax = 1. / exprate * 10.;
    dtbins = 500;
    longpulse = 0;
    rmax = (int)(5 * exprate * period);
    rbins = rmax;
    //     std::cout<<"rmax = "<<rmax <<"    tmax = "<<tmax<< std::endl; return
    //     1;
  }

  ///  for (int i=0;i<1; i++)
  ///  {

  ///________________________________________________________________
  /// the following plots concern the neutron rates (per pulse)

  // Nneutrons per pulse if > 0, as found by peak detection
  sprintf(hname, "S%03d_run%02d_Rate_%s", detNo, runNo,
          ftype); /// THis is used only when long pulse????
  if (exttrig)
    sprintf(htitle, "Neutron rate per pulse");
  else
    sprintf(htitle, "Neutron rate");
  hRate = new TH1D(hname, htitle, rbins, 0., rmax);
  // Nneutrons per pulse as found by rate evolution plot!!!
  sprintf(htitle, "Rate (neutrons per %gs) from evolution plot", period);
  sprintf(hname, "S%03d_run%02d_Rate_Neutrons_per%3.1fseconds_%s", detNo, runNo,
          period, ftype);
  hNeutronsRate = new TH1D(hname, htitle, rbins, 0., rmax);
  sprintf(axtitle, "neutrons / %gsec", period);
  hNeutronsRate->GetXaxis()->SetTitle(axtitle);

  ///________________________________________________________________
  /// the following plots concern the neutron (and gamma) rate evolution
  /// (per pulse periode multiple, as it has been calculated from the
  /// timebinwidth)

  // all particle average rate per pulse . Must exclude spark & recovery events
  sprintf(hname, "S%03d_run%02d_RateEvolutionAll_%s", detNo, runNo, ftype);
  sprintf(htitle, "All particles");
  hRateEvolution = new TH1D(hname, htitle, tbins, epochS, epochF);
  hRateEvolution->GetYaxis()->SetTitle("#LT events #GT / sec");
  hRateEvolution->GetXaxis()->SetTimeDisplay(1);
  hRateEvolution->GetXaxis()->SetTimeFormat("%H:%M:%S");
  hRateEvolution->SetMinimum(0);

  // neutron average rate per pulse . Exclude spark events && correlated (pulse
  // uncorrelated) neutrons
  sprintf(hname, "S%03d_run%02d_RateEvolutionNeutrons_%s", detNo, runNo, ftype);
  sprintf(htitle, "Rate evolution (Neutrons)");
  hNeutronsRateEvolution = new TH1D(hname, htitle, tbins, epochS, epochF);
  hNeutronsRateEvolution->GetYaxis()->SetTitle("#LT events #GT / sec");
  hNeutronsRateEvolution->GetYaxis()->SetLabelSize(0.03);
  hNeutronsRateEvolution->GetYaxis()->SetTitleSize(0.04);
  hNeutronsRateEvolution->GetYaxis()->SetTitleOffset(1.3);
  hNeutronsRateEvolution->GetXaxis()->SetTimeDisplay(1);
  hNeutronsRateEvolution->GetXaxis()->SetTimeFormat(
      "#splitline{%H:%M:%S}{%m/%d/%y}");
  hNeutronsRateEvolution->GetXaxis()->SetLabelSize(0.03);
  hNeutronsRateEvolution->GetXaxis()->SetLabelOffset(0.02);
  hNeutronsRateEvolution->SetMinimum(0);

  timebinwidth = hRateEvolution->GetBinWidth(1);
  std::cout << "Time bin = " << timebinwidth << std::endl;

  // correlated (pulse uncorrelated) neutrons average rate per pulse
  sprintf(hname, "S%03d_run%02d_RateEvolutionBackgroundNeutrons_%s", detNo,
          runNo, ftype);
  sprintf(htitle, "Uncorrelated neutrons");
  hBkgNeutronsRateEvolution = new TH1D(hname, htitle, tbins, epochS, epochF);

  // sparks or baseline recovery events. Useful for rate calculations also
  sprintf(hname, "S%03d_run%02d_Spark_Evolution_%s", detNo, runNo, ftype);
  sprintf(htitle, "Sparks or recovery");
  hSparkEvolution = new TH1D(hname, htitle, tbins, epochS, epochF);

  // auxiliary plot for the normalization of the rate per pulse.
  sprintf(hname, "S%03d_run%02d_Rate_Evolution_Check_%s", detNo, runNo, ftype);
  sprintf(htitle, "Rate normalization plot");
  hRateEvolutionCheck = new TH1D(hname, htitle, tbins, epochS, epochF);

  ///________________________________________________________________
  /// the following plots concern the rate structures WITHOUT correction for the
  /// beam trigger

  sprintf(hname, "S%03d_run%02d_Neutron_RateStructure_Th%gmV_%s", detNo, runNo,
          nTh * mV, ftype);
  sprintf(htitle, "Neutrons");
  hNeutronsRateStructure = new TH1D(hname, htitle, 250, 0., framesize);
  hNeutronsRateStructure->GetXaxis()->SetTitle("t [#mus]");
  //     hNeutronsRateStructure->SetMinimum(0);

  sprintf(hname, "S%03d_run%02d_RateStructure_%s", detNo, runNo, ftype);
  sprintf(htitle, "All events");
  hRateStructure = new TH1D(hname, htitle, 250, 0., framesize);
  hRateStructure->GetXaxis()->SetTitle("t [#mus]");
  //     hRateStructure->SetMinimum(0);

  sprintf(hname, "S%03d_run%02d_TriggerStructure_%s", detNo, runNo, ftype);
  sprintf(htitle, "Linac4 trigger ");
  hStructTrigger = new TH1D(hname, htitle, 250, 0., framesize);
  hStructTrigger->SetLineColor(kGreen + 2);

  ///________________________________________________________________
  ///  Rate structures after trigger subtraction

  sprintf(hname, "S%03d_run%02d_Rate_Structure_Trigger_NeutronsTh%gmV_%s",
          detNo, runNo, nTh * mV, ftype);
  sprintf(htitle, "Neutron Rate Structure");
  hNeutronsRateStructTrigger =
      new TH1D(hname, htitle, 250, -0.25 * framesize, 0.75 * framesize);
  hNeutronsRateStructTrigger->GetXaxis()->SetTitle("t [#mus]");
  hNeutronsRateStructTrigger->SetMinimum(0);

  sprintf(hname, "S%03d_run%02d_Rate_Structure_Trigger_%s", detNo, runNo,
          ftype);
  sprintf(htitle, "Trigger Structure");
  hRateStructTrigger =
      new TH1D(hname, htitle, 250, -0.25 * framesize, 0.75 * framesize);
  hRateStructTrigger->GetXaxis()->SetTitle("t [#mus]");
  hRateStructTrigger->SetMinimum(0);

  ///________________________________________________________________
  ///  DeltaT plots

  sprintf(hname, "S%03d_run%02d_consecutivePulses_#DeltaT_%s", detNo, runNo,
          ftype);
  sprintf(htitle, "Concecutive pulses #DeltaT");
  hDt = new TH1D(hname, htitle, dtbins, 0., tmax);
  hDt->GetXaxis()->SetTitle("#DeltaT [sec]");

  sprintf(hname, "S%03d_run%02d_consecutivePulses_#DeltaT_NeutronsTh%gmV_%s",
          detNo, runNo, nTh * mV, ftype);
  sprintf(htitle, "#DeltaT neutrons");
  hNeutronsDt = new TH1D(hname, htitle, dtbins, 0., tmax);
  hNeutronsDt->GetXaxis()->SetTitle("#DeltaT [sec]");

  ///________________________________________________________________
  ///  Amplitude plots

  sprintf(hname, "S%03d_run%02d_Amplitude_%s", detNo, runNo, ftype);
  sprintf(htitle, "All particles");
  hAMPL = new TH1D(hname, htitle, nbins * 1, 0., amplMax * mV);
  hAMPL->GetXaxis()->SetTitle("Pulse amplitude [mV]");

  sprintf(htitle, "Pulse amplitude (neutron cut)");
  sprintf(hname, "S%03d_run%02d_Amplitude_NeutronsTh%gmV_%s", detNo, runNo,
          nTh * mV, ftype);
  hNeutronsAMPL = new TH1D(hname, htitle, nbins * 1, 0., amplMax * mV);
  hNeutronsAMPL->GetXaxis()->SetTitle("Pulse amplitude [mV]");

  sprintf(htitle, "Pulse amplitude evolution (neutron cut)");
  sprintf(hname, "S%03d_run%02d_AmplitudeEvolution_NeutronsTh%gmV_%s", detNo,
          runNo, nTh * mV, ftype);
  TH2D *h2dNeutronsAMPL =
      new TH2D(hname, htitle, 100, epochS, epochF, nbins / 2, 0., amplMax * mV);
  h2dNeutronsAMPL->GetXaxis()->SetTimeDisplay(1);
  h2dNeutronsAMPL->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%m/%d/%y}");
  h2dNeutronsAMPL->GetXaxis()->SetLabelSize(0.03);
  h2dNeutronsAMPL->GetXaxis()->SetLabelOffset(0.02);
  h2dNeutronsAMPL->GetYaxis()->SetTitle("Pulse amplitude [mV]");

  ///________________________________________________________________
  ///  Charge plots

  sprintf(hname, "S%03d_run%02d_Charge_%s", detNo, runNo, ftype);
  sprintf(htitle, "All particles");
  hCH = new TH1D(hname, htitle, nbins * 1, 0., chMax);
  hCH->GetXaxis()->SetTitle("charge [a.u.]");

  sprintf(htitle, "Pulse charge (neutron cut)");
  sprintf(hname, "S%03d_run%02d_Charge_NeutronsTh%gmV_%s", detNo, runNo,
          nTh * mV, ftype);
  hNeutronsCH = new TH1D(hname, htitle, nbins * 1, 0., chMax);
  hNeutronsCH->GetXaxis()->SetTitle("charge [a.u.]");

  //     sprintf(hname,"S%03d_run%02d_Net_Charge_%s",detNo,runNo,ftype);
  //     hnCH=new TH1D(hname,htitle,nbins*2,0.,chMax);
  //     hnCH->GetXaxis()->SetTitle("charge [a.u.]");
  //     hnCH->SetLineColor(4);

  sprintf(fname2, "S%03d_run%02d_Single_Pulse_Charge_%s", detNo, runNo, ftype);
  hsCH = new TH1D(fname2, fname2, nbins * 1, 0., chMax);
  hsCH->GetXaxis()->SetTitle("charge [a.u.]");
  hsCH->SetLineColor(2);

  sprintf(htitle, "Pulse charge evolution (neutron cut)");
  sprintf(hname, "S%03d_run%02d_ChargeEvolution_NeutronsTh%gmV_%s", detNo,
          runNo, nTh * mV, ftype);
  TH2D *h2dNeutronsCH =
      new TH2D(hname, htitle, 100, epochS, epochF, nbins / 2, 0., chMax);
  h2dNeutronsCH->GetXaxis()->SetTimeDisplay(1);
  h2dNeutronsCH->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%m/%d/%y}");
  h2dNeutronsCH->GetXaxis()->SetLabelSize(0.03);
  h2dNeutronsCH->GetXaxis()->SetLabelOffset(0.02);
  h2dNeutronsCH->GetYaxis()->SetTitle("charge [a.u.]");

  ///________________________________________________________________
  ///  pulse time properties plots

  sprintf(htitle, "Rise Time , n_{th} = %g mV", nTh * 1000);
  sprintf(fname2, "S%03d_run%02d_Risetime_%s", detNo, runNo, ftype);
  hRT = new TH1D(fname2, htitle, int(rtMax / dt), 0., rtMax);
  hRT->GetXaxis()->SetTitle("Risetime [ns]");

  sprintf(htitle, "Pulse Width , n_{th} = %g mV", nTh * 1000);
  sprintf(fname2, "S%03d_run%02d_Pulse_Width_%s", detNo, runNo, ftype);
  hPW = new TH1D(fname2, htitle, int(pwMax / dt), 0., pwMax);
  hPW->GetXaxis()->SetTitle("Pulse Duration [ns]");

  sprintf(htitle, "TOT , n_{th} = %g mV", nTh * 1000);
  sprintf(fname2, "S%03d_run%02d_TOT_%s", detNo, runNo, ftype);
  hTOT = new TH1D(fname2, htitle, int(totMax / dt), 0., totMax);
  hTOT->GetXaxis()->SetTitle("TOT [ns]");

  ///________________________________________________________________
  ///  Correlation plots
  double chovamax = 2000.;
  if (dv <= 50 || vm > 490)
    chovamax = 800.;

  sprintf(htitle, "Charge over Amplitude");
  sprintf(hname, "S%03d_run%02d_Charge_ov_Amplitude_%s", detNo, runNo, ftype);
  TH1D *hCHovAmpl = new TH1D(hname, htitle, nbins * 2, 0., chovamax);
  hCHovAmpl->GetXaxis()->SetTitle("ratio [nC/V]");

  sprintf(htitle, "Pulse Charge over Amplitude");
  sprintf(hname, "S%03d_run%02d_spCharge_ov_Amplitude_%s", detNo, runNo, ftype);
  TH1D *hsCHovAmpl = new TH1D(hname, htitle, nbins * 2, 0., chovamax);
  hsCHovAmpl->SetLineColor(kRed);
  hsCHovAmpl->GetXaxis()->SetTitle("ratio [nC/V]");

  sprintf(fname2, "S%03d_run%02d_Ampl_vs_Rise_Time_C_%s", detNo, runNo, ftype);
  hAmplvsRT = new TH2D(fname2, fname2, nbins, 0., amplMax, 25, 0., rtMax);
  hAmplvsRT->GetXaxis()->SetTitle("Pulse amplitude [V]");
  hAmplvsRT->GetYaxis()->SetTitle("Risetime [#mus]");
  hAmplvsRT->SetStats(0);

  sprintf(fname2, "S%03d_run%02d_Charge_vs_Amplitude_C_%s", detNo, runNo,
          ftype);
  hCHvsAMPL = new TH2D(fname2, fname2, nbins, 0., amplMax, nbins, 0., chMax);
  hCHvsAMPL->GetXaxis()->SetTitle("Pulse amplitude [V]");
  hCHvsAMPL->GetYaxis()->SetTitle("Charge [a.u.]");
  hCHvsAMPL->SetStats(0);

  sprintf(fname2, "S%03d_run%02d_Amplitude_vs_TOT_C_%s", detNo, runNo, ftype);
  hAmplvsTOT = new TH2D(fname2, fname2, nbins, 0., amplMax, int(totMax / dt),
                        0., totMax);
  hAmplvsTOT->GetXaxis()->SetTitle("Pulse amplitude [V]");
  hAmplvsTOT->GetYaxis()->SetTitle("TOT [ns]");
  hAmplvsTOT->SetStats(0);

  //----------------------------------------------
  //---------doubling histos for nThreshold--------

  sprintf(fname2, "S%03d_run%02d_Risetime_NeutronsTh%gmV_%s", detNo, runNo,
          nTh * mV, ftype);
  sprintf(htitle, "Rise Time (n_{th} = %g mV)", nTh * 1000);
  hNeutronsRT = new TH1D(fname2, htitle, int(rtMax / dt), 0., rtMax);
  hNeutronsRT->GetXaxis()->SetTitle("Risetime [ns]");

  sprintf(fname2, "S%03d_run%02d_Pulse_Width_NeutronsTh%gmV_%s", detNo, runNo,
          nTh * mV, ftype);
  sprintf(htitle, "Pulse Width (n_{th} = %g mV)", nTh * 1000);
  hNeutronsPW = new TH1D(fname2, htitle, int(pwMax / dt), 0., pwMax);
  hNeutronsPW->GetXaxis()->SetTitle("Pulse Duration [ns]");

  sprintf(fname2, "S%03d_run%02d_TOT_NeutronsTh%gmV_%s", detNo, runNo, nTh * mV,
          ftype);
  sprintf(htitle, "TOT  (n_{th} = %g mV)", nTh * 1000);
  hNeutronsTOT = new TH1D(fname2, htitle, int(totMax / dt), 0., totMax);
  hNeutronsTOT->GetXaxis()->SetTitle("TOT [ns]");

  //-----------------------------------------------------
  ///  }

  int npt = 2;
  double DT = 4.; // 8.;  /// in [ns]
  int evpm = 1000;
  if (nevents > 100000)
    evpm = 10000;
  if (maxpoints > 10000)
    evpm = 100;

  eventNo = 0;
  int totNtrigs = 0;
  int totNsparks = 0;
  long double drawdt = 0.;
  if (detNo == 5)
    eventNo = 2000; /// skip first events because of the time change in the
                    /// osciloscope.

  std::cout << "Start processing the " << nevents << " events" << std::endl;
  while (eventNo < nevents) {
    if (draw) {
      std::cout
          << std::endl
          << "________________________________________________________________"
             "______"
          << std::endl
          << std::endl;
      ;
      std::cout << "Event to draw : ";
      cin >> eventNo;
    }

    if (eventNo < 0 || eventNo >= nevents)
      break;
    ///    if (eventNo<25 || eventNo>=150) {eventNo++; continue;}

    tree->GetEntry(eventNo);
    double maxc[] = {0., 0., 0., 0.};
    double minc[] = {0., 0., 0., 0.};
    double maxd[] = {0., 0., 0., 0.};
    double mind[] = {0., 0., 0., 0.};
    long double evtime = 1. * epoch + 1. * nn * 1e-9;
    //       long double evtime = 1.*epoch+1.*nn*1e-9 + t0;
    //       printf("t0 = %8.8lf ,  evtime = %8.8LF\n",t0,evtime);

    double totcharge = 0.;

    //       std::cout<<"frmax = "<<frmax<<"   "<<frmin<<"    "<<frmin - frmax<<
    //       std::endl;

    //  std::cout<<"1 SDD DSF ASDF SADF ASF ASF AS"<< std::endl;
    /// reset time array in case dt was modified during data taking
    int ntrigs = 0;
    int ntrigsNeutrons = 0;
    //     std::cout<< "maxpoints = "<< maxpoints << std::endl;
    ptime[0] = 0;
    for (int i = 1; i < maxpoints; i++)
      ptime[i] = ptime[i - 1] + dt;
    //       std::cout<<" dt = "<<dt<< std::endl;
    for (int i = 0; i < maxpoints; i++) {
      totcharge += bslsum - amplSum[i];
    }
    totcharge /= maxpoints;
    totcharge *= DT * 1e3; /// make it fC

    int detspark = 0;

    if (totcharge > 200.) //|| rmssum> 1.0015)
    {
      detspark = 1;
      totNsparks++;
    }

    npt = TMath::FloorNint(DT / (dt * 1e9));
    //       std::cout<<"npt = "<<npt<< std::endl;
    //  return 0;

    // check if the Event_Waveform folder is present, if not create it

    // Build the folder path once
    std::string outDir = Form("%s/Event_WaveForms/S%03d-%02d-%d-%d", WORKDIR,
                              detNo, runNo, vm, vd);

    // Create it if it does not exist
    std::error_code ec;
    if (!fs::exists(outDir)) {
      fs::create_directories(outDir, ec);
      if (ec) {
        std::cerr << "Failed to create directory: " << outDir
                  << " Error: " << ec.message() << std::endl;
      }
    }

    // if draw is on, the waveform of the event is displayed and some of its
    // properties are printed on the console. This is useful for checking the
    // quality of the data and the performance of the detector.
    if (draw) {
      for (int i = 0; i < maxpoints; i++) {
        amplC[i] = amplSum[i];
      }

      std::cout << "Event " << eventNo << "\t fit1 " << fitstatus1 << " fit2 "
                << fitstatus2 << " bsl " << bslsum << " rms " << rmssum
                << " totcharge " << totcharge << std::endl;
      std::cout << std::endl << "Pulse length = " << maxpoints << std::endl;
      long double epochX = (1. * epoch + nn * 1e-9);
      //	  TTimeStamp *tstamp = new
      // TTimeStamp((time_t)epoch+(rootConv-unixConv),(Int_t)nn);
      TTimeStamp *tstamp = new TTimeStamp(epochX + (rootConv - unixConv), 0);
      printf("Event time : %s = %10.9Lf  DT = %10.9LF s = %10.7LF ms\n",
             tstamp->AsString("l"), epochX, epochX - drawdt,
             1000. * (epochX - drawdt));
      drawdt = epochX;

      for (int ci = 0; ci < 1; ci++) {
        ecanv->cd();
        waveform = new TGraph(maxpoints, ptime, amplC);
        maxc[ci] = TMath::MaxElement(maxpoints, amplC);
        minc[ci] = TMath::MinElement(maxpoints, amplC);
        sprintf(cname, "Event %d Waveform C%d\n", evNo, ci + 1);
        waveform->SetTitle(cname);
        waveform->SetLineColor(clr[ci]);
        waveform->SetMarkerColor(clr[ci]);
        waveform->SetFillColor(0);
        if (ci == 0) {
          double fmin = frmin - frmax;
          // double fmin = frmax-frmin;
          waveform->GetHistogram()->SetMinimum(-fmin); // set min for better
                                                       // viewing
          // waveform->GetHistogram()->SetMinimum(fmin / 2.);
          std::cout << "Setting minimum at " << frmin - frmax << std::endl;
          waveform->Draw("apl");
          // waveform->GetHistogram()->GetYaxis()->SetRangeUser(fmin,-fmin/8.);
          // waveform->GetHistogram()->GetYaxis()->SetRangeUser(fmin / 2,
          // -fmin);
          waveform->GetHistogram()->GetYaxis()->SetRangeUser(fmin,
                                                             -fmin); // set y
                                                                     // range
        } else
          waveform->Draw("pl");
        // Set the axis titles even the waveform historam is not used
        waveform->GetXaxis()->SetTitle("Time [ns]");
        waveform->GetXaxis()->CenterTitle();
        waveform->GetYaxis()->SetTitle("Amplitude [V]");
        waveform->GetYaxis()->CenterTitle();
        waveform->GetYaxis()->SetTitleOffset(1.1);

        // Get the axis maximum values for range setting
        Double_t y_axis_max = waveform->GetYaxis()->GetXmax();
        Double_t y_axis_min = waveform->GetYaxis()->GetXmin();

        // std::cout << "ymax-ymin:  " << y_axis_max << "  " << y_axis_min
        //           << std::endl;

        waveform->GetYaxis()->SetRangeUser(y_axis_min - 0.1, y_axis_max + 0.1);
        //-----------------------------------------------------//
        //---- Calculate and display peak and time at peak -----//
        //-----------------------------------------------------//

        // Calculate the peak and the mean value for the waveform
        Double_t x_at_max = 0;
        double maxY = -1e18; // Initialize with a very small number
        Double_t current_x, current_y;
        // Loop through all points in the graph
        for (int i = 0; i < waveform->GetN(); ++i) {
          waveform->GetPoint(i, current_x,
                             current_y); // Get X and Y for point i
          if (current_y > maxY) {
            maxY = current_y;
            x_at_max = current_x;
          }
        }
        // Create stringstream to format the text Altingun
        std::stringstream ss, ss_2;
        ss << "Peak: [" << setprecision(3) << maxY * 1000. << " mV, "
           << x_at_max << " ns]";
        // ss_2 << "t[Peak]: " << setprecision(3) << x_at_max << " ns";

        // Create TLatex to draw the text on the canvas Altingun
        TLatex l;
        l.SetNDC(kTRUE);
        l.SetTextColor(kRed);
        l.DrawLatex(0.25, 0.86, ss.str().c_str());

        // TLatex l_2;
        // l_2.SetNDC(kTRUE);
        // l_2.SetTextColor(kRed);
        // l_2.DrawLatex(0.2, 0.75, ss_2.str().c_str());

        ecanv->Modified();
        ecanv->Update();
        // Altingun
        // ecanv->SaveAs(Form("%s/Event_WaveForms/ESTIA_spare2_tube_tests_overnight_770V/Waveform_Event%04d.png",WORKDIR,
        // eventNo));
        ecanv->SaveAs(
            Form("%s/Event_WaveForms/S%03d-%02d-%d-%d/Waveform_Event%04d.png",
                 WORKDIR, detNo, runNo, vm, vd, eventNo));

        //-----------------------------------------------------//

        //-----------------------------------------------------//
        //---- Write waveform data to text file ---------------//
        //-----------------------------------------------------//
        if (saveWF) {
          ofstream myfile;
          myfile.open(
              Form("%s/Event_WaveForms/S%03d-%02d-%d-%d/Waveform_Event%04d.txt",
                   WORKDIR, detNo, runNo, vm, vd, eventNo));
          myfile << "Time \t Amplitude\n";

          // Loop through all points in the graph
          for (int i = 0; i < waveform->GetN(); ++i) {
            waveform->GetPoint(i, current_x,
                               current_y); // Get X and Y for point i
            myfile << current_x << "\t" << current_y << "\n"; // write to file
          }
          // for (int i = 0; i < sizeof(*ptime) / sizeof(*ptime[0]); i++) {
          //   double a = ptime[i];
          //   double b = amplC[i];
          //   myfile << a << "\t" << b << "\n"; // write to file
          // }
          myfile.close();
        }
        //-----------------------------------------------------//
        //      continue;

        ///  Smoothing array when no "bit filter" !!!!

        dcanv->cd();
        DerivateArray(amplC, dampl, maxpoints, dt, npt * 4, 1);
        derivative = new TGraph(maxpoints, ptime, dampl);
        derivative->SetMarkerColor(clr[ci]);
        derivative->SetLineColor(clr[ci]);
        derivative->SetFillColor(0);
        sprintf(cname, "Derivative C%d\n", ci + 1);
        derivative->SetTitle(cname);
        if (ci == 0) {
          derivative->Draw("apl");
          derivative->SetMaximum(0.02);
          derivative->SetMinimum(-0.02);
        } else
          derivative->Draw("pl");

        ///  Derivate smoothed signals for analysis  (may not me used...)
        SmoothArray(amplC, samplC, maxpoints, 2, inverse);
        DerivateArray(samplC, dsampl, maxpoints, dt, npt * 4, 1);
        TGraph *graph22 = new TGraph(maxpoints, ptime, dsampl);
        graph22->SetMarkerColor(clr[ci + 4]);
        graph22->SetLineColor(clr[ci + 4]);
        graph22->SetLineWidth(2);
        graph22->SetFillColor(0);
        sprintf(cname, "Smoothed Derivative C%d\n", ci + 1);
        graph22->SetTitle(cname);
        graph22->Draw("pl");
        dcanv->Modified();
        dcanv->Update();

        icanv->cd();
        double intgr = IntegrateA(maxpoints, dsampl, idamplC, dt);
        int nint = 20;
        intgr = IntegratePulse(maxpoints, idamplC, iampl, dt, nint * dt);
        // continue;
        maxd[ci] = TMath::MaxElement(maxpoints, iampl);
        mind[ci] = TMath::MinElement(maxpoints, iampl);
        integralh = new TGraph(maxpoints, ptime, iampl);
        integralh->SetMarkerColor(clr[ci + 4]);
        integralh->SetLineColor(clr[ci + 4]);
        integralh->SetFillColor(0);
        sprintf(cname, "Integral %g of C%d\n", nint * dt, ci + 1);
        integralh->SetTitle(cname);
        if (ci == 0) {
          integralh->Draw("apl");
        } else
          integralh->Draw("pl");
        icanv->Modified();
        icanv->Update();

        icanv->cd();
        //      intgr = IntegratePulse(maxpoints,amplC[ci],iampl,dt,50.);
        nint = N_INTEGRATION_POINTS;
        intgr = IntegratePulse(maxpoints, idamplC, iampl, dt, nint * dt);
        graph22 = new TGraph(maxpoints, ptime, iampl);
        graph22->SetMarkerColor(clr[ci + 8]);
        graph22->SetLineColor(clr[ci + 8]);
        graph22->SetFillColor(0);
        sprintf(cname, "Integral %g of C%d\n", nint * dt, ci + 1);
        graph22->SetTitle(cname);
        graph22->Draw("pl");

        nint = 2;
        intgr = IntegratePulse(maxpoints, idamplC, iampl, dt, nint * dt);
        graph22 = new TGraph(maxpoints, ptime, iampl);
        graph22->SetMarkerColor(clr[ci + 12]);
        graph22->SetLineColor(clr[ci + 12]);
        graph22->SetFillColor(0);
        sprintf(cname, "Integral %g of C%d\n", nint * dt, ci + 1);
        graph22->SetTitle(cname);
        graph22->Draw("pl");
        ecanv->Modified();
        ecanv->Update();
      }
    } // end (if(draw)

    //       std::cout<<"2 SDD DSF ASDF SADF ASF ASF AS"<< std::endl;
    ///  subtract baseline it is done during the creation of the tree
    //       std::cout<<"maxpoints = "<<maxpoints<<"  bslsum = "<<bslsum<<
    //       std::endl;
    for (int i = 0; i < maxpoints; i++)
      amplSum[i] -= bslsum;
    /// smooth sumn array for analysis
    //       std::cout<<"2 SDD DSF ASDF SADF ASF ASF AS"<< std::endl;
    SmoothArray(amplSum, sampl, maxpoints, 3, inverse);
    //       std::cout<<"3 SDD DSF ASDF SADF ASF ASF AS"<< std::endl;
    /// derivate sum array
    DerivateArray(amplSum, dsampl, maxpoints, dt, npt, 1);

    //       std::cout<<"4 SDD DSF ASDF SADF ASF ASF AS"<< std::endl;

    int ti = 0;
    int itrig = 0;
    double t10corrected = 0.;
    ntrigs = 0;
    ntrigsNeutrons = 0;
    int correlated = 0;
    itrig = itrigger;
    if (itrig > 0)
      correlated = 1;
    //      correlated = bkg;

    /// Having the following "if (detspark)" here means that an event with a
    /// spark, of a "baseline recovery event will not be analysed!!!
    if (detspark) {
      hSparkEvolution->Fill(evtime);
      std::cout << " Spark (" << (totcharge > 10.) << ") or recovery ("
                << (rmssum > 0.0015) << ") at event " << eventNo << std::endl;
      eventNo++;
      continue;
    }

    while (ti < maxpoints - 50 && ntrigs < MAXTRIG) {
      // std::cout<<"check: "<<ntrigs+1<< std::endl;
      ti = AnalyseLongPulse(maxpoints, sampl, dsampl, ipar, threshold, dt, ti);
      if (ti < 0)
        break;
      // 	  std::cout <<"Found trig at "<<ti*dt<< "  "<< ti<< " "<<itrig<<
      // "
      // "<<itrig*dt<< "  "<<ttrig<< "  "<< std::endl;

      AddPar(ipar, &spar[ntrigs], dt);

      if (correlated)
        t10corrected = spar[ntrigs].t10 - ttrig;
      else
        t10corrected = spar[ntrigs].t10; //-1e5;

      hAMPL->Fill(spar[ntrigs].ampl * mV);
      hCH->Fill(spar[ntrigs].charge);
      hsCH->Fill(spar[ntrigs].charge - spar[ntrigs].bslch);
      hRT->Fill(spar[ntrigs].t90 - spar[ntrigs].t10);
      hTOT->Fill(spar[ntrigs].tot);
      if (!correlated) {
        hRateStructure->Fill(spar[ntrigs].t10 *
                             microsec); // t10 is ns, so to pass to us
      } else {
        hRateStructTrigger->Fill(t10corrected *
                                 microsec); // t10 is ns, so to pass to us
      }
      // std::cout << "rate structure = " << spar[ntrigs].t10/1000. <<
      // std::endl;
      hAmplvsRT->Fill(spar[ntrigs].ampl, (spar[ntrigs].t90 - spar[ntrigs].t10));
      hCHvsAMPL->Fill(spar[ntrigs].ampl,
                      spar[ntrigs].charge - spar[ntrigs].bslch);
      hAmplvsTOT->Fill(spar[ntrigs].ampl, spar[ntrigs].tot);
      hCHovAmpl->Fill(spar[ntrigs].charge / spar[ntrigs].ampl);
      // 	  hsCHovAmpl->Fill((spar[ntrigs].charge-spar[ntrigs].bslch)/spar[ntrigs].ampl);

      int cut1 = spar[ntrigs].charge / spar[ntrigs].ampl > 700. &&
                 spar[ntrigs].charge / spar[ntrigs].ampl < 1600.;
      // 	  cut1 =1;
      hPW->Fill(spar[ntrigs].width);
      if (ntrigs > 0) {
        hDt->Fill((spar[ntrigs].t10 - spar[ntrigs - 1].t10) *
                  1e-9); // in seconds
      }
      // 	  else
      tnow = 1. * epoch + nn * 1e-9 +
             spar[0].t10 * 1e-9; /// normally we arrive here only in the first
                                 /// peak per pulse!!!!
      // 	    std::cout<<"tnow set to "<<epoch<<" + "<<nn*1e-9<<" +
      // "<<t0<<"
      // "<< std::endl;
      // std::cout << "hDT filling = " <<
      // (spar[ntrigs].t10-spar[ntrigs-1].t10)*1e-9
      // << std::endl;
      /// 	  if (spar[ntrigs].tot<400)
      /// 	    hsCH->Fill(spar[ntrigs].charge);
      T10 = spar[ntrigs].t10;
      T90 = spar[ntrigs].t90;
      TB10 = spar[ntrigs].tb10;
      Ampl = spar[ntrigs].ampl;
      Charge = spar[ntrigs].charge;
      Width = spar[ntrigs].width;
      TOT = spar[ntrigs].tot;
      BSLch = spar[ntrigs].charge - spar[ntrigs].bslch;

      otree->Fill();
      // //-----------------------------
      // //doubling histos for nthreshold
      if ((spar[ntrigs].ampl) > (nTh) && (spar[ntrigs].charge) > (nThCh) &&
          cut1) //(spar[ntrigs].tot>60) &&
                //(spar[ntrigs].tot<100)(spar[ntrigs].width>60) &&
                //(spar[ntrigs].width<100))
      {
        hNeutronsAMPL->Fill(spar[ntrigs].ampl * mV);
        h2dNeutronsAMPL->Fill(tnow, spar[ntrigs].ampl * mV);
        hNeutronsCH->Fill(spar[ntrigs].charge);
        h2dNeutronsCH->Fill(tnow, spar[ntrigs].charge);
        hNeutronsRT->Fill(spar[ntrigs].t90 - spar[ntrigs].t10);
        hNeutronsTOT->Fill(spar[ntrigs].tot);
        hNeutronsPW->Fill(spar[ntrigs].width);
        hsCHovAmpl->Fill((spar[ntrigs].charge) / spar[ntrigs].ampl);
        if (!correlated) {
          // 		if (!longpulse) hRate->Fill(tnow+T10*1e-9);  /// if
          // longpulse it is beeing filled later
          hNeutronsRateStructure->Fill(spar[ntrigs].t10 *
                                       microsec); // t10 is ns, so to pass to us
          hNeutronsRateStructTrigger->Fill(
              t10corrected * microsec); // t10 is ns, so to pass to us
          if (ntrigsNeutrons > 0)
            hNeutronsDt->Fill((spar[ntrigs].t10 - spar[ntrigs - 1].t10) *
                              1e-9); // in seconds
          //     std::cout << "hDT filling = " <<
          //     (spar[ntrigs].t10-spar[ntrigs-1].t10)*1e-9 << std::endl;
          if (t10corrected < 0.)
            std::cout << "Negative tcor, Event = " << eventNo << std::endl;
        }
        ntrigsNeutrons++;
      }
      //------------------------

      ntrigs++;
    } // end loop ti

    npeaks = ntrigs;
    npeaksNeutrons = ntrigsNeutrons;
    //       std::cout<<"Found "<<ntrigsNeutrons<<" neutrons"<< std::endl;
    //       if (npeaksNeutrons>1) std::cout <<"event "<<eventNo<<" n =
    //       "<<npeaksNeutrons<<"  "<< epoch - epochS<< "  "<< evtime - epochS<<
    //       endl;

    ntrigsTot += npeaks;
    ntrigsTotNeutrons += npeaksNeutrons;
    // std::cout << "npeaks" << npeaks << std::endl;

    if (!exttrig)
      if (ntrigs == 0) {
        hRateEvolutionCheck->Fill(
            evtime); /// this is for the case of external trigger !!!
        eventNo++;
        continue;
      }

    double ts = ptime[0];             // ns
    double tf = ptime[maxpoints - 1]; // ns
    double tlive = (tf - ts) * 1e-9;  // to make it sec
    double peakspersec = 1. * npeaks / tlive;
    double neutronspersec = 1. * ntrigsNeutrons / tlive;
    ///       double tnow = epoch + nn*1e-9 + spar[npeaks-1].t10*1e-9;  /// time
    ///       of last peak in event or of the unique event if short frame//sec

    long double depoch = 1. * (long double)epoch;
    long double tnowneutrons =
        depoch + nn * 1e-9 +
        spar[ntrigsNeutrons - 1].t10 *
            1e-9; /// used for dt calculation for neutrons

    hRateEvolution->Fill(evtime, npeaks);
    hRateEvolutionCheck->Fill(evtime);

    if (bkg) {
      hBkgNeutronsRateEvolution->Fill(evtime, npeaksNeutrons);
      if (npeaksNeutrons > 0)
        hRate->Fill(npeaksNeutrons);
    } else {

      hStructTrigger->Fill(ttrig / 1000.); // t10 is ns, so to pass to us
      hNeutronsRateEvolution->Fill(evtime, npeaksNeutrons);
      if (npeaksNeutrons > 0 && longpulse)
        hRate->Fill(npeaksNeutrons);

      if (eventNo > 0 && npeaks > 0) {
        // intantaneous flux ("peak flux")
        // std::cout<<"tnow = "<<tnow<<"  nn = "<<nn*1e-9<< std::endl;
        dtlast = tnow - tlast;
        // 	    std::cout<<"tlast = "<<tlast<<" + "<< +
        // nn*1e-9+spar[npeaks-1].t10*1e-9<<"  dtlast = "<<dtlast<< std::endl;
        if (tlast > 0)
          hDt->Fill(dtlast);
      }
      tlast = tnow;

      if (eventNo > 0 && npeaksNeutrons > 0) {
        dtlastneutrons = tnowneutrons - tlastneutrons;
        // 	  if (dtlastneutrons<0)
        // 	  {
        // 	    std::cout<<"negative DT!!!!\n event ="<<eventNo<< std::endl;
        // 	    printf("tnow = %18.8lf,    tlast =  %18.8lf ,     DT =
        // %18.8lf\n",tnowneutrons,tlastneutrons,dtlastneutrons);
        //  	    std::cout<<"tnow = "<<tnowneutrons<<"  tlast =
        //  "<<tlastneutrons<<"    DT = "<< dtlastneutrons<< std::endl;
        // 	  }
        //  	  printf("tnow = %Lf,    tlast =  %Lf ,     DT =
        //  %Lf\n",tnowneutrons,tlastneutrons,dtlastneutrons);
        // 	  std::cout<<"tnow = "<<tnowneutrons<<"  tlast =
        // "<<tlastneutrons<<" DT = "<< dtlastneutrons<< std::endl;
        hNeutronsDt->Fill(dtlastneutrons);
        tlastneutrons = tnowneutrons;
      }
      //         tlastneutrons = tnowneutrons ;
    }

    // IF DRAW
    //----------------------
    if (draw) {
      std::cout << "Found " << ntrigs << " pulses " << std::endl;

      ecanv->cd();
      TGraph *graphSum = new TGraph(maxpoints, ptime, amplSum);
      graphSum->SetLineColor(1);
      graphSum->SetFillColor(0);
      graphSum->SetLineWidth(2);
      sprintf(cname, "Sum raw signal event %d", evNo);
      graphSum->SetTitle(cname);
      graphSum->Draw("pl");

      TGraph *sgraphSum = new TGraph(maxpoints, ptime, sampl);
      sgraphSum->SetLineColor(7);
      sgraphSum->SetFillColor(0);
      sgraphSum->SetLineWidth(2);
      sprintf(cname, "Smoothed Sum signal 1 event %d", evNo);
      sgraphSum->SetTitle(cname);
      sgraphSum->Draw("pl");

      //       for (int ci=0;ci<4;ci++)
      //       {
      // 	TGraph *graph22 = new TGraph(maxpoints,ptime,samplC[ci]);
      // 	graph22->SetMarkerColor(clr[ci]);
      // 	graph22->SetLineColor(clr[ci]);
      // 	graph22->SetFillColor(0);
      // 	sprintf(cname,"Max subtracted C%d\n",ci+1);
      // 	graph22->SetTitle(cname);
      // 	graph22->Draw("pl");
      //       }

      dcanv->cd();
      TGraph *graph22 = new TGraph(maxpoints, ptime, dsampl);
      graph22->SetMarkerColor(1);
      graph22->SetLineColor(1);
      graph22->SetFillColor(0);
      graph22->SetLineWidth(2);
      graph22->SetLineStyle(7);
      sprintf(cname, "Derivative\n");
      graph22->SetTitle(cname);
      graph22->Draw("pl");

      if (waveform != 0) {
        TH1F *h1 = (TH1F *)waveform->GetHistogram();
        // 	    h1->SetMaximum(TMath::MaxElement(4,maxc) + 0.005);
        // 	    h1->SetMinimum(TMath::MinElement(4,minc) - 0.01);
        // h1->GetXaxis()->SetTitle("Time [ns]");
        // h1->GetXaxis()->CenterTitle();
        // h1->GetYaxis()->SetTitle("Amplitude [V]");
        // h1->GetYaxis()->CenterTitle();
        // h1->GetYaxis()->SetTitleOffset(1.1);
        ecanv->Modified();
        ecanv->BuildLegend(0.75, 0.8, 0.99, 0.99);
        ecanv->Update();
      }

      for (int i = 0; i < ntrigs; i++) {
        ecanv->cd();
        TLine *line1 =
            new TLine(spar[i].t10, -spar[i].ampl, spar[i].tb10, -spar[i].ampl);
        line1->SetLineColor(4);
        line1->Draw();
        TLine *line2 = new TLine(spar[i].stime * dt, spar[i].sampl,
                                 spar[i].ftime * dt, spar[i].fampl);
        line2->SetLineColor(2);
        line2->Draw();
        TLine *line3 = new TLine(spar[i].ttrig, threshold,
                                 spar[i].ttrig + spar[i].tot, threshold);
        line3->SetLineColor(3);
        line3->Draw();
        // 	      std::cout<<i<<" \t "<<spar[i].ampl<< std::endl;
      }
      ecanv->Modified();
      ecanv->Update();

      dcanv->Modified();
      dcanv->BuildLegend(0.75, 0.8, 0.99, 0.99);
      dcanv->Update();
      //    continue;

      if (integralh != NULL) {
        TH1F *h2 = (TH1F *)integralh->GetHistogram();
        h2->SetMaximum(TMath::MaxElement(4, maxd) +
                       0.1 * TMath::Abs(TMath::MaxElement(4, maxd)));
        h2->SetMinimum(TMath::MinElement(4, mind) -
                       0.1 * TMath::Abs(TMath::MinElement(4, mind)));
        icanv->Modified();
        icanv->BuildLegend(0.75, 0.8, 0.99, 0.99);
        icanv->Update();
      }
    }
    //---------------end if(draw)--------------------------------

    if (ntrigs > 20000) {
      std::cout << ntrigs << " pulses in event " << eventNo << std::endl;
      return (ntrigs);
    }

    if (eventNo % (evpm) == 0) {
      std::cout << "Found ntrigs =" << ntrigs << " pulses for event " << eventNo
                << std::endl;
      std::cout << "Found ntrigsNEUTRONS =" << ntrigsNeutrons
                << " pulses for event " << eventNo << std::endl;
    }

    eventNo++;
  } // end of while (eventNo<nevents)

  if (draw) {
    gSystem->ChangeDirectory(tmpdir);
    return 0;
  }

  // For CASE 1 (pulsed beam)
  // hRateEvolution->Scale(1./timebinwidth);
  int abins = hRateEvolution->GetNbinsX();
  double *err0 = new double[abins + 1];
  for (int i = 0; i <= abins; i++)
    err0[i] = 0;
  hRateEvolutionCheck->SetError(err0);

  if (longpulse) {
    DivideHspark(hRateEvolution, hRateEvolutionCheck, hSparkEvolution);
    DivideHspark(hNeutronsRateEvolution, hRateEvolutionCheck, hSparkEvolution);
    if (bkg)
      DivideHspark(hBkgNeutronsRateEvolution, hRateEvolutionCheck,
                   hSparkEvolution);
  } else {
    DivideH(hRateEvolution, hRateEvolutionCheck);
    DivideH(hNeutronsRateEvolution, hRateEvolutionCheck);
  }

  ScaleHistoErr(hSparkEvolution, 1.0 / timebinwidth);

  // if CASE 1 (pulsed beam) with very few triggers per pulse
  // if( maxpoints < 10000) //(runtype == 'F')
  //  {
  for (int i = 0; i < hNeutronsRateEvolution->GetNbinsX(); i++) {
    double y = hNeutronsRateEvolution->GetBinContent(i);

    y *= timebinwidth; /// convert it to neutrons per time periode width
    //       double weight = sqrt(y) / hNeutronsRateEvolution->GetBinError(i);
    //        std::cout<<" y = "<<y<< std::endl;
    //       if (y>0.000025)
    hNeutronsRate->Fill(y);
  }
  //}

  /// ///////////////////////////////////
  ///  Amplitude and charge
  /// ///////////////////////////////////

  TCanvas *campl = new TCanvas("Amplitude" + ctypes, "Amplitude " + ctypes);
  campl->SetLogy();
  gStyle->SetOptFit(0);
  gStyle->SetOptStat(0);

  TF1 *f2 = new TF1("fexp", "[0]*exp(-[1]*x)", 0.8 * threshold, nTh * 2.);
  f2->SetTitle("#gamma fit");
  f2->SetLineColor(kGreen + 2);
  f2->SetParameter(1, 100.);
  f2->SetParLimits(1, 0.01, 1e7);

  hNeutronsAMPL->GetXaxis()->SetRangeUser(nTh, amplMax * mV * 0.75);
  double maxampl = hNeutronsAMPL->GetMaximum();
  int maxabin = hNeutronsAMPL->GetMaximumBin();
  hNeutronsAMPL->GetXaxis()->UnZoom();
  double maxax = hNeutronsAMPL->GetBinLowEdge(maxabin);
  double minax = nTh * mV;
  for (int i = maxabin; i > 1; i--) {
    double y = hNeutronsAMPL->GetBinContent(i);
    if (y < 0.5 * maxampl) {
      minax = hNeutronsAMPL->GetBinLowEdge(i);
      break;
    }
  }
  if (detNo == 613)
    minax = 0.045;
  // TF1* flandAmpl = new
  // TF1("flandAmpl","[0]*TMath::Landau(x,[1],[2],0)",nTh,amplMax);
  TF1 *flandAmpl = new TF1("flandAmpl", "landau", nTh, amplMax);
  flandAmpl->SetParameter(0, maxampl * 5.);
  flandAmpl->SetParLimits(0, 0.5 * maxampl, maxampl * 20.);
  flandAmpl->SetParLimits(2, 1.e-2, 1000.);
  flandAmpl->SetParLimits(1, 1.e-2, 1000.);
  flandAmpl->SetParameter(1, maxax);
  flandAmpl->SetParameter(2, 2.);
  // flandAmpl->SetParameter(0, hNeutronsAMPL->GetMaximum());

  // hAMPL->SetStats(0);
  gStyle->SetOptStat(1100);
  gStyle->SetOptFit(111);
  std::cout << "Fit " << 1 << std::endl;
  hNeutronsAMPL->Fit(flandAmpl, "B+0", "", minax, amplMax * mV);
  std::cout << "Fit " << 2 << std::endl;
  hNeutronsAMPL->Fit(flandAmpl, "BM+", "", minax, amplMax * mV);
  // std::cout<<"Fit "<<3<< std::endl;
  // hNeutronsAMPL->Fit(f2,"BM+0","",threshold*mV,nTh*1.0*mV);
  // std::cout<<"Fit "<<4<< std::endl;
  // hNeutronsAMPL->Fit(f2,"BM+","",threshold*mV,nTh*1.0*mV);
  // // hAMPL->Draw();
  hNeutronsAMPL->SetLineColor(2);
  hNeutronsAMPL->Draw("");
  hAMPL->Draw("same");
  // f2->Draw("same");
  campl->Update();
  campl->BuildLegend();
  campl->SaveAs(".png");
  campl->SaveAs(".pdf");

  hNeutronsCH->GetXaxis()->SetRangeUser(nThCh, chMax * 0.75);
  double maxch = hNeutronsCH->GetMaximum();
  int maxchbin = hNeutronsCH->GetMaximumBin();
  hNeutronsCH->GetXaxis()->UnZoom();
  double maxchx = hNeutronsCH->GetBinLowEdge(maxchbin);
  double minchx = nThCh;
  for (int i = maxchbin; i > 1; i--) {
    double y = hNeutronsCH->GetBinContent(i);
    if (y < 0.6 * maxch) {
      minchx = hNeutronsCH->GetBinLowEdge(i);
      break;
    }
  }
  TF1 *flandCh = new TF1("flandCh", "landau", nTh, chMax);
  flandCh->SetParameter(0, maxch * 5);
  flandCh->SetParLimits(0, 0.1 * maxch, maxch * 10.);
  flandCh->SetParameter(1, maxchx);
  flandCh->SetParLimits(2, 1.e-2, 1000.);
  flandCh->SetParLimits(1, 1.e-2, 1000.);

  //   flandCh->SetParLimits(2,1.e-2,  1000.);
  //   flandCh->SetParLimits(1,1.e-2,  1000.);

  TCanvas *cch = new TCanvas("Charge" + ctypes, "Charge " + ctypes);
  TF1 *f3 = new TF1("fexpCH", "[0]*exp(-[1]*x)", 0., nThCh * 2.);
  f3->SetTitle("#gamma fit");
  f3->SetParLimits(0, 1., 100000);
  f3->SetParameter(0, 1000.);
  f3->SetParameter(1, 10.);
  f3->SetParLimits(1, 1., 1000.);
  f3->SetLineColor(kGreen + 2);

  hNeutronsCH->SetStats(1);
  std::cout << "Fit " << 5 << std::endl;
  hNeutronsCH->Fit(f3, "E+", "", threshold, nThCh * 1.0);
  std::cout << "Fit " << 6 << std::endl;
  hNeutronsCH->Fit(flandCh, "B+0", "", minchx, chMax);
  std::cout << "Fit " << 7 << std::endl;
  hNeutronsCH->Fit(flandCh, "BM+", "", minchx, chMax);

  hNeutronsCH->SetMaximum(2. * hCH->GetMaximum());
  cch->SetLogy();
  hNeutronsCH->Draw();
  //   hCH->Sumw2();
  hsCH->SetLineColor(kMagenta);
  //   hsCH->Draw("same");
  hNeutronsCH->SetLineColor(2);
  hCH->Draw("same");
  f3->Draw("same");
  cch->Update();
  cch->BuildLegend();
  cch->SaveAs(".png");
  cch->SaveAs(".pdf");
  ///________________________________________________________________

  /// ///////////////////////////////////
  ///  		Rates
  /// ///////////////////////////////////
  /// This is the main canvas. Use the if 1 to easily group it
  gSystem->ChangeDirectory(plotdirname);

  TCanvas *rcanv =
      new TCanvas("Rates_" + ctypes, "Rates " + ctypes, 1600, 1100);
  rcanv->Divide(3, 2);
  rcanv->cd(1);
  gStyle->SetOptFit(111);
  gStyle->SetOptStat(1110);

  double mean = hNeutronsRate->GetMean();
  double rms = hNeutronsRate->GetRMS();
  double maxrate = hNeutronsRate->GetMaximum();
  double nentries = hNeutronsRate->GetEntries();
  int fgaus = 0;
  TF1 *frate;
  //   mean=3.3;
  //   rms=1.0;
  if (mean > 6.5) {
    frate = new TF1("gaus", "gaus", 0., 2000.);
    frate->SetParameter(1, 1.1 * mean);
    frate->SetParameter(2, rms);
    frate->SetParLimits(0, 0.2 * maxrate, 3. * maxrate);
    frate->SetParLimits(1, 0.5 * mean, 2. * mean);
    frate->SetParLimits(2, sqrt(mean) / 5., 2. * sqrt(mean));
    fgaus = 1;
  } else {
    frate = new TF1("pois", "[0]*TMath::Poisson(x,[1])", 0, 20);
    frate->SetParameter(0, 1);
    frate->SetParameter(1, mean);
  }
  frate->SetNpx(2000);
  hNeutronsRate->Sumw2();
  std::cout << "Fit " << 8 << std::endl;
  hNeutronsRate->Fit(frate, "WBN+", "", mean - 3. * rms, mean + 5. * rms);
  std::cout << "Fit " << 9 << std::endl;
  hNeutronsRate->Fit(frate, "MEB+", "", mean - 3. * rms, mean + 5. * rms);
  hNeutronsRate->SetLineColor(2);
  hNeutronsRate->Draw("");
  //   frate->Draw("same");
  char txt2[200];

  TF1 *fbsl2;
  double mean2 = hRate->GetMean();
  double rms2 = hRate->GetRMS();
  int fgaus2 = 0;
  // //   mean2 = 3.;
  if (longpulse) /// Only when long pulse the hRate histo can be used
  {
    hRate->Draw("same");
    rms = 1.1;
    if (mean2 > 6.5) {
      fbsl2 = new TF1("gaus2", "gaus", 0., 2000.);
      fbsl2->SetParameter(1, mean2);
      fbsl2->SetParameter(2, rms2);
      fgaus2 = 1;
    } else {
      fbsl2 = new TF1("pois2", "[0]*TMath::Poisson(x,[1])", 0, 20);
      fbsl2->SetParameter(0, 1);
      fbsl2->SetParameter(1, mean2);
    }
    hRate->Sumw2();
    std::cout << "Fit " << 10 << std::endl;
    hRate->Fit(fbsl2, "W+", "same", 1.6, 20.);
    std::cout << "Fit " << 11 << std::endl;
    hRate->Fit(fbsl2, "RE+", "same", 1.6, 20.);
    fbsl2->Draw("same"); // no point to draw these
  }

  double Ib = 0.;
  sscanf(ftype, "%lfuA", &Ib);

  double crate = frate->GetParameter(1);

  //   rcanv->Update();
  rcanv->cd(2);
  //   gStyle->SetOptStat(0);
  hRateEvolution->SetStats(0);
  hNeutronsRateEvolution->SetStats(0);
  double maxy = hRateEvolution->GetMaximum() * 1.6;
  hRateEvolution->SetMaximum(200);
  //   hRateEvolution->Draw();
  hNeutronsRateEvolution->SetLineColor(2);
  hNeutronsRateEvolution->SetLineWidth(2);
  hNeutronsRateEvolution->Draw("");

  if (bkg) {
    hBkgNeutronsRateEvolution->SetLineColor(kGreen + 2);
    hBkgNeutronsRateEvolution->SetLineWidth(2);
    hBkgNeutronsRateEvolution->Draw("sameE0");
  }

  hSparkEvolution->SetLineColor(4);
  hSparkEvolution->SetLineWidth(2);
  hSparkEvolution->Draw("sameE0");

  /*TPad *rpad = new TPad("RateEvolution", "rate evolution",0.5,0.58,0.9,0.88);
    rpad->Draw();
    rpad->cd();
    rpad->SetFillColor(0);
    rpad->SetBorderMode(0);
   rpad->SetBorderSize(2);
   rpad->SetTopMargin(0.01);
   rpad->SetTopMargin(0.008);
   rpad->SetBottomMargin(0.185);
   rpad->SetFrameBorderMode(0);*/ ///To place it in same canvas as hRateStructure

  //   rcanv->Update();
  rcanv->cd(3);
  gPad->SetLogy();
  gStyle->SetOptFit(111);
  gStyle->SetOptStat(1110);

  //  hAMPL->SetStats(0);
  //  hAMPL->Draw();
  hNeutronsAMPL->SetLineColor(2);
  hNeutronsAMPL->Draw();
  hAMPL->Draw("same");

  TPad *subpad =
      (TPad *)rcanv->GetPrimitive(Form("%s_%d", rcanv->GetName(), 3));
  subpad->Update(); /// VERY IMPORTANT!!!!! crashes without update!!!
  TPaveStats *st = (TPaveStats *)subpad->GetPrimitive("stats");
  // if (st==NULL) return -3243;
  st->SetX1NDC(0.55); // new x start position
  st->SetY1NDC(0.65); // new x end position
  st->SetTextSize(0.03);
  subpad->Modified();

  //   rcanv->Update();
  rcanv->cd(6);
  gPad->SetLogy();
  hNeutronsCH->Draw("");
  hCH->Draw("same");
  hNeutronsCH->SetLineColor(2);

  subpad = (TPad *)rcanv->GetPrimitive(Form("%s_%d", rcanv->GetName(), 6));
  subpad->Update(); /// VERY IMPORTANT!!!!! crashes without update!!!
  st = (TPaveStats *)subpad->GetPrimitive("stats");
  // if (st==NULL) return -3243;
  st->SetX1NDC(0.55); // new x start position
  st->SetY1NDC(0.65); // new x end position
  st->SetTextSize(0.03);
  subpad->Modified();

  //  std::cout<<"______________________________"<< std::endl;
  //  subpad->ls();
  //  std::cout<<"______________________________"<< std::endl;
  //  subpad->ls();
  //  std::cout<<"______________________________"<< std::endl;
  //  hNeutronsAMPL->ls();
  //  std::cout<<"______________________________"<< std::endl;
  //  rcanv->ls();
  //  std::cout<<"______________________________"<< std::endl;

  //   rcanv->Update();
  rcanv->cd(4);
  double ttot = tmax - 0.;
  // exponential fit
  TF1 *f1 = new TF1("f1", "[0]*exp(-[1]*x)", 0., 200.);

  f1->SetParLimits(1, 1e-6, 1e6);
  f1->SetParLimits(0, 1e-6, 1e6);

  f1->SetParameter(0, nevents / 5.);
  f1->SetParameter(1, exprate);
  f1->SetLineColor(4);

  hDt->Sumw2(1);
  hNeutronsDt->Sumw2(1);

  double tbeam = 5e-6;

  std::cout << "Fit " << 12 << std::endl;
  hDt->Fit(f1, "+QB0", "", tmax / 20., tmax); // crate[4]);

  double rates = 1. * f1->GetParameter(1);
  char title[100];
  sprintf(title, "f(x) = %g e^{-%g x}", f1->GetParameter(0),
          f1->GetParameter(1));
  f1->SetTitle(title);

  gStyle->SetOptFit(111);
  gStyle->SetOptStat(110);
  std::cout << "total rate = " << crate << " c/s \t calculated = " << rates
            << " c/s " << std::endl;
  hDt->SetLineColor(4);
  hDt->SetMarkerColor(4);
  hDt->SetMarkerStyle(2);
  hDt->Draw("E0");
  hDt->SetMinimum(0.1);
  hDt->SetMaximum(hDt->GetMaximum() * 2.);

  hNeutronsDt->Draw("same");

  TF1 *f11 = new TF1("f11", "[0]*exp(-[1]*x)", 0., tmax * 2.);
  //  f11->SetParLimits(1,1e2,1e6);
  //  f11->SetParameter(1,1e3);
  //  f11->SetLineColor(kBlue+2);
  //  hDt->Fit(f11,"+QB0","",0.,tbeam);//crate[4]);

  /* if(bkg)
      flinear->Draw("same");
      else*/
  f1->Draw("same");
  //  f11->Draw("same");

  hNeutronsDt->SetLineColor(kRed + 1);
  TF1 *f12 = new TF1("f12", "[0]*exp(-[1]*x)", 0., tmax * 2);
  f12->SetParLimits(1, 1e-6, 1e6);
  f12->SetParameter(0, 120.);
  f12->SetParameter(1, exprate);
  f12->SetParLimits(0, 0.1e-6, 1e6);
  f12->SetLineColor(kRed + 2);
  std::cout << "Fit " << 13 << std::endl;
  hNeutronsDt->Fit(f12, "+B0", "", tmax / 100., tmax); // crate[4]);
  f12->Draw("same");

  TPad *pad = (TPad *)gPad;
  pad->SetLogy();

  subpad = (TPad *)rcanv->GetPrimitive(Form("%s_%d", rcanv->GetName(), 4));
  subpad->Update(); /// VERY IMPORTANT!!!!! crashes without update!!!
  st = (TPaveStats *)subpad->GetPrimitive("stats");
  // if (st==NULL) return -3243;
  st->SetX1NDC(0.55); // new x start position
  st->SetY1NDC(0.65); // new x end position
  st->SetTextSize(0.03);
  subpad->Modified();

  std::cout << "ntrigsTotal = " << ntrigsTot << std::endl;
  double SF = ntrigsTot / (f1->GetParameter(1));
  std::cout << "SF = " << SF << std::endl;

  //   rcanv->Update();
  rcanv->cd(5);
  sprintf(
      txt2,
      " #it{detector} #color[4]{#bf{S%03d}}  \t  #it{run} #color[4]{#bf{%02d}}",
      detNo, runNo);
  TLatex *tex = new TLatex(0.02, 0.95, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.065);
  tex->SetLineWidth(2);
  tex->SetTextAlign(12);
  tex->Draw();

  sprintf(txt2,
          " V_{m} = #color[4]{-%d}V, V_{d} = #color[4]{-%d}V, peak = "
          "#color[4]{%g}mV, n = #color[4]{%g}mV",
          vm, vd, fabs(threshold * 1000.), fabs(nTh * 1000.));
  sprintf(txt2,
          " V_{m} = #bf{-%d V}, V_{d} = #bf{-%d V}, peak = #bf{%g mV}, n = "
          "#bf{%g mV}",
          vm, vd, fabs(threshold * 1000.), fabs(nTh * 1000.));
  sprintf(txt2, " V_{m} = #bf{-%d V},  V_{d} = #bf{-%d V}", vm, vd);
  tex = new TLatex(0.5, 0.88, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  sprintf(txt2, " #it{Thresholds:}  peak = #bf{%g mV},  n = #bf{%g mV}",
          fabs(threshold * 1000.), fabs(nTh * 1000.));
  tex = new TLatex(0.5, 0.80, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  double calcr = frate->GetParameter(1);
  double calcrerr = frate->GetParError(1);
  if (fgaus) {
    calcr = frate->GetParameter(1);
    calcrerr = frate->GetParameter(2);
  }
  sprintf(txt2,
          "#LTr_{n}#GT = #bf{%3.2f #pm %3.3f} #frac{n}{#it{%gs}} = #bf{%3.2f "
          "#pm %3.3f} s^{-1}",
          calcr, calcrerr, timebinwidth, calcr / timebinwidth,
          calcrerr / timebinwidth);
  //  if (fgaus)
  //    sprintf(txt2,"#LTr_{n(#it{#Deltat=%gs})}#GT = #bf{%3.2f #pm %3.3f}
  //    #frac{n}{pulse}",timebinwidth,frate->GetParameter(1),frate->GetParameter(2));
  tex = new TLatex(0.5, 0.7, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#DeltaT fit, all particles:");
  tex = new TLatex(0.5, 0.6, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2,
          "#LTr_{all events}#GT = #bf{%3.2f #pm %3.2f} #frac{counts}{sec}",
          f1->GetParameter(1), f1->GetParError(1));
  if (exprate < 1)
    sprintf(txt2,
            "#LTr_{all events}#GT = #(){#bf{%3.1f #pm %3.2f}}#times10^{-3} "
            "#frac{counts}{sec}",
            f1->GetParameter(1) * 1e3, f1->GetParError(1) * 1e3);
  tex = new TLatex(0.5, 0.55, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  runpar->grate = f1->GetParameter(1);
  runpar->sgrate = f1->GetParError(1);

  sprintf(
      txt2,
      "#LTr_{n(#it{thr=%gmV})}#GT = #bf{%3.2f #pm %3.2f} #frac{counts}{sec}",
      nTh * mV, f12->GetParameter(1), f12->GetParError(1));
  if (exprate < 1)
    sprintf(txt2,
            "#LTr_{n(#it{thr=%gmV})}#GT = #(){#bf{%3.1f #pm "
            "%3.1f}}#times10^{-3} #frac{counts}{sec}",
            nTh * mV, f12->GetParameter(1) * 1000.,
            f12->GetParError(1) * 1000.);
  tex = new TLatex(0.5, 0.46, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  runpar->rate = f12->GetParameter(1);
  runpar->srate = f12->GetParError(1);

  long double duration = (epochF - epochS) / 3600.;
  sprintf(txt2, "#it{#bf{%d sparks} were observed within #bf{%3.1LF} hours}",
          totNsparks, duration);
  if (totNsparks == 0)
    sprintf(txt2, "#it{#bf{No spark} was observed within #bf{%3.1LF} hours}",
            duration);
  else if (totNsparks == 1)
    sprintf(txt2, "#it{#bf{%d spark} was observed within #bf{%3.1LF} hours}",
            totNsparks, duration);

  tex = new TLatex(0.5, 0.335, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#it{Landau fits}");
  tex = new TLatex(0.5, 0.25, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#it{Ampl}: %s = #bf{%3.1f #pm %3.2f} mV , %s = #bf{%5.3e} ",
          flandAmpl->GetParName(1), flandAmpl->GetParameter(1) * 1000. / mV,
          flandAmpl->GetParError(1) * 1000. / mV, flandAmpl->GetParName(2),
          flandAmpl->GetParameter(2));
  tex = new TLatex(0.5, 0.18, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  runpar->ampl = flandAmpl->GetParameter(1);
  runpar->sampl = flandAmpl->GetParameter(2);

  sprintf(txt2, "#it{Charge}: %s = #bf{%3.1f #pm %3.2f} , %s = #bf{%5.3e} ",
          flandCh->GetParName(1), flandCh->GetParameter(1),
          flandCh->GetParError(1), flandCh->GetParName(2),
          flandCh->GetParameter(2));
  tex = new TLatex(0.5, 0.1, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  runpar->charge = flandCh->GetParameter(1);
  runpar->scharge = flandCh->GetParameter(2);

  rcanv->Update();

  gSystem->ChangeDirectory(allplotdirname);
  rcanv->SaveAs(".png");
  rcanv->SaveAs(".pdf");

  TCanvas *canvrtev =
      new TCanvas("RateEvolution_" + ctypes, "Rate Evolution " + ctypes);
  hNeutronsRateEvolution->Draw("");
  if (bkg)
    hBkgNeutronsRateEvolution->Draw("sameE0");
  hSparkEvolution->Draw("sameE0");
  //   hRateEvolution->Draw("sameE0");
  canvrtev->BuildLegend();
  canvrtev->Modified();
  canvrtev->Update();

  gSystem->ChangeDirectory(allplotdirname);
  canvrtev->SaveAs(".png");
  canvrtev->SaveAs(".pdf");

  /// end of rate plots

  // // // //

  TCanvas *camplch =
      new TCanvas("CHovAmplitude" + ctypes, "Charge over Amplitude " + ctypes);
  gStyle->SetOptFit(111);
  gStyle->SetOptStat(1100);
  hsCHovAmpl->Sumw2();
  hsCHovAmpl->Draw();
  hCHovAmpl->Draw("same");

  int maxbin = hCHovAmpl->GetMaximumBin();
  double maxbinval = hCHovAmpl->GetBinLowEdge(maxbin);
  TF1 *fgaus3 = new TF1("fgaus4", "gaus", maxbin - 100., maxbin + 100.);
  fgaus3->SetParameter(2, 50.);
  fgaus3->SetParameter(0, hCHovAmpl->GetMaximum());
  fgaus3->SetParameter(1, maxbinval);
  std::cout << "Fit " << 14 << std::endl;
  hCHovAmpl->Fit(fgaus3, "W", "", maxbinval - 150., maxbinval + 150.);
  runpar->chovampl = fgaus3->GetParameter(1);
  runpar->schovampl = fgaus3->GetParameter(2);

  gStyle->SetOptFit(111);

  TCanvas *sccanv =
      new TCanvas("SignalCuts" + ctypes, "Signal Cuts " + ctypes, 1600, 1100);
  sccanv->Divide(4, 2);

  sccanv->cd(1);
  gStyle->SetOptStat(1100);

  hCHovAmpl->Draw();
  hsCHovAmpl->Draw("same");

  sccanv->cd(2);
  hRateStructure->SetStats(0);
  hRateStructure->GetXaxis()->SetLabelSize(0.05);
  hRateStructure->GetXaxis()->SetTitleSize(0.05);
  hRateStructure->GetYaxis()->SetLabelSize(0.05);
  maxy = hRateStructure->GetMaximum() * 1.2;
  hRateStructure->SetMaximum(maxy);
  hRateStructure->Draw();
  hNeutronsRateStructure->SetLineColor(2);
  hNeutronsRateStructure->Draw("same");
  hStructTrigger->Draw("same");

  TCanvas *canvstruct =
      new TCanvas("RateStructures_" + ctypes, "Rate Structures " + ctypes);
  hRateStructure->Draw();
  hNeutronsRateStructure->Draw("same");
  hStructTrigger->Draw("same");
  canvstruct->BuildLegend();
  canvstruct->Modified();
  canvstruct->Update();
  gSystem->ChangeDirectory(allplotdirname);
  canvstruct->SaveAs(".png");
  canvstruct->SaveAs(".pdf");

  sccanv->cd(2);
  hRateStructTrigger->SetMinimum(0.1);
  hRateStructTrigger->SetMaximum(2000);
  hRateStructTrigger->Draw("same");
  gPad->SetLogy();
  hNeutronsRateStructTrigger->SetMinimum(0.1);
  hNeutronsRateStructTrigger->SetLineColor(2);
  hNeutronsRateStructTrigger->Draw("same");
  gPad->BuildLegend();

  sccanv->cd(6);
  gStyle->SetOptStat(1100);
  hNeutronsRT->Draw("");
  hRT->Draw("same");
  hNeutronsRT->SetLineColor(2);
  TF1 *fgrt = new TF1("fgrt", "gaus", 0., rtMax);
  fgrt->SetNpx(1000);
  fgrt->SetParameter(0, hNeutronsRT->GetMaximum());
  fgrt->SetParameter(1, hNeutronsRT->GetMean());
  hNeutronsRT->Sumw2();
  std::cout << "Fit " << 15 << std::endl;
  hNeutronsRT->Fit(fgrt, "mb+", "same",
                   hNeutronsRT->GetMean() - 3 * hNeutronsRT->GetRMS(),
                   hNeutronsRT->GetMean() + 3 * hNeutronsRT->GetRMS());
  //  hNeutronsRT->Draw("same");
  runpar->risetime = fgrt->GetParameter(1);
  runpar->srisetime = fgrt->GetParameter(2);

  sccanv->cd(4);
  hNeutronsPW->SetLineColor(2);
  hNeutronsPW->Draw("");
  hPW->Draw("same");
  TF1 *fgw = new TF1("fgw", "gaus", 0., pwMax);
  fgw->SetNpx(1000);
  fgw->SetParameter(0, hNeutronsPW->GetMaximum());
  runpar->width = hNeutronsPW->GetMean();
  fgw->SetParameter(1, runpar->width);
  hNeutronsPW->Sumw2();
  std::cout << "Fit " << 16 << std::endl;
  hNeutronsPW->Fit(fgw, "mb+", "same",
                   runpar->width - 3 * hNeutronsPW->GetRMS(),
                   runpar->width + 3 * hNeutronsPW->GetRMS());
  runpar->width = fgw->GetParameter(1);
  runpar->swidth = fgw->GetParameter(2);

  sccanv->cd(3);
  hNeutronsTOT->Draw("");
  hTOT->Draw("same");
  hNeutronsTOT->SetLineColor(2);
  TF1 *fgtot = new TF1("fgtot", "gaus", 0., rtMax);
  fgtot->SetNpx(1000);
  fgtot->SetParameter(0, hNeutronsTOT->GetMaximum());
  fgtot->SetParameter(1, hNeutronsTOT->GetMean());
  hNeutronsTOT->Sumw2();
  std::cout << "Fit " << 17 << std::endl;
  hNeutronsTOT->Fit(fgtot, "mb+", "same",
                    hNeutronsTOT->GetMean() - 3 * hNeutronsTOT->GetRMS(),
                    hNeutronsTOT->GetMean() + 3 * hNeutronsTOT->GetRMS());
  //  hNeutronsRT->Draw("same");
  runpar->tot = fgtot->GetParameter(1);
  runpar->stot = fgtot->GetParameter(2);

  sccanv->cd(7);
  hAmplvsRT->Draw("colz");
  sccanv->cd(5);
  hCHvsAMPL->Draw("colz");
  sccanv->cd(8);
  hAmplvsTOT->Draw("colz");

  sccanv->Modified();
  sccanv->Update();
  sccanv->SaveAs(".png");
  sccanv->SaveAs(".pdf");

  /// ....................................................... ///
  /// /////////////// drawing summary plots /////////////////////
  /// /////////////// output in one ps file /////////////////////
  /// _______________________________________________________ ///

  //   sprintf(psname,"%s/Summary%s.ps",plotdirname,ctypes.Data());
  //   TPostScript *psfile = new TPostScript(psname,4121);
  //   psfile->NewPage();

  char stst[1000];
  sprintf(stst, "/%s Summary plots\n", ctypes.Data());
  //   psfile->PrintRaw();

  TCanvas *pcanv =
      new TCanvas("Performace_" + ctypes, "Performance " + ctypes, 1600, 1100);
  pcanv->Divide(3, 2);
  pcanv->cd(6);
  gStyle->SetOptFit(111);
  gStyle->SetOptStat(1110);

  hNeutronsRate->Draw("");
  ///  fbsl2->Draw("same");  // no point to draw these

  pcanv->cd(5);
  hNeutronsRateEvolution->Draw("");

  if (bkg) {
    hBkgNeutronsRateEvolution->Draw("sameE0");
  }
  hSparkEvolution->Draw("sameE0");

  pcanv->cd(2);
  hNeutronsAMPL->Draw();
  pad = (TPad *)gPad;
  pad->SetLogy();
  hAMPL->Draw("same");

  //   pcanv->Update();
  pcanv->cd(3);
  gPad->SetLogy();
  hNeutronsCH->Draw("");
  hCH->Draw("same");
  //   pcanv->Update();
  pcanv->cd(4);
  hDt->Draw("E0");
  hNeutronsDt->Draw("same");

  f1->Draw("same");
  f12->Draw("same");

  pad = (TPad *)gPad;
  pad->SetLogy();

  //   pcanv->Update();
  pcanv->cd(1);

  sprintf(
      txt2,
      " #it{detector} #color[4]{#bf{S%03d}}  \t  #it{run} #color[4]{#bf{%02d}}",
      detNo, runNo);
  tex = new TLatex(0.02, 0.95, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.065);
  tex->SetLineWidth(2);
  tex->SetTextAlign(12);
  tex->Draw();

  //   sprintf(txt2," V_{m} = #color[4]{-%d}V, V_{d} = #color[4]{-%d}V, peak =
  //   #color[4]{%g}mV, n =
  //   #color[4]{%g}mV",vm,vd,fabs(threshold*1000.),fabs(nTh*1000.));
  //   sprintf(txt2," V_{m} = #bf{-%d V}, V_{d} = #bf{-%d V}, peak = #bf{%g mV},
  //   n = #bf{%g mV}",vm,vd,fabs(threshold*1000.),fabs(nTh*1000.));
  sprintf(txt2, " V_{m} = #bf{-%d V},  V_{d} = #bf{-%d V}", vm, vd);
  tex = new TLatex(0.5, 0.88, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  sprintf(txt2, " #it{Thresholds:}  peak = #bf{%g mV},  n = #bf{%g mV}",
          fabs(threshold * 1000.), fabs(nTh * 1000.));
  tex = new TLatex(0.5, 0.80, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();
  sprintf(txt2,
          "#LTr_{n}#GT = #bf{%3.2f #pm %3.3f} #frac{n}{#it{%gs}} = #bf{%3.2f "
          "#pm %3.3f} s^{-1}",
          calcr, calcrerr, timebinwidth, calcr / timebinwidth,
          calcrerr / timebinwidth);
  //  if (fgaus)
  //    sprintf(txt2,"#LTr_{n(#it{#Deltat=%gs})}#GT = #bf{%3.2f #pm %3.3f}
  //    #frac{n}{pulse}",timebinwidth,frate->GetParameter(1),frate->GetParameter(2));
  tex = new TLatex(0.5, 0.7, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#DeltaT fit, all particles:");
  tex = new TLatex(0.5, 0.6, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2,
          "#LTr_{all events}#GT = #bf{%3.2f #pm %3.2f} #frac{counts}{sec}",
          f1->GetParameter(1), f1->GetParError(1));
  if (exprate < 1)
    sprintf(txt2,
            "#LTr_{all events}#GT = #(){#bf{%3.1f #pm %3.2f}}#times10^{-3} "
            "#frac{counts}{sec}",
            f1->GetParameter(1) * 1e3, f1->GetParError(1) * 1e3);
  tex = new TLatex(0.5, 0.55, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(
      txt2,
      "#LTr_{n(#it{thr=%gmV})}#GT = #bf{%3.2f #pm %3.2f} #frac{counts}{sec}",
      nTh * mV, f12->GetParameter(1), f12->GetParError(1));
  if (exprate < 1)
    sprintf(txt2,
            "#LTr_{n(#it{thr=%gmV})}#GT = #(){#bf{%3.1f #pm "
            "%3.1f}}#times10^{-3} #frac{counts}{sec}",
            nTh * mV, f12->GetParameter(1) * 1000.,
            f12->GetParError(1) * 1000.);
  tex = new TLatex(0.5, 0.46, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#it{#bf{%d sparks} were observed within #bf{%3.1LF} hours}",
          totNsparks, duration);
  if (totNsparks == 0)
    sprintf(txt2, "#it{#bf{No spark} was observed within #bf{%3.1LF} hours}",
            duration);
  else if (totNsparks == 1)
    sprintf(txt2, "#it{#bf{%d spark} was observed within #bf{%3.1LF} hours}",
            totNsparks, duration);

  tex = new TLatex(0.5, 0.335, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetTextColor(kRed);
  if (totNsparks == 0)
    tex->SetTextColor(kBlue);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#it{Landau fits}");
  tex = new TLatex(0.5, 0.25, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#it{Ampl}: %s = #bf{%3.1f #pm %3.2f} mV , %s = #bf{%5.3e} ",
          flandAmpl->GetParName(1), flandAmpl->GetParameter(1) * 1000. / mV,
          flandAmpl->GetParError(1) * 1000. / mV, flandAmpl->GetParName(2),
          flandAmpl->GetParameter(2));
  tex = new TLatex(0.5, 0.18, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  sprintf(txt2, "#it{Charge}: %s = #bf{%3.1f #pm %3.2f} , %s = #bf{%5.3e} ",
          flandCh->GetParName(1), flandCh->GetParameter(1),
          flandCh->GetParError(1), flandCh->GetParName(2),
          flandCh->GetParameter(2));
  tex = new TLatex(0.5, 0.1, txt2);
  tex->SetTextFont(132);
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->SetTextAlign(23);
  tex->Draw();

  gSystem->ChangeDirectory(plotdirname);
  pcanv->SaveAs(".png");
  pcanv->SaveAs(".pdf");
  pcanv->Update();
  //   psfile->Off();

  //  psfile->NewPage();

  //  psfile = new TPostScript(psname,4121);
  //  psfile->NewPage();

  TCanvas *scanv =
      new TCanvas("Signals_" + ctypes, "Signal timing " + ctypes, 1600, 1100);
  scanv->Divide(3, 2);

  scanv->cd(1);
  gStyle->SetOptStat(1100);
  hNeutronsRT->Draw("");
  hRT->Draw("same");
  //  fgrt->SetNpx(1000);

  scanv->cd(2);
  hNeutronsPW->SetLineColor(2);
  hNeutronsPW->Draw("");
  hPW->Draw("same");
  //  fgw->SetNpx(1000);

  scanv->cd(3);
  hNeutronsTOT->Draw("");
  hTOT->Draw("same");
  //  fgtot->SetNpx(1000);

  scanv->cd(4);

  hsCHovAmpl->Draw();
  hCHovAmpl->Draw("same");

  scanv->cd(5);

  h2dNeutronsAMPL->Draw("colz");

  scanv->cd(6);

  h2dNeutronsCH->Draw("colz");

  scanv->Update();

  //  psfile->NewPage();

  char pname[1000];
  sprintf(pname, "PedestalsS%03d-%02d-%d-%d-%s", detNo, runNo, vm, vd, RTYPE);
  TCanvas *pedcanv = (TCanvas *)ifile->Get(pname);
  pedcanv->Draw();
  pedcanv->Update();
  gSystem->ChangeDirectory(plotdirname);
  pedcanv->SaveAs(".png");
  // // // pedcanv->SaveAs(".pdf");
  pedcanv->Write();

  //  psfile->Close();

  gSystem->ChangeDirectory(plotdirname);
  scanv->Modified();
  scanv->Update();
  scanv->SaveAs(".png");
  scanv->SaveAs(".pdf");
  scanv->Write();

  //   psfile->NewPage();
  //   scanv->Print(psname);
  //   psfile->Close();

  //  pcanv->Modified();
  //  pcanv->Draw();
  //  pcanv->Update();
  scanv->Modified();
  //  scanv->Draw();

  char psname[1000];
  sprintf(psname, "Summary_%s.pdf", ctypes.Data());
  char snames[1000];
  sprintf(snames, "*S%03d_%02d*.png *S%03d-%02d*.png", detNo, runNo, detNo,
          runNo);

  sprintf(command, "cd %s\nmontage %s -tile 1x2 -geometry 1600 %s", plotdirname,
          snames, psname);
  std::cout << "Creating Summary file:\n" << command << std::endl;
  int comtst = system(command);
  std::cout << "Done!" << std::endl;

  RegisterRunParameters(runpar, basedirname);

  ofile->Write("", TObject::kOverwrite);
  gSystem->ChangeDirectory(tmpdir);

  return 0;
}
