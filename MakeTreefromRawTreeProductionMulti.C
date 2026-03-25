#include <TROOT.h>
#include <TSystem.h>
#include <TMath.h>
#include <TH1.h>
#include <TTree.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TGaxis.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TLatex.h>
#include <TString.h>
#include <iomanip>
#include <TTimeStamp.h>
#include "MyFunctions.C"

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
// 	cout<<fnames<<endl;
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
// //          cout<<"["<<i<<" | "<<X[i]<<"] - "; 
//        }
//        if (X[i]>mean+4*sigma)
// 	 break;
//    }
//    return (sum);
// }

int FindZeros(int n, double* data)
{
   int n0=0;
   for (int i=1;i<n;i++)
   {
     if (data[i]==0.0)
       if (data[i-1]==0)
	 n0 ++; 
//      cout<<"["<<data[i]<<"/"<<(data[i]==0)<<"] ";
   }
   return n0;
}

int month2int_littleendian(const char *month)
{
  const char hash[] = { 3, 12, 8, 2, 1, 11, 7, 5, 0, 10, 4, 9, 6 };
  return hash[(* (const int32_t *) month & ~0x20202020 + 146732) % 13];
}


int MakeTreefromRawTreeProductionMulti(int detNo=0, int runNo=1, int trigger=0, string filetype = "")  ///put trigger = ChannelNo chosen to analyse! 
{
  gROOT->LoadMacro("MyFunctions.C");
  
  if (trigger>4){
    cout<<"Wrong trigger channel : "<<trigger<<endl;
    return -15;
  }
  
  const double mV =1000.;
  
  int mask[] = {0,0};
  
  int vm=475;
  int vd=850;

  float bthick,dgap;
  int angle=0;
  
  char command[5000];
  char fname[4][5000];
  char fname2[5000],ofilename[5000],ifilename[5000];
  char fnametmp[2000];
  vector<string> fnames[4];

  char ofname[1000];

  char rtype[1000];
  strcpy(rtype,RTYPE);

  char basedirname[1000];
  char outdirname[1000];
  sprintf(basedirname,"%s",BASEDIRNAME);
  sprintf(outdirname,"%s",OUTDIRNAME);

  if (detNo>=RUNMIN3 && detNo<=RUNMAX3)
  {
    char afile[500]; 
    sprintf(afile,"%s/tmpfile.tmp",basedirname);
    FILE *ftmp=fopen(afile,"w");
    if (ftmp == NULL)
    {
      cout<<afile<<" can not be created. Probably the directory '"<<basedirname<<"' does not exit. Exiting..."<<endl;
      exit (-12);
    }
    fclose(ftmp);
    sprintf(command,"cd %s\nls -d S%06d-%02d-*%s_%s*.root > %s 2>/dev/null",basedirname,detNo,runNo,filetype.c_str(),rtype,afile);

    int tst=system(command);
//    cout<<command<<endl<<"returned: "<<tst<<endl;
    if (tst !=0)
    {
      cout<<command<<endl<<"returned: "<<tst<<endl;
      cout<<"Probably the tree of the run "<<runNo<<" of detector No S"<<detNo<<" was not found in directory "<<basedirname<<endl<<"Exiting..."<<endl;
      return tst;
    }
    ftmp=fopen(afile,"r");
    if (fgets(fnametmp,200,ftmp) == NULL)
    {
      cout<<"Failed to read the filename for detector S"<<detNo<<" run "<<runNo<<" at "<<basedirname<<endl<<"Exiting..."<<endl;
      return -2;
    }
    int rtmp,dtmp;
    char ftypetmp[500];
    strcpy(ftypetmp,"");
    int stst = sscanf(fnametmp,"S%06d-%02d-%d-%d_%30[^ /,\n\t]",&dtmp,&rtmp,&vm,&vd,ftypetmp);
    cout <<"arguments read = "<<stst<<endl;
    filetype.assign(ftypetmp);
    cout<<"detector = "<< std::setfill('0') << std::setw(6)<<dtmp<<" (S"<< std::setfill('0') << std::setw(6)<<detNo<<")"<<endl;
    if (trigger==1)
    {
        cout<< "chosen osciloscope channel = "<<trigger<<endl;
        dtmp = dtmp / 10000;
        if (dtmp<50)
          cout<< "chosen detector is = S"<< std::setfill('0') << std::setw(3)<< dtmp <<endl;
        else 
          cout<< "chosen detector is = S"<< std::setfill('0') << std::setw(3)<< dtmp <<"  (FSpare0"<<dtmp%10<<endl;
    }
    else if (trigger==2)
    {
        cout<< "chosen osciloscope channel = "<<trigger<<endl;
        dtmp = (dtmp / 100) % 100;
        if (dtmp<50)
          cout<< "chosen detector is = S"<< std::setfill('0') << std::setw(3)<< dtmp <<endl;
        else 
          cout<< "chosen detector is = S"<< std::setfill('0') << std::setw(3)<< dtmp <<"  (FSpare0"<<dtmp%10<<")"<<endl;
        
    }
    else if (trigger==3)
    {
        cout<< "chosen osciloscope channel = "<<trigger<<endl;
        dtmp = dtmp % 100;
//         cout<< "chosen detector is = F0"<<dtmp<<endl;
        if (dtmp<50)
          cout<< "chosen detector is = S"<< std::setfill('0') << std::setw(3)<< dtmp <<endl;
        else 
          cout<< "chosen detector is = S"<< std::setfill('0') << std::setw(3)<< dtmp <<"  (FSpare0"<<dtmp%10<<")"<<endl;
        
    }
    else 
    {
        cout<< "the detectors should be connected to channels 1, 2 or 3. Exiting..."<<endl;
        return -1;
    }
    cout<<"detector = "<<dtmp<<" (S"<<detNo<<")"<<endl;
    cout<<"run = "<<rtmp<<endl;
//     if (trigger==1)
//         cout<<"attention, run No changed from "<<rtmp<<" to "<<rtmp+60<<" ==> position SPR-1 !!!"<<endl;
//     else if (trigger==2)
//         cout<<"attention, run No changed from "<<rtmp<<" to "<<rtmp+70<<" ==> position SPR-2 !!!"<<endl;
//     else if (trigger==3)
        cout<<"attention, run No changed from "<<rtmp<<" to "<<rtmp+80<<" ==> position SPR-3 !!!"<<endl;
    cout<<"Vm = "<<vm<<endl;
    cout<<"Vd = "<<vd<<endl;
//     cout<<"Mylar thickness = "<<bthick<<endl;
//     cout<<"Drift gap = "<<dgap<<endl;
    if (stst>4) 
      cout<<"filetype = "<<filetype<<endl;
    fclose(ftmp);
    sprintf(command,"rm %s\n",afile);
    tst=system(command);
    
    detNo=dtmp;
    if (trigger==1)
        runNo = rtmp+0;
    else if (trigger==2)
        runNo = rtmp+0;
    else if (trigger==3)
        runNo = rtmp+0;

    sprintf(ofname,"S%03d-%02d-%d-%d_%s",dtmp,runNo,vm,vd,ftypetmp);
  }
  else if (detNo>=RUNMIN2 && detNo<=RUNMAX2)
  {
    char afile[500]; 
    sprintf(afile,"%s/tmpfile.tmp",basedirname);
    FILE *ftmp=fopen(afile,"w");
    if (ftmp == NULL)
    {
      cout<<afile<<" can not be created. Probably the directory '"<<basedirname<<"' does not exit. Exiting..."<<endl;
      exit (-12);
    }
    fclose(ftmp);
    sprintf(command,"cd %s\nls -d S%04d-%02d-*%s_%s*.root > %s 2>/dev/null",basedirname,detNo,runNo,filetype.c_str(),rtype,afile);

    int tst=system(command);
//    cout<<command<<endl<<"returned: "<<tst<<endl;
    if (tst !=0)
    {
      cout<<command<<endl<<"returned: "<<tst<<endl;
      cout<<"Probably the tree of the run "<<runNo<<" of detector No S"<<detNo<<" was not found in directory "<<basedirname<<endl<<"Exiting..."<<endl;
      return tst;
    }
    ftmp=fopen(afile,"r");
    if (fgets(fnametmp,200,ftmp) == NULL)
    {
      cout<<"Failed to read the filename for detector S"<<detNo<<" run "<<runNo<<" at "<<basedirname<<endl<<"Exiting..."<<endl;
      return -2;
    }
    int rtmp,dtmp;
    char ftypetmp[500];
    strcpy(ftypetmp,"");
    int stst = sscanf(fnametmp,"S%04d-%02d-%d-%d_%30[^ /,\n\t]",&dtmp,&rtmp,&vm,&vd,ftypetmp);
    cout <<"arguments read = "<<stst<<endl;
    filetype.assign(ftypetmp);
    cout<<"detector = "<<dtmp<<" (S"<<detNo<<")"<<endl;
    if (trigger==2)
    {
        cout<< "chosen osciloscope channel = "<<trigger<<endl;
        dtmp = dtmp / 100;
        cout<< "chosen detector is = S0"<<dtmp<<endl;
    }
    else if (trigger==3)
    {
        cout<< "chosen osciloscope channel = "<<trigger<<endl;
        dtmp = dtmp % 100;
        cout<< "chosen detector is = S0"<<dtmp<<endl;
        
    }
    else 
    {
        cout<< "the detectors should be connected to channels 2 or 3. Exiting..."<<endl;
        return -1;
    }
    cout<<"detector = "<<dtmp<<" (S"<<detNo<<")"<<endl;
    cout<<"run = "<<rtmp<<endl;
// //     if (trigger==2)
// //         cout<<"attention, run No changed from "<<rtmp<<" to "<<rtmp+40<<" ==> position NEAR !!!"<<endl;
// //     else if (trigger==3)
// //         cout<<"attention, run No changed from "<<rtmp<<" to "<<rtmp+50<<" ==> position FAR !!!"<<endl;
    cout<<"Vm = "<<vm<<endl;
    cout<<"Vd = "<<vd<<endl;
//     cout<<"Mylar thickness = "<<bthick<<endl;
//     cout<<"Drift gap = "<<dgap<<endl;
    if (stst>4) 
      cout<<"filetype = "<<filetype<<endl;
    fclose(ftmp);
    sprintf(command,"rm %s\n",afile);
    tst=system(command);
    
    detNo=dtmp;
    if (trigger==2)
        runNo = rtmp + 0;
    else if (trigger==3)
        runNo = rtmp + 0;

    sprintf(ofname,"S%03d-%02d-%d-%d_%s",dtmp,runNo,vm,vd,ftypetmp);
  }
  else
  {
    cout<<"Available detectors: "<<MINRUN<<" - "<<MAXRUN<<endl;
    return (-2);
  }

  
  const char *ftype = filetype.c_str();  /// add here any directory supplement

//   strcpy (ofname,fnametmp);
  char *pch;
  pch = strstr(ofname,"raw_tree.root");
//   sprintf(ofname,"%s_tree.root",pch);
  strcpy (pch,"_tree.root");
#ifdef DEBUG
  strcpy (pch,"_DEBUGtree.root");
#endif
  cout<<"output filename = "<<ofname<<endl;

//     return 0;

  
  char mesh[1000];
  sprintf(mesh,"%d",vm);
  char drift[1000];
  sprintf(drift,"%d",vd);

  int drft;
  sscanf(drift,"%d",&drft);

  const int MaxFiles = MAX_N_FILES;
  const int MAXSEG = 100000;
  const int FRAMESIZE = 500000;
  
  double *ptime;
  ptime = new double[FRAMESIZE];


  cout <<"********************\n\n\n Now starting processing "<<fnametmp<<"..."<<endl<< endl;  

  replaceEOL(fnametmp);
  sprintf(ifilename,"%s/%s",basedirname,fnametmp);
  TFile *infile = new TFile(ifilename);
  if (!infile->IsOpen()){
    cout<<"Failed to open \""<<ifilename<<"\"\ncorresponding to the run "<<detNo<<"\nExiting..."<<endl;
    return (-1);
  }
  
  int spoints[]={0,0,0,0};
  int evNo=0;
  double Dt[4]={0.,0.};	
  double t0[]={0.,0.,0.,0.};
  double gain[]={0.,0.,0.,0.};
  double offset[]={0.,0.,0.,0.};
  double rmax[]={0.,0.,0.,0.};
  double rmin[]={0.,0.,0.,0.};
  double range[]={0.,0.,0.,0.};
  double t0o;
  double dt, odt;
  unsigned long long int epoch;
  unsigned long long int nn;
  int itrigger;//time of the trigger
  char date[4][50];
  double *amplC[4];
  int active[] = {0,0,0,0};
//   for (int i=0; i<2; i++)
  
  char treename[200];
  sprintf(treename,"TreeWithRawData");
  char treetitle[200];
  sprintf(treetitle,"Raw data tree");

  TTree *intree = (TTree*)infile->Get(treename);
  TBranch* branch;

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
  
  unsigned long long int iepoch=0;
  branch = intree->GetBranch("epoch");
  branch->SetAddress(&iepoch);
  unsigned long long int inn;
  branch = intree->GetBranch("nn");
  branch->SetAddress(&inn);
  char idate[50];
  branch = intree->GetBranch("date");
  branch->SetAddress(&idate);

  for (int i=0;i<4; i++)
  {
    TString channel = TString::Itoa(i+1,10);
    TString bname = "npoints"+channel;
    TString btype = bname+"/I";
    branch = intree->GetBranch(bname);
    if (branch==NULL) 
      continue;
    active[i]=1;
    branch->SetAddress(&spoints[i]);
    bname = "amplC"+channel;
    cout << bname << endl;
    btype = bname+"[npoints"+channel+"]/D";
    branch = intree->GetBranch(bname);
    amplC[i] = new double[FRAMESIZE]; 
    branch->SetAddress(&amplC[i][0]);
  }

//   intree->SetCacheSize(200000000);
//   long int bufsize = intree->GetCacheSize();
//   cout<<"Cache size = "<<bufsize<<endl;
// //   intree->InitializeBranchLists(true);
//   intree->SetBasketSize("*",32000);
//   intree->AddBranchToCache("*");
//   intree->LoadBaskets();
//   bufsize = intree->GetCacheSize();
//   cout<<"Cache size = "<<bufsize<<endl;
  const int nevents = intree->GetEntries();

  int actch=-1;
  int framesize=0;
  double bslmax = 1.;
  double bslmin = -1.;
  for (int i=0;i<4; i++)
  {
    double max=0.;
    double min=0.;
    if ((active[i]) && (trigger==i+1))
    {
      actch=i;
      intree->GetEntry(nevents/2);
      
      framesize = spoints[i];
      cout<<"Analysing active channel "<< i <<" , osciloscope channel "<<trigger<<endl;
      cout<<"\n\nFramesize = "<<framesize<<" points"<<endl;
      cout<<"Gain = "<<gain[i]<<" V"<<endl;
      cout<<"Offset = "<<offset[i]<<" V"<<endl;
      cout<<"Range Max = "<< rmax[i]*mV<<" mV";
      cout<<"\t\tRange Min = "<< rmin[i]*mV<<"\t";
      range[i]=rmax[i]-rmin[i];
      cout<<"Full range = "<<range[i]*mV<<" mV  -->  " << range[i]/8 *mV <<" mV / div  --> "<<range[i]/256*mV<<" mV / bit"<<endl<<endl;
      bslmax = rmax[i];
      bslmin = rmin[i] + range[i]*(6./8.);
//       cout<<"Corresponding to "<<endl<<endl<<endl;
      break;  /// only the first active channel is used!!!!!!
    }
  }
  dt = idt*1e9;
  if (actch<0) 
  {
    cout<<"No active channel found, or the only active channel is declared as external trigger !!! Exiting..."<<endl;
    return -12;
  }
  long double meanoffset = (rmax[actch] + rmin[actch])/2. ;
  cout<<"Mean offset = "<< meanoffset*mV <<" mV"<<endl<<endl;
  long double vpb = range[actch]/256.;
  //   drft=651;
  char dirname[1000];
  int runb = 0;

  sprintf(dirname,"%s/S%03d-%02d-%d-%d",basedirname,detNo,runNo,vm,vd);
   
  if (runb)
   sprintf(dirname,"%sb",dirname); 

  if (strlen(ftype)>1)
    sprintf(dirname,"%s-%s",dirname,ftype);

  cout<<"\n\n\nSearching for files at directory: "<<dirname<<endl<<endl;
  
  char rfname[200];
  char detid[20];
  sprintf(detid,"S%03d",detNo);
  char runid[20];
  sprintf(runid,"%02d",runNo);
  char cangle[20];
  sprintf(cangle,"%d",angle);

  TString rtypes(detid);
  rtypes+="-";
  rtypes+=runid;
  rtypes+="-";
  rtypes+=mesh;
  rtypes+="-";
  rtypes+=drift;
  rtypes+="-";
//   rtypes+=cangle;
//   rtypes+="-";
  rtypes+=rtype;
  
  char catt[30];
  
  if (runb)
    rtypes+="_b";

  TString ctypes(detid);
  ctypes+="-";
  ctypes+=runid;
  ctypes+="-";
  ctypes+=mesh;
  ctypes+="-";
  ctypes+=drift;
  ctypes+="-";
//   ctypes+=cangle;
//   ctypes+="-";
  ctypes+=ftype;

  sprintf(ofilename,"%s/%s",outdirname,ofname);
  TFile *ofile = new TFile(ofilename,"RECREATE");

  double binscale =1.;
  if (framesize < 1000)
    binscale = 1.0;
  int nbins = 256 / 8 * 2 -1;
  
  char htitle[100];

  TH1F *hbsl, *hbsl0;
  TH1D *hbslall, *hbsl0all;

  sprintf(fname2,"Signal_bsl");
  hbsl = new TH1F(fname2,fname2,nbins,bslmin,bslmax);
  hbsl->GetXaxis()->SetTitle("[V]");
  sprintf(fname2,"Signal_bsl_tot");
  sprintf(htitle,"Signal baseline from all events");
  hbslall = new TH1D(fname2,htitle,nbins,bslmin,bslmax);
  hbslall->GetXaxis()->SetTitle("[V]");

  sprintf(fname2,"Subtracted_bsl");
  hbsl0 = new TH1F(fname2,fname2,(int)nbins*binscale,-range[actch]/8.,range[actch]/8.);
  hbsl0->GetXaxis()->SetTitle("[V]");
  sprintf(fname2,"Subtracted_bsl_tot");
  hbsl0all = new TH1D(fname2,fname2,(int)nbins*binscale,-range[actch]/8.,range[actch]/8.);
  hbsl0all->GetXaxis()->SetTitle("[V]");

  double *amplCo;
  for (int i=0; i<2; i++)
    amplCo = new double[FRAMESIZE]; 

  double *amplSum; //in this case (lina4) is just ampl - baseline 
    amplSum = new double[FRAMESIZE];
  
  double *tbase = new double[FRAMESIZE];
  for (int i=0;i<FRAMESIZE;i++)
    tbase[i]=i*4.;
    
  double *EVTIME = new double[nevents];  
  double *BSL = new double[nevents];  
  double *RMS = new double[nevents];  
  double *sBSL = new double[nevents];  
  double *sRMS = new double[nevents];  
  
  
    
  int maxpoints=0;

  TTree *outtree;
  
  sprintf(treename,"RawDataTree");
  sprintf(treetitle,"Raw data tree");
  outtree = new TTree(treename,treetitle);

  outtree->Branch("eventNo", &evNo, "eventNo/I");
  outtree->Branch("dt", &odt, "dt/D");
  outtree->Branch("epoch", &epoch, "epoch/l");
  outtree->Branch("nn", &nn, "nn/l");
  // tree->Branch("date", &date, "date/C");
  outtree->Branch("t0", &t0o, "t0/D");
  outtree->Branch("itrigger",&itrigger,"itrigger/I");
  double ttrig;
  outtree->Branch("ttrig",&ttrig,"ttrig/D");
  int fitstatus1=0, fitstatus2=0;
  outtree->Branch("fitstatus1",&fitstatus1,"fitstatus1/I");
  outtree->Branch("fitstatus2",&fitstatus2,"fitstatus2/I");
  double frangemax,frangemin;
  outtree->Branch("rmax",&frangemax,"rmax/D");
  outtree->Branch("rmin",&frangemin,"rmin/D");

  outtree->Branch("sumpoints", &maxpoints, "sumpoints/I");
  outtree->Branch("amplSum", amplSum, "amplSum[sumpoints]/D");
//   outtree->Branch("amplSum2", amplSum[1], "amplSum2[sumpoints]/D");
  double bslsum=0.;
  double rmssum=0.1;
  outtree->Branch("bslSum", &bslsum, "bslSum/D");
  outtree->Branch("rmsSum", &rmssum, "rmsSum/D");
  
  cout<<"\n\n Tree is ready!"<<endl;

#ifdef DEBUG 
//    TCanvas *cbsl = new TCanvas("baseline"+rtypes,"baseline "+rtypes,1600*4/4,1200*3/4);
   TCanvas *cbsl = new TCanvas("baseline"+rtypes,"baseline "+rtypes,1600*4/4,1200*2/4);
    cbsl->Divide(2,1);
#endif  
  TTimeStamp* tstamp[2][MAXSEG];  
  double tshift[2]={0.,0.};
  int dd,mm,yy,hour,min,sec;
  char month[20];
  int nseg;

  double bsl[2]={0.,0.};
  double rms[2]={0.,0.};
  
  
  sprintf(fname2,"gausC");
  TF1* fbsl = new TF1(fname2,"gaus",-1.,1.);
  fbsl->SetNpx(2000);
  sprintf(fname2,"gaus0");
  TF1* fbsl0 = new TF1(fname2,"gaus",-1.,1.);
  fbsl0->SetNpx(2000);

  sprintf(fname2,"gausCtot");
  TF1* fbslall = new TF1(fname2,"gaus",-1.,1.);
  fbslall->SetNpx(2000);
  sprintf(fname2,"gaus0tot");
  TF1* fbsl0all = new TF1(fname2,"gaus",-1.,1.);
  fbsl0all->SetNpx(2000);

  gStyle->SetOptFit(11);
  gStyle->SetOptStat(1100);

  int ngpoints = 0;
  int eventS = 0;
  int eventF = nevents;
// #ifdef DEBUG 
//     eventS = 16438;
//     cout<<"Give event No to debug: ";
//     cin>>eventS;
//     eventF = eventS;
// #endif

  intree->GetEntry(0);
  intree->GetEntry(nevents-1);
  cout<<"Loading tree baskets in virtual memory"<<endl;
  int nbaskets=intree->LoadBaskets(4000000000);
  cout<<"Loaded "<<nbaskets<<" baskets "<<endl;

  outtree->OptimizeBaskets(100000000,1.1,"d" );
  
//   eventS=5000;
  double tdt = -1.;  
  for (int nEv=eventS; nEv<nevents && nEv<eventF+1; nEv+=1)
    { 
#ifdef DEBUG 
//     eventS = 16438;
    cout<<"Give event No to debug: ";
//     cin>>nEv;
    if (nEv<0) return (-1);
//     eventF = eventS;
#endif
      
      intree->GetEntry(nEv);
      odt=idt*1e+9;
      evNo = eventNo;
      frangemax = rmax[actch];
      frangemin = rmin[actch];
      
      long double tmpx;
      double tmpy;
      
      ///  repeat the ci loop in order to read the data. Read data for each segment of each channel. Fill the tree every time 
      ///  all 4 channels of the same segment is read. Note that ALL 4 FILES REMAIN OPEN and the pointer is at the end at each segment!!!
      
      /// loop over all channels of the same segment !!!  

      if (tdt != idt)
      {
	for (int k=0; k<inpoints+10; k++ ) /// time array
	  ptime[k]=k*idt;// tmpx*1e9;
	dt = idt*1e9;  //ns
	tdt=idt;
      }
      epoch=iepoch;
      nn=inn;
      double  epochF = (1.*iepoch + inn*1e-9);

      itrigger = -100;

      hbsl->Reset("");

      if (trigger>0)
      {
	int trgch = trigger-1;
	for (int k=0; k<spoints[trgch]-2; k++ )  ///processing trigger (ci=0)
	{
	  if(amplC[trgch][k]>1.) 
	  {
	    itrigger =k;
	    break;
	  }
	}
      }
      ttrig = itrigger * dt;

      long double sumsig=0.;
      long double diff=0.;
      for (int k=0; k<spoints[actch]; k++ ) /// read the segment data points (ci=1)
      {
	amplCo[k]=amplC[actch][k];       /// storing original waveform
        hbsl->Fill(amplCo[k]);    /// event baselin calculation
	sumsig+=(amplCo[k]-meanoffset);
#ifdef DEBUGMSG 
  	diff = (amplCo[k]-meanoffset) + diff;
	cout<<k<<" = "<<setw(15)<<amplCo[k]<<" --> "<<setprecision(10)<< setw(15)<<(amplCo[k]-meanoffset)  <<" mV  --> sum = "<<sumsig<<"  diff = "<<diff<<endl;
	diff = (amplCo[k]-meanoffset);
#endif 
      }
      
#ifdef DEBUG 
#ifdef DEBUGMSG 
      cout<<"Sumsig = "<<sumsig  <<" --> "<<sumsig/502<<"   gain = "<<gain[actch]<<" --> gain/2 = "<<gain[actch]/2.<<endl ;
      cout<< " V/bit = "<< vpb << " --> V/bit/2 = " <<vpb/2. <<endl ;
#endif
      cbsl->cd(1);
      TGraph* gr = new TGraph(spoints[actch],tbase,amplCo);
      gr->Draw("a*l");
//       return -13;
#endif
#ifdef DEBUG 
	 cbsl->cd(2);
	 gStyle->SetOptStat(1111110);
	 hbsl->Draw();
	 cbsl->Modified();
	 cbsl->Update();
     cout<<"WTF?"<<endl;
#endif
      
/// check if histo is empty. In that case either signal completely out of range or problematic event.
/// For this reason we check the sumsig. If it is 0 or very close to 0, reject completely the event!!!!
      
      int hentries= hbsl->Integral();
      if (hentries == 0)
      {
	 cout<<"_____________________________________________________________"<<endl;
	 cout<<"Event "<<evNo <<" has no point in the range of the baseline histo with "<<hbsl->GetEntries()<<" entries!"<<endl;
	 cout<<"There are "<<hbsl->GetBinContent(0)<<" underflow and "<<hbsl->GetBinContent(nbins+1)<<" points!"<<endl;
	 cout<<"Sumsig for " << spoints[actch] << " is " << sumsig*mV << " mV with a gain of " << vpb*mV<< " mV/bit"<<endl;
	 if (fabs(sumsig)<vpb/10. )
	 {
	    cout<<"The event is REJECTED!!!"<<endl;
	    cout<<"-------------------------------------------------------------"<<endl;
	    continue;
	 }
	 else 
	 {
	    int ovfl=hbsl->GetBinContent(nbins+1);
	    int unfl=hbsl->GetBinContent(0);
	    int hnp = spoints[actch]/2;
	    if (ovfl>=hnp-1 && unfl>=hnp-1 && fabs(ovfl-unfl)<=1)
	    {	      
	      cout<<"It seems that there might be one point within the range of the scope that makes the frame inegral non-zero."<<endl;
	      cout<<"However, this point is out of the bsl histo range, and the overrange / underange points are shared equally."<<endl; 
	      cout<<"The event is REJECTED!!!"<<endl;
	      cout<<"-------------------------------------------------------------"<<endl;
	      continue;
	    }
	    else
	    {
	      cout<<"The event is registered, thought the bsl histo integral is zero!"<<endl;
	      cout<<"-------------------------------------------------------------"<<endl;
	    }
	}
      }
      else if (hentries <= 20)
      {	 
	 cout<<"_____________________________________________________________"<<endl;
	 cout<<"Event "<<evNo <<" has very few points point in the range of the baseline histo with "<<hbsl->GetEntries()<<" entries!"<<endl;
	 cout<<"There are "<<hbsl->GetBinContent(0)<<" underflow and "<<hbsl->GetBinContent(nbins+1)<<" points!"<<endl;
	 cout<<"Sumsig for " << spoints[actch] << " is " << sumsig*mV << " mV with a gain of " << vpb*mV<< " mV/bit"<<endl;
      }
      

      t0o = t0[actch];
      
      if (1 )// || (framesize<10000 && nEv<500) )
      {
	FilterHisto(hbsl,0.03);

	int maxb      = hbsl->GetMaximumBin();
	double maxAmp =  hbsl->GetBinContent(maxb);
	double meant = hbsl->GetBinLowEdge(maxb);
	double rmst  = hbsl->GetRMS();
	double var = hbsl->GetBinWidth(maxb);

	if (rmst>0.005) rmst=0.005;
	
	fbsl->SetParLimits(0,0.5*maxAmp,2.*maxAmp);
	fbsl->SetParLimits(1,meant-var,meant+2*var);
	fbsl->SetParLimits(2,vpb/2.,10.*vpb);
	fbsl->SetParameter(0,maxAmp);
	fbsl->SetParameter(2,rmst);
	fbsl->SetParameter(1,meant+var/2.);
// // 	cbsl->cd(1);

	sprintf(fname2,"event %d baseline",evNo );
	hbsl->SetTitle(fname2);
	fitstatus1 = hbsl->Fit(fbsl,"BQN","",meant-3*rmst,meant+3*rmst);
      
    rmssum=100.;
	bsl[1]=fbsl->GetParameter(1);
	rms[1]=fbsl->GetParameter(2);
    rmssum = rms[1]*1000.;
//    bslsum = bsl[1];
	
	EVTIME[ngpoints]=epochF;
	BSL[ngpoints]=fbsl->GetParameter(1)*1000.;
	RMS[ngpoints]=fbsl->GetParameter(2)*1000.;
	sBSL[ngpoints]=fbsl->GetParError(1)*1000.;
	sRMS[ngpoints]=fbsl->GetParError(2)*1000.;
	ngpoints++;
// // 	cbsl->Modified();
// // 	cbsl->Update();

	hbslall->Add(hbsl);

	bsl[0]=0.0;
	rms[0]=0.0;

      }

      maxpoints = inpoints;
      
      if (maxpoints <= 0) continue;


      int tst = SubtractBaseline(maxpoints,amplCo,amplSum,bsl[1]);//amplCo corresponds to signal (initial C2) and is place in amplSum[0]

// //       hbsl0->Reset("");  /// this is the subtracted event baseline !!!!
// // 
// //       for (int k=0; k<maxpoints; k++ ) /// read the segment data points
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
// //       // 	  cout<<"meant C"<<ci+1<<" = "<<meant<<endl;
// // 
// //       if (rmstt<0.005) /// don't accumulate wierd events in total baseline histo;
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
// //       sprintf(fname2,"bsl-corrected event %d baseline",evNo );
// //       hbsl0->SetTitle(fname2);
// //       fitstatus2 = hbsl0->Fit(fbsl0,"BQ","",meantt-0.0075,meantt+0.0075);
// //       bslsum=fbsl0->GetParameter(1);
// //       rmssum=fbsl0->GetParameter(2);
// //       hbsl0->Draw("E0");
      
      if (fitstatus1 != 0 )//|| fitstatus2 || 0)
      {
	cout<<"evNo = "<<evNo<<"\t  f1 = "<<fitstatus1<<" f2 = "<<fitstatus2;
	cout<<" bsl = " <<bsl[1]<<" rms = " << rms[1];
	cout<<" bsl0 = " <<bslsum<<" rms0 = " << rmssum<<endl;
	cout<<" SumSig = " <<sumsig<<" histogram Integral = " << hentries<<endl;
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
      if ((nEv==eventF-1) || (framesize<10000 && nEv%50==0))
      {
// //         cbsl->cd(3);
	double maxbt0 = hbslall->GetMaximumBin();
 	double maxv = hbslall->GetBinContent(maxbt0);
        double meantt0 = hbslall->GetBinLowEdge(maxbt0);
        double rmstt0 = hbslall->GetRMS();
	
	fbslall->SetParameter(0,maxv);
	fbslall->SetParameter(1,meantt0);
	fbslall->SetParameter(2,rmstt0);
	fbslall->SetParLimits(0,0.5*maxv,1.5*maxv);
	fbslall->SetParLimits(1,bsl[1]-0.01,bsl[1]+0.01);
	fbslall->SetParLimits(2,0.0005,0.03);

	hbslall->Fit(fbslall,"BQN","",bsl[1]-0.01,bsl[1]+0.01);
// // 	hbslall->Draw("E0");
	bsl[1]=fbslall->GetParameter(1);
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

      if (nEv==20) 
      {
         outtree->OptimizeBaskets(100000000,1.1,"d" );
      }
      outtree->Fill();
      
      if (framesize>20000)
      {
		if ((nEv+1) % 50 ==0)
		  cout<<"Processed "<<nEv+1<<" events (evNo = "<<evNo<<") out of "<<nevents<<endl;

		if (nevents % 200 == 0)
		  ofile->Write("",TObject::kOverwrite);
      }
      else
      {
	if ((nEv+1) % 500 ==0)
	{
	  cout<<"Processed "<<nEv+1<<" events (evNo = "<<evNo<<") out of "<<nevents<<endl;
	  TTimeStamp *tstamp = new TTimeStamp();
	  tstamp->Set();
	  cout<<"\t\tprocessing time : "<<tstamp->AsString("l")<<endl;
// 	  if ((nEv+1) == 100)
// 	  {
// 	    outtree->OptimizeBaskets();
// 	    intree->OptimizeBaskets();
// 	  }
	}

	if (nevents % 1000 == 0)
	  ofile->Write("",TObject::kOverwrite);
      }
      
    }  
  
  
  
  
  
  
  TCanvas *ebsl = new TCanvas("baselineEvolution"+rtypes,"Baseline Evolution "+rtypes);
  TGraphErrors* grBSL = new TGraphErrors(ngpoints,EVTIME,BSL,0,sBSL);
  grBSL->SetTitle("BASELINE " + rtypes);
  grBSL->SetLineColor(4);
  grBSL->Draw("AP");
  TH1F *h1 = grBSL->GetHistogram();
  h1->SetBins(100000,EVTIME[0]-0.05*(EVTIME[ngpoints-1]-EVTIME[0]),EVTIME[ngpoints-1]+0.05*(EVTIME[ngpoints-1]-EVTIME[0]));
  grBSL->GetXaxis()->SetTimeDisplay(1);
  grBSL->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d/%m/%y}");
  grBSL->GetXaxis()->SetLabelOffset(0.04);
  grBSL->GetYaxis()->SetTitle("[mV]");
//   grBSL->SetMinimum(0);
  grBSL->Write("baselineEvolution");
  

  TCanvas *erms = new TCanvas("baselineRMSEvolution"+rtypes,"Baseline RMS Evolution "+rtypes);
  TGraphErrors* grRMS = new TGraphErrors(ngpoints,EVTIME,RMS,0,sRMS);
  grRMS->SetTitle("RMS " + rtypes);
  grRMS->SetLineColor(2);
  grRMS->Draw("AP");
  TH1F *h2 = grRMS->GetHistogram();
  h2->SetBins(100000,EVTIME[0]-0.05*(EVTIME[ngpoints-1]-EVTIME[0]),EVTIME[ngpoints-1]+0.05*(EVTIME[ngpoints-1]-EVTIME[0]));
  grRMS->GetXaxis()->SetTimeDisplay(1);
  grRMS->GetXaxis()->SetLabelOffset(0.04);
  grRMS->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d/%m/%y}");
  grRMS->GetYaxis()->SetTitle("[mV]");
//   grRMS->SetMinimum(grRMS->GetMinimum()*0.7);
//   grRMS->SetMaximum(grRMS->GetMaximum()*1.3);
  grRMS->Write("rslRMSEvolution");
  
  TCanvas *ebslrms = new TCanvas("BSL+RMS_Evolution"+rtypes,"Baseline & RMS Evolution "+rtypes);
  TGraphErrors* grBSLRMS = new TGraphErrors(ngpoints,EVTIME,BSL,0,RMS);
  grBSLRMS->SetTitle("BSLRMS " + rtypes);
  grBSLRMS->SetLineColor(4);
  grBSLRMS->Draw("AP");
  TH1F *h3 = grBSLRMS->GetHistogram();
  h3->SetBins(100000,EVTIME[0]-0.05*(EVTIME[ngpoints-1]-EVTIME[0]),EVTIME[ngpoints-1]+0.05*(EVTIME[ngpoints-1]-EVTIME[0]));
//   TH1D *h1 = grBSL->GetHistogram();
  grBSLRMS->GetXaxis()->SetTimeDisplay(1);
  grBSLRMS->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d/%m/%y}");
  grBSLRMS->GetXaxis()->SetLabelOffset(0.04);
  grBSLRMS->GetYaxis()->SetTitle("[mV]");
//   grBSLRMS->SetMinimum(0);
  grBSLRMS->Write("BSLandRMSevolution");

//   gStyle->SetOptFit(110);
  TCanvas *ctotbsl = new TCanvas("Pedestals"+rtypes,"Pedestals "+rtypes,1600*4/4,1200*3/4);
  ctotbsl->Divide(2,2);
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
  sprintf(tmpdir,"%s",gSystem->pwd());
  cout<<"Actual directory: "<<tmpdir<<endl; 
  char plotdirname[500];
  sprintf(plotdirname,"%s/S%03d/",PLOTDIR,detNo);//,abs(threshold));
  
  gSystem->mkdir(plotdirname,kTRUE);
  gSystem->ChangeDirectory(plotdirname); 
  ctotbsl->SaveAs(".pdf");
  ctotbsl->SaveAs(".png");
  gSystem->ChangeDirectory("./../..");
  gSystem->ChangeDirectory(tmpdir);

  
  cout<<"End of file processing.\n"<<evNo<<" events were found"<<endl;
  ofile->Write("",TObject::kOverwrite);
  

  return 0;
}

//#endif
