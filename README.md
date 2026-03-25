####################################################################################################
####################################################################################################
# LECROY BINARY READER
####################################################################################################
####################################################################################################

A complete data processing pipeline for converting LeCroy oscilloscope binary files (`.trc`) 
into ROOT format for physics analysis. This software is designed for nuclear/particle physics 
experiments requiring synchronized multi-channel waveform analysis.

## Overview

This project provides a three-stage data processing pipeline:

1. **Binary Conversion** (`bin2tree`) - Converts proprietary LeCroy binary format to ROOT trees
2. **Raw Processing** (`makeTree`/`makeTreeMulti`) - Applies baseline correction and extracts 
     calibrated waveforms
3. **Physics Analysis** (`analyseTree`) - Extracts pulse parameters (amplitude, charge, timing) 
     and produces physics quantities

## Directory Structure

Lecroy_Binary_Reader/Bin2Tree/<br>
├── bin2tree.cxx # Main conversion program<br>
├── Makefile # Build configuration<br>
├── obj/ # Object files (created during build)<br>
├── dep/ # Dependency files (created during build)<br>
├── bin/ # Executables (created during build)<br>
├── PathNames.h # User configuration paths<br>
└── readLeCroyBinary.h # LeCroy binary format structure<br>


Lecroy_Binary_Reader/
├── makeTree.cxx # Launcher for single-channel processing<br>
├── makeTreeMulti.cxx # Launcher for multi-channel processing<br>
├── analyseTree.cxx # Launcher for physics analysis<br>
├── MakeTreefromRawTreeProduction.C # ROOT macro for raw processing<br>
├── MakeTreefromRawTreeProductionMulti.C # ROOT macro for multi-channel processing<br>
├── MakeTreefromRawTreeProductionDeb.C # Debug version of raw processing<br>
├── AnalyseTreeProduction.C # ROOT macro for physics analysis<br>
├── MyFunctions.C # Utility functions<br>
├── MyFunctions.h # Function declarations and constants<br>
├── Makefile # Build configuration<br>
├── obj/ # Object files (created during build)<br>
├── dep/ # Dependency files (created during build)<br>
└── bin/ # Executables (created during build)<br>

## Prerequisites
### Required Software
- **ROOT** (version 6.14 or later) - [Download](https://root.cern/install/)
- **C++ compiler** with C++11 support (g++ or clang)
- **make** build system

### Environment Setup
Ensure ROOT is properly installed and configured:

### Check ROOT installation
root-config --version

### Verify ROOT libraries
root-config --libs

## Directory Structure

### 1. Clone the Repository

	git clone https://github.com/AltingunAli/Lecroy_Binary_Reader.git
	cd Lecroy_Binary_Reader


### 2. Configure Paths

	Edit PathNames.h to set your data directories:


	// Raw data location (where .trc files are stored)
	const char *DATA_PATH_NAME = "/path/to/your/raw/data";

	// Output directory for raw ROOT trees
	const char *WORK_DIR_NAME = "/path/to/your/data/trees";

	// Output directory for processed trees
	const char *OUTDIRNAME = "/path/to/your/processed/trees";
	
	// Directory for plots
	const char *PLOTDIR = "/path/to/your/plots";

	// Base working directory (must be set correctly)
	#define WORKDIR "/path/to/Lecroy_Binary_Reader"

### 3. Build the Code

	# Clean previous builds (optional)
	make clean

	# Build all executables
	make

	# The executables will be created in the bin/ directory
	ls bin/

## Data Organization

	The code expects raw LeCroy binary files organized as:

	/path/to/raw/data/
	└── S001-01-500-200/           # S{detNo}-{runNo}-{Vm}-{Vd}
	    ├── C1trace00001.trc             # Channel 1, file 00001
	    ├── C1trace00002.trc             # Channel 1, file 00002
	    ├── C2trace00001.trc             # Channel 2, file 00001
	    ├── C2trace00002.trc             # Channel 2, file 00002
	    ├── C3trace00001.trc             # Channel 3, file 00001
	    └── C4trace00001.trc             # Channel 4, file 00001

## Naming Convention:

	S001 - Detector number (3-digit, can be 3, 4, or 6 digits)

	01 - Run number (2-digit)

	500 - Mesh voltage (Vm) in volts

	200 - Drift voltage (Vd) in volts

	C1 - Channel 1

	00001 - File sequence number (5-digit)

## Usage

	### Step 1: Convert Binary to Raw ROOT Tree

		cd bin

		# Basic usage
		./bin2tree <detNo> [<runNo>] [<inputFolder>] [<outputFolder>]

		# Examples
		./bin2tree 1           # Process detector 1, run 1 (default)
		./bin2tree 1 5         # Process detector 1, run 5
		./bin2tree 1 5 /custom/input /custom/output  # Custom paths


		What this does:

    			Searches for folder S001-05-* in the data directory

	    		Reads all .trc files for channels 1-4
	
    			Creates S001-05-{Vm}-{Vd}_PRODUCTIONraw_tree.root

	    		Automatically launches the next processing stage
 

	### Step 2: Process Raw Tree (Baseline Correction)

		This step is automatically triggered by bin2tree, but can be run manually:
		
			# Process all channels together
			./makeTree <detNo> <runNo>

			# Process specific channels (for large detector systems)
			./makeTreeMulti <detNo> <runNo> <channel>   # channel: 1, 2, or 3
		
		What this does:

    			Calculates baseline from waveform histogram

    			Subtracts baseline to isolate signal

    			Saves baseline-corrected waveforms in _tree.root

    			Generates diagnostic plots (baseline evolution, RMS evolution)


	### Step 3: Physics Analysis
		
		# Run analysis with default thresholds
		./analyseTree <detNo> <runNo>

		# With custom thresholds (threshold in mV, nTh in mV)
		# Modify AnalyseTreeProduction.C to change parameters

		What this does:
			Extracts pulse parameters from baseline-corrected waveforms:

        			Amplitude (mV)

        			Charge (arbitrary units)

        			Rise time (ns)

        			Pulse width (ns)

        			Time-over-threshold (ns)

			Generates physics plots (amplitude spectra, charge distributions, rate evolution)

			Calculates neutron/gamma discrimination based on thresholds

			Saves parameter tree and summary plots

## Output Files

	### Stage 1 Output (Raw Tree)

		/Data/dataTrees/
		└── S001-01-500-200_PRODUCTIONraw_tree.root
    		└── TreeWithRawData         # Raw waveforms with calibration

	### Stage 2 Output (Processed Tree)

		/Data/processedTrees/
		└── S001-01-500-200_PRODUCTION_tree.root
	  	  └── RawDataTree             # Baseline-corrected waveforms
        		├── eventNo             # Event number
        		├── amplSum             # Baseline-subtracted waveform
        		├── bslSum              # Baseline value (mV)
        		├── rmsSum              # Noise RMS (mV)
        		├── itrigger            # Trigger position (samples)
        		├── ttrig               # Trigger time (ns)
        		├── epoch               # Unix timestamp (seconds)
        		├── nn                  # Nanosecond part
        		└── fitstatus1          # Fit quality flag

	### Stage 3 Output (Physics Analysis)

		/Data/processedTrees/
		└── S001-01-500-200_PRODUCTION_<particle>_treeParam_aThXmV_nThXmV.root
    			└── ParameterTree           # Pulse parameters for each event

		/plots/S001/                   # Diagnostic and physics plots
		├── RateEvolution_*.png        # Rate over time
		├── Amplitude_*.png            # Amplitude spectrum
		├── Charge_*.png               # Charge distribution
		├── Summary_*.pdf              # Complete analysis summary
		└── Pedestals_*.png            # Baseline monitoring

## Configuration Parameters

	---------------------------------------------------------------------------
	PathNames.h
	---------------------------------------------------------------------------
	Parameter		Description

	DATA_PATH_NAME		Directory containing raw .trc files
	WORK_DIR_NAME		Directory for raw ROOT trees
	OUTDIRNAME		Directory for processed trees
	PLOTDIR			Directory for output plots
	RUN_TYPE		Data type identifier (e.g., "PRODUCTIONraw")
	RUNMIN/RUNMAX		Valid detector number ranges
	MAXVECSIZE		Maximum vector size for waveform storage
	FRAMESIZE		Maximum points per waveform

	---------------------------------------------------------------------------
	MakeTreefromRawTreeProduction.C
	---------------------------------------------------------------------------
	Parameter	Default		Description

	trigger		0		Channel with trigger signal (1-4)
	threshold	-200.0 mV	Amplitude threshold for pulse detection
	nTh		-300.0 mV	Neutron discrimination threshold
	inverse		1		Signal polarity (1 for positive, -1 for negative)

## Troubleshooting

	Common Issues

	### 1. "Folder for detector Sxxx run xx not found"

    		Check raw data path in PathNames.h

    		Verify folder naming: Sxxx-xx-* (e.g., S001-01-500-200)

	### 2. "No binary (*.trc) files found"

    		Ensure .trc files exist in the raw data folder

		    Check file naming: C1*.trc, C2*.trc, etc.

	### 3. "Failed to open root file"

		    Verify output directory exists and is writable

		    Check that Stage 1 completed successfully

	### 4. "Missing separator" error during make

		    Use the provided Makefile (tabs vs spaces issue fixed)

		    Run make clean and try again

	### 5. ROOT library not found

		    Ensure ROOT is installed: which root

		    Add ROOT to your PATH: source /path/to/root/bin/thisroot.sh

## Debug Mode

	For debugging, enable debug flags in MyFunctions.h:


		#define DEBUG 1          # Enable debug output
		#define DEBUGMSG 1       # Enable detailed message output

	Then rebuild:

	make clean
	make
	
## Example Workflow

	### 1. Set up paths (edit PathNames.h first)
	cd /path/to/Lecroy_Binary_Reader

	### 2. Build the code
	make

	### 3. Convert binary files to ROOT tree
	cd bin
	./bin2tree 1 1

	### 4. The raw processing will start automatically
	You'll see output like:
		Executing: ./makeTree 1 1
		Creation of processed data tree has started...

	### 5. Run analysis manually if needed
	./analyseTree 1 1

	### 6. View results
	Open root file:
	root -l /path/to/Data/processedTrees/S001-01-500-200_PRODUCTION_tree.root

	### View plots:
	ls /path/to/plots/S001/

## File Descriptions
	---------------------------------------------------------------------------
	Core Programs
	---------------------------------------------------------------------------


	File			Purpose

	bin2tree.cxx		Converts LeCroy .trc to raw ROOT tree
	makeTree.cxx		Launches single-channel raw processing
	makeTreeMulti.cxx	Launches multi-channel raw processing
	analyseTree.cxx		Launches physics analysis

	---------------------------------------------------------------------------
	ROOT Macros
	---------------------------------------------------------------------------

	File					Purpose

	MakeTreefromRawTreeProduction.C		Baseline subtraction for single channel
	MakeTreefromRawTreeProductionMulti.C	Baseline subtraction for multi-channel
	AnalyseTreeProduction.C			Pulse parameter extraction and physics analysis

	---------------------------------------------------------------------------
	Utility Files
	---------------------------------------------------------------------------

	File				Purpose
	MyFunctions.C/h			Common functions (smoothing, derivative, integration)
	readLeCroyBinary.h		LeCroy binary format structure
	PathNames.h			User configuration

## Contributing

	When modifying the code:

    		Test with a small dataset first

    		Enable debug mode to verify changes

    		Update this README if adding features

		Maintain backward compatibility with existing data

License
--

Contact
--

Acknowledgments

    Original code by T. Papaevangelou

    Modified by A.M. Altingun

    Based on LeCroy binary format specifications



This README provides comprehensive documentation for users to understand, install, and use Lecroy Binary Reader software. It covers:

1. **Project overview** - What the software does
2. **Directory structure** - Where everything goes
3. **Installation** - How to build and configure
4. **Data organization** - Expected input format
5. **Step-by-step usage** - How to run each stage
6. **Output description** - What files are produced
7. **Configuration** - How to customize parameters
8. **Troubleshooting** - Common issues and solutions
9. **Example workflow** - Complete usage example
10. **File descriptions** - Purpose of each file





