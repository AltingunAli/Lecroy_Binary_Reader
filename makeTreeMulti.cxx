#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<iostream>
#include<vector>
#include<math.h>
#include<fstream>

#include "MyFunctions.h"

using namespace std;

int main(int argc, char **argv)
{
  int detid=0;
  int runid=0;
  int line=0;
  if (argc>3)  {
    int tst=sscanf(argv[1],"%d", &detid);
    int tst2=sscanf(argv[2],"%d", &runid);
    int tst3=sscanf(argv[3],"%d", &line);
    if (tst<=0||tst2<=0||tst3<=0) {
      cout<<"syntax: makeTree <detector number> <run number> <channel number>"<<endl; 
      return -1;
    }
  }
  else {
    cout<<"syntax: makeTree <detector number> <run number> <channel number>"<<endl; 
    return -2;
  }
    
  if (runid<MINRUN || runid>MAXRUN){
    cout<<"run "<<runid<<" out of bounts ("<<MINRUN<<"-"<<MAXRUN<<")"<<endl;
    return -3;
  }
  
  char fname[100];
  snprintf(fname, sizeof(fname), "%s/logs/logS%03d-%02d-ch%d.txt", WORKDIR, detid, runid, line);
  FILE *ftmp = fopen(fname, "w");
  fclose(ftmp);
  char command[2000];
  snprintf(command, sizeof(command), "root -b -x '%s/MakeTreefromRawTreeProductionMulti.C+(%d,%d,%d)' -q >> %s/logs/logS%03d-%02d-ch%d.txt &\n", WORKDIR, detid, runid, line, WORKDIR, detid, runid, line);
  cout << "executing:\n" << command << endl;
  system(command);

  return (1);
}
