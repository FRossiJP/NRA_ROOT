//	This macro was created by Rossi Fabiana on 2024-12-17
//	Last modified on 2025-03-27
/*
	How to run this macro:
		Single file -> root ParseDT_DTCF_250203.C\(\"ROOT/Th####/[Run#].root\",RefRunNumber\)
		All files -> for i in ROOT/Th####/20*.root; do root -b -q ParseDT_DTCF_250203.C\(\"$i\",RefRunNumber\); done
				  -> for i in ROOT/Th0100/20*.root; do root -b -q ParseDT_DTCF_250203.C\(\"$i\",RefRunNumber\); done
				  -> for i in ROOT/Th0500/20*.root; do root -b -q ParseDT_DTCF_250203.C\(\"$i\",RefRunNumber\); done
				  -> for i in ROOT/Th1000/20*.root; do root -b -q ParseDT_DTCF_250203.C\(\"$i\",RefRunNumber\); done
				  -> for i in ROOT/Th1500/20*.root; do root -b -q ParseDT_DTCF_250203.C\(\"$i\",RefRunNumber\); done
				  -> for i in ROOT/Th2000/20*.root; do root -b -q ParseDT_DTCF_250203.C\(\"$i\",RefRunNumber\); done
	
	**********************************************************************
		
		1) Evaluate the lost count due to the detector paralyzed DT
		2) Calculate the DT correction factor
		3) Correct the number of counts
		4) Create the TOF spectra
		
		Output files are:
		1) TH1D: /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/TOF_[Run#].root
			where the histograms of TOF, Lost, DTCF are stored
		2) Plot: /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/[Run#]_[DetType]_[HistType].png
		
	**********************************************************************
*/

//#include <filesystem>
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
//namespace fs = std::filesystem;

const TString MainDir = "/home/ndg/Fabi/JFY2024/01_ROOT/202502_KyotoExp";
const TString DataDir = "/list";
// const TString ProgramDir = "/progs";
const TString RootDir = "/ROOT";
// const TString RootDateDir = "/20250203";
const TString RootPlotDir = "/Plot";
const Int_t EXPyear= 2025;

const Int_t delMAX = 20;
// const Int_t maxChannel = 10;
// const Int_t maxSpec = 4;
// const Int_t maxGate = 3;
const Bool_t PrintMemoON = 1; // = 1 to activate the print on screen
const Bool_t PrintInpFileON = 1; // = 1 to activate the print on screen
const Bool_t PrintOutFileON = 1; // = 1 to activate the print on screen
const Bool_t PrintReadingON = 1; // = 1 to activate the print on screen
const Bool_t PrintReadOutON = 0; // = 1 to activate the print on screen
// const Bool_t PrintDeadTimeON = 0; // = 1 to activate the print on screen
// const Bool_t PrintFitResultON = 0; // = 1 to activate the print on screen

const Bool_t SaveFigureON = 1; // = 1 to save the DT histogram as png files
// const Bool_t SaveNOEON = 0; // = 1 to save the NOE in a output file

// const Int_t HourFileMAX = 100; // Max number of files per hour
// const Int_t lineTitle = 0; // Number of lines to skip in the data file
const Int_t lineMax = 200; // Max number of lines in the data file

const Double_t RefTime = 3600.; // seconds
const Double_t ns_to_us = 0.001; // to convert from ns to us
const Double_t shift = 0.001;
// const Int_t tbin_tot = 1250; // 1 ch = 2 ns
// const Double_t t_min = 0.0; // ns
// const Double_t t_max = 2500.0; // ns
const Int_t tbin_tot_full = 10000000; // 1 ch = 2 ns 
const Double_t t_min_full = 0.0; // us
const Double_t t_max_full = 20000.0; // us
const Double_t t_bin_size = 0.002; // = 20000/10000000
const Int_t RebinFactor = 100;
const Int_t MAXRunNumber = 27; // Max number of runs
const Int_t MAXnumChannel = 17; // Max number of detector's channels
const Double_t LimitQl = 65535; // Max number for Ql
const Double_t LimitQs = 32767; // Max number for Qs
const Double_t PSDartifact = 0.500008; // = Ql / Qs
const Int_t DetType = 2; // Type of detectors to sum (H01-H01 / SO1-S09)

const Double_t Threshold = 100;
const Double_t ThMIN = 100; // Minimum value for the threshold
const Double_t ThMIN_EJ270 = 4000; // Minimum value for the EJ-270 threshold
const Int_t ThSTE = 20; // Number of step for increasing the threshold
const Double_t ThMAX = 60000; // Maximum number for the threshold
const Double_t ql_lim = 60000;
const Double_t fit_psd_n = 0.5;
const Int_t MAXpar = 4;
const Int_t MAXparameter = 8; // Number of parameters for the polynomial fit
const Int_t MAXparEllipse = 4; // Number of parameters for the ellipse
const Int_t MAXvalue = 2; // value and error
const Int_t MaxSCV = 3;
//const Int_t RefRunNumber = 2005;

const Double_t qlMIN_GS20 = 3400.;
const Double_t qlMAX_GS20 = 6000.;
const Double_t psdMIN_GS20 = 0.5;
const Double_t psdMAX_GS20 = 0.7;

void ParseDT_DTCF_250203(char *file, Int_t RefRunNumber) // , Double_t Threshold
{
	// const TString MainDir = string(fs::current_path());
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

	if (PrintMemoON == 1)
	{
		cout << "**********************************************************************" << endl;
		cout << "1) Evaluate the lost count due to the detector paralyzed DT" << endl;
		cout << "2) Calculate the DT correction factor" << endl;
		cout << "3) Correct the number of counts" << endl << endl;
		cout << "The formula for the evaluation of the corrected count, Nc(I), is:" << endl << endl;
		cout << "                        No(I)           " << endl;
		cout << "    Nc(I) = ----------------------------" << endl;
		cout << "                  sum(No(J))    No(I)   " << endl;
		cout << "             1 - ----------- - -------  " << endl;
		cout << "                      Nb         Nb     " << endl << endl;
		cout << "where     Nb = number of bursts (total events in the ""Linac"" channel;" << endl;
		cout << "       No(I) = observed counts in time channel I;" << endl;
		cout << "  sum(No(J)) = lost counts due to paralyzed dead time (evaluated as the Interval in time channel)." << endl << endl;
		cout << "ChannelNumber =  0     / 1     / 2     / 3     / 4     / 5     / 6     / 7     / 8     / 9     / 10    / 11    /" << endl;
		cout << "    (exp. 2024): GS20  / H01   / H02   / H03   / H04   / H05   / H06   / mGS20 / Linac / BF3   /       /       /" << endl << endl;
		cout << "Output files are: " << endl;
		cout << Form("  1) TH1D:   %s%s/Th%04.0f/TOF_[Run#].root", MainDir.Data(), RootDir.Data(), Threshold) << endl;
		cout << Form("             (the histograms of TOF, Lost, DTCF are stored here for the GS20 and the PSD detectors)") << endl;
		cout << Form("  2) Plot:  %s%s/Th%04.0f%s/[Run#]_[DetType]_[HistType].png", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data()) << endl;
		cout << "**********************************************************************" << endl << endl;

	}
/*
	fs::path dir = Form("%s%s/Th%04.0f%s",MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data());
	if (!fs::exists(dir)) {
        	fs::create_directory(dir);
        	std::cout << "mkdir now!" << dir << std::endl;
    	} else {
        	std::cout << "already mkdir" << dir << std::endl;
    	}
*/

// ******************************* TOF & DTCF **************************************
	
	ifstream finMemo;
	TString FNameMemo = Form("%s%s/Th%04.0f/2025_RunInfo.txt", MainDir.Data(), RootDir.Data(), Threshold); // "ROOT/Th1000/2025_RunInfo.txt";
	Int_t CountRun = 0;
	
	//EXPyear = stoi(FNameMemo(59,4)); // stoi(FNameMemo(12,4));
	Int_t temp1, RunNum[MAXRunNumber], temp3, Shots[MAXRunNumber];
	Double_t temp2, MeasTime[MAXRunNumber];
	Double_t Color[MAXRunNumber] = {kBlack, kWhite, kGray,
					kRed, kRed, kRed+2, kRed-9,
					kGray+2,
					kMagenta+2, kMagenta-9, kMagenta,
					kOrange+2, kOrange-9, kOrange,
					kBlue, kBlue+2, kBlue-9,
					kBlack, kGray+2, kGray,
					kCyan-9, kCyan, kCyan+2,
					kViolet, kWhite, kViolet+2, kViolet-9};
	TString Label[MAXRunNumber] = {"Blank", "Blank+Filter", "Blank+BPE", // 2001
					"EU_3", "EU_3", "EU_3+Filter", "EU_3+BPE", // 2004
					"Blank+Filter", // 2008
					"EU_2+Filter", "EU_2+BPE", "EU_2", // 2009
					"EU_1+Filter", "EU_1+BPE", "EU_1", // 2012
					"HP Pb", "HP Pb+Filter", "HP Pb+BPE", // 2015
					"Blank", "Blank+Filter", "Blank+BPE", // 2018
					"HP Pb+EU_3+BPE", "HP Pb+EU_3", "HP Pb+EU_3+Filter", // 2021
					"HP Pb EJ-270 testing", "ABORTED", "HP Pb EJ-270 testing +Filter", "HP Pb EJ-270 testing +BPE"}; // 2024
	
	finMemo.open(FNameMemo);
	if (PrintReadingON == 1) {cout << Form(" #  Run#   MeasTime [s]     Shots            (exp in %4d)", EXPyear) << endl;}
	while(finMemo >> temp1 >> temp2 >> temp3)
	{
		RunNum[CountRun] = temp1;
		MeasTime[CountRun] = temp2;
		Shots[CountRun] = temp3;
		if (PrintReadingON == 1) {cout << Form("%2d  %4d  %10.2f    %10d", CountRun, RunNum[CountRun], MeasTime[CountRun], Shots[CountRun]) << endl;}
		CountRun++;
	}
	if (PrintReadingON == 1) {cout << Form("The number of runs done in %4d are %2d", EXPyear, CountRun) << endl << endl;}
	
	TString TString ChLabel[MAXnumChannel+1], ChType[MAXnumChannel+1];
	Int_t Deadtime[MAXnumChannel]; // 2ns
	
	ChLabel[0] = "mini_GS20";    ChType[0] = "Li6_Glass";     Deadtime[0] = 450;
	ChLabel[1] = "H01";          ChType[1] = "Plastic";       Deadtime[1] = 650;
	ChLabel[2] = "H02";          ChType[2] = "Plastic";       Deadtime[2] = 650;
	ChLabel[3] = "H03";          ChType[3] = "Plastic";       Deadtime[3] = 650;
	ChLabel[4] = "H04";          ChType[4] = "Plastic";       Deadtime[4] = 650;
	ChLabel[5] = "H05";          ChType[5] = "Plastic";       Deadtime[5] = 650;
	ChLabel[6] = "H06";          ChType[6] = "Plastic";       Deadtime[6] = 650;
	ChLabel[7] = "H07";          ChType[7] = "Plastic";       Deadtime[7] = 650;
	ChLabel[8] = "H08";          ChType[8] = "Plastic";       Deadtime[8] = 650;
	ChLabel[9] = "H09";          ChType[9] = "Plastic";       Deadtime[9] = 650;
	ChLabel[10] = "H10";         ChType[10] = "Plastic";      Deadtime[10] = 650;
	ChLabel[11] = "EJ-270";      ChType[11] = "Li6_Plastic";  Deadtime[11] = 650;
	ChLabel[12] = "Layer_GS20";  ChType[12] = "Li6_Glass";    Deadtime[12] = 450;
	ChLabel[13] = "BF3";         ChType[13] = "Other";        Deadtime[13] = 450;
	ChLabel[14] = "";            ChType[14] = "";             Deadtime[14] = 0;
	ChLabel[15] = "";            ChType[15] = "";             Deadtime[14] = 0;
	ChLabel[16] = "Linac";       ChType[16] = "Beam";         Deadtime[14] = 0;
	ChLabel[17] = "TOTAL";       ChType[17] = "";             Deadtime[14] = 0;
	
	TString FName = file, SearchUnderscore = "_", SearchSlash = "/", SearchTerm = ".root";
	Int_t delim[delMAX], delimSize = 0;
	
	for (int i = 0; i < FName.Length(); i++)
	{
		if ((FName(i,1) == SearchUnderscore) || (FName(i,1) == SearchSlash) || (FName(i,5) == SearchTerm))
		{
			delimSize++;
			delim[delimSize] = i;
		}
	}
	Int_t RunNumber, RunColor, RunShots;
	Double_t RunTime, ScaleTime;
	TString RunLabel;
	
	RunNumber = stoi(FName(delim[2]+1,delim[3]-delim[2]-1));
	
	for (int i=0; i<MAXRunNumber; i++)
	{
		if (RunNumber == RunNum[i])
		{
			RunColor = Color[i];
			RunLabel = Label[i];
			RunShots = Shots[i];
			RunTime = MeasTime[i];
			ScaleTime = RefTime/RunTime; // Scale TOF to 1h measurement
			if (PrintInpFileON == 1) {cout << Form("   %s : t_meas = %.2f [s] (x%.3f)  Shots = %d", RunLabel.Data(), RunTime, ScaleTime, RunShots) << endl;}
			break;
		}
	}
	
	TFile *fin = TFile::Open(FName);
	Int_t ch, tof, interval;
	Double_t ql, qs, psd;
	TTree *tData = (TTree*)fin->Get("tData");
	tData->SetBranchAddress("ch",&ch);
	tData->SetBranchAddress("tof",&tof);
	tData->SetBranchAddress("interval",&interval);
	tData->SetBranchAddress("ql",&ql);
	tData->SetBranchAddress("qs",&qs);
	tData->SetBranchAddress("psd",&psd);
	
	TString FNameFit = Form("%s%s/Th%04.0f/TH2D_%04d.root", MainDir.Data(), RootDir.Data(), Threshold, RefRunNumber);
	if (PrintInpFileON == 1) {cout << Form("Extracting fitting parameters from the file %s", FNameFit.Data()) << endl << endl;}
	TFile *finFit = TFile::Open(FNameFit);
	Int_t rn, countCH = 0;
	Double_t x_c, y_c; // Center of the ellipse
	Double_t a, b; // Semi-Major and Semi-Minor Axes
	Double_t par[MAXparameter], FITparMIN[MAXnumChannel][MAXparameter], FITparMAX[MAXnumChannel][MAXparameter];
	Double_t EllipseParamsMIN[MAXnumChannel][MAXparEllipse], EllipseParamsMAX[MAXnumChannel][MAXparEllipse];
	// Initialize arrays
	for (int i = 0; i < MAXnumChannel; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			EllipseParamsMIN[i][j] = 0.0; // Initialize min ellipse params
			EllipseParamsMAX[i][j] = 0.0; // Initialize max ellipse params
		}
		for (int j = 0; j < MAXparameter; j++)
		{
			FITparMIN[i][j] = 0.0; // Initialize min FIT parameters
			FITparMAX[i][j] = 0.0; // Initialize max FIT parameters
		}
	}
	TTree *tFit = (TTree*)finFit->Get("tFit");
	tFit->SetBranchAddress("rn", &rn);
	tFit->SetBranchAddress("ch", &ch);
	for (int j=0; j<MAXparameter; j++)
	{
		tFit->SetBranchAddress(Form("par%d", j), &par[j]);
	}
	tFit->SetBranchAddress("x_c",&x_c);
	tFit->SetBranchAddress("y_c",&y_c);
	tFit->SetBranchAddress("a",&a);
	tFit->SetBranchAddress("b",&b);
	
	if (PrintInpFileON == 1) {cout << "Processing Channels" << endl;}
	for (int i = 0; i < tFit->GetEntries(); i += MaxSCV) // Loop over blocks of size MaxSCV
	{
		// Get the first row of the block (min values)
		tFit->GetEntry(i); // Min row
//		TString currentLabel = ChLabel[ch];
		TString currentType = ChType[ch];
		if (currentType == ("")) // (currentLabel == "")
		{
			if (PrintInpFileON == 1) {cout << Form("ch%02d %s: SKIP", ch, currentLabel.Data()) << endl;}
		}
		// Handle "GS20" channels and EJ-270
		if (currentType.Contains("Li6")) // (currentLabel.Contains("GS20") || currentLabel.Contains("270"))
		{
			// Store min ellipse parameters (first row in block)
			EllipseParamsMIN[ch][0] = x_c; // x-center (min)
			EllipseParamsMIN[ch][1] = y_c; // y-center (min)
			EllipseParamsMIN[ch][2] = a;   // semi-major axis (min)
			EllipseParamsMIN[ch][3] = b;   // semi-minor axis (min)
			if (PrintInpFileON == 1) {cout << Form("ch%02d %s (Min): x_c=%12.3e, y_c=%12.3e, a=%12.3e, b=%12.3e", ch, currentLabel.Data(), x_c, y_c, a, b) << endl;}
			
			// Get the last row of the block (max values)
			if (i + MaxSCV - 1 < tFit->GetEntries())
			{
				tFit->GetEntry(i + MaxSCV - 1); // Max row
				EllipseParamsMAX[ch][0] = x_c; // x-center (max)
				EllipseParamsMAX[ch][1] = y_c; // y-center (max)
				EllipseParamsMAX[ch][2] = a;   // semi-major axis (max)
				EllipseParamsMAX[ch][3] = b;   // semi-minor axis (max)
				if (PrintInpFileON == 1) {cout << Form("ch%02d %s (Max): x_c=%12.3e, y_c=%12.3e, a=%12.3e, b=%12.3e", ch, currentLabel.Data(), x_c, y_c, a, b) << endl;}
			}
		}
		else if (currentType.Contains("Plastic")) // (currentLabel != "Linac" && currentLabel != "TOTAL") // Non-empty and not special channels
		{
			// Store min parameters (first row in block)
			for (int j = 0; j < MAXparameter; j++)
			{
				FITparMIN[ch][j] = par[j];
			}
	
			if (PrintInpFileON == 1)
			{
				cout << Form("ch%02d %s (Min): ", ch, currentLabel.Data());
				for (int j = 0; j < MAXparameter; j++) {cout << Form("%12.3e", par[j]);}
				cout << endl;
			}
	
			// Get the last row of the block (max parameters)
			if (i + MaxSCV - 1 < tFit->GetEntries())
			{
				tFit->GetEntry(i + MaxSCV - 1); // Max row
				for (int j = 0; j < MAXparameter; j++)
				{
					FITparMAX[ch][j] = par[j];
				}
	
				if (PrintInpFileON == 1)
				{
					cout << Form("ch%02d %s (Max): ", ch, currentLabel.Data());
					for (int j = 0; j < MAXparameter; j++) {cout << Form("%12.3e", par[j]);}
					cout << endl;
				}
			}
		}
		// Increment the channel counter
		countCH++;
	}
	if (PrintInpFileON == 1) {cout << Form("Finished processing file: %s", FNameFit.Data()) << endl << endl;}
	
	if (PrintInpFileON == 1) {cout << Form("READING the file: %s", FName.Data()) << endl;}
	Int_t array_old_tof[MAXnumChannel+1];
	Int_t array_noe[MAXnumChannel+1];
	for (int i=0; i<MAXnumChannel+1; i++)
	{
		array_noe[i] = 0;
		array_old_tof[i] = 0;
	}
	
	Long64_t all_event = tData->GetEntries();
	if (PrintOutFileON == 1) {cout << endl << "Number of Events (NOE): " << all_event << endl;}
	
	TH1D *hTOF[MAXnumChannel], *hTOF_g[MAXnumChannel], *hTOF_n[MAXnumChannel], *hLostTOF[MAXnumChannel], *hDTCF[MAXnumChannel], *hTOF_DTC_1h[MAXnumChannel], *hTOF_g_DTC_1h[MAXnumChannel], *hTOF_n_DTC_1h[MAXnumChannel], *hTOF_g_SUM_1h[DetType], *hTOF_n_SUM_1h[DetType];
	
	for (int i=0; i<MAXnumChannel; i++)
	{
		hTOF[i] = new TH1D(Form("hTOF_ch%02d_%s",i,ChLabel[i].Data()),Form("hTOF_ch%02d_%s",i,ChLabel[i].Data()),tbin_tot_full,t_min_full,t_max_full);
		hTOF_g[i] = new TH1D(Form("hTOF_g_ch%02d_%s",i,ChLabel[i].Data()),Form("hTOF_g_ch%02d_%s",i,ChLabel[i].Data()),tbin_tot_full,t_min_full,t_max_full);
		hTOF_n[i] = new TH1D(Form("hTOF_n_ch%02d_%s",i,ChLabel[i].Data()),Form("hTOF_n_ch%02d_%s",i,ChLabel[i].Data()),tbin_tot_full,t_min_full,t_max_full);
		hLostTOF[i] = new TH1D(Form("hLost_ch%02d_%s",i,ChLabel[i].Data()),Form("hLost_ch%02d_%s",i,ChLabel[i].Data()),tbin_tot_full,t_min_full,t_max_full);
		hDTCF[i] = new TH1D(Form("hDTCF_ch%02d_%s",i,ChLabel[i].Data()),Form("hDTCF_ch%02d_%s",i,ChLabel[i].Data()),tbin_tot_full,t_min_full,t_max_full);
	}
	
	Double_t tof_lost, fit_psdMIN[MAXnumChannel], fit_psdMAX[MAXnumChannel], x_last = ThMAX;
	for (int i=0; i<all_event; i++)
	{
		tData->GetEntry(i);
		if (ql > Threshold && ql < ql_lim)
		{
			Double_t timeofflight = tof*ns_to_us - shift;
			array_noe[MAXnumChannel]++;
			for (int j=0; j<MAXnumChannel; j++) // for (int j=0; j<2; j++)
			{
				TString currentType = ChType[j];
				if (ch == j)
				{
					array_noe[j]++;
					hTOF[j]->Fill(timeofflight);
//					if (((i > 1000) && (i < 1020)) && (PrintReadingON == 1)) {cout << Form("tof %10d TOF %10.5f ", tof, timeofflight) << endl;}
					for (int k=1; k<=Deadtime[j]; k++)
					{
						tof_lost = tof - t_bin_size*k;
//						if (((i > 1000) && (i < 1020)) && (k < 5) && (PrintReadingON == 1)) {cout << Form("lost %10.5f ", tof_lost);}
						hLostTOF[j]->Fill(tof_lost);
					}
//					if (((i > 1000) && (i < 1020)) && (PrintReadingON == 1)) {cout << endl;}
					if (currentType.Contains("Plastic")) // (j == 4 || j == 6) // if (j >= 0 && j <= 1)
					{
						if (ql <= x_last)
						{
							fit_psdMIN[j] = FITparMIN[j][0] + FITparMIN[j][1]*ql + FITparMIN[j][2]*pow(ql, 2) + FITparMIN[j][3]*pow(ql, 3) + FITparMIN[j][4]*pow(ql, 4) + FITparMIN[j][5]*pow(ql, 5) + FITparMIN[j][6]*pow(ql, 6);
							fit_psdMAX[j] = FITparMAX[j][0] + FITparMAX[j][1]*ql + FITparMAX[j][2]*pow(ql, 2) + FITparMAX[j][3]*pow(ql, 3) + FITparMAX[j][4]*pow(ql, 4) + FITparMAX[j][5]*pow(ql, 5) + FITparMAX[j][6]*pow(ql, 6);
//							if (ql > 1500. && ql < 1505.) {cout << Form("ch%d (ql=%.0f): %12.3e %.3f", j, ql, FITpar[j][0], fit_psd[j]) << endl;}
						}
						else {fit_psdMIN[j] = FITparMIN[j][7]; fit_psdMAX[j] = FITparMAX[j][7];}
						if (psd > fit_psdMAX[j] && psd < fit_psd_n) {hTOF_n[j]->Fill(timeofflight);} // if (psd > 0.3 && psd < 0.4) {hTOF_n[j]->Fill(timeofflight);}
						else if (psd <= fit_psdMIN[j]) {hTOF_g[j]->Fill(timeofflight);} // (psd >= 0.16 && psd <=0.23) {hTOF_g[j]->Fill(timeofflight);}
					}
					if (currentType.Contains("Li6")) // (j == 0 || j == 11 || j == 12) // GS20 and EJ-270
					{
						Double_t insideMinEllipse = pow((ql - EllipseParamsMIN[j][0]) / EllipseParamsMIN[j][2], 2) + pow((psd - EllipseParamsMIN[j][1]) / EllipseParamsMIN[j][3], 2);
						if (insideMinEllipse <= 1)
						{
							hTOF_n[j]->Fill(timeofflight);
						}
					}
				}
			}
		}
	}
	
	Int_t Bursts;
	if (PrintOutFileON == 1)
	{
		for (int i=0; i<MAXnumChannel; i++) // for (int i=0; i<2; i++)
		{
			TString currentType = ChType[i];
			if (currentType != ("")) {cout << Form("NOE for ch%02d: ", i) << array_noe[i] << endl;} // (ChLabel[i] != "")
			if (currentType.Contains("Beam")) {Bursts = array_noe[i];} // (ChLabel[i] == "Linac")
		}
		cout << endl;
	}
	
	TString ofile = Form("%s%s/Th%04.0f/TOF_%04d.root", MainDir.Data(), RootDir.Data(), Threshold, RunNumber);
	
	Double_t tofMIN = 1.e-2, tofMAX = 1.e4;
	Double_t tofMIN_1h = 1., tofMAX_1h = 1.e4;
	Double_t countMIN = 1.e-1, countMAX = 5.e3;
	Double_t countMIN_1h = 5.e-2, countMAX_1h = 1.e3;
	Double_t countMIN_SUM1h = 5.e-2, countMAX_SUM1h = 1.e5;
	Double_t DTCFMIN = 1.e0, DTCFMAX = 1.000005;
	
	TCanvas *canvTOF[MAXnumChannel], *canvLost[MAXnumChannel], *canvDTCF[MAXnumChannel], *canvTOF1h[MAXnumChannel], *canvTOF_SUM1h;
	TLegend *legTOF[MAXnumChannel], *legLost[MAXnumChannel], *legDTCF[MAXnumChannel], *legTOF1h[MAXnumChannel], *legTOF_SUM1h;
	TString histName[MAXnumChannel][3];
	
	TFile *fout = new TFile(ofile,"recreate");
	for (int i=0; i<MAXnumChannel; i++) // for (int i=0; i<2; i++)
	{
		TString currentType = ChType[i];
		if (currentType != ("")) // (ChLabel[i] != "") // (i >= 0 && i <= 1) // (i >= 0 && i <= 7)
		{
			legTOF[i] = new TLegend(0.75, 0.85, 0.99, 0.99);
			legTOF[i]->SetMargin(0.30);
			canvTOF[i] = new TCanvas(Form("TOF_%04d_%s", RunNumber, ChLabel[i].Data()), Form("TOF_%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvTOF[i]->SetMargin(0.10,0.025,0.10,0.05);
			canvTOF[i]->SetGridx(); canvTOF[i]->SetGridy();
			canvTOF[i]->SetLogx(); canvTOF[i]->SetLogy();
			hTOF[i]->GetXaxis()->CenterTitle();
			hTOF[i]->GetXaxis()->SetTitle("TOF [#mus]");
			hTOF[i]->GetXaxis()->SetTitleOffset(1.20);
			hTOF[i]->GetXaxis()->SetRangeUser(tofMIN, tofMAX);
			hTOF[i]->GetYaxis()->CenterTitle();
			hTOF[i]->GetYaxis()->SetTitle("Count");
			hTOF[i]->GetYaxis()->SetTitleOffset(1.20);
			hTOF[i]->GetYaxis()->SetRangeUser(countMIN, countMAX);
			hTOF[i]->SetLineColor(RunColor);
			hTOF[i]->Draw("hist");
			legTOF[i]->SetHeader("TOF","c");
			legTOF[i]->AddEntry(hTOF[i],Form("%04d (%s) %s", RunNumber, RunLabel.Data(), ChLabel[i].Data()),"l");
			if (currentType.Contains("Plastic")) // (i == 4 || i == 6) // if (i >= 0 && i <= 1)
			{
				hTOF_g[i]->SetLineColor(kBlack);
				hTOF_g[i]->Draw("hist, same");
				hTOF_n[i]->SetLineColor(kRed);
				hTOF_n[i]->Draw("hist, same");
				legTOF[i]->AddEntry(hTOF_g[i],"Gamma","l");
				legTOF[i]->AddEntry(hTOF_n[i],"Neutron","l");
			}
			if (currentType.Contains("Li6")) // (i == 0 || i == 11 || i == 12) // if (i >= 0 && i <= 1)
			{
				hTOF_n[i]->SetLineColor(kRed);
				hTOF_n[i]->Draw("hist, same");
				legTOF[i]->AddEntry(hTOF_n[i],"Neutron","l");
			}
			legTOF[i]->Draw();
			
			legLost[i] = new TLegend(0.75, 0.85, 0.99, 0.99);
			legLost[i]->SetMargin(0.30);
			canvLost[i] = new TCanvas(Form("Ql_%04d_%s", RunNumber, ChLabel[i].Data()), Form("LostTOF_%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvLost[i]->SetMargin(0.10,0.025,0.10,0.05); // left, right, bottom, top
			canvLost[i]->SetGridx(); canvLost[i]->SetGridy();
			canvLost[i]->SetLogx(); canvLost[i]->SetLogy();
			hLostTOF[i]->GetXaxis()->CenterTitle();
			hLostTOF[i]->GetXaxis()->SetTitle("TOF [#mus]");
			hLostTOF[i]->GetXaxis()->SetTitleOffset(1.20);
			hLostTOF[i]->GetXaxis()->SetRangeUser(tofMIN, tofMAX);
			hLostTOF[i]->GetYaxis()->CenterTitle();
			hLostTOF[i]->GetYaxis()->SetTitle("Count");
			hLostTOF[i]->GetYaxis()->SetTitleOffset(1.20);
			hLostTOF[i]->GetYaxis()->SetRangeUser(countMIN, countMAX);
			hLostTOF[i]->SetLineColor(RunColor);
			hLostTOF[i]->SetLineWidth(3);
			hLostTOF[i]->Draw("hist");
			legLost[i]->SetHeader("Lost TOF","c");
			legLost[i]->AddEntry(hLostTOF[i],Form("%04d (%s) %s", RunNumber, RunLabel.Data(), ChLabel[i].Data()),"l");
			legLost[i]->Draw();
			
			hTOF[i]->Write();
			hLostTOF[i]->Write();
			
			TAxis *xaxis = hTOF[i]->GetXaxis();
			if (PrintReadOutON == 1) {cout << xaxis->GetNbins() << endl;}
			
			for (int k=1; k<=xaxis->GetNbins(); k++)
			{
				Double_t a = hLostTOF[i]->GetBinContent(k);
				Double_t b = RunShots;
				Double_t c = hTOF[i]->GetBinContent(k);
				Double_t DTCF, err_DTCF;
				
				DTCF = 1.0 / (1.0 - (a/b) - (c/(2.0*b)));
				Double_t err_a = sqrt(a);
				Double_t err_b = sqrt(b);
				Double_t err_c = sqrt(c);
				Double_t pDer_a = pow(DTCF,2.0) * (1.0/b); // partial derivative with a as variable
				Double_t pDer_b = pow(DTCF,2.0) * (-(a/pow(b,2.0)) -(c/(2*pow(b,2.0)))); // partial derivative with b as variable
				Double_t pDer_c = pow(DTCF,2.0) * (1.0/(2.0*b)); // partial derivative with c as variable
				err_DTCF = sqrt(pow(err_a*pDer_a,2.0) + pow(err_b*pDer_b,2.0) + pow(err_c*pDer_c,2.0));
				if (PrintReadOutON == 1) {if (k>=650 && k<=700) {cout << Form("%02d %.10e %.10e %.10e (%.10e)   ", k, a, c, DTCF, err_DTCF) << endl;}}
				
				hDTCF[i]->SetBinContent(k, DTCF);
				hDTCF[i]->SetBinError(k, err_DTCF);
			}
			
			legDTCF[i] = new TLegend(0.15, 0.85, 0.39, 0.99);
			legDTCF[i]->SetMargin(0.30);
			canvDTCF[i] = new TCanvas(Form("DTCF_%04d_%s", RunNumber, ChLabel[i].Data()), Form("DTCF_%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvDTCF[i]->SetMargin(0.10,0.025,0.10,0.05); // left, right, bottom, top
			canvDTCF[i]->SetGridx(); canvDTCF[i]->SetGridy();
			canvDTCF[i]->SetLogx(); // canvDTCF[i]->SetLogy();
			hDTCF[i]->GetXaxis()->CenterTitle();
			hDTCF[i]->GetXaxis()->SetTitle("TOF [#mus]");
			hDTCF[i]->GetXaxis()->SetTitleOffset(1.20);
			hDTCF[i]->GetXaxis()->SetRangeUser(tofMIN, tofMAX);
			hDTCF[i]->GetYaxis()->CenterTitle();
			hDTCF[i]->GetYaxis()->SetTitle("Dead-Time Correction Factor");
			hDTCF[i]->GetYaxis()->SetTitleOffset(1.20);
//			hDTCF[i]->GetYaxis()->SetRangeUser(DTCFMIN, DTCFMAX);
			hDTCF[i]->SetLineColor(RunColor);
			hDTCF[i]->SetLineWidth(3);
			hDTCF[i]->Draw("hist");
			legDTCF[i]->SetHeader("DTCF","c");
			legDTCF[i]->AddEntry(hDTCF[i],Form("%04d (%s) %s", RunNumber, RunLabel.Data(), ChLabel[i].Data()),"l");
			legDTCF[i]->Draw();
			
			hDTCF[i]->Write();
			
			histName[i][0] = Form("hTOF_DTC_1h_ch%02d_%s",i,ChLabel[i].Data());
			histName[i][1] = Form("hTOF_g_DTC_1h_ch%02d_%s",i,ChLabel[i].Data());
			histName[i][2] = Form("hTOF_n_DTC_1h_ch%02d_%s",i,ChLabel[i].Data());
//			for (int k=0; k<3; k++) {cout << histName[i][k] << endl;}
			hTOF_DTC_1h[i] = (TH1D*)hTOF[i]->Clone(histName[i][0]);
			hTOF_DTC_1h[i]->SetName(histName[i][0]);
			hTOF_DTC_1h[i]->SetTitle(histName[i][0]);
			hTOF_DTC_1h[i]->Multiply(hDTCF[i]);
			hTOF_DTC_1h[i]->Scale(ScaleTime);
			hTOF_DTC_1h[i]->Rebin(RebinFactor);
			legTOF1h[i] = new TLegend(0.75, 0.85, 0.99, 0.99);
			legTOF1h[i]->SetMargin(0.30);
			canvTOF1h[i] = new TCanvas(Form("TOF_1h_%04d_%s", RunNumber, ChLabel[i].Data()), Form("TOF_1h_%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvTOF1h[i]->SetMargin(0.10,0.025,0.10,0.05);
			canvTOF1h[i]->SetGridx(); canvTOF1h[i]->SetGridy();
			canvTOF1h[i]->SetLogx(); canvTOF1h[i]->SetLogy();
			hTOF_DTC_1h[i]->GetXaxis()->CenterTitle();
			hTOF_DTC_1h[i]->GetXaxis()->SetTitle("TOF [#mus]");
			hTOF_DTC_1h[i]->GetXaxis()->SetTitleOffset(1.20);
			hTOF_DTC_1h[i]->GetXaxis()->SetRangeUser(tofMIN_1h, tofMAX_1h);
			hTOF_DTC_1h[i]->GetYaxis()->CenterTitle();
			hTOF_DTC_1h[i]->GetYaxis()->SetTitle("Count [/3600s]");
			hTOF_DTC_1h[i]->GetYaxis()->SetTitleOffset(1.20);
			hTOF_DTC_1h[i]->GetYaxis()->SetRangeUser(countMIN_1h, countMAX_1h);
//			if (i == 12) {hTOF_DTC_1h[i]->GetYaxis()->SetRangeUser(1.0, 1.e5);}
			hTOF_DTC_1h[i]->SetLineColor(RunColor);
			hTOF_DTC_1h[i]->Draw("hist");
			legTOF1h[i]->SetHeader("TOF (DT corrected; 1h meas)","c");
			legTOF1h[i]->AddEntry(hTOF_DTC_1h[i],Form("%04d (%s) %s", RunNumber, RunLabel.Data(), ChLabel[i].Data()),"l");
			if (currentType.Contains("Plastic")) // (i == 4 || i == 6) // if (i >= 0 && i <= 1)
			{
				hTOF_g_DTC_1h[i] = (TH1D*)hTOF_g[i]->Clone(histName[i][1]);
//				hTOF_g_DTC_1h[i]->SetName(histName[i][1]);
				hTOF_g_DTC_1h[i]->SetTitle(histName[i][1]);
				hTOF_g_DTC_1h[i]->Multiply(hDTCF[i]);
				hTOF_g_DTC_1h[i]->Scale(ScaleTime);
				hTOF_g_DTC_1h[i]->Rebin(RebinFactor);
				hTOF_n_DTC_1h[i] = (TH1D*)hTOF_n[i]->Clone(histName[i][2]);
//				hTOF_n_DTC_1h[i]->SetName(histName[i][2]);
				hTOF_n_DTC_1h[i]->SetTitle(histName[i][2]);
				hTOF_n_DTC_1h[i]->Multiply(hDTCF[i]);
				hTOF_n_DTC_1h[i]->Scale(ScaleTime);
				hTOF_n_DTC_1h[i]->Rebin(RebinFactor);
				hTOF_g_DTC_1h[i]->SetLineColor(kBlack);
				hTOF_g_DTC_1h[i]->Draw("hist, same");
				hTOF_n_DTC_1h[i]->SetLineColor(kRed);
				hTOF_n_DTC_1h[i]->Draw("hist, same");
				legTOF1h[i]->AddEntry(hTOF_g_DTC_1h[i],"Gamma","l");
				legTOF1h[i]->AddEntry(hTOF_n_DTC_1h[i],"Neutron","l");
			}
			if (currentType.Contains("Li6")) // (i == 0 || i == 11 || i == 12)
			{
				hTOF_n_DTC_1h[i] = (TH1D*)hTOF_n[i]->Clone(histName[i][2]);
				hTOF_n_DTC_1h[i]->SetTitle(histName[i][2]);
				hTOF_n_DTC_1h[i]->Multiply(hDTCF[i]);
				hTOF_n_DTC_1h[i]->Scale(ScaleTime);
				hTOF_n_DTC_1h[i]->Rebin(RebinFactor);
				hTOF_n_DTC_1h[i]->SetLineColor(kRed);
				hTOF_n_DTC_1h[i]->Draw("hist, same");
				legTOF1h[i]->AddEntry(hTOF_n_DTC_1h[i],"Neutron","l");
			}
			legTOF1h[i]->Draw();
			canvTOF1h[i]->Modified();
			canvTOF1h[i]->Update();
			canvTOF1h[i]->Draw();
//			gSystem->ProcessEvents();
			
			hTOF_DTC_1h[i]->Write();
			if (currentType.Contains("Plastic")) // (i == 4 || i == 6) // if (i >= 0 && i <= 1)
			{
				hTOF_g_DTC_1h[i]->Write();
				hTOF_n_DTC_1h[i]->Write();
			}
			if (currentType.Contains("Li6")) // (i == 0 || i == 11 || i == 12)
			{
				hTOF_n_DTC_1h[i]->Write();
			}
			
			if (SaveFigureON == 1)
			{
				canvTOF[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_TOF.png", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvLost[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_LostTOF.png", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvDTCF[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_DTCF.png", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvTOF1h[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_TOF_DT_1h.png", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
			}
		}
	}
	
	TString hSUMName[DetType][2], DetLabel[DetType];
	DetLabel[0] = "H";
	DetLabel[1] = "G";
	
	canvTOF_SUM1h = new TCanvas(Form("TotalTOF_1h_%04d", RunNumber), Form("TotalTOF_1h_%04d", RunNumber), 1500, 1000);
	canvTOF_SUM1h->SetMargin(0.10,0.025,0.10,0.05); // left, right, bottom, top
	legTOF_SUM1h = new TLegend(0.75, 0.85, 0.99, 0.99);
	legTOF_SUM1h->SetMargin(0.30);
	legTOF_SUM1h->SetHeader(Form("Total TOF (run %04d: %s)", RunNumber, RunLabel.Data()),"c");
	canvTOF_SUM1h->SetGridx(); canvTOF_SUM1h->SetGridy();
	canvTOF_SUM1h->SetLogx(); canvTOF_SUM1h->SetLogy();
	hTOF_DTC_1h[0]->SetLineColor(kBlue); // mini_GS20
	hTOF_DTC_1h[0]->SetLineWidth(3);
	hTOF_DTC_1h[0]->GetYaxis()->SetRangeUser(countMIN_SUM1h, countMAX_SUM1h);
	hTOF_DTC_1h[0]->Draw("hist");
	legTOF_SUM1h->AddEntry(hTOF_DTC_1h[0],Form("%s", ChLabel[0].Data()),"l");
	
	for (int i=0; i<DetType-1; i++)
	{
		hSUMName[i][0] = Form("hTOF_g_SUM_1h_%s",DetLabel[i].Data());
		hSUMName[i][1] = Form("hTOF_n_SUM_1h_%s",DetLabel[i].Data());
		hTOF_g_SUM_1h[i] = (TH1D*)hTOF_g_DTC_1h[4]->Clone(hSUMName[i][0]); // H04
		hTOF_g_SUM_1h[i]->SetTitle(hSUMName[i][0]);
		hTOF_n_SUM_1h[i] = (TH1D*)hTOF_n_DTC_1h[4]->Clone(hSUMName[i][1]);
		hTOF_n_SUM_1h[i]->SetTitle(hSUMName[i][1]);
		hTOF_g_SUM_1h[i]->Add(hTOF_g_DTC_1h[6]); // Sum H06 to H04
		hTOF_n_SUM_1h[i]->Add(hTOF_n_DTC_1h[6]); // Sum H06 to H04
		hTOF_g_SUM_1h[i]->GetXaxis()->CenterTitle();
		hTOF_g_SUM_1h[i]->GetXaxis()->SetTitle("TOF [#mus]");
		hTOF_g_SUM_1h[i]->GetXaxis()->SetTitleOffset(1.20);
		hTOF_g_SUM_1h[i]->GetXaxis()->SetRangeUser(tofMIN_1h, tofMAX_1h);
		hTOF_g_SUM_1h[i]->GetYaxis()->CenterTitle();
		hTOF_g_SUM_1h[i]->GetYaxis()->SetTitle("Count [/3600s]");
		hTOF_g_SUM_1h[i]->GetYaxis()->SetTitleOffset(1.20);
		hTOF_g_SUM_1h[i]->GetYaxis()->SetRangeUser(countMIN_SUM1h, countMAX_SUM1h);
		hTOF_g_SUM_1h[i]->SetLineColor(kBlack);
		hTOF_g_SUM_1h[i]->SetLineWidth(3);
		hTOF_g_SUM_1h[i]->SetLineStyle(1);
		legTOF_SUM1h->AddEntry(hTOF_g_SUM_1h[i],Form("Gamma (%s & %s)", ChLabel[4].Data(), ChLabel[6].Data()),"l");
		hTOF_g_SUM_1h[i]->Draw("hist, same");
		
		hTOF_n_SUM_1h[i]->GetXaxis()->CenterTitle();
		hTOF_n_SUM_1h[i]->GetXaxis()->SetTitle("TOF [#mus]");
		hTOF_n_SUM_1h[i]->GetXaxis()->SetTitleOffset(1.20);
		hTOF_n_SUM_1h[i]->GetXaxis()->SetRangeUser(tofMIN_1h, tofMAX_1h);
		hTOF_n_SUM_1h[i]->GetYaxis()->CenterTitle();
		hTOF_n_SUM_1h[i]->GetYaxis()->SetTitle("Count [/3600s]");
		hTOF_n_SUM_1h[i]->GetYaxis()->SetTitleOffset(1.20);
		hTOF_n_SUM_1h[i]->GetYaxis()->SetRangeUser(countMIN_SUM1h, countMAX_SUM1h);
		hTOF_n_SUM_1h[i]->SetLineColor(kRed);
		hTOF_n_SUM_1h[i]->SetLineWidth(3);
		hTOF_n_SUM_1h[i]->SetLineStyle(1);
		legTOF_SUM1h->AddEntry(hTOF_n_SUM_1h[i],Form("Neutron (%s & %s)", ChLabel[4].Data(), ChLabel[6].Data()),"l");
		hTOF_n_SUM_1h[i]->Draw("hist, same");
		
		hTOF_g_SUM_1h[i]->Write();
		hTOF_n_SUM_1h[i]->Write();
	}
	legTOF_SUM1h->Draw();
	if (SaveFigureON == 1)
	{
		canvTOF_SUM1h->SaveAs(Form("%s%s/Th%04.0f%s/%04d_TotalTOF.png", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data(), RunNumber));
		canvTOF_SUM1h->SaveAs(Form("%s%s/Th%04.0f%s/%04d_TotalTOF.pdf", MainDir.Data(), RootDir.Data(), Threshold, RootPlotDir.Data(), RunNumber));
	}
	
	fout->Close();
}