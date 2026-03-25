#include "readLeCroyBinary.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "../MyFunctions.h"

#include <TFile.h>
#include <TH1.h>
#include <TMath.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>

using namespace std;

int ffread(unsigned short int byteOrder, void *val, unsigned int nByte,
           FILE *fileIn);

void printWavedesc(wavedesc);
// void replaceEOL(char *fnames);
// void replaceEOLT(char *fnames);
// const double rootConv = 2871763200.;
// const double unixConv = 2082844800.;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "%s <detNo> [<runNo>] [<inputFolder> [<outputFolder>]]\n\n",
            argv[0]);
    return 1;
  }

  // BaseName is the name of the channels in the tree. It will be used as the
  // branch name for the channel data in the tree
  char BaseName[4][100];
  for (int i = 0; i < 4; i++)
    snprintf(BaseName[i], 100, "C%d", i + 1);

  // define data folder path. Base path is fixed, the top folder is given as
  // first argument
  char basedatapath[1000];
  char basedirname[1000];
  char dirname[1000];
  char ofname[1000];
  char fnametmp[1000];
  char fname[4][1000];

  // Define the output file name. It will be used as the name of the tree file
  // that will be created
  char dtype = 'S';

  // Define the active channels. If a channel is active, its data will be read
  // and stored in the tree. If a channel is not active, it will be ignored.
  int active[] = {1, 1, 1, 1}; /// if 1 then the Channel is active

  // Run type  - PRODUCTION, CALIBRATION, TEST, etc.
  char rtype[1000];
  strcpy(rtype, RUN_TYPE);

  // Initialiaze the command line argument variables
  int detNo = atoi(argv[1]); // detector number is the first argument
  int runNo =
      1; // default run number is 1, but it can be given as the second argument
  if (argc > 2) // if the second argument is given, it is the run number
    runNo = atoi(argv[2]);

  // Set the base and output directory names. The base directory is where the
  // raw data is located, and the output directory is where the tree file will
  // be created. The base directory can be given as the third argument, and the
  // output directory can be given as the fourth argument.
  snprintf(basedatapath, 1000, "%s", DATA_PATH_NAME);
  if (argc > 3)
    snprintf(basedatapath, 1000, "%s", argv[3]);

  snprintf(basedirname, 1000, "%s", WORK_DIR_NAME);
  if (argc > 4)
    snprintf(basedirname, 1000, "%s", argv[4]);

  char command[1000]; // command array for system commands
  int vm, vd; // mesh and drift voltages, will be read from the folder name
  float bthick,
      dgap; // mylar thickness and drift gap, will be read from the folder name
  string filetype = ""; //
  int width = 3; // width of the detector number in the folder name, it will be
                 // determined based on the detector number

  // Search for the folder that contains the raw data for the given detector
  // number and run number. The folder name should contain the detector number,
  // run number, mesh and drift voltages, mylar thickness and drift gap, and
  // file type (if any). The folder name should be in the format
  // SXXX-YY-Vm-Vd-bthick-dgap-filetype, where:
  //  XXX is the detector number,
  //  YY is the run number,
  //  Vm is the mesh voltage,
  //  Vd is the drift voltage,
  // filetype is any additional information about the run type (e.g. PRODUCTION,
  // CALIBRATION, etc.). The folder name should be searched in the base
  // directory.
  if (detNo > 100)
    width = 4;
  if (detNo > 10000)
    width = 6;

  // Check if the detector number is within the valid range.
  // If it is, search for the folder that contains the raw data for the given
  // detector number and run number.
  if ((detNo >= RUNMIN && detNo <= RUNMAX) ||
      (detNo >= RUNMIN2 && detNo <= RUNMAX2) ||
      (detNo >= RUNMIN3 && detNo <= RUNMAX3)) {
    char afile[500];
    snprintf(afile, 500, "%s/tmpfile.tmp", basedirname);
    FILE *ftmp = fopen(afile, "w");
    if (ftmp == NULL) {
      std::cout << afile << " can not be created. Probablly the directory '"
                << basedirname << "' does not exit. Exiting..." << std::endl;
      exit(-12);
    }
    fclose(ftmp);
    if (detNo < 100)
      snprintf(command, 1000, "cd %s\nls -d S%03d-%02d*/ > %s", basedatapath,
               detNo, runNo, afile);
    else if (detNo < 10000)
      snprintf(command, 1000, "cd %s\nls -d S%04d-%02d*/ > %s", basedatapath,
               detNo, runNo, afile);
    else
      snprintf(command, 1000, "cd %s\nls -d S%06d-%02d*/ > %s", basedatapath,
               detNo, runNo, afile);

    int tst = system(command);
    std::cout << "___________________________________________________\n";
    // std::cout << command << std::endl << "returned: " << tst << std::endl;
    if (tst == 0) {
      std::cout << "Folder for detector S" << detNo << " run " << runNo
                << " found in directory: \n"
                << basedatapath << "\n"
                << std::endl;
    } else if (tst != 0) {
      std::cout << command << std::endl << "returned: " << tst << std::endl;
      std::cout << "Probably run No " << detNo << " was not found in directory "
                << basedirname << std::endl
                << "Exiting..." << std::endl;
      return tst;
    }

    // Read the folder name from the temporary file and extract the information
    // about the run type, mesh and drift voltages, mylar thickness and drift
    // gap from the folder name.
    ftmp = fopen(afile, "r");
    if (fgets(fnametmp, 200, ftmp) == NULL) {
      std::cout << "Failed to read the filename for run " << detNo << " at "
                << basedirname << std::endl
                << "Exiting..." << std::endl;
      return -2;
    }
    // Write the data to the log file
    int rtmp, dtmp;
    char ftypetmp[500];
    strcpy(ftypetmp, "");

    replaceEOL(
        fnametmp); // Remove the end of line character from the folder name
    // std::cout << "Det No = " << detNo << std::endl; // Print the detector
    // number
    std::cout << "Folder name = |" << fnametmp << "|"
              << std::endl; // Print the folder name
    int stst =
        -3434; // Initialize the variable that will store the number of
               // arguments read from the folder name, -3434 is a random number.

    // The folder name can have different formats depending on the detector
    // number. For example, for detector numbers less than 100, the folder name
    // can be in the format SXXX-YY-Vm-Vd-filetype, where XXX is the
    // detector number. For detector numbers greater than or equal to 10000, the
    // folder name can be in the format SXXXXX-YY-Vm-Vd-filetype,
    // where XXXXX is the detector number. For detector numbers between 100 and
    // 9999, the folder name can be in either of these formats. Therefore, we
    // need to check the detector number and use the appropriate format to
    // extract the information from the folder name.
    if (detNo < RUNMAX)
      stst = sscanf(fnametmp, "%c%03d-%02d-%3d-%3d%30[^ /,\n\t]", &dtype, &dtmp,
                    &rtmp, &vm, &vd, ftypetmp);
    else if (detNo >= 10000)
      stst = sscanf(fnametmp, "%c%06d-%02d-%3d-%3d%30[^ /,\n\t]", &dtype, &dtmp,
                    &rtmp, &vm, &vd, ftypetmp);
    else if (detNo >= RUNMAX) {
      // std::cout << "WE are here " << std::endl;
      stst = sscanf(fnametmp, "%c%4d-%02d-%3d-%3d%30[^ /,\n\t]", &dtype, &dtmp,
                    &rtmp, &vm, &vd, ftypetmp);
    }

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
    if (stst > 5)
      std::cout << "filetype = " << filetype << std::endl;
    fclose(ftmp);

    snprintf(command, 1000, "rm %s\n", afile);
    tst = system(command);
  } else {
    std::cout << "Available runs: " << RUNMIN << " - " << RUNMAX << std::endl;
    return (-2);
  }

  const char *ftype = filetype.c_str(); /// add here any directory supplement
  char mesh[100];
  snprintf(mesh, 100, "%d", vm);
  char drift[100];
  snprintf(drift, 100, "%d", vd);
  int drft;
  sscanf(drift, "%d", &drft);

  const int MaxFiles = 100000;
  const int MAXSEG = 10000;

  std::cout << "___________________________________________________"
               "\n\nStarting the .trc to "
               "root tree conversion ..."
            << std::endl;
  int runb = 0;
  snprintf(dirname, 1000, "%s/S%03d-%02d-%d-%d", basedatapath, detNo, runNo, vm,
           vd);
  if (detNo > 100)
    snprintf(dirname, 1000, "%s/S%04d-%02d-%d-%d", basedatapath, detNo, runNo,
             vm, vd);
  if (detNo > 10000)
    snprintf(dirname, 1000, "%s/S%06d-%02d-%d-%d", basedatapath, detNo, runNo,
             vm, vd);

  if (runb)
    snprintf(dirname, 1000, "%sb", dirname);
  if (strlen(ftype) > 1)
    snprintf(dirname, 1000, "%s%s", dirname, ftype);

  std::cout << "\nSearching for files at directory: " << dirname << std::endl
            << std::endl;

  /// create an ascii file (files.txt) that will contain all the filenames that
  /// will be analysed. make sure that the file does not exist by over-writing
  /// and deleting it.
  for (int i = 0; i < 4; i++) {
    snprintf(fname[i], 1000, "%s/files%d.txt", dirname, i + 1);
    std::cout << "    Creating file list: |" << fname[i] << "|" << std::endl;
    FILE *ftmp = fopen(fname[i], "w");
    if (ftmp == NULL) {
      std::cout << "Failed to write in directory " << dirname << std::endl
                << "Exiting..." << std::endl;
      return -5;
    }
    fclose(ftmp);
    snprintf(command, 1000, "rm %s\n", fname[i]);
    int tst = system(command);
    snprintf(command, 1000, "ls %s/C%d*?????.trc > %s\n", dirname, i + 1,
             fname[i]);
    snprintf(command, 1000, "ls %s/ | grep C%d | grep .trc > %s\n", dirname,
             i + 1, fname[i]);
    //       std::cout << command<<std::endl;
    tst = system(command);
  }

  std::cout << "\n    File lists have been created!\n" << std::endl;

  // Print the number fo files found for each channel
  vector<string> fnames[4];
  int nfiles[4] = {0, 0, 0, 0};
  int nfiles2 = 0;
  FILE *infile[4];

  for (int i = 0; i < 4; i++)
    infile[i] = fopen(fname[i], "r");
  //   FILE *infile2=fopen(fname2,"r");
  int Ndetectors = 0;

  while (nfiles2 < MaxFiles) {
    int tst = 0;
    /// start processing the data files.
    for (int i = 0; i < 4; i++) {
      if (fgets(fnametmp, 200, infile[i]) == NULL)
        continue;
      //        std::cout<<"pulse 1 = #"<<fnametmp<<std::endl;
      if (strlen(fnametmp) > 1) {
        //         std::cout<<"pulse 1 = "<<fnames[nfiles]<<" - pulse 2 =
        //         "<<fnames2[nfiles]<<std::endl;
        replaceEOL(fnametmp);
        fnames[i].push_back(fnametmp);
        nfiles[i]++;
        tst++;
      }
    }
    if (tst > 0)
      nfiles2++;
    else
      break;
  }
  //     std::cout <<std::endl;
  for (int i = 0; i < 4; i++) {
    if (infile[i])
      fclose(infile[i]);
    if (nfiles[i] > 0) {
      std::cout << "C" << i + 1 << " : found " << nfiles[i] << " files"
                << std::endl;
      Ndetectors++;
      active[i] = 1;
    } else {
      std::cout << "C" << i + 1 << " : No files found at " << dirname
                << std::endl;
      active[i] = 0;
    }
  }

  if (Ndetectors <= 0) {
    std::cout << "No binary (*.trc) files found in the directory " << dirname
              << " for any of the channels.\n Exiting..." << std::endl;
    return (-4);
  }

  /// Alignement of filenames (this will skip events when a file of one channel
  /// is missing, without missaligning the events)
  char namebase[1000];
  int maxf[] = {0, 0, 0, 0};
  int minf[] = {0, 0, 0, 0};
  for (int ci = 0; ci < 4; ci++) {
    if (!active[ci])
      continue;

    strcpy(fnametmp, fnames[ci][0].c_str());

    if (strlen(fnametmp) > 0)
      snprintf(namebase, 1000, "%s", &fnametmp[2]);
    namebase[strlen(namebase) - 9] = '\0';
  }

  std::cout << "\nnamebase = " << namebase << std::endl;
  for (int ci = 0; ci < 4; ci++) {
    if (!active[ci]) {
      maxf[ci] = -1;
      minf[ci] = 99999999;
      continue;
    }
    strcpy(fnametmp, fnames[ci][0].c_str());
    snprintf(command, 1000, "C%d%s%%05d.trc", ci + 1, namebase);
    //      std::cout<<command<<std::endl;
    sscanf(fnametmp, command, &minf[ci]);
    std::cout << "minf(" << ci << ") = " << minf[ci] << "\t\t";
    strcpy(fnametmp, fnames[ci][nfiles[ci] - 1].c_str());
    snprintf(command, 1000, "C%d%s%%05d.trc", ci + 1, namebase);
    //      std::cout<<command<<std::endl;
    sscanf(fnametmp, command, &maxf[ci]);
    std::cout << "maxf(" << ci << ") = " << maxf[ci] << std::endl;
  }

  int fnamemin = TMath::MinElement(4, minf);
  int fnamemax = TMath::MaxElement(4, maxf);

  nfiles2 = fnamemax - fnamemin + 1; /// correction for possibly missing files

  if (Ndetectors <= 0)
    return (-1);

  // Create a root file and a tree to store the data.
  // The tree will have branches for the event number, time, and the amplitude
  // of the signal for each channel.
  int maxFile = 100000;
  int offSet[4] = {0, 0, 0, 0};
  // double *amplC[4];
  std::vector<std::vector<double>> amplC(4);
  for (int i = 0; i < 4; i++) {
    if (active[i])
      // amplC[i] = new double[5000000];
      // amplC[i] = new double[25000003];
      amplC[i].reserve(MAXVECSIZE);
  }

  int evNo = 0;
  int spoints[] = {0, 0, 0, 0};
  int tchan = -1;
  FILE *file;
  uint32_t number = 0;
  char date[40];
  unsigned long long int epoch;
  unsigned long long int nn;
  double dt, t1;
  vector<double> w1, w2, w3, w4;

  snprintf(ofname, 1000, "%s/S%03d-%02d-%d-%d", basedirname, detNo, runNo, vm,
           vd);
  if (detNo > 100)
    snprintf(ofname, 1000, "%s/S%04d-%02d-%d-%d", basedirname, detNo, runNo, vm,
             vd);
  if (detNo > 10000)
    snprintf(ofname, 1000, "%s/S%06d-%02d-%d-%d", basedirname, detNo, runNo, vm,
             vd);
  if (runb)
    snprintf(basedirname, 1000, "%sb", basedirname);
  if (strlen(ftype) > 1)
    snprintf(basedirname, 1000, "%s%s", basedirname, ftype);

  snprintf(ofname, 1000, "%s_%s_tree.root", ofname, rtype);
  std::cout << "\n\nCreating root file " << ofname << std::endl << std::endl;

  TFile *ofile = new TFile(ofname, "RECREATE");

  double t0[] = {0., 0., 0., 0.};
  double gain[] = {0., 0., 0., 0.};
  double offset[] = {0., 0., 0., 0.};
  double rmax[] = {0., 0., 0., 0.};
  double rmin[] = {0., 0., 0., 0.};

  char treename[200];
  snprintf(treename, 200, "TreeWithRawData");
  char treetitle[200];
  snprintf(treetitle, 200, "Raw data tree");

  TTree tree(treename, treetitle);
  int npoints;
  tree.Branch("npoints", &npoints, "npoints/I");
  tree.Branch("eventNo", &evNo, "eventNo/I");
  //     tree.Branch("t1", &t1, "t1/D");
  //     tree.Branch("t2", &t2, "t2/D");
  //     tree.Branch("t3", &t3, "t3/D");
  tree.Branch("t0", t0, "t[4]/D");
  tree.Branch("gain", gain, "gain[4]/D");
  tree.Branch("offset", offset, "offset[4]/D");
  tree.Branch("rmax", rmax, "rmax[4]/D");
  tree.Branch("rmin", rmin, "rmin[4]/D");

  tree.Branch("dt", &dt, "dt/D");
  tree.Branch("epoch", &epoch, "epoch/l");
  tree.Branch("nn", &nn, "nn/l");
  tree.Branch("date", &date, "date/C");

  //     tree.Branch("c1ampl", c1, "c1ampl[npoints]/D");
  //     tree.Branch("c2ampl", c2, "c2ampl[npoints]/D");
  //     tree.Branch("c3ampl", c3, "c3ampl[npoints]/D");

  for (int i = 0; i < 4; i++) {

    if (!active[i])
      continue;

    TString channel = TString::Itoa(i + 1, 10);

    //    tree.Branch("npoints", &spoints2, "npoints/I");
    TString bname = "npoints" + channel;
    TString btype = bname + "/I";
    tree.Branch(bname, &spoints[i], btype);
    //    std::cout<<bname<<" --- "<<btype <<std::endl;

    bname = "amplC" + channel;
    btype = bname + "[npoints" + channel + "]/D";
    tree.Branch(bname, &amplC[i][0], btype);
    //     std::cout<<bname<<" --- "<<btype <<std::endl;
  }
  std::cout << "Start data processing...\n" << std::endl;

  // loop over the number of the files and fill the tree
  for (int ev = fnamemin; ev <= fnamemax; ev++) {
    int skipfile = 0;
    FILE *inf[4];
    for (int ci = 0; ci < 0; ci++) {

      if (!active[ci])
        continue;

      snprintf(fnametmp, 1000, "%s/C%d%s%05d.trc", dirname, ci + 1, namebase,
               ev);

      inf[ci] = fopen(fnametmp, "r");
      // std::cout << fnametmp << std::endl;
      if (!(inf[ci])) { // file couldn't be opened
        cerr << "Error: " << fnametmp
             << " file could not be opened. Skipping files # " << ev << " !!!"
             << std::endl;
        skipfile = 1;
        break; /// this is for ci loop
      } else
        fclose(inf[ci]);
    }
    if (skipfile)
      continue;

    char fileName[1000];

    wavedesc desc1;
    vector<double> triggerTime1;
    vector<int> wave1;
    if (active[1 - 1]) {
      snprintf(fileName, 1000, "%s/C%d%s%05d.trc", dirname, 1, namebase, ev);
      // fprintf(stderr, "file: %s\n", fileName);
      if ((file = fopen(fileName, "r")) == NULL) {
        std::cout << "can not open file " << fileName << std::endl;
        continue; // return 1;
      }

      int tmp = readLeCroyBinary(file, &desc1, &triggerTime1, &wave1);
      if (tmp) {
        cerr << "Unknown format (Ch2): " << tmp << std::endl << std::endl;
        return 1;
      }
      // #################################//
      // printWavedesc(desc1);
      // std::cin.get();
      // #################################//
      gain[0] = desc1.vertical_gain;
      offset[0] = desc1.vertical_offset;
      rmax[0] = desc1.max_value * gain[0] - offset[0];
      rmin[0] = desc1.min_value * gain[0] - offset[0];
      fclose(file);
    }

    wavedesc desc2;
    vector<double> triggerTime2;
    vector<int> wave2;
    if (active[2 - 1]) {
      snprintf(fileName, 1000, "%s/C%d%s%05d.trc", dirname, 2, namebase, ev);
      // 	  fprintf(stderr, "file: %s\n", fileName);
      if ((file = fopen(fileName, "r")) == NULL) {
        std::cout << "can not open file " << fileName << std::endl;
        continue; // return 1;
      }
      int tmp = readLeCroyBinary(file, &desc2, &triggerTime2, &wave2);
      fclose(file);

      if (tmp) {
        cerr << "Unknown format (Ch2): " << tmp << std::endl;
        continue;
      }
      // #################################//
      // printWavedesc(desc2);
      // std::cin.get();
      // #################################//
      gain[1] = desc2.vertical_gain;
      offset[1] = desc2.vertical_offset;
      rmax[1] = desc2.max_value * gain[1] - offset[1];
      rmin[1] = desc2.min_value * gain[1] - offset[1];
      fclose(file);
    }

    wavedesc desc3;
    vector<double> triggerTime3;
    vector<int> wave3;
    if (active[3 - 1]) {
      snprintf(fileName, 1000, "%s/C%d%s%05d.trc", dirname, 3, namebase, ev);
      // fprintf(stderr, "file: %s\n", fileName);
      if ((file = fopen(fileName, "r")) == NULL) {
        std::cout << "can not open file " << fileName << std::endl;
        continue; // return 1;
      }
      int tmp = readLeCroyBinary(file, &desc3, &triggerTime3, &wave3);
      if (tmp) {
        cerr << "Unknown format (Ch3): " << tmp << std::endl << std::endl;
        return 1;
      }
      // #################################//
      // printWavedesc(desc3);
      // std::cin.get();
      // #################################//
      gain[2] = desc3.vertical_gain;
      offset[2] = desc3.vertical_offset;
      // 	  std::cout << "MAX =
      // "<<desc3.max_value*gain[2]-offset[2]<<std::endl; 	  std::cout <<
      // "MIN =
      // "<<desc3.min_value*gain[2]-offset[2]<<std::endl; 	  std::cout <<
      // "FULL RANGE = "<<(desc3.max_value)*gain[2]-offset[2] -
      // (desc3.min_value)*gain[2]-offset[2] <<" =
      // "<<(desc3.max_value*gain[2]-offset[2] -
      // desc3.min_value*gain[2]-offset[2])/8 <<" per div"<<std::endl;
      rmax[2] = desc3.max_value * gain[2] - offset[2];
      rmin[2] = desc3.min_value * gain[2] - offset[2];
      fclose(file);
    }

    wavedesc desc4;
    vector<double> triggerTime4;
    vector<int> wave4;
    if (active[4 - 1]) {
      snprintf(fileName, 1000, "%s/C%d%s%05d.trc", dirname, 4, namebase, ev);
      // fprintf(stderr, "file: %s\n", fileName);
      if ((file = fopen(fileName, "r")) == NULL) {
        std::cout << "can not open file " << fileName << std::endl;
        continue; // return 1;
      }

      int tmp = readLeCroyBinary(file, &desc4, &triggerTime4, &wave4);

      if (tmp) {
        cerr << "Unknown format (Ch4): " << tmp << std::endl << std::endl;
        return 1;
      }
      // #################################//
      // printWavedesc(desc4);
      // std::cin.get();
      // #################################//
      gain[3] = desc4.vertical_gain;
      offset[3] = desc4.vertical_offset;
      rmax[3] = desc4.max_value * gain[3] - offset[3];
      rmin[3] = desc4.min_value * gain[3] - offset[3];
      fclose(file);
    }

    //       1 with 2
    if (active[1 - 1] && active[2 - 1])
      if ((int)desc1.trigger_time_Y != (int)desc2.trigger_time_Y ||
          (int)desc1.trigger_time_M != (int)desc2.trigger_time_M ||
          (int)desc1.trigger_time_D != (int)desc2.trigger_time_D ||
          (int)desc1.trigger_time_h != (int)desc2.trigger_time_h ||
          (int)desc1.trigger_time_m != (int)desc2.trigger_time_m ||
          fabs(desc1.trigger_time_s - desc2.trigger_time_s) > 1.e-3) {
        cerr << "Not compatible files 1" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       1 with 3
    if (active[1 - 1] && active[3 - 1])
      if ((int)desc1.trigger_time_Y != (int)desc3.trigger_time_Y ||
          (int)desc1.trigger_time_M != (int)desc3.trigger_time_M ||
          (int)desc1.trigger_time_D != (int)desc3.trigger_time_D ||
          (int)desc1.trigger_time_h != (int)desc3.trigger_time_h ||
          (int)desc1.trigger_time_m != (int)desc3.trigger_time_m ||
          fabs(desc1.trigger_time_s - desc3.trigger_time_s) > 1.e-3) {
        cerr << "Not compatible files 2" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       1 with 4
    if (active[1 - 1] && active[4 - 1])
      if ((int)desc1.trigger_time_Y != (int)desc4.trigger_time_Y ||
          (int)desc1.trigger_time_M != (int)desc4.trigger_time_M ||
          (int)desc1.trigger_time_D != (int)desc4.trigger_time_D ||
          (int)desc1.trigger_time_h != (int)desc4.trigger_time_h ||
          (int)desc1.trigger_time_m != (int)desc4.trigger_time_m ||
          fabs(desc1.trigger_time_s - desc4.trigger_time_s) > 1.e-3) {
        cerr << "Not compatible files 3" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       2 with 3
    if (active[2 - 1] && active[3 - 1])
      if ((int)desc3.trigger_time_Y != (int)desc2.trigger_time_Y ||
          (int)desc3.trigger_time_M != (int)desc2.trigger_time_M ||
          (int)desc3.trigger_time_D != (int)desc2.trigger_time_D ||
          (int)desc3.trigger_time_h != (int)desc2.trigger_time_h ||
          (int)desc3.trigger_time_m != (int)desc2.trigger_time_m ||
          fabs(desc3.trigger_time_s - desc2.trigger_time_s) > 1.e-3) {
        cerr << "Not compatible files 2 3" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       1 with 2
    if (active[1 - 1] && active[2 - 1])
      if (desc1.wave_array_count != desc2.wave_array_count ||
          desc1.subarray_count != desc2.subarray_count ||
          desc1.horizontal_interval != desc2.horizontal_interval) {
        cerr << "Not compatible files 4" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       2 with 3
    if (active[2 - 1] && active[3 - 1])
      if (desc2.wave_array_count != desc3.wave_array_count ||
          desc2.subarray_count != desc3.subarray_count ||
          desc2.horizontal_interval != desc3.horizontal_interval) {
        cerr << "Not compatible files 5" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       1 with 4
    if (active[1 - 1] && active[4 - 1])
      if (desc1.wave_array_count != desc4.wave_array_count ||
          desc1.subarray_count != desc4.subarray_count ||
          desc1.horizontal_interval != desc4.horizontal_interval) {
        cerr << "Not compatible files 6" << std::endl << std::endl;
        continue;
        return 1;
      }

    //       fstream fout;
    //       fout.open("run1.dat",ios::out);

    int II = 0;
    if (active[0]) {
      II = desc1.subarray_count; /// it is the number of events per file! ==
                                 /// SEGMENTS. Use the first active channel to
                                 /// determine!
      tchan = 0;
    } else if (active[1]) {
      II = desc2.subarray_count;
      tchan = 1;
    } else if (active[2]) {
      II = desc3.subarray_count;
      tchan = 2;
    } else if (active[3]) {
      II = desc4.subarray_count;
      tchan = 3;
    } else {
      std::cout << "No active channel found!" << std::endl;
      return (-5);
    }
    // 	std::cout <<"file "<<ev<<  "event"<<evNo<< "II = "<<II << std::endl;

    for (int ii = 0; ii < II; ii++) {
      int chk = 0;
      int bit = 0;
      number = 0;

      if (II > 1) {
        if (active[0])
          t0[0] = triggerTime1[ii];
        if (active[1])
          t0[1] = triggerTime2[ii];
        if (active[2])
          t0[2] = triggerTime3[ii];
        if (active[3])
          t0[3] = triggerTime4[ii];
      } else
        for (int ci = 0; ci < 4; ci++)
          t0[ci] = -1;

      double intpart;
      double sec;

      switch (tchan) {
      case 0:
        snprintf(date, 40, "%04d/%02d/%02d_%02d:%02d:%02d",
                 (int)desc1.trigger_time_Y, (int)desc1.trigger_time_M,
                 (int)desc1.trigger_time_D, (int)desc1.trigger_time_h,
                 (int)desc1.trigger_time_m, (int)desc1.trigger_time_s);
        dt = desc1.horizontal_interval;
        sec = modf(desc1.trigger_time_s, &intpart) + desc1.horizontal_offset;
        break;
      case 1:
        snprintf(date, 40, "%04d/%02d/%02d_%02d:%02d:%02d",
                 (int)desc2.trigger_time_Y, (int)desc2.trigger_time_M,
                 (int)desc2.trigger_time_D, (int)desc2.trigger_time_h,
                 (int)desc2.trigger_time_m, (int)desc2.trigger_time_s);
        dt = desc2.horizontal_interval;
        sec = modf(desc2.trigger_time_s, &intpart) + desc2.horizontal_offset;
        break;
      case 2:
        snprintf(date, 40, "%04d/%02d/%02d_%02d:%02d:%02d",
                 (int)desc3.trigger_time_Y, (int)desc3.trigger_time_M,
                 (int)desc3.trigger_time_D, (int)desc3.trigger_time_h,
                 (int)desc3.trigger_time_m, (int)desc3.trigger_time_s);
        dt = desc3.horizontal_interval;
        sec = modf(desc3.trigger_time_s, &intpart) + desc3.horizontal_offset;
        break;
      case 3:
        snprintf(date, 40, "%04d/%02d/%02d_%02d:%02d:%02d",
                 (int)desc4.trigger_time_Y, (int)desc4.trigger_time_M,
                 (int)desc4.trigger_time_D, (int)desc4.trigger_time_h,
                 (int)desc4.trigger_time_m, (int)desc4.trigger_time_s);
        dt = desc4.horizontal_interval;
        sec = modf(desc4.trigger_time_s, &intpart) + desc4.horizontal_offset;
        break;
      default:
        return (-5);
      }

      date[19] = 0;
      struct tm tim;
      strptime(date, "%Y/%m/%d_%H:%M:%S", &tim);
      time_t tepoch = mktime(&tim);
      strptime(date, "%Y/%m/%d_%H:%M:%S", &tim);
      tepoch = mktime(&tim);

      //  	    std::cout<<tepoch << " + "<<sec;
      //  	    std::cout<<" adding "<<t0[1];
      sec += t0[tchan]; /// here we add the DT of the extra segments!!!!!! Use
                        /// the time of the first active segment
                        // // 	    std::cout<<" --> "<<tepoch;
                        // // 	    printf(" --> %5.5f\n",tepoch+sec);

      epoch = (unsigned long long int)tepoch;
      epoch =
          epoch - rootConv + unixConv; // to pass from root times to unix time
      // 	    nn = (unsigned long long int)number;
      nn = (unsigned long long int)(sec * 1e9); // to pass it to ns

      // 	    printf(" --> %5.5f ,  nn = %llu\n",tepoch+sec,nn);

      //  	   std::cout<<desc2.trigger_time_s<<"
      //  "<<desc2.horizontal_offset<<"
      //  "<<t0[0]<<"   "<<t0[1]<<std::endl;

      w1.clear();
      w2.clear();
      w3.clear();
      w4.clear();

      if (active[0]) {
        spoints[0] = desc1.wave_array_count / desc1.subarray_count;
        for (int kk = 0; kk < spoints[0]; kk++) {
          w1.push_back(wave1[kk + ii * spoints[0]] * desc1.vertical_gain -
                       desc1.vertical_offset);
          // amplC[0][kk] = w1[kk];
          amplC[0].push_back(w1[kk]);
        }
      }
      if (active[1]) {
        // std::cout << "WTF is going on 1!" << std::endl;
        spoints[1] = desc2.wave_array_count / desc2.subarray_count;
        w2.resize(spoints[1]);
        // ##########################################################################

        // std::cout << "spoints[1] = " << spoints[1]
        //           << " size of wave2: " << wave2.size() << std::endl;

        // std::cout << "spoints[1] = " << spoints[1]
        //           << " size of wave2: " << wave2.size() << std::endl;
        // std::cout << " desc2.wave_array_count = " << desc2.wave_array_count
        //           << " desc2.subarray_count = " << desc2.subarray_count
        //           << std::endl;

        // std::cin.get();
        // ##########################################################################
        for (int kk = 0; kk < spoints[1]; kk++) {
          size_t index = kk + (size_t)ii * spoints[1]; // Cast to size_t
          w2[kk] = wave2[index] * desc2.vertical_gain -
                   desc2.vertical_offset; // Direct assignment

          // for (int kk = 0; kk < spoints[1]; kk += 10) {
          //   size_t index = kk + (size_t)ii * spoints[1]; // Cast to size_t
          //   w2[kk] = wave2[index] * desc2.vertical_gain -
          //            desc2.vertical_offset; // Direct assignment

          // // In data processing loop, decimate:
          // for (int kk = 0; kk < spoints[1]; kk += 10) { // Every 10th point
          //   amplC[1][kk / 10] =
          //       wave2[kk] * desc2.vertical_gain - desc2.vertical_offset;
          // }

          // ##########################################################################

          // amplC[1][kk] = w2[kk];
          amplC[1].push_back(w2[kk]);

          // if (kk % 1000000 == 0) {
          //   std::cout << "Processed " << kk << "/" << spoints[1] <<
          //   std::endl;
          // }

          // if (kk == spoints[1] - 1) {
          //   std::cout << std::endl;
          //   std::cout << "kk ii spoints[1] : " << kk << "  " << ii << "  "
          //             << spoints[1] << " : wave2[" << kk + ii * spoints[1]
          //             << "] = " << wave2[kk + ii * spoints[1]]
          //             << " gain = " << desc2.vertical_gain
          //             << " offset = " << desc2.vertical_offset << std::endl;
          // }

          // std::cout << "kk ii spoints[1] : " << kk << "  " << ii << "  "
          //                   << spoints[1] << " : wave2[" << kk + ii *
          //                   spoints[1]
          //                   << "] = " << wave2[kk + ii * spoints[1]]
          //                   << " gain = " << desc2.vertical_gain
          //                   << " offset = " << desc2.vertical_offset <<
          //                   std::endl;

          // // Debug every 1M points
          // if (kk % 1000000 == 0) {
          //   std::cout << "Processed " << kk << "/" << spoints[1] <<
          //   std::endl;
          // }
        }
        // ##########################################################################

        // for (int kk = 0; kk < spoints[1]; kk++) {

        //   size_t index = kk + ii * spoints[1];
        //   if (index >= wave2.size()) { // ← SAFETY CHECK
        //     std::cerr << "Index out of bounds: " << index << std::endl;
        //     break;
        //   }

        //   double value =
        //       wave2[index] * desc2.vertical_gain - desc2.vertical_offset;
        //   w2.push_back(value);
        //   amplC[1][kk] = value; // Copy to amplC[1]

        //   // w2.push_back(wave2[kk + ii * spoints[1]] * desc2.vertical_gain
        //   -
        //   //              desc2.vertical_offset);

        //   std::cout << "kk ii spoints[1] : " << kk << "  " << ii << "  "
        //             << spoints[1] << " : wave2[" << kk + ii * spoints[1]
        //             << "] = " << wave2[kk + ii * spoints[1]]
        //             << " gain = " << desc2.vertical_gain
        //             << " offset = " << desc2.vertical_offset << std::endl;

        //   amplC[1][kk] = w2[kk];
        // }
        // ##########################################################################
      }
      if (active[2]) {
        spoints[2] = desc3.wave_array_count / desc3.subarray_count;
        // std::cout << "spoints[2] = " << spoints[2]
        //           << " size of wave3: " << wave3.size() << std::endl;
        // std::cout << " desc3.wave_array_count = " << desc3.wave_array_count
        //           << " desc3.subarray_count = " << desc3.subarray_count
        //           << std::endl;

        for (int kk = 0; kk < spoints[2]; kk++) {
          w3.push_back(wave3[kk + ii * spoints[2]] * desc3.vertical_gain -
                       desc3.vertical_offset);
          // std::cout << "kk ii spoints[2] : " << kk << "  " << ii << "  "
          //           << spoints[2] << " : wave3[" << kk + ii * spoints[2]
          //           << "] = " << wave3[kk + ii * spoints[2]]
          //           << " gain = " << desc3.vertical_gain
          //           << " offset = " << desc3.vertical_offset << std::endl;
          // 		std::cout<<wave3[kk + ii*spoints[2]]<<"  ";
          // amplC[2][kk] = w3[kk];
          amplC[2].push_back(w3[kk]);
        }
        // 	      std::cout<<std::endl<<std::endl;
      }
      if (active[3]) {
        spoints[3] = desc4.wave_array_count / desc4.subarray_count;
        // std::cout << "spoints[3] = " << spoints[3]
        //           << " size of wave4: " << wave4.size() << std::endl;
        // std::cout << " desc4.wave_array_count = " << desc4.wave_array_count
        //           << " desc4.subarray_count = " << desc4.subarray_count
        //           << std::endl;
        for (int kk = 0; kk < spoints[3]; kk++) {
          w4.push_back(wave4[kk + ii * spoints[3]] * desc4.vertical_gain -
                       desc4.vertical_offset);
          // std::cout << "kk ii spoints[3] : " << kk << "  " << ii << "  "
          //           << spoints[3] << " : wave4[" << kk + ii * spoints[3]
          //           << "] = " << wave4[kk + ii * spoints[3]]
          //           << " gain = " << desc4.vertical_gain
          //           << " offset = " << desc4.vertical_offset << std::endl;
          // amplC[3][kk] = w4[kk];
          amplC[3].push_back(w4[kk]);
        }
      }
      npoints = spoints[tchan];
      // 	    spoints[0] = desc1.wave_array_count/desc1.subarray_count;
      // 	    spoints[1] = desc2.wave_array_count/desc2.subarray_count;
      // 	    spoints[2] = desc3.wave_array_count/desc3.subarray_count;
      // 	    spoints[3] = desc4.wave_array_count/desc4.subarray_count;

      // Print the file and event information based on the value of fnamemax and
      // the current event number (ev) and event count (evNo)
      if ((fnamemax <= 10 && ii == II - 1) ||
          ((fnamemax > 10 && fnamemax <= 100) &&
           (ev % 10 == 0 || ev == fnamemax) && ii == II - 1) ||
          ((fnamemax > 100 && fnamemax <= 1000) &&
           (ev % 100 == 0 || ev == fnamemax) && ii == II - 1)) {
        std::cout << "File:" << setw(4) << ev << "  event:" << setw(5) << evNo
                  << " frame: " << setw(3) << ii + 1 << "/" << II << "  "
                  << " nsamples = " << npoints << "  " << date
                  << " , epoch = " << epoch << " , nn = " << nn
                  << " , sec = " << sec << " , t0 = " << t0[tchan] << tchan
                  << std::endl;
        for (int j = 0; j < 4; j++) {
          if (active[j]) {
            // std::cout <<"Channel C" <<j+1<< " : R_max = "<<rmax[j]<<" \t"  <<
            // " R_min = "<<rmin[j]<<std::endl; std::cout << "\tFULL RANGE =
            // "<<rmax[j]-rmin[j] <<" V  ==>  "<<(rmax[j]-rmin[j])/8*1000
            // <<"mV / div"<<std::endl;
          }
        }
      }
      if (evNo == 10) {
        //            tree.OptimizeBaskets(100000000,1.1,"d" );
        tree.OptimizeBaskets(100000000, 1.1, "");
      }
      tree.Fill();
      evNo++;

      // //             std::cout << "\n\n";
      // fprintf(stdout, "%s %.3lf %llu\n", date, epoch+t1, nn);
    } // for ii, events per file
    //       fout.close();
  } // end of loop over the number of the files and fill the tree

  int tstt;
  if (evNo < 50000) {
    //       tree.OptimizeBaskets(100000000,1.1,"d" );
    tree.OptimizeBaskets(100000000, 1.1, "");
    ofile->Write("", TObject::kOverwrite);
    //       std::cout<<"type a number to exit!!!"<<std::endl;
    // //       cin>>tstt;
  } else if (evNo >= 50000) {
    ofile->Write("", TObject::kOverwrite);
  }
  cerr << flush << "\rDone\n";

  // Call the launcher scripts to run MakeTreefromRawTreeProduction.C
  snprintf(command, 1000, "cd %s\n ./makeTree %d %d \n", WORKDIR, detNo, runNo);
  if (detNo >= 10000) {
    snprintf(command, 1000, "cd %s\n ./makeTreeMulti %d %d 1 \n", WORKDIR,
             detNo, runNo);
    std::cout << "Executing " << command << std::endl;
    tstt = system(command);
    std::cout
        << "Creation of processed data tree 1 has started! Will sleep for 5 "
           "sec before continuing!"
        << std::endl;
    for (int i = 0; i < 5; i++) {
      std::cout << "\rexit in " << 5 - i << "... " << flush;
      gSystem->Sleep(1000);
    }
    snprintf(command, 1000, "cd %s\n ./makeTreeMulti %d %d 2 \n", WORKDIR,
             detNo, runNo);
    std::cout << "Executing " << command << std::endl;
    tstt = system(command);
    std::cout
        << "Creation of processed data tree 2 has started! Will sleep for 5 "
           "sec before exiting!"
        << std::endl;
    for (int i = 0; i < 5; i++) {
      std::cout << "\rexit in " << 5 - i << "... " << flush;
      gSystem->Sleep(1000);
    }
    snprintf(command, 1000, "cd %s\n ./makeTreeMulti %d %d 3 \n", WORKDIR,
             detNo, runNo);
    std::cout << "Executing " << command << std::endl;
    tstt = system(command);
    std::cout
        << "Creation of processed data tree 3 has started! Will sleep for 5 "
           "sec before exiting!"
        << std::endl;
    for (int i = 0; i < 5; i++) {
      std::cout << "\rexit in " << 5 - i << "... " << flush;
      gSystem->Sleep(1000);
    }
  } else if (detNo >= RUNMAX) {
    snprintf(command, 1000, "cd %s\n ./makeTreeMulti %d %d 2 \n", WORKDIR,
             detNo, runNo);
    std::cout << "Executing " << command << std::endl;
    tstt = system(command);
    std::cout
        << "Creation of processed data tree 1 has started! Will sleep for 5 "
           "sec before continuing!"
        << std::endl;
    for (int i = 0; i < 5; i++) {
      std::cout << "\rexit in " << 5 - i << "... " << flush;
      gSystem->Sleep(1000);
    }
    snprintf(command, 1000, "cd %s\n ./makeTreeMulti %d %d 3 \n", WORKDIR,
             detNo, runNo);
    std::cout << "Executing " << command << std::endl;
    tstt = system(command);
    std::cout
        << "Creation of processed data tree 2 has started! Will sleep for 5 "
           "sec before exiting!"
        << std::endl;
    for (int i = 0; i < 5; i++) {
      std::cout << "\rexit in " << 5 - i << "... " << flush;
      gSystem->Sleep(1000);
    }
  } else {
    snprintf(command, 1000, "cd %s\n ./makeTree %d %d \n", WORKDIR, detNo,
             runNo);
    std::cout << "Executing " << command << std::endl;
    tstt = system(command);
    std::cout
        << "Creation of processed data tree has started! Will sleep for 10 "
           "sec before exiting!"
        << std::endl;
    for (int i = 0; i < 10; i++) {
      std::cout << "\rexit in " << 10 - i << "... " << flush;
      gSystem->Sleep(1000);
    }
  }

  for (int i = 0; i < 10; i++) {
    std::cout << "\rexit in " << 10 - i << "... " << flush;
    gSystem->Sleep(1000);
  }
  std::cout << "\nBy!" << std::endl;
  snprintf(command, 1000, "cd %s/Bin2Tree \n", WORKDIR);
  //     std::cout<<"Executing "<<command<<std::endl;
  return 0;
}

// Read LeCroy binary file and fill the wavedesc structure, trigger time vector,
// and wave vector
int readLeCroyBinary(FILE *file, wavedesc *desc, vector<double> *triggerTime,
                     vector<int> *wave) {
  wavedesc header;
  uint8_t tmp[9];
  do {
    if (fread(tmp, 8, 1, file) != 1)
      return 1;
    tmp[8] = 0;
    fseek(file, -7, SEEK_CUR);
  } while (strcmp((char *)tmp, "WAVEDESC") != 0);
  fseek(file, -1, SEEK_CUR);

  // BEGIN OF WAVEDESC BLOCK
  if (fread(&header, sizeof(wavedesc), 1, file) != 1)
    return 2;
  *desc = (wavedesc)header;
  // END OF WAVEDESC BLOCK

  // BEGIN OF USERTEXT BLOCK
  fseek(file, header.user_text, SEEK_CUR);
  // END OF USERTEXT BLOCK

  // BEGIN OF TRIGTIME ARRAY BLOCK
  for (int ii = 0; ii < header.subarray_count && header.subarray_count > 1;
       ii++) {
    double val0, val1;
    if (ffread(header.comm_order1, &val0, 8, file) != 1)
      return 3;
    if (ffread(header.comm_order1, &val1, 8, file) != 1)
      return 4;
    triggerTime->push_back(val0 - val1);
    // 	std::cout<<ii+1<<" triggers found"<<std::endl;
  }
  // END OF TRIGTIME ARRAY BLOCK

  // BEGIN OF RISTIME ARRAY BLOCK
  fseek(file, header.ris_time_array, SEEK_CUR);
  // END OF RISTIME ARRAY BLOCK

  // BEGIN OF DATA_ARRAY_1 ARRAY BLOCK
  for (int ii = 0; ii < header.wave_array_count; ii++) {
    int comm_type;
    header.comm_order1 == 1 ? comm_type = header.comm_type1
                            : comm_type = header.comm_type2;

    // #########################################################################//

    // std::cout << "comm_order1: " << (int)header.comm_order1
    //           << " comm_order2: " << (int)header.comm_order2
    //           << " comm_type1: " << (int)header.comm_type1
    //           << " comm_type2: " << (int)header.comm_type2 << std::endl;
    //   std::cout << "comm_type: " << comm_type << std::endl;
    // #########################################################################//
    switch (comm_type) {
    case 0:
      int8_t val0;
      if (ffread(header.comm_order1, &val0, comm_type + 1, file) != 1)
        return 5;
      // if(fread(&val0, comm_type + 1, 1, file) != 1) return 1;
      wave->push_back((int)val0);
      break;
    case 1:
      int16_t val1;
      // if (ffread((uint16_t)header.comm_order1, &val1, comm_type + 1, file)
      // != 1)
      if (ffread(header.comm_order1, &val1, comm_type + 1, file) != 1)
        return 6;
      // if(fread(&val1, comm_type + 1, 1, file) != 1) return 1;
      wave->push_back((int)val1);
      break;
      //   //
      //   ################################################################//
      // case 89: // ← ADD THIS LINE - your Ch2 files use 89 for 16-bit
      //   // 16-bit data
      //   int16_t val89;
      //   if (ffread((uint16_t)header.comm_order1, &val89, 2, file) != 1)
      //     return 6;
      //   wave->push_back((int)val89);
      //   break;
      // default:
      //   printf("Unknown comm_type=%d\n", comm_type);
      //   return 7;
      //   //
      //   ################################################################//
    }
  }
  // END OF DATA_ARRAY_1 ARRAY BLOCK

  // BEGIN OF DATA_ARRAY_2 ARRAY BLOCK
  fseek(file, header.wave_array_2, SEEK_CUR);
  // END OF DATA_ARRAY_2 ARRAY BLOCK

  return 0;
}

int ffread(unsigned short int byteOrder, void *val, unsigned int nByte,
           FILE *fileIn) {
  const uint16_t one = 1;
  uint8_t ii;
  uint8_t *cc;
  uint8_t *tmp;
  if (fread(val, nByte, 1, fileIn) != 1)
    return -1;
  if (*((uint8_t *)(&one)) == byteOrder)
    return 1;
  else {
    cc = (uint8_t *)val;
    tmp = (uint8_t *)malloc(nByte);
    for (ii = 0; ii < nByte; ii++) {
      tmp[ii] = cc[ii];
    }
    for (ii = 0; ii < nByte; ii++) {
      cc[ii] = tmp[nByte - ii - 1];
    }
    free(tmp);
    return 0;
  }
}
// Print the contents of a wavedesc structure to standard error for debugging purposes
void printWavedesc(wavedesc desc) {
  cerr << "descriptor_name\t" << desc.descriptor_name << "\n";
  cerr << "template_name\t" << desc.template_name << "\n";
  cerr << "comm_type1\t" << (int)desc.comm_type1 << "\n";
  cerr << "comm_type2\t" << (int)desc.comm_type2 << "\n";
  cerr << "comm_order1\t" << (int)desc.comm_order1 << "\n";
  cerr << "comm_order2\t" << (int)desc.comm_order2 << "\n";
  cerr << "wave_descriptor\t" << desc.wave_descriptor << "\n";
  cerr << "user_text\t" << desc.user_text << "\n";
  cerr << "res_desc1\t" << desc.res_desc1 << "\n";
  cerr << "trigtime_array\t" << desc.trigtime_array << "\n";
  cerr << "ris_time_array\t" << desc.ris_time_array << "\n";
  cerr << "res_array1\t" << desc.wave_array_1 << "\n";
  cerr << "wave_array_1\t" << desc.wave_array_1 << "\n";
  cerr << "wave_array_2\t" << desc.wave_array_2 << "\n";
  cerr << "res_array2\t" << desc.res_array2 << "\n";
  cerr << "res_array3\t" << desc.res_array3 << "\n";
  cerr << "instrument_name\t" << desc.instrument_name << "\n";
  cerr << "instrument_number\t" << desc.instrument_number << "\n";
  cerr << "trace_label\t" << desc.trace_label << "\n";
  cerr << "reserved1\t" << desc.reserved1 << "\n";
  cerr << "reserved2\t" << desc.reserved2 << "\n";
  cerr << "wave_array_count\t" << desc.wave_array_count << "\n";
  cerr << "pnts_per_screen\t" << desc.pnts_per_screen << "\n";
  cerr << "first_valid_pnt\t" << desc.first_valid_pnt << "\n";
  cerr << "last_valid_pnt\t" << desc.last_valid_pnt << "\n";
  cerr << "first_point\t" << desc.first_point << "\n";
  cerr << "sparsing_factor\t" << desc.sparsing_factor << "\n";
  cerr << "segment_index\t" << desc.segment_index << "\n";
  cerr << "subarray_count\t" << desc.subarray_count << "\n";
  cerr << "sweeps_per_acq\t" << desc.sweeps_per_acq << "\n";
  cerr << "points_per_pair\t" << desc.points_per_pair << "\n";
  cerr << "pair_offset\t" << desc.pair_offset << "\n";
  cerr << "vertical_gain\t" << desc.vertical_gain << "\n";
  cerr << "vertical_offset\t" << desc.vertical_offset << "\n";
  cerr << "max_value\t" << desc.max_value << "\n";
  cerr << "min_value\t" << desc.min_value << "\n";
  cerr << "nominal_bits\t" << desc.nominal_bits << "\n";
  cerr << "nom_subarray_count\t" << desc.nom_subarray_count << "\n";
  cerr << "horizontal_interval\t" << desc.horizontal_interval << "\n";
  cerr << "horizontal_offset\t" << desc.horizontal_offset << "\n";
  cerr << "pixel_offset\t" << desc.pixel_offset << "\n";
  cerr << "vertunit\t" << desc.vertunit << "\n";
  cerr << "horunit\t" << desc.horunit << "\n";
  cerr << "horiz_uncertainty\t" << desc.horiz_uncertainty << "\n";
  cerr << "trigger_time\t" << desc.trigger_time_Y << "/"
       << (int)desc.trigger_time_M << "/" << (int)desc.trigger_time_D << " "
       << (int)desc.trigger_time_h << ":" << (int)desc.trigger_time_m << ":"
       << desc.trigger_time_s << "\n";
  cerr << "trigger_time_r\t" << desc.trigger_time_r << "\n";
  cerr << "acq_duration\t" << desc.acq_duration << "\n";
  cerr << "record_type\t" << desc.record_type << "\n";
  cerr << "processing_done\t" << desc.processing_done << "\n";
  cerr << "reserved5\t" << desc.reserved5 << "\n";
  cerr << "ris_sweeps\t" << desc.ris_sweeps << "\n";
  cerr << "timebase\t" << desc.timebase << "\n";
  cerr << "vert_coupling\t" << desc.vert_coupling << "\n";
  cerr << "probe_att\t" << desc.probe_att << "\n";
  cerr << "fixed_vert_gain\t" << desc.fixed_vert_gain << "\n";
  cerr << "bandwidth_limit\t" << desc.bandwidth_limit << "\n";
  cerr << "vertical_vernier\t" << desc.vertical_vernier << "\n";
  cerr << "acq_vert_offset\t" << desc.acq_vert_offset << "\n";
  cerr << "wave_source\t" << desc.wave_source << "\n";
  cerr << "\n\n";
}

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
// void replaceEOLT(char *fnames)
// {
//     int nchar=0;
//     while (fnames[nchar]!='\n')
//     {
//       nchar++;
//     }
//     fnames[nchar]='\t';
// //     fnames[nchar+1]='\0';
//
// }

