#ifndef MYFUNCTIONS_C
#define MYFUNCTIONS_C 1

#include "MyFunctions.h"

void MyFunctions(){
  cout<<"Loading functions library..."<<endl;}

  
void AssignRun(RUNPAR* allruns,RUNPAR *runpar,int irun)
{
  allruns[irun].detNo=runpar->detNo;
  allruns[irun].runNo=runpar->runNo;
  allruns[irun].ampl=runpar->ampl;
  allruns[irun].sampl=runpar->sampl;
  allruns[irun].charge=runpar->charge;
  allruns[irun].scharge=runpar->scharge;
  allruns[irun].rate=runpar->rate;
  allruns[irun].srate=runpar->srate;
  allruns[irun].grate=runpar->grate;
  allruns[irun].sgrate=runpar->sgrate;
  allruns[irun].tot=runpar->tot;
  allruns[irun].stot=runpar->stot;
  allruns[irun].risetime=runpar->risetime;
  allruns[irun].srisetime=runpar->srisetime;
  allruns[irun].width=runpar->width;
  allruns[irun].swidth=runpar->swidth;
  allruns[irun].chovampl=runpar->chovampl;
  allruns[irun].schovampl=runpar->schovampl;
   
}
void RegisterRunParameters(RUNPAR* runpar, const char* dirname)
{
  const char title[18][12]={"detNo","runNo","ampl","sampl","charge","scharge","rate","srate","grate","sgrate","tot","stot","risetime","srisetime","width","swidth","chovampl","schovampl"};
  RUNPAR *allruns = new RUNPAR[TOTRUNS*100];
  for(int i=0;i<TOTRUNS*100;i++) {
    allruns[i].runNo=-1;
    allruns[i].detNo=-1;
  }
  char filename[1000];
  snprintf(filename, sizeof(filename), "%s/ParameterTable.txt",dirname);
  cout<<RED<<"Searching parameters file: |"<<filename<<"|"<<endlr;
  
  ifstream inf;
  inf.open(filename); 
  int entries=0;
  if (!inf.is_open()) /// running for the first time!
  {
     cout<<"\n\n\nParameter file "<<filename<<" doesn't exist. Creating new ...\n\n\n"<<endl;
     FILE* outf = fopen(filename,"w");
     for (int i=0;i<18;i++) fprintf(outf,"%12s ",title[i]);
     fprintf(outf,"\n");
     fclose(outf);
  }
  else /// read existing table entries 
  {
    inf.ignore(1000,'\n');  /// skip the headerline
    while (!inf.eof() )
    {
       int dettmp=-1;
       int runtmp=-1;
       inf>>dettmp;
       if (inf.eof()) break;
       inf>>runtmp;
//        int irun = runtmp-MINRUN;
       int irun = 100*dettmp + runtmp;
       
       if (irun<0) break;
       allruns[irun].detNo=dettmp;
       allruns[irun].runNo=runtmp;
       inf>>allruns[irun].ampl;
       inf>>allruns[irun].sampl;
       inf>>allruns[irun].charge;
       inf>>allruns[irun].scharge;
       inf>>allruns[irun].rate;
       inf>>allruns[irun].srate;
       inf>>allruns[irun].grate;
       inf>>allruns[irun].sgrate;
       inf>>allruns[irun].tot;
       inf>>allruns[irun].stot;
       inf>>allruns[irun].risetime;
       inf>>allruns[irun].srisetime;
       inf>>allruns[irun].width;
       inf>>allruns[irun].swidth;
       inf>>allruns[irun].chovampl;
       inf>>allruns[irun].schovampl;
       entries++;
    }
    cout<<endl<<"Found "<<entries<<" existing entries in the table file "<<filename<<endl<<endl;
    inf.close();
  }
  /// adding new parameters now!
//  int irun = runpar->runNo - MINRUN;
   int irun = 100*runpar->detNo + runpar->runNo ;
   AssignRun(allruns,runpar,irun);
   FILE* outf = fopen(filename,"w");
   if (outf==NULL){ cout<<"FAILED TO OPEN OUTFILE"<<endl; }
   fprintf(outf,"%6s ",title[0]);
   fprintf(outf,"%6s ",title[1]);
   for (int i=2;i<18;i++) fprintf(outf,"%14s ",title[i]);
   fprintf(outf,"\n");
   for (int i=0;i<TOTRUNS*100;i++)
   {
//      irun = allruns[i].runNo - MINRUN;
     irun = i;
     if (allruns[irun].detNo <= 0 || allruns[irun].runNo<0) continue;
//      cout<<"Writing entry "<<irun<<" corresponding to "<< allruns[irun].runNo<<endl;
     fprintf(outf,"%6d %6d %14e %14e %14e %14e %14e %14e %14e %14e ",allruns[irun].detNo, allruns[irun].runNo, allruns[irun].ampl, allruns[irun].sampl, allruns[irun].charge, allruns[irun].scharge, allruns[irun].rate, allruns[irun].srate, allruns[irun].grate, allruns[irun].sgrate);
     fprintf(outf,"%14e %14e %14e %14e %14e %14e %14e %14e\n",allruns[irun].tot, allruns[irun].stot, allruns[irun].risetime, allruns[irun].srisetime, allruns[irun].width, allruns[irun].swidth, allruns[irun].chovampl, allruns[irun].schovampl);
   }
   fclose(outf);
  
}

void DrawPlot(int run, TH1F *histo, int print)
{
///     const char *txt = histo->GetName();
     const char *txt = histo->GetTitle();
     char txt2[1000];
//      TPostScript *ps;

     snprintf(txt2, sizeof(txt2), "plots%d/%s.eps",run,txt);

     if (print)
     {
//         ps = new TPostScript(txt2,113);
//         ps->NewPage();
     }

     TCanvas* canv = new TCanvas(txt,txt,1);
     canv->SetFillColor(10);
     canv->SetFillColor(10);
     canv->SetBorderSize(2);
     canv->SetGridx();
     canv->SetGridy();
//     canv->SetLogy();
     canv->SetFrameBorderSize(12);
    gStyle->SetOptStat(1000010);
    gStyle->SetPalette(1,0);
//    histo2->SetMinimum(0.1);
    histo->GetXaxis()->SetTitleFont(22);
    histo->GetYaxis()->SetTitleFont(22);
    histo->GetXaxis()->SetLabelFont(22);
    histo->GetYaxis()->SetLabelFont(22);
    histo->GetXaxis()->SetTitleColor(1);
    histo->GetXaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetTitleOffset(1.0);
    histo->GetXaxis()->SetTitleSize(0.05);
    histo->GetYaxis()->SetTitleSize(0.05);
    histo->SetMarkerStyle(21);
    histo->SetMarkerSize(0.34);
    histo->SetMarkerColor(4);
    histo->SetFillColor(0);
    histo->SetLineColor(4);
    histo->SetLineWidth(1);
    histo->Draw();
    canv->Update();

//     if (print)
//       ps->Close();
}

void DrawPlot2D( TH2D *histo, int print)
{
//     const char *txt = histo->GetName();
     const char *txt = histo->GetTitle();
     char txt2[1000];
//      TPostScript *ps;

        snprintf(txt2, sizeof(txt2), "plots/%s.eps",txt);
     
     if (print)
     {
//         ps = new TPostScript(txt2,113);
//         ps->NewPage();
     }
     
     snprintf(txt2, sizeof(txt2), "%s",txt);

     TCanvas *canv = new TCanvas(txt2,txt,845,800);
     canv->SetFillColor(10);
     canv->SetFillColor(10);
     canv->SetBorderSize(2);
     canv->SetGridx();
     canv->SetGridy();
//     canv->SetLogy();
     canv->SetFrameBorderSize(12);
     gStyle->SetOptStat(1000010);
     gStyle->SetPalette(1,0);
    
//    histo2->SetMinimum(0.1);
    histo->GetXaxis()->SetTitleFont(22);
    histo->GetXaxis()->SetTitleColor(1);
    histo->GetYaxis()->SetTitleFont(22);
    histo->GetXaxis()->SetLabelFont(22);
    histo->GetYaxis()->SetLabelFont(22);
    histo->GetXaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetTitleOffset(1.0);
//    histo->GetXaxis()->SetLabelOffset(0.02);
    histo->GetXaxis()->SetTitleSize(0.05);
    histo->GetYaxis()->SetTitleSize(0.05);
    histo->SetMarkerStyle(21);
    histo->SetMarkerSize(1.4);
    histo->SetFillColor(0);
    histo->SetLineColor(4);
    histo->SetLineWidth(1);
    histo->Draw("colz");
    
    canv->Update();
  
//     if (print)
//       ps->Close();
}

double distance(double x1,double y1,double x2,double y2)
{
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

float calculateTime(double Rstart, double Rend, int SCpoints, double *lt, int *Condition)
{
	float time=0.;

	for (int i=1; i<SCpoints; i++)
	{
		if (Rstart > lt[i]) continue;
		if (Rend < lt[i]) return (time/60.);
		if (Condition[i] && Condition[i-1])
		{
			if (lt[i]-lt[i-1]<70.)
				time+=(lt[i]-lt[i-1]);
		};
	};
	return (time/60.);
}
float calculateBGTime(double Rstart, double Rend, int SCpoints, double *lt, int *Condition, int *Tracking)
{
	float time=0.;

	for (int i=1; i<SCpoints; i++)
	{
		if (Rstart > lt[i]) continue;
		if (Rend < lt[i]) return (time/60.);
		if (Condition[i] && Condition[i-1] && !Tracking[i])
		{
			if (lt[i]-lt[i-1]<70.)
				time+=(lt[i]-lt[i-1]);
		};
	};
	return (time/60.);
}


void DrawaGraph(TGraph* gr, char* txt, int same)
{

  gr->SetTitle(txt);
  if (same)
  {
    gr->SetMarkerStyle(25);
    gr->SetMarkerSize(.54);
    gr->SetMarkerColor(same);
    gr->SetFillColor(0);
    gr->SetLineColor(same);
    gr->SetLineWidth(1);
    gr->Draw("PL");
  }
  else
  {
     char txt2[2000];
     strcpy(txt2,txt);
     char *ch;
     while ((ch=strrchr(txt2,' '))!=NULL)
       *ch='_';
     
     TCanvas* canv = new TCanvas(txt2,txt2,1);
     canv->SetFillColor(10);
     canv->SetFillColor(10);
     canv->SetBorderSize(2);
     canv->SetGridx();
     canv->SetGridy();
//     canv->SetLogy();
     canv->SetFrameBorderSize(12);
    gStyle->SetOptStat(1000010);
    gStyle->SetPalette(1,0);
    gr->Draw("APL");
    TH1F *histo=gr->GetHistogram();
//    histo->SetMinimum(0.1e-5);
   // histo->SetMaximum(1e2);
    histo->GetXaxis()->SetTitleFont(22);
    histo->GetYaxis()->SetTitleFont(22);
    histo->GetXaxis()->SetLabelFont(22);
    histo->GetYaxis()->SetLabelFont(22);
    histo->GetXaxis()->SetTitleColor(1);
    histo->GetXaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetTitleOffset(1.0);
    histo->GetXaxis()->SetTitleSize(0.05);
    histo->GetYaxis()->SetTitleSize(0.05);
    histo->GetYaxis()->SetTitle(txt);
     
    gr->SetMarkerStyle(24);
    gr->SetMarkerSize(0.55);
    gr->SetMarkerColor(4);
    gr->SetFillColor(0);
    gr->SetLineColor(4);
    gr->SetLineWidth(1);
    canv->Update();

  }
}


void DrawaGraphErr(TGraphErrors* gr, char* txt, int same, int time=0)
{

  gr->SetTitle(txt);
  if (same)
  {
    gr->SetMarkerStyle(25);
    gr->SetMarkerSize(.54);
    gr->SetMarkerColor(same);
    gr->SetFillColor(0);
    gr->SetLineColor(same);
    gr->SetLineWidth(1);
    gr->Draw("PL");
  }
  else
  {
     char txt2[2000];
     strcpy(txt2,txt);
     char *ch;
     while ((ch=strrchr(txt2,' '))!=NULL)
       *ch='_';
     
     TCanvas* canv = new TCanvas(txt2,txt2,1);
     canv->SetFillColor(10);
     canv->SetFillColor(10);
     canv->SetBorderSize(2);
     canv->SetGridx();
     canv->SetGridy();
//     canv->SetLogy();
     canv->SetFrameBorderSize(12);
    gStyle->SetOptStat(1000010);
    gStyle->SetPalette(1,0);
    gr->Draw("APL");
    TH1F *histo=gr->GetHistogram();
//    histo->SetMinimum(0.1e-5);
   // histo->SetMaximum(1e2);
    histo->GetXaxis()->SetTitleFont(22);
    histo->GetYaxis()->SetTitleFont(22);
    histo->GetXaxis()->SetLabelFont(22);
    histo->GetYaxis()->SetLabelFont(22);
    histo->GetXaxis()->SetTitleColor(1);
    histo->GetXaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetLabelSize(0.04);
    histo->GetYaxis()->SetTitleOffset(1.0);
    histo->GetXaxis()->SetTitleSize(0.05);
    histo->GetYaxis()->SetTitleSize(0.05);
    histo->GetYaxis()->SetTitle(txt);
    if (time>0)
    {
       histo->GetXaxis()->SetTimeDisplay(time);
       histo->GetXaxis()->SetTimeDisplay(time);     
       histo->GetXaxis()->SetTimeFormat("#splitline{%02d/%02m}{%H:%M}");
       histo->GetXaxis()->SetTimeFormat("#splitline{%H:%M:%S}{%d-%m-%y}");
       histo->GetXaxis()->SetLabelOffset(0.021);
       histo->GetXaxis()->SetLabelSize(0.03);
    }
     
    gr->SetMarkerStyle(24);
    gr->SetMarkerSize(0.55);
    gr->SetMarkerColor(4);
    gr->SetFillColor(0);
    gr->SetLineColor(4);
    gr->SetLineWidth(1);
    if (time<-5) 
    {
       canv->BuildLegend();
       cout <<" Build Legend!!!!"<<endl;
    }
    canv->Update();

  }
}


int FitaGraph(double* x, double*y, int nbins, double* par, double endval)
{
   //int npar=0;
   TGraph* gr=new TGraph(nbins,x,y);
   TF1 *f2= new TF1("FitChi2","pol9",0.001,endval);
   gr->Fit(f2,"NB","",0.001,endval);
   for (int i=0;i<f2->GetNpar();i++)
     par[i] = f2->GetParameter(i);
   return (f2->GetNpar());



}

int GetProjectionFitParameters(TH2D* histo2d, double* mean, double* sigma, double* gmean, double* gsigma, double* runs, int maxr)
{    
    TH1D *proj;
//     TH1D* proj2;
    int nbins=histo2d->GetNbinsX();
//     int nybins=histo2d->GetNbinsY();
    //double Energy[dbins], rms[dbins], sigma[dbins];
   
   double nothing=gmean[0];
   nothing=gsigma[0];
    
    int gpoints =0;
// //      TCanvas* c1= new TCanvas("DL_fit","DL_fit");
// //      c1->SetFillColor(10);
// //      c1->SetBorderSize(1);
// //      c1->SetFrameBorderSize(0);
// //      gPad->SetFillColor(10);
// //      gPad->SetBorderSize(1);
// //      gPad->SetGridx();
// //      gPad->SetGridy();
// //      gPad->SetFrameBorderSize(0);
// //      gStyle->SetOptStat(1111);
// //      gStyle->SetOptFit(111);
// //      TH1::StatOverflows(kTRUE);
    proj->StatOverflows(kTRUE);
    for (int n=1; n<nbins; n++)
    {
       
       proj = histo2d->ProjectionY("proj",n,n+1,"");
       
       double runNo=histo2d->GetXaxis()->GetBinLowEdge(n);
       if (runNo>maxr) break;
       
       if (proj->Integral()<=0) continue;
       
       runs[gpoints] = runNo;
       

       sigma[gpoints] = proj->GetRMS();
       if ( sigma[gpoints]<=0. ) continue;       
       
       mean[gpoints] = proj->GetMean();
       if ( mean[gpoints]<=0. ) continue;       
       
       cout <<"fitting slice " <<n<<endl;///proj->GetMean()<< endl;;
     
//        double miny=proj->GetBinLowEdge(1);
//        double maxy=proj->GetBinLowEdge(nbins-1);

////////////gaus is here!!!!!!       
// //        TF1* f1 = new TF1("gauss","gaus",miny,maxy);
// //        proj->Fit(f1,"NQR");
// //        
// // 
// //        gsigma[gpoints] = f1->GetParameter(2);
// //        
// //        gmean[gpoints] = f1->GetParameter(1);
// //        if ( gmean[gpoints]<=0 ) continue;       

///       gStyle->SetOptStat(1111111);
//         c1->Update();
///     cin >> n;
       gpoints++;
     }
    
    return (gpoints);   
}

void DrawGraphsFrom(TH2D* pthc, TH2D* pth, int maxr, char* txt1, char* txt2)
{
  
  double mean[10000], sigma[10000], gmean[10000], gsigma[10000], fitruns[10000];
  double bmean[10000], bsigma[10000], bgmean[10000], bgsigma[10000], bfitruns[10000];
  double err0[10000];
  char txt[1000];
  
  int ntst=GetProjectionFitParameters(pthc, mean, sigma, gmean, gsigma, fitruns, maxr);
  int bntst=GetProjectionFitParameters(pth, bmean, bsigma, bgmean, bgsigma, bfitruns, maxr);
  
  
  TGraphErrors* gr1= new TGraphErrors(ntst,fitruns,mean,err0,err0);
//   TGraphErrors* gr2= new TGraphErrors(ntst,fitruns,gmean,err0,err0);
  TGraphErrors* bgr1= new TGraphErrors(bntst,bfitruns,bmean,err0,err0);
//   TGraphErrors* bgr2= new TGraphErrors(bntst,bfitruns,bgmean,err0,err0);
  
  double max1=TMath::MaxElement(ntst,mean);
//   double max2=TMath::MaxElement(ntst,gmean);
  double max1b=TMath::MaxElement(bntst,bmean);
//   double max2b=TMath::MaxElement(bntst,bgmean);
  double max = TMath::Max(max1,max1b);
//   max1 = TMath::Max(max2,max2b);
//   max = TMath::Max(max1,max);
  
  double min1=TMath::MinElement(ntst,mean);
//   double min2=TMath::MinElement(ntst,gmean);
  double min1b=TMath::MinElement(bntst,bmean);
//   double min2b=TMath::MinElement(bntst,bgmean);
  double min = TMath::Min(min1,min1b);
//   min1 = TMath::Min(min2,min2b);
//   min = TMath::Min(min1,min);
  
  TH1F *htmp=(TH1F*) gr1->GetHistogram();
  htmp->SetMaximum(1.1*max);
  htmp->SetMinimum(0.9*min);
  
  snprintf(txt, sizeof(txt), "%s", txt1);
  DrawaGraphErr(gr1, txt, 0, 0);
  snprintf(txt, sizeof(txt), "Gaus %s", txt1);
//   DrawaGraphErr(gr2, txt, 3, 0);
 
  snprintf(txt, sizeof(txt), "%s bkg", txt1);
  DrawaGraphErr(bgr1, txt, 2, 0);
  snprintf(txt, sizeof(txt), "Gaus %s bkg", txt1);
//   DrawaGraphErr(bgr2, txt, 6, -13);
  
      TGraph *graph = new TGraph(4);
      graph->SetName("LowBkgRuns");
      graph->SetTitle("Low bkg Runs");
      graph->SetFillColor(5);
      graph->SetFillStyle(3005);
      graph->SetPoint(0,15075.,0.9*min);
      graph->SetPoint(1,15075.,1.1*max);
      graph->SetPoint(2,15110.,1.1*max);
      graph->SetPoint(3,15110.,0.9*min);
     /// graph->Draw("f");
    
    graph->Draw("F");
  
  
  gr1= new TGraphErrors(ntst,fitruns,sigma,err0,err0);
//   gr2= new TGraphErrors(ntst,fitruns,gsigma,err0,err0);
  bgr1= new TGraphErrors(bntst,bfitruns,bsigma,err0,err0);
//   bgr2= new TGraphErrors(bntst,bfitruns,bgsigma,err0,err0);
  
  max1=TMath::MaxElement(ntst,sigma);
//   max2=TMath::MaxElement(ntst,gsigma);
  max1b=TMath::MaxElement(bntst,bsigma);
//   max2b=TMath::MaxElement(bntst,bgsigma);
  max = TMath::Max(max1,max1b);
//   max1 = TMath::Max(max2,max2b);
//   max = TMath::Max(max1,max);
  
  min1=TMath::MinElement(ntst,sigma);
//   min2=TMath::MinElement(ntst,gsigma);
  min1b=TMath::MinElement(bntst,bsigma);
//   min2b=TMath::MinElement(bntst,bgsigma);
  min = TMath::Min(min1,min1b);
//   min1 = TMath::Min(min2,min2b);
//   min = TMath::Min(min1,min);
  
  htmp=(TH1F*) gr1->GetHistogram();
  htmp->SetMaximum(1.1*max);
  htmp->SetMinimum(0.9*min);
  
  snprintf(txt, sizeof(txt), "%s", txt2);
  DrawaGraphErr(gr1, txt, 0, 0);
  snprintf(txt, sizeof(txt), "Gaus %s", txt2);
//   DrawaGraphErr(gr2, txt, 3, 0);
 
  snprintf(txt, sizeof(txt), "%s bkg", txt2);
  DrawaGraphErr(bgr1, txt, 2, 0);
  snprintf(txt, sizeof(txt), "Gaus %s bkg", txt2);

      TGraph *graph2 = new TGraph(4);
      graph2->SetName("LowBkgRunsW");
      graph2->SetTitle("Low bkg Runs");
      graph2->SetFillColor(5);
      graph2->SetFillStyle(3005);
      graph2->SetPoint(0,15075.,0.9*min);
      graph2->SetPoint(1,15075.,1.1*max);
      graph2->SetPoint(2,15110.,1.1*max);
      graph2->SetPoint(3,15110.,0.9*min);
      graph2->SetFillStyle(3004);
     /// graph->Draw("f");
      graph2->Draw("F");
//   DrawaGraphErr(bgr2, txt, 6, -13);
}  

int GetProjectionTParameters(TH2D* histo2d, double* mean, double* sigma, double* gmean, double* gsigma, double* runs)
{    
    TH1D *proj;
///    TH1D* proj2;
    int nbins=histo2d->GetNbinsX();
    int nybins=histo2d->GetNbinsY();
    nybins+=0*gsigma[0];
    //double Energy[dbins], rms[dbins], sigma[dbins];
   
  TCanvas *c1= new TCanvas("demo","demo");
  
    
    int gpoints =0;
    
  //  proj->StatOverflows(kFALSE);
    for (int n=1; n<=nbins; n++)
    {
       
       proj = histo2d->ProjectionY("proj",n,n+1,"");
       
       double dt=histo2d->GetXaxis()->GetBinWidth(1);
       
       double runNo=histo2d->GetXaxis()->GetBinLowEdge(n)+dt/2.;
       
       proj->GetXaxis()->SetRange(2,proj->GetNbinsX()-2); 
       
       if (proj->Integral()<=0) continue;
       
//        proj->Sumw2();
       runs[gpoints] = runNo;
       gmean[gpoints] = dt/2.;
       

       sigma[gpoints] = proj->GetRMS();
       if ( sigma[gpoints]<=0. ) continue;       
       
       mean[gpoints] = proj->GetMean();
       if ( mean[gpoints]<=0. ) continue;       
    
       double miny= mean[gpoints]-1.5*sigma[gpoints];
       double maxy= mean[gpoints]+1.2*sigma[gpoints];

       double sumy=0,sumxy=0;
       for (int i=1;i< proj->GetNbinsX()-1; i++)
       {
           sumy+=proj->GetBinContent(i);
	   sumxy+=proj->GetBinContent(i)+proj->GetBinCenter(i); 
       }
       mean[gpoints] = sumxy/sumy;
       
          
       cout <<"fitting slice " <<n<<endl;///proj->GetMean()<< endl;;
     
//        double miny=proj->GetBinLowEdge(1);
//        double maxy=proj->GetBinLowEdge(nbins-1);
/// /////////gaus is here!!!!!!       
       TF1* f1 = new TF1("gauss","gaus",miny,maxy);
       proj->Fit(f1,"QRW");
       
       sigma[gpoints] = (2.3*f1->GetParameter(2)) / f1->GetParameter(1);
//        sigma[gpoints] = (f1->GetParameter(2));
       
       mean[gpoints] = f1->GetParameter(1);///*174.94;
       if ( gmean[gpoints]<=0 ) continue;       

       proj->SetMarkerStyle(1);
       proj->SetMinimum(0);
       gStyle->SetOptStat(1111111);
          c1->Update();
// int tst;
// cin>>tst;
//      cin >> n;
       gpoints++;
     }
//     delete proj;
    return (gpoints);   
}

void DrawXsliceFitsFrom(TH2D* pthc, char* txt1, char* txt2, int timef, double*mean, double* sigma, double* times)
{
  double  xerr[10000], gsigma[10000], fitruns[10000];
  double err0[10000];
  char txt[1000];
  
  int ntst=GetProjectionTParameters(pthc, mean, sigma, xerr, gsigma, fitruns);
  
  TGraphErrors* gr1= new TGraphErrors(ntst,fitruns,mean,xerr,err0);
  TGraphErrors* gr2= new TGraphErrors(ntst,times,mean,xerr,sigma);
  
  snprintf(txt, sizeof(txt), "%s", txt1);
  DrawaGraphErr(gr1, txt, 0, timef);
  snprintf(txt, sizeof(txt), "%s", txt1);
  DrawaGraphErr(gr2, txt, 0, 1);
 
  
  
  gr1= new TGraphErrors(ntst,fitruns,sigma,xerr,err0);
  gr2= new TGraphErrors(ntst,times,sigma,xerr,err0);
  
  
  snprintf(txt, sizeof(txt), "%s_graph", txt2);
  DrawaGraphErr(gr1, txt, 0, timef);
  snprintf(txt, sizeof(txt), "%s", txt2);
  DrawaGraphErr(gr2, txt, 0, 1);
 


}

double SumArray(double* ampl,int ni,int nf)
{
   double s=0.;
   for (int i=ni;i<=nf;i++)
     s+=ampl[i];
   return s;
}

void SmoothArray(double *arr, double *sarr, int apoints, int np, int inverse)
{
   if (np<=0) np=1;
   np = (np / 2) * 2 + 1; // make it odd >= np
   for (int i=0;i<np/2-1;i++)
     sarr[i]=arr[i]*inverse;
   double sum = SumArray(arr,0,np-1);
   sarr[np/2]=sum/np*inverse;
   
   for (int i=np/2+1;i<apoints-np/2;i++)
   {
     sum -= arr[i-(np/2+1)];
     sum += arr[i+np/2];
     sarr[i] = sum/np*inverse;

//      cout<<i<<endl;
  }
   for (int i=np/2+1-1;i>=0;i--)
   {
     sarr[i] = sarr[i+1];
   }
   for (int i=apoints-np/2;i<apoints;i++)
   {
     sarr[i] = sarr[i-1];
   }

}

void DerivateArray(double *arr, double *sarr, int apoints, double dt, int np, int neg=1)
{
   if (neg<0)
     neg=-1;
   else
     neg=1;
   if (np>20) np=20;
   if (np<1) np=1;  /// 1 <= np <= 10
//    dt*=1e9;
//    double sum = 0.;
   for (int i=0;i<np;i++)
     sarr[i]=0.;
   for (int i=apoints-np;i<apoints;i++)
     sarr[i]=0.;
   
   for (int i=np;i<apoints-np;i++)
//    for (int i=0;i<apoints-np;i++)
   {
     sarr[i] = neg*(arr[i+np] - arr[i-np])/(2*np*dt);
//      sarr[i] = (arr[i+np] - arr[i-np])/(2.*np);
   }
}

void DerivateArrayT(double *arr, double *sarr, int apoints, double dt, double DT, int neg=1)
{
   if (neg<0)
     neg=-1;
   else
     neg=1;
   int np = TMath::FloorNint(DT/dt);
   if (np>12) np=12;
   if (np<1) np=1;  /// 1 <= np <= 12
   
//    double sum = 0.;
   for (int i=0;i<np;i++)
     sarr[i]=0.;
   for (int i=apoints-np;i<apoints;i++)
     sarr[i]=0.;
   
   for (int i=np;i<apoints-np;i++)
//    for (int i=0;i<apoints-np;i++)
   {
     sarr[i] = neg*(arr[i+np] - arr[i-np])/(2*np*dt);
//      sarr[i] = (arr[i+np] - arr[i-np])/(2.*np);
   }

}

double FindFraction(TSpline *spline, double zero, double min, double ampl, double fraction)
{ 
   const double precision = 1e-6;  /// 1 ps
   double mid = (zero+min)/2.;
//    if (ampl>1.) ampl = 1.;
   if (fabs(min-zero)<precision)
      return (mid);
   if (fabs(spline->Eval(min) - ampl*fraction)<precision)
      return (min);
   else if (fabs(spline->Eval(zero) - ampl*fraction)<precision)
      return (zero);
   
   if (spline->Eval(mid) < ampl*fraction)
   {
       min = mid;
       double frac = FindFraction(spline, zero, min, ampl, fraction);
       return (frac);
   }
   else
   {
       zero = mid;
       double frac = FindFraction(spline, zero, min, ampl, fraction);
       return (frac);
   }
} 

void FindTimesS(TGraph* graph, DPARAM *par)
{
   double *data = graph->GetY();
   double *T = graph->GetX();
   int points = graph->GetN();
   
   /// find maximum of pulse derivative...
   int max=TMath::LocMax(points,data);
   par->maxtime=T[max];
   par->ampl=data[max];
   double ampl=data[max];

   /// find minimum of pulse derivative...
   int min=TMath::LocMin(points,data);
   double amplmin=data[min];
   par->t3 = T[min];
 
//    cout <<"____________________________\n min point at: "<<min<<endl;
   ///Find start of pulse
   int zero=max;
   while (data[zero]>0. && zero>10) zero--;
   par->t1 = T[zero];
//    cout <<" zero crossing at: "<<zero<<endl;
   /// Find end of Pulse
   int zero2=max;
   while (data[zero2]>0. && zero2<points) zero2++;
   
   int zero3=min;
   while (data[zero3]<0. && zero3>1)
   {
     zero3--;
     if (fabs(data[zero3])<fabs(data[min])*0.2) 
       break;
   }
   par->t2=T[zero3];
   
///   TGraph *graph = new TGraph(points,t,data);
   TGraph *graphS = new TGraph(zero2-zero+1,&T[zero],&data[zero]);
   TSpline *spline = new TSpline3("spline",graphS);
   double smax = 0.;
   for (double x = T[zero]; x< T[zero2]; x+=0.0005)  //times in ns!
   {
     if (spline->Eval(x)>smax)
     {
       par->maxtimeS = x;
       smax = spline->Eval(x);
       par->sampl=smax;
     }
   }
//    char *ch = "test";
//    DrawaGraph(graphS, ch, 0);
//    spline->Draw("same");

   for (double x = T[zero2]; x > par->maxtimeS; x-=0.0005)
   {
     if (spline->Eval(x)*spline->Eval(x-0.0005)<=0.)
     {
       par->ftime = x-0.00025;
       break;
     }
   }
   double sum=0.;
   for (double x = T[zero]; x <= par->ftime; x+=0.0005)
     sum += spline->Eval(x);
   
   sum*=0.0005 ;
   
   par->intg = sum;
  
  
///   double miny = f1->Eval(mean);
   
   par->time50 = FindFraction(spline,T[zero],T[max],ampl, 0.5);
///    cout<<"T(50%) = "<<par[2] <<" ampl = "<<ampl<<" tmax = "<< par[0]<<"   spline tmax = "<< par[3]<<"   spline t20 = "<< par[4]<<endl;
   delete spline;
   delete graphS;
///   delete graph;

}

void FindTimesSD(TGraph* graph, DPARAM *par, double tstart, double tend, double *tf)
{
   double *data = graph->GetY();
   double *T = graph->GetX();
   double dt = T[1]-T[0];
   int points = graph->GetN();
   
   /// find maximum of pulse
   int max=TMath::LocMax(points,data);
   par->maxtime=T[max];
   par->ampl=data[max];
   double ampl=data[max];

   ///Find start of pulse
   int istart=max;
   while (data[istart] > par->rms && istart>10) istart--;
   par->t1 = T[istart];

   /// find end of pulse 
   int iend = max;
   while (data[iend] > par->rms && iend<points) iend++;
   par->t2 = T[iend];
   
   int spoints = 0;
   int ppoints = 0;
   double intg = 0.;
   double charge = 0.;
   int its=0,ite=0;
   int Start=1, End=1;   
   for(int i=0; i<points; i++)
   {
     if (Start && T[i]>=tstart)
     {
       its = i;
       Start = 0;
     }
     if (End && T[i]>tend)
     {
       ite = i;
       End = 0;
     }
     if ((!Start) && End)
     {
       intg += data[i];
       spoints++;
     }
     if (i>=istart && i<=iend)
     {
       charge += data[i];
       ppoints++;
     }
   }

///   recalculate the maximum from the spline (maxtimeS and smax)
   TGraph *graphS = new TGraph(spoints,&T[its],&data[its]);
   TSpline *spline = new TSpline3("spline",graphS);
   double smax = 0.;
   for (double x = T[istart]; x< T[iend]; x+=0.0005)  //times in ns!
   {
     if (spline->Eval(x)>smax)
     {
       par->maxtimeS = x;
       smax = spline->Eval(x);
       par->sampl=smax;
     }
   }

   par->intg = intg*dt/(par->ampl) *0.1;//*dt;
   par->charge = charge/80.; /// (normalize to ~80 points)

//    char *ch = "test";
//    DrawaGraph(graphS, ch, 0);
//    spline->Draw("same");

//    for (double x = T[istart2]; x > par->maxtimeS; x-=0.0005)
//    {
//      if (spline->Eval(x)*spline->Eval(x-0.0005)<=0.)
//      {
//        par->ftime = x-0.00025;
//        break;
//      }
//    }

   if (fabs(smax-par->ampl)/(smax+par->ampl)>0.03) smax=par->ampl;
   
   double t0 = par->maxtimeS;
   double t1 = par->t1;
   double dx = 0.00025;
   
   for (int i=6; i>=0; i--)
   {
     double ampl0 = smax * (i+1.) / 10.;
     for (double x = t0; x>t1; x-=dx)
     {
       double y1 = spline->Eval(x-dx) - ampl0;
       double y2 = spline->Eval(x) - ampl0;
       if ( y1*y2 <=0. )
       {
	 if (y1==0)
	   tf[i] = x-dx;
	 else
	 tf[i] = x-dx + dx * y1/(fabs(y1)+fabs(y2));
	 
	 t0 = tf[i];
// 	 cout<<tf[i]<<" ";
	 break;
       }
     }
   }
/// find the threshold crossing point
   par->ttrig=100.;
   double ampl0 = Threshold*1.75;
   for (double x = par->maxtimeS; x>t1; x-=dx)
     {
       double y1 = spline->Eval(x-dx) - ampl0;
       double y2 = spline->Eval(x) - ampl0;
       if ( y1*y2 <=0. )
       {
	 if (y1==0)
	   par->ttrig = x-dx;
	 else
	 par->ttrig = x-dx + dx * y1/(fabs(y1)+fabs(y2));
	 
// 	 cout<<tf[i]<<" ";
	 break;
       }
     }


  
///   double miny = f1->Eval(mean);
   
//    par->time50 = FindFraction(spline,T[istart],T[max],ampl, 0.5);
///    cout<<"T(50%) = "<<par[2] <<" ampl = "<<ampl<<" tmax = "<< par[0]<<"   spline tmax = "<< par[3]<<"   spline t20 = "<< par[4]<<endl;
   delete spline;
   delete graphS;
///   delete graph;

}




void FindTimesF(TF1* f1,double t1,double t2,double* tf)
{
   double ampl = f1->GetMaximum(t1,t2);
   double xmax = f1->GetMaximumX(t1,t2);
   double dx = 0.00025;
   double t0 = xmax;
   double y1,y2;
   for (int i=6; i>=0; i--)
   {
     double ampl0 = ampl * (i+1.) / 10.;
     for (double x = t0; x>t1; x-=dx)
     {
       y1 = f1->Eval(x-dx) -ampl0;
       y2 = f1->Eval(x) -ampl0;
       if ( y1*y2 <=0. )
       {
	 if (y1==0)
	   tf[i] = x-dx;
	 else
	 tf[i] = x-dx + dx * y1/(fabs(y1)+fabs(y2));
	 
	 t0 = tf[i];
	 break;
       }
     }
   }
  
}

void FindTimesG(TGraph* gr1,double t1,double t2,double* tf)
{
   double dx = 0.00025;
   double y1,y2;

   double *data = gr1->GetY();
   double *T = gr1->GetX();
   int points = gr1->GetN();
   int npoints = 0;
   int ts=0,te=0;
   for (int i=0; i<points; i++)
   {
     if (T[i]>=t1-dx){
       ts=i;
       break;
     }
   }
   for (int i=ts; T[i]<t2; i++)
   {
     te=i;
     npoints++;
   }
       
   TGraph *graphS = new TGraph(npoints,&T[ts],&data[ts]);
   TSpline *f1 = new TSpline3("spline",graphS);

   double ampl = f1->Eval(t1);
   double xmax = t1;

   for (double x = t1; x<t2; x+=dx)
   {
     y1=f1->Eval(x);
     if (y1>ampl)
     {
       ampl=y1;
       xmax=x;
     }
   }
//       char ch[20];
//       snprintf(ch, sizeof(ch), "testTrig");
//       DrawaGraph(graphS, ch, 0);
//       f1->Draw("same");
//   return ;   
   double t0 = xmax;
   
   for (int i=6; i>=0; i--)
   {
     double ampl0 = ampl * (i+1.) / 10.;
     for (double x = t0; x>t1; x-=dx)
     {
       y1 = f1->Eval(x-dx) -ampl0;
       y2 = f1->Eval(x) -ampl0;
       if ( y1*y2 <=0. )
       {
	 if (y1==0)
	   tf[i] = x-dx;
	 else
	 tf[i] = x-dx + dx * y1/(fabs(y1)+fabs(y2));
	 
	 t0 = tf[i];
// 	 cout<<tf[i]<<" ";
	 break;
       }
     }
   }
//    cout<<endl;
   delete f1; 
   delete graphS;
}


void IntegrateG(TGraph* graph, DPARAM *par)
{
 par->charge = 0.;
 if (graph) {
   double *data = graph->GetY();
   double *T = graph->GetX();
   int points = graph->GetN();
//    cout<<" Graph has "<<points <<" points"<<endl;
   double tstart = par->t1;  
   int t0=0;
   while (T[t0]<tstart && t0<points) {t0++;}
   
   double sum = 0.;
   for (int i=t0 ; T[i]<par->t3 || data[i]>0.; i++)
   {
     if (i> points)
     {
//        cout<<" end of points reached"<<endl;
//        cout <<i <<endl;
//        cout  <<endl;
       break;
     }
     double dt = T[i+1]-T[i];
     sum += (data[i]+data[i+1]) / (2.*dt);
   }
   par->charge = sum;
 }
}

double IntegrateA(int npoints, double* data, double* integral,double dt)
{
  double sum=0.;
//   dt*=1e9;
  if (npoints<4) return sum;
  
  for (int i=0;i<4;i++) integral[i]=0;
  
  for (int i=4;i<npoints;i++)
  {
    integral[i] = integral[i-4] + 2*(data[i-3]+data[i-1])*dt;
    sum+=integral[i];
  }
  return (sum);
}
    
double IntegratePulse(int npoints, double* data, double* integral,double dt,double tint)
{
//   dt*=1e9;
  double sum=data[0]*dt;
  if (npoints<2) return sum;
  
  int tst = TMath::FloorNint(tint/dt);
  
  integral[0]=data[0]*dt;
  if (tst<2) tst = 2;
 
  for (int i=1; i<tst; i++){
    integral[i]=data[i]*dt+integral[i-1];
    sum+=data[i]*dt;
  }
  
  for (int i=tst; i<npoints; i++){
    integral[i]=data[i]*dt+integral[i-1]-data[i-tst]*dt;
    sum+=data[i]*dt;
  }
  return (sum);
}
    
  
int AnalyseIntegratedPulse(int points, double* data, IPARAM *par, double threshold, double dt)
{
  /// use the integrated+filtered pulse to define a region where a trigger occured. (integral above threshold) 
  ///pulses are considered negative!!!
//   dt*=1e9;
  if (points < 20) return -1;
  int ntrig=0;
  int tpoint=0;
  par->tot=0;
  for (int i=0; i<points; i++)   {
    if (data[i]<=threshold) {
      tpoint = i;
      ntrig=1;
      par->tot=1;
      break;
    }
  }
//   cout<<"tpoint = "<<tpoint*dt<<endl;
  if (tpoint>=points-10 || tpoint == 0) return 0;

  double miny = data[tpoint];
  par->charge=0.;

  for (int i=tpoint; i<points; i++)
  {
    if (data[i]<miny)
    {
      par->ampl=data[i];
      par->maxtime=i;
      miny=data[i];
    }
    par->charge+=data[i];
    if (data[i]<=threshold)
      par->tot++;
    if (data[i]>threshold/10.)
    {
      par->ftime=i;
      break;
    }
  }
  /// fast scan for risetime, risecharge and t_start
  par->t90=tpoint;
  par->t10=tpoint;
  par->stime=tpoint;
  for (int i=par->maxtime; i>0; i--)
  {
     if (data[i]>=par->ampl*0.9)
     {
       par->t90=i;
       break;
     }
  }
  par->risecharge=0.;
  for (int i=par->t90; i>0; i--)
  {
    par->risecharge+=data[i]; 
    if (data[i]>=par->ampl*0.1)
    {
      par->t10=i;
      break;
    }
  }
  
  for (int i=par->t10; i>0; i--)
  {
    if (i<tpoint)
      par->charge+=data[i];
      par->stime=i;
    if (data[i]>threshold/100. || (data[i+1]-data[i-1])/dt >=-0.0001 )
    {
      break;
    }
  }
  par->width = par->ftime-par->stime;
  par->ampl*=-1.;
  par->charge*=-1.*dt;
//   cout<<"tstart = "<<par->stime*dt<<endl;
//   cout<<"tend = "<<par->ftime*dt<<endl;
//   cout<<"tmax = "<<par->maxtime*dt<<endl;
//   cout<<"ampl = "<<par->ampl<<endl;
//   cout<<"t10 = "<<par->t10*dt<<endl;
//   cout<<"t90 = "<<par->t90*dt<<endl;
//   cout<<"rt = "<<(par->t90-par->t10)*dt<<endl;
//   cout<<"charge = "<<par->charge*dt/N_INTEGRATION_POINTS<<endl;
//   cout<<"risecharge = "<<par->risecharge*dt/N_INTEGRATION_POINTS<<endl;
  return (ntrig);
}
      
int SubtractRefferenceChannel(int* spoints,double** data, double* sampl,int cref, double* bsl)
{
  int points = spoints[cref]; 
  if (points<0)
  {
    cout<<"Pulse to subtract is empty!!!"<<endl;
    return -1;
  }
  double f[] = {0.,0.,0.,0.}; 
  for (int i=0;i<4;i++)
  {
    if (i!=cref && spoints[i]>0)
    {
      f[i]++;
      f[cref]--;
    }
  }
    
  for (int i=0;i<points;i++)
  {
      sampl[i]=f[0]*(data[0][i]-bsl[0]) + f[1]*(data[1][i]-bsl[1]) + f[2]*(data[2][i]-bsl[2]) + f[3]*(data[3][i]-bsl[3]); 
  }
  
  return (-f[cref]);
}
 
int SubtractMaxChannel(int points,double** data, double* sampl, double* bsl, int* mask)
{
  if (points<=0)
  {
    cout<<"Nothing to subtract!!!"<<endl;
    return -1;
  }
//   cout<<"points  = "<<points<<endl;
  double max = -1000.;  
  for (int i=0;i<points;i++)
  {
    max = -1000.;
    for (int ci=0;ci<4;ci++)
    {
      if (mask[ci]) continue;
      data[ci][i] -= bsl[ci];
      if (data[ci][i]>max) 
	max = data[ci][i];
    }
    sampl[i]=0;
//     if (i<100) 
//      cout<<" max = "<<max<<endl;
    for (int ci=0;ci<4;ci++)
    {
      if (mask[ci]) continue;
      data[ci][i]-=max;
      sampl[i]+=data[ci][i];
    }
//     if (i<30) cout<<i<<endl;
  }
  
  return (max);
}
 
int SubtractTwoChannels(int points,double** data, double* sampl, double* bsl, int* mask)
{
  if (points<=0)
  {
    cout<<"Nothing to subtract!!!"<<endl;
    return -1;
  }
//   cout<<"points  = "<<points<<endl;
//   double max;  
  for (int i=0;i<points;i++)
  {
    int i1 = mask[0];
    int i2 = mask[1];
    double y1=data[i1][i]-bsl[i1];
    double y2=data[i2][i]-bsl[i2];
    if (y2-y1<0)
    {
      sampl[i]=y2-y1;
    }
    else
      sampl[i]=y1-y2;
  }
//     max = -1000.;
//     for (int ci=0;ci<2;ci++)
//     {
//       data[mask[ci]][i] -= bsl[ci];
//       if (data[mask[ci]][i]>max) 
// 	max = data[mask[ci]][i];
//     }
//     sampl[i]=0;
// //     if (i<100) 
// //      cout<<" max = "<<max<<endl;
//     for (int ci=0;ci<2;ci++)
//     {
//       data[mask[ci]][i]-=max;
//       sampl[i]+=data[mask[ci]][i];
//     }
// //     if (i<30) cout<<i<<endl;
//   }
  
  return (points);
}
 
int SubtractBaselineOld(int points,double** data, double* sampl, double* bsl, int* mask)
{
  if (points<=0)
    {
      cout<<"Nothing to subtract!!!"<<endl;
      return -1;
    }
//   cout<<"points  = "<<points<<endl;
//   double max;  
  for (int i=0;i<points;i++)
    {
      int i1 = mask[0];
      int i2 = mask[1];
      double y1=data[i2][i]-bsl[i2];
      sampl[i]=y1;
    }
  
  return (points);
} 

int SubtractBaseline(int points,double* data, double* sampl, double bsl)
{
  if (points<=0)
    {
      cout<<"Nothing to subtract!!!"<<endl;
      return -1;
    }
//   cout<<"points  = "<<points<<endl;
//   double max;  
  for (int i=0;i<points;i++)
    {
      double y1=data[i]-bsl;
      sampl[i]=y1;
    }
  
  return (points);
} 

void Pallette1()
{
   static Int_t  colors[50];
   static Bool_t initialized = kFALSE;

   Double_t Red[3]    = { 1.00, 0.00, 0.00};
   Double_t Green[3]  = { 0.00, 1.00, 0.00};
   Double_t Blue[3]   = { 1.00, 0.00, 1.00};
   Double_t Length[3] = { 0.00, 0.50, 1.00 };

   if(!initialized){
      Int_t FI = TColor::CreateGradientColorTable(3,Length,Red,Green,Blue,50);
      for (int i=0; i<50; i++) colors[i] = FI+i;
      initialized = kTRUE;
      return;
   }
   gStyle->SetPalette(50,colors);
}

void Pallette2()
{
   static Int_t  colors[50];
   static Bool_t initialized = kFALSE;

   Double_t Red[3]    = { 1.00, 0.50, 0.00};
   Double_t Green[3]  = { 0.50, 0.00, 1.00};
   Double_t Blue[3]   = { 1.00, 0.00, 0.50};
   Double_t Length[3] = { 0.00, 0.50, 1.00 };

   if(!initialized){
      Int_t FI = TColor::CreateGradientColorTable(3,Length,Red,Green,Blue,50);
      for (int i=0; i<50; i++) colors[i] = FI+i;
      initialized = kTRUE;
      return;
   }
   gStyle->SetPalette(50,colors);
}


int AnalyseLongPulse(int points, double* data, double* drv, IPARAM *par, double threshold, double dt, int tshift)
{
  /// use the integrated+filtered pulse to define a region where a trigger occured. (integral above threshold) 
  ///pulses are considered negative!!!
  if (points - tshift < 20) return -1;
  int ntrig=0;
  int tpoint=0;
  double drvtrig = 0.00002;
  if (threshold <0.0025)
    drvtrig = 0.000005;
  par->tot=0;
  
  for (int i=tshift; i<points; i++)   {
    if (data[i]<=threshold) {
      tpoint = i;
      ntrig=1;
//       par->tot=1;
      break;
    }
    else 
      tpoint=i;
  }
  if (ntrig<=0) return (-1); // cout<<"No trigger in event!"<<endl;
  
//    cout<<"tpoint = "<<tpoint*dt<<endl;
  if (tpoint>=points-10) return (-1);

  double miny = data[tpoint];
  par->charge=0.;
  par->maxtime=tpoint;
  par->ampl=data[tpoint];

  for (int i=tpoint; i<points; i++)
  {
    if (data[i]<miny)
    {
      par->ampl=data[i];
      par->maxtime=i;
      miny=data[i];
    }
    if (data[i]<=threshold)
    {
      par->tot++;
      par->ftime=i; /// this is added to avoid a pulse at the end of the data that does not return to 0!!!
    }
    else //if (data[i]>threshold)
    {
      par->ftime=i;
      par->tot--;
      break;
    }
    /// note down the point the signal has gone above the threshold
  }
  /// fast scan for risetime, risecharge and t_start
  par->t90=tpoint;
  par->t10=tpoint;
  par->stime=tpoint;
  par->ttrig=tpoint; 
  for (int i=par->maxtime; i>0; i--)
  {
     if (data[i]>=par->ampl*0.9)
     {
       par->t90=i;
       break;
     }
  }
  par->risecharge=0.;
  for (int i=par->t90; i>0; i--)
  {
    par->risecharge+=data[i]; 
    if (data[i]>=par->ampl*0.1 || (data[i]>0.5*threshold && fabs(drv[i])<=drvtrig ))
    {
      par->t10=i;
      break;
    }
  }

  for (int i=par->maxtime; i<points; i++)
  {
    if (data[i]>=par->ampl*0.1 || (data[i]>0.5*threshold && fabs(drv[i])<=drvtrig ))
    {
      par->tb10=i;
      break;
    }
  }
  
  for (int i=par->t10; i>0; i--)
  {
//     cout<<data[i]<<"  "<<fabs(drv[i])<<"   "<<threshold<<endl;
    par->stime=i;
//     if (i<tpoint)
//     {
//       par->charge+=data[i];
//     }
    if (data[i]>threshold/5. || (fabs(drv[i])<=drvtrig && data[i]>threshold*0.8 ) )
    {
      par->stime=i;
      break;
    }
  }
  for (int i=par->ftime; i<points; i++)
  {
    par->ftime=i;
    if (data[i]>threshold/5. || (fabs(drv[i])<=drvtrig && data[i]>threshold*0.8 ) )
    {
      break;
    }
  }
  
  par->charge=0.;
  for (int i=par->stime;i<=par->ftime;i++)
    par->charge+=data[i];

  par->sampl = data[par->stime];
  par->fampl = data[par->ftime];
  par->bslch = -0.5 * (data[par->stime] + data[par->ftime])*(par->ftime - par->stime +1.);
  par->width = par->ftime-par->stime;
  par->ampl*=-1.;
  par->charge*=-1.*dt;   ///charge is calculated in V * ns. 
//   cout<<"tstart = "<<par->stime*dt<<endl;
//   cout<<"t10 = "<<par->t10*dt<<endl;
//   cout<<"t90 = "<<par->t90*dt<<endl;
//   cout<<"tmax = "<<par->maxtime*dt<<endl;
//   cout<<"tb10 = "<<par->tb10*dt<<endl;
//   cout<<"tend = "<<par->ftime*dt<<endl;
//   
//   cout<<"rt = "<<(par->t90-par->t10)*dt<<endl;
//   cout<<"tot = "<<(par->tot)*dt<<endl;
//   cout<<"DT = "<<(par->tb10-par->t10)*dt<<endl;
//   cout<<"DTall = "<<(par->ftime-par->stime)*dt<<endl;
//   
//   cout<<"ampl = "<<par->ampl<<endl;
//   cout<<"charge = "<<par->charge*dt/N_INTEGRATION_POINTS<<endl;
//   cout<<"risecharge = "<<par->risecharge*dt/N_INTEGRATION_POINTS<<endl;
  return (par->ftime);

  // Amplitude: 0 V ─────────────────────────────────────────────────────────
  //                   '\'                                  
  //                    '\'                                 
  //                     '\' ← t10 (10% on rising edge)      
  //                      '\'                               
  //                       '\'                              
  //                        '\' ← t90 (90% on rising edge)   
  //                         '\'                            
  //                          '\'                           
  //                           '\' ← Peak (100%)            
  //                            '\'                         
  //                             '\'                        
  //                              '\' ← tb10 (10% on falling edge)
  //                               '\'                      
  //                                '\'                     
  //                                 '\' ← Returns to baseline
  //                                  ────────────────────
}

void AddPar(IPARAM* ipar, IPARAM* spar, double dt)
{
   spar->bsl=ipar->bsl;
   spar->rms=ipar->rms;
   spar->stime=ipar->stime;
   spar->ftime=ipar->ftime;
   spar->maxtime=ipar->maxtime;
   spar->tot=ipar->tot * dt;
   spar->ampl=ipar->ampl;
   spar->charge=ipar->charge * dt;
   spar->risecharge=ipar->risecharge * dt;
   spar->t10=ipar->t10 * dt;
   spar->tb10=ipar->tb10 * dt;
   spar->t90=ipar->t90 * dt;
   spar->ttrig=ipar->ttrig * dt;
   spar->width=ipar->width * dt;
   spar->sampl=ipar->sampl;
   spar->fampl=ipar->fampl;
   spar->bslch=ipar->bslch*dt; 
  
}
void DivideH(TH1D* h1, TH1D* h2)
{
   double x, y, sx;
   int n1 = h1->GetNbinsX();
   int n2 = h2->GetNbinsX();
//    if (n1 > n2) n1 = n2;
   
   for (int i=0;i<=n1; i++)
   {
     x = h1->GetBinContent(i);
     y = h1->GetBinWidth(i);  /// in seconds!
     sx = sqrt(x);
     if (y!=0)
     {
        x/=y;
	sx/=y;
     }
     h1->SetBinContent(i,x);
     h1->SetBinError(i,sx);
   }
}

void DivideHspark(TH1D* h1, TH1D* h2, TH1D* hspark)
{
   double x, y, sx, nsparks;
   double binwidth = h1->GetBinWidth(1);
   double npulses = floor(binwidth / 1.2) ;
   if (npulses==0) 
   {
     cout<<" ATTENTION !!!! Time binwidth smaller than LINAC4 periode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
     npulses = 1;
   }
//    npulses++;
   cout<<"****  Rate evolution BinWidth = "<<binwidth<<endl;
   cout<<"****  Corresponding to "<<npulses<<" Linac4 pulses"<<endl;
   int n1 = h1->GetNbinsX();
   int n2 = h2->GetNbinsX();
   if (n1 != n2) 
   {
     cout<<"ATTENTION !!!!!!   Plot and normalization plot do not have the same number of bins!!!!!!!"<<endl;
     if (n1>n2) n1 = n2;
   }
   for (int i=0;i<=n1; i++)
   {
     x = h1->GetBinContent(i);
     y = h2->GetBinContent(i);
     nsparks = hspark->GetBinContent(i);
     
     if (y+nsparks > npulses) {
       cout<<" ******     found bin with "<<y<<" entries and " <<nsparks<<" sparks & recoveries    *******"<<endl;
       npulses = y;
     }
     y = npulses; 
     sx = sqrt(x);
     if (y!=0)
     {
        x/=y;
	sx/=y;
     }
     h1->SetBinContent(i,x);
     h1->SetBinError(i,sx);
   }
}

void ScaleHistoErr(TH1D* h1, double scale)
{
   double x, sx;
   double binwidth = h1->GetBinWidth(1);

   int n1 = h1->GetNbinsX();
   for (int i=0;i<=n1; i++)
   {
     x = h1->GetBinContent(i);
     sx = sqrt(x);
     if (scale!=0)
     {
        x*=scale;
	sx*=scale;
     }
     h1->SetBinContent(i,x);
     h1->SetBinError(i,sx);
   }
}

int FilterHisto(TH1* h1, double scale=0.1)
{
   double x, sx;
   scale=fabs(scale);
   if (scale>1) return 0;
   sx = h1->GetMaximum();

   int n1 = h1->GetNbinsX();
   for (int i=0;i<=n1; i++)
   {
     x = h1->GetBinContent(i);
     if (x < scale*sx)
     {
       h1->SetBinContent(i,0.);
     }
   }
   return 1;
}


TCanvas *statsEditing() {
   // Create and plot a test histogram with stats
   TCanvas *se = new TCanvas;
   TH1F *h = new TH1F("h","test",100,-3,3);
   h->FillRandom("gaus",3000);
   gStyle->SetOptStat();
   h->Draw();
   se->Update();
   // Retrieve the stat box
   TPaveStats *ps = (TPaveStats*)se->GetPrimitive("stats");
   ps->SetName("mystats");
   TList *listOfLines = ps->GetListOfLines();
   // Remove the RMS line
   TText *tconst = ps->GetLineWith("RMS");
   listOfLines->Remove(tconst);
   // Add a new line in the stat box.
   // Note that "=" is a control character
   TLatex *myt = new TLatex(0,0,"Test = 10");
   myt ->SetTextFont(42);
   myt ->SetTextSize(0.04);
   myt ->SetTextColor(kRed);
   listOfLines->Add(myt);
   // the following line is needed to avoid that the automatic redrawing of stats
   h->SetStats(0);
   se->Modified();
   return se;
}



#endif
