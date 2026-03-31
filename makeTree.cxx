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
  if (argc>2)  {
    int tst=sscanf(argv[1],"%d", &detid);
    int tst2=sscanf(argv[2],"%d", &runid);
    if (tst<=0||tst2<=0) {
      cout<<"syntax: makeTree <detector number> <run number>"<<endl; 
      return -1;
    }
  }
  else {
    cout<<"syntax: makeTree <detector number> <run number>"<<endl; 
    return -2;
  }
    
  if (runid<MINRUN || runid>MAXRUN){
    cout<<"run "<<runid<<" out of bounts ("<<MINRUN<<"-"<<MAXRUN<<")"<<endl;
    return -3;
  }
  
  //  std::cout << "WTF 1!" << std::endl;

  char fname[100];
  // std::cout << "WTF 2!" << std::endl;

  snprintf(fname, sizeof(fname), "%s/logs/logS%03d-%02d.txt", WORKDIR, detid, runid);
  // std::cout << "WTF 3!" << std::endl;

  FILE *ftmp = fopen(fname, "w");
  // std::cout << "WTF 4!  " << fname << std::endl;

  if (ftmp == nullptr)
    std::cout << "ftmp is fucked!" << std::endl;

  fclose(ftmp);
  // std::cout << "WTF 5!" << std::endl;

  char command[2000];
  snprintf(command, sizeof(command),
           "root -b -x '%s/MakeTreefromRawTreeProduction.C+(%d,%d)' -q >> "
           "%s/logs/logS%03d-%02d.txt &\n",
           WORKDIR, detid, runid, WORKDIR, detid, runid);
  cout << "executing:\n" << command << endl;
  system(command);

  return (1);
}