//	This macro was created by Rossi Fabiana on 2025-04-17
//	Last modified on 2025-04-18
/*
	
	How to run this macro:
		1) Convert the excel files in .csv files
		2) Make sure there is no text in the Current column (for example - OFF - in the 2025/02/03 file)
		3) Run for all files -> 
			for file in LINAC_2025_*.csv; do fname=$(basename "$file"); date_part=$(echo "$fname" | sed -E 's/LINAC_([0-9]{4})_([0-9]{2})_([0-9]{2}).csv/\1-\2-\3/'); rootfile="LINAC_${date_part}.root"; root -q "ImportLINAC_Current.C(\"$file\", \"$date_part\", \"$rootfile\")"; done
	**********************************************************************
*/
// #include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <ctype.h>
#include <list>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <sys/stat.h>
#include <time.h>
#include <TTimeStamp.h>
 
#include "TApplication.h"
#include "TCanvas.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TFile.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TMath.h"
#include "TH3.h"
#include "THStack.h"
#include "TTree.h"
#include "Varargs.h"
#include "strlcpy.h"
#include "TBuffer.h"
#include "TError.h"
#include "Bytes.h"
#include "TClass.h"
#include "TMD5.h"
#include "TVirtualMutex.h"
#include "ThreadLocalStorage.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "TEnv.h"
#include "TDirectory.h"
#include "TLine.h"
#include "TChain.h"
#include "TGraphErrors.h"
#include "TMatrixD.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TTreeIndex.h"

using namespace std;
using namespace TMath;
// namespace fs = filesystem;

const Bool_t PrintMemoON = 1; // = 1 to activate the print on screen

void ImportLINAC_Current(const char* filename, const char* date_str, const char* output_rootfile)
{
//	const TString MainDir = string(fs::current_path());
	gStyle->SetCanvasColor(0);
	gStyle->SetFrameBorderMode(0);
	gStyle->SetFrameBorderSize(0);
	gStyle->SetFrameFillStyle(0);
	gStyle->SetFrameFillColor(0);
	gStyle->SetCanvasColor(0);
	gStyle->SetCanvasBorderMode(0);
	gStyle->SetCanvasBorderSize(0);
	gStyle->SetPadColor(0);
	gStyle->SetPadBorderMode(0);
	gStyle->SetPadBorderSize(0);
	gStyle->SetTitleFillColor(0);
	gStyle->SetStatColor(0);
	gStyle->SetLineWidth(2);
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);//10111);
	gStyle->SetOptTitle(0);
	gStyle->SetPalette(1,0);
	gStyle->SetPaintTextFormat("g");

/*	if (PrintMemoON == 1)
	{
		cout << "**********************************************************************" << endl;
		cout << "1) Convert the .csv file from the LINAC in .root files for each day of the experiment" << endl;
		cout << "Output file: " << endl;
		cout << Form("  1) tData:  %s%s/Th%04.0f/[Run#].root", MainDir.Data(), RootDir.Data(), Ql_MIN) << endl << endl;
		cout << "**********************************************************************" << endl << endl;
	}*/

	// Open CSV file
	ifstream file(filename);
	if (!file.is_open())
	{
		cerr << "Error: Cannot open file " << filename << endl;
		return;
	}
	
	// Parse date string "YYYY-MM-DD"
	Int_t year, month, day;
	sscanf(date_str, "%d-%d-%d", &year, &month, &day);
	
	// Prepare ROOT file and tree
	TFile *fout = new TFile(output_rootfile, "RECREATE");
	TTree *tLINAC = new TTree("tLINAC", "DAQ Data with Timestamps");
//	tLINAC->SetScanField(0);
//	tLINAC->SetScanPrecision(15);
	
	Double_t current;
	TTimeStamp timestamp;
	Double_t timestamp_sec;
	char Time[20], Date[20];
	
	tLINAC->Branch("current", &current, "current/D");
	tLINAC->Branch("Time", Time, "Time/C");
	tLINAC->Branch("Date", Date, "Date/C");
	tLINAC->Branch("timestamp_sec", &timestamp_sec, "timestamp_sec/D");
	
	string line;
	Int_t line_count = 0;
	Int_t dataIndex = 0;
	
	while (getline(file, line))
	{
		++line_count;
	
		if (line_count < 18) continue; // Skip header rows
	
		stringstream ss(line);
		string cell;
		Int_t col = 0;
		current = 0;
	
		while (getline(ss, cell, ','))
		{
			if (col == 3)
			{ // Column D (0-indexed)
				try {current = stof(cell);}
				catch (...) {current = -9999;}  // use an invalid marker if parsing fails
				break;
			}
			++col;
		}
	
		// Calculate time: 1-minute intervals from 00:00
		Int_t hour = dataIndex / 60;
		Int_t minute = dataIndex % 60;
	
		struct tm tinfo = {0};
		tinfo.tm_year = year - 1900;
		tinfo.tm_mon  = month - 1;
		tinfo.tm_mday = day;
		tinfo.tm_hour = hour;
		tinfo.tm_min  = minute;
		tinfo.tm_sec  = 0;
	
		time_t seconds_since_epoch = mktime(&tinfo);
	
		timestamp.Set(year, month, day, hour, minute, 0, 0, kTRUE, 0);
		timestamp_sec = seconds_since_epoch;
		snprintf(Date, sizeof(Date), "%04d%02d%02d", year, month, day);
		snprintf(Time, sizeof(Time), "%02d:%02d", hour, minute);
	
		tLINAC->Fill();
		++dataIndex;
	}
	
	// Save
	fout->cd();
	tLINAC->Write();
	fout->Close();
	file.close();
	
	cout << "Finished writing: " << output_rootfile << endl;
}