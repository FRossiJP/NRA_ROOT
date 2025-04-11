//	This macro was created by Rossi Fabiana on 2024-12-12
//	Last modified on 2025-03-27
/*
	How to run this macro:
		Single file -> root Plot_PolyFIT_250401.C\(\"ROOT/Th####/[Run#].root\",Ql_MIN,Ql_MAX\)
		All files -> for i in ROOT/Th####/20*.root; do root -b -q Plot_PolyFIT_250401.C\(\"$i\",Ql_MIN,Ql_MAX\); done
				  -> for i in ROOT/Th0100/20*.root; do root -b -q Plot_PolyFIT_250401.C\(\"$i\",100,60000\); done
				  -> for i in ROOT/Th0500/20*.root; do root -b -q Plot_PolyFIT_250401.C\(\"$i\",500,60000\); done
				  -> for i in ROOT/Th1000/20*.root; do root -b -q Plot_PolyFIT_250401.C\(\"$i\",1000,60000\); done
				  -> for i in ROOT/Th1500/20*.root; do root -b -q Plot_PolyFIT_250401.C\(\"$i\",1500,60000\); done
				  -> for i in ROOT/Th2000/20*.root; do root -b -q Plot_PolyFIT_250401.C\(\"$i\",2000,60000\); done
		
		**********************************************************************
		1) Plot Qs, Ql and PSD histograms for the file Run#.root file
		Output files are:
		1) TH1D:  /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/TH1D_[Run#].root
		2) TH2D:  /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/TH2D_[Run#].root
		3) NOE:   /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/NOE_[Run#].out
		4) FOM:   /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/FOM_[Run#].out
		5) Plot:  /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/Plot/[Run#]_[DetType]_[HistType].(png)
		
		NOTE:
			Activate the "SaveNOEON" to save the NOE in 3)
			Activate the "CalculateFOM" to save the FOM in 4)
			     -> merge all the files: for n in $(seq 2011 2029); do for i in FOM_$n*.out; do cat $i >> FOM2025.txt; done; done
			Activate the "SaveFigureON" to save the figures in 5)
		
		**********************************************************************
*/

#include <filesystem>
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
namespace fs = std::filesystem;

//const TString MainDir = "/home/ndg/Fabi/JFY2024/01_ROOT/202504_Optimization";
const TString DataDir = "/list";
//const TString ProgramDir = "/progs";
const TString RootDir = "/ROOT";
// const TString RootDateDir = "/20250203";
const TString RootPlotDir = "/Plot";
const Int_t EXPyear= 2025;

const Int_t delMAX = 20;
const Int_t maxChannel = 10;
const Int_t maxSpec = 4;
const Int_t maxGate = 3;
const Bool_t PrintMemoON = 1; // = 1 to activate the print on screen
const Bool_t PrintInpFileON = 1; // = 1 to activate the print on screen
const Bool_t PrintOutFileON = 1; // = 1 to activate the print on screen
const Bool_t PrintReadingON = 0; // = 1 to activate the print on screen
const Bool_t PrintReadingMemoON = 1; // = 1 to activate the print on screen
// const Bool_t PrintDeadTimeON = 1; // = 1 to activate the print on screen
const Bool_t PrintFOMResultON = 1; // = 1 to activate the print on screen

const Bool_t CalculateFOM = 1; // = 1 to calculate FOM
const Double_t MeasTime = 3600.; // seconds
const Int_t DrawChoice = 1; // = 0 Just visualize the Interval ; = 1 Visualize the PSD fit and the QlPSD ; = 2 Visualize all
const Bool_t SaveFigureON = 1; // = 1 to save the DT histogram as png files
const Bool_t SaveNOEON = 1; // = 1 to save the NOE in a output file

// const Int_t HourFileMAX = 100; // Max number of files per hour
// const Int_t lineTitle = 0; // Number of lines to skip in the data file
// const Int_t lineMax = 200; // Max number of lines in the data file

const Double_t ns_to_us = 0.001; // to convert from ns to us
const Double_t shift = 0.001;
const Int_t tbin_tot = 1250; // 1 ch = 2 ns
const Double_t t_min = 0.0; // ns
const Double_t t_max = 2500.0; // ns
const Int_t MAXnumChannel = 17; // Max number of detector's channels
const Int_t MAXRunNumber = 14; // Max number of runs
// const Double_t LimitQl = 65535; // Max number for Ql
// const Double_t LimitQs = 32767; // Max number for Qs
// const Double_t PSDartifact = 0.500008; // = Ql / Qs

//const Double_t Threshold = 1000;
const Double_t ThMIN = 100; // Minimum value for the threshold
const Double_t ThMIN_EJ270 = 4000; // Minimum value for the EJ-270 threshold
const Int_t ThSTE = 20; // Number of step for increasing the threshold
const Double_t ThMAX = 10000; // Maximum number for the threshold
//const Double_t ql_lim = 11000;
const Int_t MAXpar = 4;
const Int_t MAXparameter = 7; // 8;
const Int_t MAXvalue = 2; // value and error
const Int_t MaxSCV = 3;
// const Double_t gSCV = 3; // Sigma confidence value (99.7%)
// const Double_t nSCV = 3; // Sigma confidence value (99.7%)
// const Double_t gSCV = 4; // Sigma confidence value (99.9937%)
// const Double_t nSCV = 4; // Sigma confidence value (99.9937%)

void Plot_PolyFIT_250401(char *file, Double_t Ql_MIN, Double_t Ql_MAX)
{
	const TString MainDir = string(fs::current_path());
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
		cout << "1) Plot Qs, Ql and PSD histograms for the file Run#.root file" << endl;
		cout << "Output files are: " << endl;
		cout << Form("  1) TH1D:  %s%s/Th%04.0f/TH1D_[Run#].root", MainDir.Data(), RootDir.Data(), Ql_MIN) << endl;
		cout << Form("       This also contains the FOM values if CalculateFOM=1") << endl;
		cout << Form("  2) TH2D:  %s%s/Th%04.0f/TH2D_[Run#].root", MainDir.Data(), RootDir.Data(), Ql_MIN) << endl;
		cout << Form("       This also contains the fit parameters for the H0~ and the ellipse parameters for the GS20") << endl;
		cout << Form("  3) NOE:   %s%s/Th%04.0f/NOE_[Run#].out", MainDir.Data(), RootDir.Data(), Ql_MIN) << endl;
		cout << Form("  4) FOM:   %s%s/Th%04.0f/FOM_[Run#].out", MainDir.Data(), RootDir.Data(), Ql_MIN) << endl;
		cout << Form("  5) Plot:  %s%s/Th%04.0f%s/[Run#]_[DetType]_[HistType].png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data()) << endl;
		cout << "**********************************************************************" << endl << endl;
	}

	fs::path dir = Form("%s%s/Th%04.0f%s",MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data());
        if (!fs::exists(dir)) {
                fs::create_directory(dir);
                std::cout << "mkdir now!" << dir << std::endl;
        } else {
                std::cout << "already mkdir" << dir << std::endl;
        }


// ******************************* PLOT **************************************
	string filestr = file;
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
	Int_t RunNumber, RunColor;
	size_t lastSlashPos = filestr.find_last_of('/');
        string fileName = filestr.substr(lastSlashPos + 1);
        size_t dotPos = fileName.find('.');
        string numberStr = fileName.substr(0, dotPos);
        cout<<numberStr<<endl;
        RunNumber = stoi(numberStr);
	if (PrintInpFileON == 1) {cout << Form("READING the file: %s", FName.Data()) << endl;}
	
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
	
	Int_t array_old_tof[MAXnumChannel+1];
	Int_t array_noe[MAXnumChannel+1];
	for (int i=0; i<MAXnumChannel+1; i++)
	{
		array_noe[i] = 0;
		array_old_tof[i] = 0;
	}
	
	Long64_t all_event = tData->GetEntries();
	if (PrintOutFileON == 1) {cout << endl << "Number of Events (NOE): " << all_event << endl;}
	
	TString ChLabel[MAXnumChannel+1], ChType[MAXnumChannel+1];
	Int_t xlbin_tot, xsbin_tot, xpbin_tot;
	Double_t xl_min, xl_max, xs_min, xs_max, xp_min, xp_max;
	
	ChLabel[0] = "mini_GS20";    ChType[0] = "Li6_Glass";
	ChLabel[1] = "H01";          ChType[1] = "Plastic";
	ChLabel[2] = "H02";          ChType[2] = "Plastic";
	ChLabel[3] = "H03";          ChType[3] = "Plastic";
	ChLabel[4] = "H04";          ChType[4] = "Plastic";
	ChLabel[5] = "H05";          ChType[5] = "Plastic";
	ChLabel[6] = "H06";          ChType[6] = "Plastic";
	ChLabel[7] = "H07";          ChType[7] = "Plastic";
	ChLabel[8] = "H08";          ChType[8] = "Plastic";
	ChLabel[9] = "H09";          ChType[9] = "Plastic";
	ChLabel[10] = "H10";         ChType[10] = "Plastic";
	ChLabel[11] = "EJ-270";      ChType[11] = "Li6_Plastic";
	ChLabel[12] = "Layer_GS20";  ChType[12] = "Li6_Glass";
	ChLabel[13] = "BF3";         ChType[13] = "Other";
	ChLabel[14] = "";            ChType[14] = "";
	ChLabel[15] = "";            ChType[15] = "";
	ChLabel[16] = "Linac";       ChType[16] = "Beam";
	ChLabel[17] = "TOTAL";       ChType[17] = "";
	// Ql                // Qs                // PSD                     // // Ql-PSD
	xlbin_tot = 6600;    xsbin_tot = 3280;    xpbin_tot = 512;           // x2lbin_tot = 37000;   ybin_tot = 512;
	xl_min = 0.0;        xs_min = 0.0;        xp_min = 0.0;              // x2l_min = 0.0;        y_min = 0.0;
	xl_max = 66000;      xs_max = 23800;      xp_max = 1.0;              // x2l_max = 37000;      y_max = 1.0;

	// Qs-Ql                      // Ql-PSD                      // Ql-Tail
	Double_t x2sbin_tot = 3280;   Double_t x2lbin_tot = 35000;   Double_t x3lbin_tot = 700;
	Double_t x2s_min = 0.0;       Double_t x2l_min = 0.0;        Double_t x3l_min = 0.0;
	Double_t x2s_max = 32800;     Double_t x2l_max = 35000;      Double_t x3l_max = 35000;
	Double_t y2lbin_tot = 35000;  Double_t ybin_tot = 512;       Double_t y2bin_tot = 700;
	Double_t y2l_min = 0.0;       Double_t y_min = 0.0;          Double_t y2_min = 0.0;
	Double_t y2l_max = 3500;      Double_t y_max = 1.0;          Double_t y2_max = 35000;
								  Double_t z_min = 0.0;
								  Double_t z_max = 500; // 500.0; 10;
	
	TH1D *hInterval[MAXnumChannel], *h_Ql[MAXnumChannel], *h_Qs[MAXnumChannel], *h_PSD[MAXnumChannel], *h_PSD_FIT[ThSTE][MAXnumChannel];
	TH2D *h_QsQl[MAXnumChannel], *h_QlPSD[MAXnumChannel], *h_QlTail[MAXnumChannel];
	
	Double_t Color[MAXRunNumber] = {kBlack, kGreen+2, kGray, kBlue-9, kCyan, kCyan-9, kViolet-9, kGray+2, kMagenta+2, kMagenta-9,
									kMagenta, kOrange+2, kOrange-9, kOrange};
	TString Label[MAXRunNumber] = {"2011", "2016", "2018", "2019", "2020", "2021", "2022", "2023", "2024", "2025",
								   "2026", "2027", "2028", "2029"};
/*	Double_t global_gMIN[MAXRunNumber] = {0.10, 0.10, 0.08, 0.06, 0.06, 0.10, 0.10, 0.10, 0.10, 0.08,
										  0.10, 0.10, 0.10, 0.09};
	Double_t global_gMAX[MAXRunNumber] = {0.36, 0.26, 0.21, 0.20, 0.19, 0.25, 0.25, 0.24, 0.24, 0.20,
										  0.23, 0.26, 0.27, 0.22};
	Double_t global_nMIN[MAXRunNumber] = {0.37, 0.27, 0.23, 0.21, 0.20, 0.26, 0.26, 0.25, 0.25, 0.21,
										  0.25, 0.28, 0.29, 0.23};
	Double_t global_nMAX[MAXRunNumber] = {0.52, 0.50, 0.44, 0.42, 0.40, 0.46, 0.44, 0.44, 0.44, 0.40,
										  0.42, 0.48, 0.50, 0.42};*/
	Double_t global_gMIN[MAXRunNumber] = {0.22, 0.14, 0.10, 0.08, 0.07, 0.12, 0.12, 0.10, 0.10, 0.10,
										  0.12, 0.12, 0.12, 0.11};
	Double_t global_gMAX[MAXRunNumber] = {0.32, 0.22, 0.18, 0.16, 0.15, 0.20, 0.18, 0.18, 0.20, 0.18,
										  0.22, 0.24, 0.24, 0.20};
	Double_t global_nMIN[MAXRunNumber] = {0.36, 0.30, 0.24, 0.22, 0.21, 0.28, 0.26, 0.24, 0.26, 0.22,
										  0.27, 0.28, 0.30, 0.24};
	Double_t global_nMAX[MAXRunNumber] = {0.48, 0.40, 0.36, 0.34, 0.30, 0.40, 0.40, 0.40, 0.38, 0.34,
										  0.38, 0.42, 0.44, 0.36};
/*
	Double_t global_gMIN[MAXRunNumber] = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
										  0.01, 0.01, 0.01, 0.01};
	Double_t global_gMAX[MAXRunNumber] = {0.35, 0.25, 0.20, 0.18, 0.16, 0.22, 0.20, 0.20, 0.20, 0.20,
										  0.20, 0.25, 0.24, 0.20};
	Double_t global_nMIN[MAXRunNumber] = {0.35, 0.25, 0.20, 0.18, 0.16, 0.22, 0.20, 0.20, 0.20, 0.20,
										  0.20, 0.25, 0.24, 0.20};
	Double_t global_nMAX[MAXRunNumber] = {0.60, 0.60, 0.60, 0.60, 0.60, 0.60, 0.60, 0.60, 0.60, 0.60,
										  0.60, 0.60, 0.60, 0.60};
*/
	
	ifstream finMemo;
	TString FNameMemo = Form("%s%s/Th%04.0f/2025_RunInfo.txt", MainDir.Data(), RootDir.Data(), Ql_MIN); // "ROOT/Th0100/2025_RunInfo.txt";
	Int_t CountRun = 0;
	
	Int_t temp1, RunNum[MAXRunNumber], temp3, inputQl[MAXRunNumber], temp4, inputQs[MAXRunNumber], inQl, inQs;
	Double_t temp2, MeasTime[MAXRunNumber], tmeas;
	
	finMemo.open(FNameMemo);
	if (PrintReadingMemoON == 1) {cout << Form("#   Run#   MeasTime [s]   input Qs   input Ql") << endl;}
	while(finMemo >> temp1 >> temp2 >> temp3 >> temp4)
	{
		RunNum[CountRun] = temp1;
		MeasTime[CountRun] = temp2;
		inputQs[CountRun] = temp3;
		inputQl[CountRun] = temp4;
		if (PrintReadingMemoON == 1) {cout << Form("%2d  %4d  %10.2f  %10d  %10d", CountRun, RunNum[CountRun], MeasTime[CountRun], inputQs[CountRun], inputQl[CountRun]) << endl;}
		CountRun++;
	}
	if (PrintReadingMemoON == 1) {cout << Form("The number of runs done in %4d are %2d", EXPyear, CountRun) << endl << endl;}
	
	TString RunLabel;
	for (int i=0; i<MAXRunNumber; i++)
	{
		if (RunNumber == RunNum[i])
		{
			RunColor = Color[i];
			RunLabel = Label[i];
			tmeas = MeasTime[i];
			inQs = inputQs[i];
			inQl = inputQl[i];
//			if (PrintInpFileON == 1) {cout << Form("Run#: %5d  Color: %d  Label: %s", RunNumber, RunColor, RunLabel.Data()) << endl << endl;}
		}
	}
	
	cout << "Create the histograms for the following detectors:" << endl;
	for (int i=0; i<MAXnumChannel; i++)
	{
		TString currentType = ChType[i];
		if (currentType != ("")) // (ChLabel[i] != "")
		{
			cout << Form("ch%02d: %s",i,ChLabel[i].Data()) << endl;
			hInterval[i] = new TH1D(Form("hInt_ch%02d_%s",i,ChLabel[i].Data()),Form("hInt_ch%02d_%s",i,ChLabel[i].Data()),tbin_tot,t_min,t_max);
			h_Ql[i] = new TH1D(Form("hQl_ch%02d_%s",i,ChLabel[i].Data()),Form("hQl_ch%02d_%s",i,ChLabel[i].Data()),xlbin_tot,xl_min,xl_max);
			h_Qs[i] = new TH1D(Form("hQs_ch%02d_%s",i,ChLabel[i].Data()),Form("hQs_ch%02d_%s",i,ChLabel[i].Data()),xsbin_tot,xs_min,xs_max);
			h_PSD[i] = new TH1D(Form("hPSD_ch%02d_%s",i,ChLabel[i].Data()),Form("hPSD_ch%02d_%s",i,ChLabel[i].Data()),xpbin_tot,xp_min,xp_max);
			h_QsQl[i] = new TH2D(Form("hQsQl_ch%02d_%s",i,ChLabel[i].Data()),Form("hQsQl_ch%02d_%s",i,ChLabel[i].Data()),x2sbin_tot,x2s_min,x2s_max,y2lbin_tot,y2l_min,y2l_max);
			h_QlPSD[i] = new TH2D(Form("hQlPSD_ch%02d_%s",i,ChLabel[i].Data()),Form("hQlPSD_ch%02d_%s",i,ChLabel[i].Data()),x2lbin_tot,x2l_min,x2l_max,ybin_tot,y_min,y_max);
			h_QlTail[i] = new TH2D(Form("hQlTail_ch%02d_%s",i,ChLabel[i].Data()),Form("hQlTail_ch%02d_%s",i,ChLabel[i].Data()),x3lbin_tot,x3l_min,x3l_max,y2bin_tot,y2_min,y2_max);
		}
	}
	for (int i=0; i<ThSTE; i++)
	{
		for (int j=0; j<MAXnumChannel; j++)
		{
			h_PSD_FIT[i][j] = new TH1D(Form("hPSD%02d_ch%02d_%s",i,j,ChLabel[j].Data()),Form("hPSD%02d_ch%02d_%s",i,j,ChLabel[j].Data()),xpbin_tot,xp_min,xp_max);
		}
	}
	
	Int_t tinterval;
	Double_t qlong, qshort, pulseshape, pulseshapeerror, tail;
	Double_t QlongMIN[MAXnumChannel][ThSTE], QlongMAX[MAXnumChannel][ThSTE], QlongErrMAX[ThSTE];
	
	for (int i=0; i<all_event; i++)
	{
		tData->GetEntry(i);
		if (ql > Ql_MIN && ql < Ql_MAX)
		{
			Double_t timeoflight = tof*ns_to_us - shift;
			array_noe[MAXnumChannel]++;
			for (int j=0; j<MAXnumChannel; j++)
			{
				if (ch == j)
				{
					array_noe[j]++;
					tinterval = tof - array_old_tof[j];
					hInterval[j]->Fill(tinterval);
					array_old_tof[j] = tof;
					qlong = ql;
					qshort = qs;
					tail = qlong - qshort;
					pulseshape = psd;
					pulseshapeerror = sqrt(psd);
					h_Ql[j]->Fill(qlong);
					h_Qs[j]->Fill(qshort);
					h_PSD[j]->Fill(pulseshape);
					h_QsQl[j]->Fill(qshort, qlong);
					h_QlPSD[j]->Fill(qlong, pulseshape);
					h_QlTail[j]->Fill(qlong, tail);
				}
			}
		}
	}
	
	for (int k=0; k<MAXnumChannel; k++)
	{
		TString currentType = ChType[k];
		for (int i=0; i<ThSTE; i++) // Cycle for doing multiple PSD with different gates. This is used for the poly fitting
		{
			if (currentType.Contains("Plastic")) //(k >= 1 && k <= 9)
			{
				QlongMIN[k][i] = Ql_MIN + ((i)*((ThMAX-ThMIN)/ThSTE));
				QlongMAX[k][i] = Ql_MIN + ((i+1)*((ThMAX-ThMIN)/ThSTE));
				if (PrintReadingON == 1) {cout << Form("ch%02d - Qlong: %.0f %.0f", k, QlongMIN[k][i], QlongMAX[k][i]) << endl;}
			}
			else if (currentType.Contains("Li6")) // (k == 0 || k >= 10)
			{
				QlongMIN[k][i] = ThMIN_EJ270 + ((i)*((ThMAX-(ThMIN_EJ270))/ThSTE));
				QlongMAX[k][i] = ThMIN_EJ270 + ((i+1)*((ThMAX-(ThMIN_EJ270))/ThSTE));
				if (PrintReadingON == 1) {cout << Form("ch%02d - Qlong: %.0f %.0f", k, QlongMIN[k][i], QlongMAX[k][i]) << endl;}
			}
			for (int j=0; j<all_event; j++)
			{
				tData->GetEntry(j);
				if ((ch == k) && (ql > QlongMIN[k][i]) && (ql < QlongMAX[k][i]))
				{
					pulseshape = psd;
					pulseshapeerror = sqrt(psd);
					h_PSD_FIT[i][k]->Fill(pulseshape);
				}
			}
		}
	}
	
	TString ofile1D = Form("%s%s/Th%04.0f/TH1D_%04d.root", MainDir.Data(), RootDir.Data(), Ql_MIN, RunNumber);
	TString ofile2D = Form("%s%s/Th%04.0f/TH2D_%04d.root", MainDir.Data(), RootDir.Data(), Ql_MIN, RunNumber);
/*	TString ofileFIT = Form("%s%s/Th%04.0f/PSD_TH2D_%04d.root", MainDir.Data(), RootDir.Data(), Ql_MIN, RunNumber);
	TFile *foutFIT = new TFile(ofileFIT,"recreate");*/
	TF1 *gauG[ThSTE][MAXnumChannel], *gauN[ThSTE][MAXnumChannel], *polG[ThSTE][MAXnumChannel], *polN[ThSTE][MAXnumChannel], *g1[ThSTE][MAXnumChannel], *n1[ThSTE][MAXnumChannel], *total[ThSTE][MAXnumChannel], *n_GS20[MAXnumChannel];
	Double_t par1[ThSTE][MAXnumChannel][MAXpar], par2[ThSTE][MAXnumChannel][MAXpar], par[ThSTE][MAXnumChannel][MAXparameter];
	Double_t g1MIN[MAXnumChannel][ThSTE] = {
		{0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08},  // mini_GS20
		{0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15},  // H01
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H02
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H03
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14},  // H04
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H05
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14},  // H06
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H07
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H08
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H09
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // H10
		{0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08},  // EJ-270
		{0.08, 0.08, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.10, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12, 0.12},  // Layer_GS20
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // BF3
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // 
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // 
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18}   // Linac
		};
		
	Double_t g1MAX[MAXnumChannel][ThSTE] = {
		{0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24},  // mini_GS20
		{0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24, 0.24},  // H01
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H02
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H03
		{0.30, 0.26, 0.24, 0.24, 0.24, 0.22, 0.22, 0.21, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20},  // H04
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H05
		{0.30, 0.26, 0.24, 0.24, 0.24, 0.22, 0.22, 0.21, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20},  // H06
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H07
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H08
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H09
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // H10
		{0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // EJ-270
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // Layer_GS20
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // BF3
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // 
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18},  // 
		{0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.20, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18, 0.18}   // Linac
		};
	
	Double_t n1MIN[MAXnumChannel][ThSTE] = {
		{0.40, 16000, 0.0, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // mini_GS20
		{0.28, 0.28, 0.28, 0.28, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26, 0.28, 0.28, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26},  // H01
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H02
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H03
		{0.32, 0.30, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26, 0.25, 0.25, 0.24, 0.24, 0.24, 0.24},  // H04
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H05
		{0.32, 0.30, 0.28, 0.28, 0.28, 0.28, 0.28, 0.28, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26, 0.26, 0.25, 0.24, 0.24, 0.24, 0.24},  // H06
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H07
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H08
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H09
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // H10
		{0.22,  5500, 0.0, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // EJ-270
		{0.40, 12000, 0.0, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // Layer_GS20
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // BF3
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // 
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22},  // 
		{0.25, 0.25, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22, 0.22}   // Linac
		};
	
	Double_t n1MAX[MAXnumChannel][ThSTE] = {
		{0.54, 20000, 0.0, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // mini_GS20
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H01
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H02
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H03
		{0.42, 0.42, 0.42, 0.42, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.36, 0.36, 0.36, 0.36, 0.36, 0.36, 0.36, 0.36},  // H04
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H05
		{0.42, 0.42, 0.42, 0.42, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.36, 0.36, 0.36, 0.36, 0.36, 0.36, 0.36, 0.36},  // H06
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H07
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H08
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H09
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // H10
		{0.34,  9000, 0.0, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // EJ-270
		{0.54, 18000, 0.0, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00},  // Layer_GS20
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // BF3
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // 
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38},  // 
		{0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38, 0.38}   // Linac
		}; 
	
	for (int i=0; i<MAXnumChannel+1; i++)
	{
		TString currentType = ChType[i];
		if (currentType != "") // (ChLabel[i] != "") // (i >= 0 && i <= 1)
		{
			if (PrintOutFileON == 1)
			{
				cout << Form("OUTPUT file for TH1D: %s", ofile1D.Data()) << endl;
				cout << Form("OUTPUT file for TH2D: %s", ofile2D.Data()) << endl;
//				cout << Form("OUTPUT file for Poly Fitting: %s", ofileFIT.Data()) << endl;
				cout << Form("NOE for channel %02d: ", i) << array_noe[i] << endl;
			}
		}
	}
	if (PrintOutFileON == 1) {cout << Form("NOE above Ql=%.0f : ", Ql_MIN) << array_noe[MAXnumChannel] << endl;}
	if (SaveNOEON == 1)
	{
		FILE *fout_ascii;
		TString FName_ascii = Form("%s%s/Th%04.0f/NOE_%04d.out", MainDir.Data(), RootDir.Data(), Ql_MIN, RunNumber);
		if ((fout_ascii=fopen(FName_ascii.Data(),"w")) == NULL)
		{
			cerr << Form("Can't create %s", FName_ascii.Data());
			exit(1);
		}
		
		for (int i=0; i<MAXnumChannel+1; i++)
		{
			if (i!=0)
			{
				fprintf(fout_ascii," \t");
			}
			fprintf(fout_ascii, "%d", array_noe[i]);
		}
		fclose(fout_ascii);
	}
	
	Double_t psdMIN, psdMAX;
	psdMIN = 0.05; psdMAX = 1.00;
	
	TCanvas *canvInt[MAXnumChannel], *canvQl[MAXnumChannel], *canvQs[MAXnumChannel], *canvPSD[MAXnumChannel], *canvPSD_FIT[ThSTE][MAXnumChannel], *canvQsQl[MAXnumChannel], *canvQlPSD[MAXnumChannel], *canvQlTail[MAXnumChannel];
	TLegend *legInt[MAXnumChannel], *legQl[MAXnumChannel], *legQs[MAXnumChannel], *legPSD[MAXnumChannel], *legPSD_FIT[ThSTE][MAXnumChannel], *legQsQl[MAXnumChannel], *legQlPSD[MAXnumChannel], *legQlTail[MAXnumChannel];
	
	Double_t sigmaG[ThSTE][MAXnumChannel][MAXvalue], AmpG[ThSTE][MAXnumChannel][MAXvalue], chi[ThSTE][MAXnumChannel], xG[ThSTE][MAXnumChannel][MAXvalue], PSDgMAX[ThSTE][MAXnumChannel][MAXvalue][MaxSCV], FWHMg[ThSTE][MAXnumChannel][MAXvalue], FWHMgMAX[ThSTE][MAXnumChannel][MAXvalue];
	Double_t sigmaN[ThSTE][MAXnumChannel][MAXvalue], AmpN[ThSTE][MAXnumChannel][MAXvalue], xN[ThSTE][MAXnumChannel][MAXvalue], PSDnMIN[ThSTE][MAXnumChannel][MAXvalue][MaxSCV], PSDnMAX[ThSTE][MAXnumChannel][MAXvalue][MaxSCV], FWHMn[ThSTE][MAXnumChannel][MAXvalue], FWHMnMAX[ThSTE][MAXnumChannel][MAXvalue], QlnMIN[ThSTE][MAXnumChannel][MAXvalue][MaxSCV], QlnMAX[ThSTE][MAXnumChannel][MAXvalue][MaxSCV];
	Double_t S[ThSTE][MAXnumChannel][MAXvalue], FOM[ThSTE][MAXnumChannel][MAXvalue];
	
	TGraphErrors *grG[MAXnumChannel][MaxSCV], *grG1[MAXnumChannel], *grN[MAXnumChannel][MaxSCV], *grN1[MAXnumChannel];
	
	Double_t gSCV[MaxSCV] = {2, 3, 4}; // Sigma confidence value (99.7% & 99.9937%)
	Double_t nSCV[MaxSCV] = {2, 3, 4}; // Sigma confidence value (99.7% & 99.9937%)
	FILE *fout_FOM;
	TString FName_FOM = Form("%s%s/Th%04.0f/FOM_%04d.out", MainDir.Data(), RootDir.Data(), Ql_MIN, RunNumber);
	if ((fout_FOM=fopen(FName_FOM.Data(),"w")) == NULL)
	{
		cerr << Form("Can't create %s", FName_FOM.Data());
		exit(1);
	}
	
	TFile *fout1D = new TFile(ofile1D,"recreate");
	for (int i=0; i<MAXnumChannel; i++)
	{
		TString currentType = ChType[i];
		if (currentType != "") // (ChLabel[i] != "") // (i >= 0 && i <= 1)
		{
			legInt[i] = new TLegend(0.75, 0.85, 0.99, 0.99);
			legInt[i]->SetMargin(0.05);
			canvInt[i] = new TCanvas(Form("Int_%04d_%s", RunNumber, ChLabel[i].Data()), Form("Int_%04d_%s", RunNumber, ChLabel[i].Data()), 800, 400);
			canvInt[i]->SetMargin(0.05,0.025,0.10,0.05);
			canvInt[i]->SetGridx(); 
			canvInt[i]->SetGridy();
			hInterval[i]->GetXaxis()->CenterTitle();
			hInterval[i]->GetXaxis()->SetTitle("TOF [ns]");
			hInterval[i]->GetXaxis()->SetTitleOffset(1.20);
			hInterval[i]->GetYaxis()->CenterTitle();
			hInterval[i]->GetYaxis()->SetTitle("");
			hInterval[i]->GetYaxis()->SetTitleOffset(1.20);
			hInterval[i]->SetLineColor(RunColor);
			hInterval[i]->Draw("hist");
			legInt[i]->SetHeader(Form("Interval %04d %s", RunNumber, ChLabel[i].Data()),"c");
//			legInt[i]->AddEntry(hInterval[i],Form("%04d %s", RunNumber, ChLabel[i].Data()),"l");
			legInt[i]->Draw();
			
			legQl[i] = new TLegend(0.60, 0.80, 0.99, 0.99);
			legQl[i]->SetMargin(0.05);
			canvQl[i] = new TCanvas(Form("Ql_%04d_%s", RunNumber, ChLabel[i].Data()), Form("Ql_%04d_%s", RunNumber, ChLabel[i].Data()), 800, 600);
			canvQl[i]->SetMargin(0.10,0.025,0.10,0.05); // left, right, bottom, top
			canvQl[i]->SetGridy();
			canvQl[i]->SetLogy();
			h_Ql[i]->GetXaxis()->CenterTitle();
			h_Ql[i]->GetXaxis()->SetTitle("");
			h_Ql[i]->GetXaxis()->SetTitleOffset(1.20);
			h_Ql[i]->GetYaxis()->CenterTitle();
			h_Ql[i]->GetYaxis()->SetTitle("");
			h_Ql[i]->GetYaxis()->SetTitleOffset(1.20);
			h_Ql[i]->SetLineColor(RunColor);
			h_Ql[i]->SetLineWidth(3);
			h_Ql[i]->Draw("hist");
			legQl[i]->SetHeader(Form("Q_{L} %04d %s", RunNumber, ChLabel[i].Data()),"c");
//			legQl[i]->AddEntry(h_Ql[i],Form("%04d %s", RunNumber, ChLabel[i].Data()),"l");
			legQl[i]->Draw();
			
			legQs[i] = new TLegend(0.75, 0.85, 0.99, 0.99);
			legQs[i]->SetMargin(0.05);
			canvQs[i] = new TCanvas(Form("Qs_%04d_%s", RunNumber, ChLabel[i].Data()), Form("Qs_%04d_%s", RunNumber, ChLabel[i].Data()), 800, 600);
			canvQs[i]->SetMargin(0.10,0.025,0.10,0.05); // left, right, bottom, top
			canvQs[i]->SetGridy();
			canvQs[i]->SetLogy();
			h_Qs[i]->GetXaxis()->CenterTitle();
			h_Qs[i]->GetXaxis()->SetTitle("");
			h_Qs[i]->GetXaxis()->SetTitleOffset(1.20);
			h_Qs[i]->GetYaxis()->CenterTitle();
			h_Qs[i]->GetYaxis()->SetTitle("");
			h_Qs[i]->GetYaxis()->SetTitleOffset(1.20);
			h_Qs[i]->SetLineColor(RunColor);
			h_Qs[i]->SetLineWidth(3);
			h_Qs[i]->Draw("hist");
			legQs[i]->SetHeader(Form("Q_{S} %04d %s", RunNumber, ChLabel[i].Data()),"c");
//			legQs[i]->AddEntry(h_Qs[i],Form("%04d %s", RunNumber, ChLabel[i].Data()),"l");
			legQs[i]->Draw();
			
			legPSD[i] = new TLegend(0.75, 0.85, 0.99, 0.99);
			legPSD[i]->SetMargin(0.05);
			canvPSD[i] = new TCanvas(Form("PSD_%04d_%s", RunNumber, ChLabel[i].Data()), Form("PSD_%04d_%s", RunNumber, ChLabel[i].Data()), 800, 600);
			canvPSD[i]->SetMargin(0.10,0.025,0.10,0.05); // left, right, bottom, top
			canvPSD[i]->SetGridx();
			canvPSD[i]->SetGridy();
			h_PSD[i]->GetXaxis()->CenterTitle();
			h_PSD[i]->GetXaxis()->SetTitle("");
			h_PSD[i]->GetXaxis()->SetTitleOffset(1.20);
			h_PSD[i]->GetXaxis()->SetRangeUser(psdMIN, psdMAX);
			h_PSD[i]->GetYaxis()->CenterTitle();
			h_PSD[i]->GetYaxis()->SetTitle("");
			h_PSD[i]->GetYaxis()->SetTitleOffset(1.20);
			h_PSD[i]->SetLineColor(RunColor);
			h_PSD[i]->SetLineWidth(3);
			h_PSD[i]->Draw("hist");
			legPSD[i]->SetHeader(Form("PSD %04d %s", RunNumber, ChLabel[i].Data()),"c");
//			legPSD[i]->AddEntry(h_Qs[i],Form("%04d %s", RunNumber, ChLabel[i].Data()),"l");
			legPSD[i]->Draw();
			
			if ((currentType.Contains("Plastic") && (CalculateFOM == 1))
			{
				for (int j=0; j<MAXRunNumber; j++)
				{
					if (RunNumber == RunNum[j])
					{
						CountRun = j;
					}
				}
				gauG[0][i] = new TF1(Form("gausG_%04d_g%03d", RunNumber, i), "gaus", global_gMIN[CountRun], global_gMAX[CountRun]);
				gauN[0][i] = new TF1(Form("gausN_%04d_g%03d", RunNumber, i), "gaus", global_nMIN[CountRun], global_nMAX[CountRun]);
				gauG[0][i]->SetLineColor(kRed);
				gauN[0][i]->SetLineColor(kRed);
				h_PSD[i]->Fit(gauG[0][i],"QR");
				h_PSD[i]->Fit(gauN[0][i],"QR+");
				gauG[0][i]->Draw("same");
				gauN[0][i]->Draw("same");
				gauG[0][i]->GetParameters(&par[0][i][0]);
				gauN[0][i]->GetParameters(&par[0][i][3]);
				
				total[0][i] = new TF1(Form("Double_%04d_g%03d", RunNumber, i), "gaus(0)+gaus(3)", 0.01, 0.6); // 0.01, 0.6 // global_gMIN[CountRun], global_nMAX[CountRun]
				total[0][i]->SetLineColor(kBlue);
				total[0][i]->SetLineWidth(2);
				total[0][i]->SetParameters(par[0][i]);
				total[0][i]->FixParameter(1, par[0][i][1]);
				total[0][i]->FixParameter(4, par[0][i][4]);
				h_PSD[i]->Fit(total[0][i],"R+");
				total[0][i]->Draw("same");
				
				chi[0][i] = total[0][i]->GetChisquare();
				AmpG[0][i][0] = total[0][i]->GetParameter(0);
				AmpG[0][i][1] = total[0][i]->GetParError(0);
				xG[0][i][0] = total[0][i]->GetParameter(1);
				xG[0][i][1] = total[0][i]->GetParError(1);
				sigmaG[0][i][0] = total[0][i]->GetParameter(2);
				sigmaG[0][i][1] = total[0][i]->GetParError(2);
				AmpN[0][i][0] = total[0][i]->GetParameter(3);
				AmpN[0][i][1] = total[0][i]->GetParError(3);
				xN[0][i][0] = total[0][i]->GetParameter(4);
				xN[0][i][1] = total[0][i]->GetParError(4);
				sigmaN[0][i][0] = total[0][i]->GetParameter(5);
				sigmaN[0][i][1] = total[0][i]->GetParError(5);
				
				S[0][i][0] = xN[0][i][0]-xG[0][i][0];
				S[0][i][1] = sqrt(pow(xN[0][i][1],2) + pow(xG[0][i][1],2));
				FWHMg[0][i][0] = 2 * sqrt(2*log(2)) * sigmaG[0][i][0];
				FWHMg[0][i][1] = 2 * sqrt(2*log(2)) * sigmaG[0][i][1];
				FWHMn[0][i][0] = 2 * sqrt(2*log(2)) * sigmaN[0][i][0];
				FWHMn[0][i][1] = 2 * sqrt(2*log(2)) * sigmaN[0][i][1];
				FOM[0][i][0] = S[0][i][0] / (FWHMg[0][i][0] + FWHMn[0][i][0]);
				FOM[0][i][1] = FOM[0][i][0] * sqrt(pow(S[0][i][1] / S[0][i][0],2) + ((pow(FWHMg[0][i][1],2) + pow(FWHMn[0][i][1],2)) / (pow(FWHMg[0][i][0]+FWHMn[0][i][0],2))));
				if (PrintFOMResultON == 1) {cout << Form("S = %.2f   FOM = %.2f", S[0][i][0], FOM[0][i][0]) << endl;}
				
				if (i!=0)
				{
					fprintf(fout_FOM," \t");
				}
				fprintf(fout_FOM, "%4d %2d %10.3f %5d %5d %15.5e %15.5e %15.5e %15.5e %15.5e %15.5e %15.5e %15.5e %15.5e\n", RunNumber, i, tmeas, inQs, inQl, S[0][i][0], S[0][i][1], FWHMg[0][i][0], FWHMg[0][i][1], FWHMn[0][i][0], FWHMn[0][i][1], FOM[0][i][0], FOM[0][i][1], chi[0][i]);
			}
			
			if (currentType.Contains("Li6")) //(i == 0 || i == 11 || i == 12) // mini_GS20, EJ-270, Layer_GS20
			{
				canvPSD[i]->cd();
				n_GS20[i] = new TF1(Form("%04d_g%03d", RunNumber, i), "gaus", n1MIN[i][0], n1MAX[i][0]);
				n_GS20[i]->SetLineColor(kRed);
				n_GS20[i]->SetLineWidth(2);
				h_PSD[i]->Fit(n_GS20[i],"QR");
				AmpN[0][i][0] = n_GS20[i]->GetParameter(0);
				AmpN[0][i][1] = n_GS20[i]->GetParError(0);
				xN[0][i][0] = n_GS20[i]->GetParameter(1);
				xN[0][i][1] = n_GS20[i]->GetParError(1);
				sigmaN[0][i][0] = n_GS20[i]->GetParameter(2);
				sigmaN[0][i][1] = n_GS20[i]->GetParError(2);
				FWHMn[0][i][0] = 2.355 * sigmaN[0][i][0];
				FWHMn[0][i][1] = 2.355 * sigmaN[0][i][1];
				FWHMnMAX[0][i][0] = xN[0][i][0] + FWHMn[0][i][0] / 2;
				FWHMnMAX[0][i][1] = sqrt(pow(xN[0][i][1], 2) + pow(FWHMn[0][i][1] / 2, 2));
				n_GS20[i]->Draw("same");
				for (int k=0; k<MaxSCV; k++)
				{
					PSDnMAX[0][i][0][k] = xN[0][i][0] + nSCV[k]*sigmaN[0][i][0];
					PSDnMAX[0][i][1][k] = sqrt(pow(xN[0][i][1],2) + pow(nSCV[k]*sigmaN[0][i][1],2));
					PSDnMIN[0][i][0][k] = xN[0][i][0] - nSCV[k]*sigmaN[0][i][0];
					PSDnMIN[0][i][1][k] = sqrt(pow(xN[0][i][1],2) + pow(nSCV[k]*sigmaN[0][i][1],2));
					legPSD[i]->AddEntry((TObject*)0, Form("PSD_{n} = %.3f (%.0f#sigma = %.3f)", xN[0][i][0], nSCV[k], PSDnMAX[0][i][0][k]), "");
/*					legPSD[i]->AddEntry((TObject*)0, Form("PSD_{n} = %.3f", xN[0][i][0]), "");
					legPSD[i]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", nSCV[k], PSDnMAX[0][i][0][k]), "");
					legPSD[i]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", nSCV[k], FWHMnMAX[0][i][0]), "");*/
				}
				canvPSD[i]->Update();
				
				canvQl[i]->cd();
				n_GS20[i] = new TF1(Form("%04d_g%03d", RunNumber, i), "gaus", n1MIN[i][1], n1MAX[i][1]);
				n_GS20[i]->SetLineColor(kRed);
				n_GS20[i]->SetLineWidth(2);
				h_Ql[i]->Fit(n_GS20[i],"QR");
				legPSD[i]->AddEntry(h_PSD[i],Form("%04d %s", RunNumber, ChLabel[i].Data()),"l");
				AmpN[1][i][0] = n_GS20[i]->GetParameter(0);
				AmpN[1][i][1] = n_GS20[i]->GetParError(0);
				xN[1][i][0] = n_GS20[i]->GetParameter(1);
				xN[1][i][1] = n_GS20[i]->GetParError(1);
				sigmaN[1][i][0] = n_GS20[i]->GetParameter(2);
				sigmaN[1][i][1] = n_GS20[i]->GetParError(2);
				FWHMn[1][i][0] = 2.355 * sigmaN[1][i][0];
				FWHMn[1][i][1] = 2.355 * sigmaN[1][i][1];
				FWHMnMAX[1][i][0] = xN[1][i][0] + FWHMn[1][i][0]/2;
				FWHMnMAX[1][i][1] = sqrt(pow(xN[1][i][1], 2) + pow(FWHMn[1][i][1]/2, 2));
				n_GS20[i]->Draw("same");
				for (int k=0; k<MaxSCV; k++)
				{
					if (k == 0)
					{
						QlnMAX[1][i][0][k] = xN[1][i][0] + FWHMn[1][i][0]/2;
						QlnMAX[1][i][1][k] = sqrt(pow(xN[1][i][1], 2) + pow(FWHMn[1][i][1]/2, 2));
						QlnMIN[1][i][0][k] = xN[1][i][0] - FWHMn[1][i][0]/2;
						QlnMIN[1][i][1][k] = sqrt(pow(xN[1][i][1], 2) + pow(FWHMn[1][i][1]/2, 2));
						legQl[i]->AddEntry((TObject*)0, Form("Q_{L} = %.1f (FWHM: %.1f-%.1f)", xN[1][i][0], QlnMIN[1][i][0][k], QlnMAX[1][i][0][k]), "");
					}
					QlnMAX[0][i][0][k] = xN[1][i][0] + nSCV[k]*sigmaN[1][i][0];
					QlnMAX[0][i][1][k] = sqrt(pow(xN[1][i][1],2) + pow(nSCV[k]*sigmaN[0][i][1],2));
					QlnMIN[0][i][0][k] = xN[1][i][0] - nSCV[k]*sigmaN[1][i][0];
					QlnMIN[0][i][1][k] = sqrt(pow(xN[1][i][1],2) + pow(nSCV[k]*sigmaN[0][i][1],2));
					legQl[i]->AddEntry((TObject*)0, Form("Q_{L} = %.1f (%.0f#sigma: %.1f-%.1f)", xN[1][i][0], nSCV[k], QlnMIN[0][i][0][k], QlnMAX[0][i][0][k]), "");
/*					legQl[i]->AddEntry((TObject*)0, Form("Q_{L} = %.3f", xN[1][i][0]), "");
					legQl[i]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", nSCV[k], QlnMAX[0][i][0][k]), "");
					legQl[i]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", nSCV[k], FWHMnMAX[1][i][0]), "");*/
				}
				canvQl[i]->Update();
			}
			
			hInterval[i]->Write();
			h_Ql[i]->Write();
			h_Qs[i]->Write();
			h_PSD[i]->Write();
			
			if (SaveFigureON == 1)
			{
				canvInt[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_Int.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvQl[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_Ql.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvQs[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_Qs.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvPSD[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_PSD.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
			}
			
			if (DrawChoice == 0 || DrawChoice == 1)
			{
//				canvQl[i]->Close();
				canvQs[i]->Close();
			}
			if (DrawChoice == 0)
			{
				canvPSD[i]->Close();
				canvQl[i]->Close();
			}
		}
	}
	fclose(fout_FOM);
	fout1D->Close();
	
	for (int i=0; i<ThSTE; i++)
	{
		for (int j=0; j<MAXnumChannel; j++)
		{
			TString currentType = ChType[j];
			if (currentType.Contains("Plastic")) //(j == 4 || j == 6) // (ChLabel[j] != "") (j >= 1 && j <= 6) // j == 4 || j == 6 || j == 11
			{
				legPSD_FIT[i][j] = new TLegend(0.75, 0.80, 0.99, 0.99);
				legPSD_FIT[i][j]->SetMargin(0.15);
				canvPSD_FIT[i][j] = new TCanvas(Form("PSD%02d_%04d_%s", i, RunNumber, ChLabel[j].Data()), Form("PSD%02d_%04d_%s", i, RunNumber, ChLabel[j].Data()), 800, 600);
				canvPSD_FIT[i][j]->SetMargin(0.10, 0.025, 0.10, 0.05); // left, right, bottom, top
				canvPSD_FIT[i][j]->SetGridx(); canvPSD_FIT[i][j]->SetGridy();
				h_PSD_FIT[i][j]->GetXaxis()->CenterTitle();
				h_PSD_FIT[i][j]->GetXaxis()->SetTitle("PSD");
				h_PSD_FIT[i][j]->GetXaxis()->SetTitleOffset(1.20);
				h_PSD_FIT[i][j]->GetXaxis()->SetRangeUser(psdMIN, psdMAX);
				h_PSD_FIT[i][j]->GetYaxis()->CenterTitle();
				h_PSD_FIT[i][j]->GetYaxis()->SetTitle("");
				h_PSD_FIT[i][j]->GetYaxis()->SetTitleOffset(1.20);
				h_PSD_FIT[i][j]->SetLineColor(kBlack);
				h_PSD_FIT[i][j]->SetLineWidth(3);
				
				legPSD_FIT[i][j]->SetHeader(Form("PSD [%.0f ~ %.0f]", QlongMIN[j][i], QlongMAX[j][i]),"c");
				g1[i][j] = new TF1(Form("%04d_g%03d", RunNumber, i), "gaus", g1MIN[j][i], g1MAX[j][i]);
				g1[i][j]->SetLineColor(kRed);
				g1[i][j]->SetLineWidth(2);
				h_PSD_FIT[i][j]->Fit(g1[i][j],"QR");
				n1[i][j] = new TF1(Form("%04d_g%03d", RunNumber, i), "gaus", n1MIN[j][i], n1MAX[j][i]);
				n1[i][j]->SetLineColor(kRed);
				n1[i][j]->SetLineWidth(2);
				h_PSD_FIT[i][j]->Fit(n1[i][j],"QR");
				legPSD_FIT[i][j]->AddEntry(h_PSD_FIT[i][j],Form("%04d %s", RunNumber, ChLabel[j].Data()),"l");
				
				AmpG[i][j][0] = g1[i][j]->GetParameter(0);
				AmpG[i][j][1] = g1[i][j]->GetParError(0);
				xG[i][j][0] = g1[i][j]->GetParameter(1);
				xG[i][j][1] = g1[i][j]->GetParError(1);
				sigmaG[i][j][0] = g1[i][j]->GetParameter(2);
				sigmaG[i][j][1] = g1[i][j]->GetParError(2);
				FWHMg[i][j][0] = 2.355 * sigmaG[i][j][0];
				FWHMg[i][j][1] = 2.355 * sigmaG[i][j][1];
				FWHMgMAX[i][j][0] = xG[i][j][0] + FWHMg[i][j][0] / 2;
				FWHMgMAX[i][j][1] = sqrt(pow(xG[i][j][1], 2) + pow(FWHMg[i][j][1] / 2, 2));
				
				AmpN[i][j][0] = n1[i][j]->GetParameter(0);
				AmpN[i][j][1] = n1[i][j]->GetParError(0);
				xN[i][j][0] = n1[i][j]->GetParameter(1);
				xN[i][j][1] = n1[i][j]->GetParError(1);
				sigmaN[i][j][0] = n1[i][j]->GetParameter(2);
				sigmaN[i][j][1] = n1[i][j]->GetParError(2);
				FWHMn[i][j][0] = 2.355 * sigmaN[i][j][0];
				FWHMn[i][j][1] = 2.355 * sigmaN[i][j][1];
				FWHMnMAX[i][j][0] = xN[i][j][0] + FWHMn[i][j][0] / 2;
				FWHMnMAX[i][j][1] = sqrt(pow(xN[i][j][1], 2) + pow(FWHMn[i][j][1] / 2, 2));
				
				for (int k=0; k<MaxSCV; k++)
				{
					PSDgMAX[i][j][0][k] = xG[i][j][0] + gSCV[k]*sigmaG[i][j][0];
					PSDgMAX[i][j][1][k] = sqrt(pow(xG[i][j][1],2) + pow(gSCV[k]*sigmaG[i][j][1],2));
					PSDnMAX[i][j][0][k] = xN[i][j][0] + nSCV[k]*sigmaN[i][j][0];
					PSDnMAX[i][j][1][k] = sqrt(pow(xN[i][j][1],2) + pow(nSCV[k]*sigmaN[i][j][1],2));
					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("PSD_{#gamma} = %.3f (%.0f#sigma = %.3f)", xG[i][j][0], gSCV[k], PSDgMAX[i][j][0][k]), "");
/*					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("PSD_{#gamma} = %.3f", xG[i][j][0]), "");
					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", gSCV[k], PSDgMAX[i][j][0][k]), "");
					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", gSCV[k], FWHMgMAX[i][j][0]), "");*/
					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("PSD_{n} = %.3f (%.0f#sigma = %.3f)", xN[i][j][0], nSCV[k], PSDnMAX[i][j][0][k]), "");
/*					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("PSD_{n} = %.3f", xN[i][j][0]), "");
					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", nSCV[k], PSDnMAX[i][j][0][k]), "");
					legPSD_FIT[i][j]->AddEntry((TObject*)0, Form("   (%.0f#sigma = %.3f)", nSCV[k], FWHMnMAX[i][j][0]), "");*/
					
					if (i == 0) {grG[j][k] = new TGraphErrors(ThSTE); grN[j][k] = new TGraphErrors(ThSTE);}
					grG[j][k]->SetPoint(i, QlongMAX[j][i], PSDgMAX[i][j][0][k]);
					grG[j][k]->SetPointError(i, QlongErrMAX[i], PSDgMAX[i][j][1][k]);
					grG[j][k]->SetPoint(i, QlongMAX[j][i], PSDgMAX[i][j][0][k]);
					grG[j][k]->SetPointError(i, QlongErrMAX[i], PSDgMAX[i][j][1][k]);
					grN[j][k]->SetPoint(i, QlongMAX[j][i], PSDnMAX[i][j][0][k]);
					grN[j][k]->SetPointError(i, QlongErrMAX[i], PSDnMAX[i][j][1][k]);
				}
				
				if (i == 0) {grG1[j] = new TGraphErrors(ThSTE); grN1[j] = new TGraphErrors(ThSTE);}
				grG1[j]->SetPoint(i, QlongMAX[j][i], FWHMgMAX[i][j][0]);
				grG1[j]->SetPointError(i, QlongErrMAX[i], FWHMgMAX[i][j][1]);
				grN1[j]->SetPoint(i, QlongMAX[j][i], FWHMnMAX[i][j][0]);
				grN1[j]->SetPointError(i, QlongErrMAX[i], FWHMnMAX[i][j][1]);
				
				if (DrawChoice == 1 || DrawChoice == 2)
				{
					h_PSD_FIT[i][j]->Draw("E"); // ("hist");
					g1[i][j]->Draw("same");
					n1[i][j]->Draw("same");
					legPSD_FIT[i][j]->Draw();
				}
				
//				h_PSD_FIT[i][j]->Write();
				
				if (SaveFigureON == 1)
				{
					canvPSD_FIT[i][j]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_PSD%02d.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[j].Data(), i));
				}
				
				if (DrawChoice == 0 || DrawChoice == 1)
				{
					canvPSD_FIT[i][j]->Close();
				}
			}
		}
	}
	
	TFile *fout2D = new TFile(ofile2D,"recreate");
	TF1 *p1G[MAXnumChannel][MaxSCV], *p1N[MAXnumChannel][MaxSCV];
	Double_t par0, par1, par2, par3, par4, par5, par6, par7;
	Double_t x_last = ThMAX, y_last;
	Int_t n_points[MaxSCV];
	Double_t x_val, y_val[MaxSCV];
	Int_t rn;
	
	TEllipse *EllipseN[MAXnumChannel][MaxSCV];
	Double_t x_c, y_c; // Center of the ellipse
	Double_t a, b; // Semi-Major and Semi-Minor Axes
	
	TTree *tFit = new TTree("tFit","tFit");
	tFit->Branch("rn",&rn,"rn/I"); // Run number
	tFit->Branch("ch",&ch,"ch/I"); // Channel number
	tFit->Branch("par0",&par0,"par0/D");
	tFit->Branch("par1",&par1,"par1/D");
	tFit->Branch("par2",&par2,"par2/D");
	tFit->Branch("par3",&par3,"par3/D");
	tFit->Branch("par4",&par4,"par4/D");
	tFit->Branch("par5",&par5,"par5/D");
	tFit->Branch("par6",&par6,"par6/D");
	tFit->Branch("par7",&par7,"par7/D"); // This is the linear
	tFit->Branch("x_c",&x_c,"x_c/D");
	tFit->Branch("y_c",&y_c,"y_c/D");
	tFit->Branch("a",&a,"a/D");
	tFit->Branch("b",&b,"b/D");
	
	for (int i=0; i<MAXnumChannel; i++)
	{
		TString currentType = ChType[i];
		if (currentType != "") // (ChLabel[i] != "") // (i >= 1 && i <= 6) // j == 4 || j == 6
		{
			legQsQl[i] = new TLegend(0.75, 0.90, 0.86, 0.99);
			legQsQl[i]->SetMargin(0.08);
			canvQsQl[i] = new TCanvas(Form("QsQl_%04d_%s", RunNumber, ChLabel[i].Data()), Form("QsQl%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvQsQl[i]->SetMargin(0.10,0.123,0.10,0.05); // left, right, bottom, top
			canvQsQl[i]->SetGridx(); canvQsQl[i]->SetGridy();
			h_QsQl[i]->GetXaxis()->CenterTitle();
			h_QsQl[i]->GetXaxis()->SetTitle("Q_{S}");
			h_QsQl[i]->GetXaxis()->SetRangeUser(0.,Ql_MAX);
			h_QsQl[i]->GetXaxis()->SetTitleOffset(1.20);
			h_QsQl[i]->GetYaxis()->CenterTitle();
			h_QsQl[i]->GetYaxis()->SetTitle("Q_{L}");
			h_QsQl[i]->GetYaxis()->SetTitleOffset(1.50);
			h_QsQl[i]->Draw("colz");
			legQsQl[i]->SetHeader(Form("%04d %s", RunNumber, ChLabel[i].Data()),"c");
			legQsQl[i]->Draw();
			
			legQlPSD[i] = new TLegend(0.30, 0.85, 0.99, 0.99); // legQlPSD[i] = new TLegend(0.75, 0.90, 0.86, 0.99);
			legQlPSD[i]->SetMargin(0.05); // legQlPSD[i]->SetMargin(0.08);
			canvQlPSD[i] = new TCanvas(Form("QlPSD_%04d_%s", RunNumber, ChLabel[i].Data()), Form("QlPSD_%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvQlPSD[i]->SetMargin(0.10,0.123,0.10,0.05); // left, right, bottom, top
			canvQlPSD[i]->SetGridx(); canvQlPSD[i]->SetGridy();
			h_QlPSD[i]->GetXaxis()->CenterTitle();
			h_QlPSD[i]->GetXaxis()->SetTitle("Q_{L}");
			h_QlPSD[i]->GetXaxis()->SetRangeUser(0.,Ql_MAX);
			h_QlPSD[i]->GetXaxis()->SetTitleOffset(1.20);
			h_QlPSD[i]->GetYaxis()->CenterTitle();
			h_QlPSD[i]->GetYaxis()->SetTitle("PSD");
			h_QlPSD[i]->GetYaxis()->SetTitleOffset(1.50);
			h_QlPSD[i]->SetMinimum(z_min); // Set the minimum value of the z scale
			h_QlPSD[i]->SetMaximum(z_max); // Set the maximum value of the z scale
			h_QlPSD[i]->Draw("colz");
			legQlPSD[i]->SetHeader(Form("%04d %s", RunNumber, ChLabel[i].Data()),"c");
			
			if (currentType.Contains("Plastic")) // (i == 4 || i == 6) // (i >= 1 && i <= 6) // i == 4 || i == 6 || i == 11
			{
				for (int k=0; k<MaxSCV; k++)
				{
					grG[i][k]->SetMarkerStyle(20);
					grG[i][k]->SetMarkerColor(kRed);
					grG[i][k]->Draw("P same");
/*					grN[i][k]->SetMarkerStyle(20);
					grN[i][k]->SetMarkerColor(kMagenta);
					grN[i][k]->Draw("P same");*/
					
					//p1G[i] = new TF1(Form("H%02d", i+1), "pol6", ThMIN, ThMAX); // Ql_MIN+0.5*ThMIN // "[0]*Exp(-1.0*[1]*x)+[2]"
					// Define a custom function that behaves like a polynomial up to x_last, then becomes constant
					// y_last[k] = grG[i][k]->Eval(x_last);
					n_points[k] = grG[i][k]->GetN();
					grG[i][k]->GetPoint(n_points[k] - 1, x_val, y_val[k]); // last point of grG[i]
					y_last = y_val[k]; // y-coordinate of the last point
					p1G[i][k] = new TF1(Form("H%02d_%d", i+1, k), [x_last, y_last](double *x, double *params)
					{
						double x_val = x[0];
						if (x_val <= x_last)
						{
							// Polynomial function up to x_last
							return params[0] + params[1]*x_val + params[2]*pow(x_val, 2) + 
							params[3]*pow(x_val, 3) + params[4]*pow(x_val, 4) +
							params[5]*pow(x_val, 5) + params[6]*pow(x_val, 6);
						}
						else
						{
							// Constant value after x_last
							return y_last;
						}
					}, ThMIN, x2l_max, 7);  // 7 parameters for the 6th-degree polynomial
					
					p1G[i][k]->SetLineColor(kRed);
					p1G[i][k]->SetLineWidth(3); p1G[i][k]->SetLineStyle(2);
					if (currentType.Contains("Plastic")) {grG[i][k]->Fit(p1G[i][k],"QRN");}
					else if (currentType.Contains("Li6")) {grG[i][k]->Fit(p1G[i][k],"QRN","",ThMIN_EJ270,x2l_max);}
					p1G[i][k]->Draw("same");
					rn = RunNumber;
					ch = i;
					par0 = p1G[i][k]->GetParameter(0);
					par1 = p1G[i][k]->GetParameter(1);
					par2 = p1G[i][k]->GetParameter(2);
					par3 = p1G[i][k]->GetParameter(3);
					par4 = p1G[i][k]->GetParameter(4);
					par5 = p1G[i][k]->GetParameter(5);
					par6 = p1G[i][k]->GetParameter(6);
					par7 = y_last;
/*					p1N[i][k] = new TF1(Form("H%02d_%d", i+1, k), "pol6", ThMIN, ThMAX); // Ql_MIN+0.5*ThMIN // "[0]*Exp(-1.0*[1]*x)+[2]"
					p1N[i][k]->SetLineColor(kRed);
					p1N[i][k]->SetLineWidth(3); p1N[i][k]->SetLineStyle(2);
					grN[i][k]->Fit(p1N[i][k],"QR");
					p1N[i][k]->Draw("same");*/
					legQlPSD[i]->AddEntry(grG[i][k],Form("(%.0f#sigma) %.2e+%.2ex+%.2ex^{2}+%.2ex^{3}+%.2ex^{4}+%.2ex^{5}+%.2ex^{6} / %.2e", gSCV[k], par0, par1, par2, par3, par4, par5, par6, par7),"p");
					tFit->Fill();
					par0 = 0; par1 = 0; par2 = 0; par3 = 0; par4 = 0; par5 = 0; par6 = 0; par7 = 0;
				}
				
/*				grG1[i]->SetMarkerStyle(20);
				grG1[i]->SetMarkerColor(kMagenta);
				grG1[i]->Draw("P same");*/
/*				grN1[i]->SetMarkerStyle(20);
				grN1[i]->SetMarkerColor(kMagenta);
				grN1[i]->Draw("P same");*/
				
				// tFit->Fill();
			}
			
			if (currentType.Contains("Li6")) // (i == 0 || i == 11 || i == 12) // mini_GS20, EJ-270, Layer_GS20
			{
				for (int k=0; k<MaxSCV; k++)
				{
					rn = RunNumber;
					ch = i;
					x_c = (QlnMAX[0][i][0][k] + QlnMIN[0][i][0][k]) / 2.0;
					y_c = (PSDnMAX[0][i][0][k] + PSDnMIN[0][i][0][k]) / 2.0;
					a = (QlnMAX[0][i][0][k] - QlnMIN[0][i][0][k]) / 2.0;
					b = (PSDnMAX[0][i][0][k] - PSDnMIN[0][i][0][k]) / 2.0;
					if (PrintReadingON == 1) {cout << "k: " << k << ", x_c: " << x_c << ", y_c: " << y_c << ", a: " << a << ", b: " << b << endl;}
					EllipseN[i][k] = new TEllipse(x_c, y_c, a, b);
					EllipseN[i][k]->SetFillStyle(0);
					EllipseN[i][k]->SetLineColor(kRed);
					EllipseN[i][k]->SetLineWidth(3); EllipseN[i][k]->SetLineStyle(2);
					canvQlPSD[i]->cd();
					EllipseN[i][k]->Draw("same");
					legQlPSD[i]->AddEntry(EllipseN[i][k],Form("(%.0f#sigma) (x-%.2e)^{2} / %.2e^{2} + (y-%.2e)^{2} / %.2e^{2} = 1", gSCV[k], x_c, y_c, a, b),"p");
					tFit->Fill();
					canvQlPSD[i]->Update();
					x_c = 0; y_c = 0; a = 0; b = 0;
				}
			}
			
			legQlPSD[i]->Draw();
			
			legQlTail[i] = new TLegend(0.75, 0.90, 0.86, 0.99);
			legQlTail[i]->SetMargin(0.08);
			canvQlTail[i] = new TCanvas(Form("QlTail_%04d_%s", RunNumber, ChLabel[i].Data()), Form("QlTail_%04d_%s", RunNumber, ChLabel[i].Data()), 1500, 1000);
			canvQlTail[i]->SetMargin(0.10,0.123,0.10,0.05); // left, right, bottom, top
			canvQlTail[i]->SetGridx(); canvQlTail[i]->SetGridy();
			h_QlTail[i]->GetXaxis()->CenterTitle();
			h_QlTail[i]->GetXaxis()->SetTitle("Q_{L}");
			h_QlTail[i]->GetXaxis()->SetRangeUser(0.,Ql_MAX);
			h_QlTail[i]->GetXaxis()->SetTitleOffset(1.20);
			h_QlTail[i]->GetYaxis()->CenterTitle();
			h_QlTail[i]->GetYaxis()->SetTitle("Tail");
			h_QlTail[i]->GetYaxis()->SetTitleOffset(1.50);
			h_QlTail[i]->Draw("colz");
			legQlTail[i]->SetHeader(Form("%04d %s", RunNumber, ChLabel[i].Data()),"c");
			legQlTail[i]->Draw();
			
			h_QsQl[i]->Write();
			h_QlPSD[i]->Write();
			h_QlTail[i]->Write();
			
			if (SaveFigureON == 1)
			{
				canvInt[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_Int.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvQsQl[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_QsQl.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvQlPSD[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_QlPSD.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
				canvQlTail[i]->SaveAs(Form("%s%s/Th%04.0f%s/%04d_%s_QlTail.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), RunNumber, ChLabel[i].Data()));
			}
			
			if (DrawChoice == 0 || DrawChoice == 1)
			{
				canvQsQl[i]->Close();
				canvQlTail[i]->Close();
			}
			if (DrawChoice == 0)
			{
				canvQlPSD[i]->Close();
			}
		}
	}
	tFit->Write();
	fout2D->Close();
}
