//	This macro was created by Rossi Fabiana on 2024-12-12
//	Last modified on 2025-03-27
/*
	
	How to run this macro:
		Single file -> root -b -q Parse_250203.C\(\"list/list_[Run#].asc\",Ql_MIN,Ql_MAX\)
		All files -> for i in list/list_*.asc; do root -b -q Parse_250401.C\(\"$i\",Ql_MIN,Ql_MAX\); done
				  -> for i in list/list_*.asc; do root -b -q Parse_250401.C\(\"$i\",100,60000\); done
				  -> for i in list/list_*.asc; do root -b -q Parse_250401.C\(\"$i\",500,60000\); done
				  -> for i in list/list_*.asc; do root -b -q Parse_250401.C\(\"$i\",1000,60000\); done
				  -> for i in list/list_*.asc; do root -b -q Parse_250401.C\(\"$i\",1500,60000\); done
				  -> for i in list/list_*.asc; do root -b -q Parse_250401.C\(\"$i\",2000,60000\); done
		
	**********************************************************************
	1) Convert from list_Run#.asc to Run#.root file
	list_Run#.asc has the format: CHANNEL    TOF    INTERVAL    Ql    Qs    PSD
	Run# = run number (4 digits)
	ChannelNumber =  0     / 1     / 2     / 3     / 4     / 5     / 6     / 7     / 8     / 9     / 10    / 11    / 12    / 13    / 14    / 15    / 16    /
		(exp. 2026): mGS20 / H01   / H02   / H03   / H04   / H05   / H06   / H07   / H08   / H09   / H10   / EJ270 / mGS20 / Layer /       /       / Linac /
	
	Output file:
	1) tData:  /home/ndg/Fabi/JFY2024/01_ROOT/202502_Data/ROOT/[Run#].root
	
	NOTE:
		Lines with Ql = 65535 and Qs = 32767 -> psd = 0.500008 are ignored due to the artifact of number size
	
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
// namespace fs = std::filesystem;

const TString MainDir = "/home/ndg/Fabi/JFY2024/01_ROOT/202503_Optimization";
const TString RootDir = "/ROOT";

const Int_t delMAX = 20;
const Bool_t PrintMemoON = 1; // = 1 to activate the print on screen
const Bool_t PrintInpFileON = 1; // = 1 to activate the print on screen
const Bool_t PrintReadingON = 0; // = 1 to activate the print on screen

const Double_t LimitQl = 65535; // Max number for Ql
const Double_t LimitQs = 32767; // Max number for Qs
const Double_t PSDartifact = 0.500008; // = Ql / Qs

void Parse_250401(string file, Double_t Ql_MIN, Double_t Ql_MAX)	//main function like (cint)
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

	if (PrintMemoON == 1)
	{
		cout << "**********************************************************************" << endl;
		cout << "1) Convert from list_Run#.asc to Run#.root file" << endl;
		cout << "list_Run#.asc has the format: CHANNEL    TOF    INTERVAL    Ql    Qs    PSD" << endl;
		cout << "     Run# = run number (4 digits)" << endl;
		cout << "ChannelNumber =  0     / 1     / 2     / 3     / 4     / 5     / 6     / 7     / 8     / 9     / 10    / 11    / 12    / 13    / 14    / 15    / 16    /" << endl;
		cout << "    (exp. 2025): mGS20 / H01   / H02   / H03   / H04   / H05   / H06   / H07   / H08   / H09   / H10   / EJ270 / Layer /       /       /       / Linac /" << endl << endl;
		cout << "Output file: " << endl;
		cout << Form("  1) tData:  %s%s/Th%04.0f/[Run#].root", MainDir.Data(), RootDir.Data(), Ql_MIN) << endl << endl;
		cout << "NOTE: " << endl;
		cout << Form("    Lines with Ql = %.0f and Qs = %.0f -> psd = %2.6f are ignored", LimitQl, LimitQs, PSDartifact) << endl << endl;
		cout << "**********************************************************************" << endl << endl;
	}
/*
	//std::cout << "Current path is " << fs::current_path() << endl;
	fs::path dir =	Form("%s%s/Th%04.0f",MainDir.Data(), RootDir.Data(), Ql_MIN);
	if (!fs::exists(dir)) {
                fs::create_directory(dir);
                std::cout << "mkdir now!" << dir << std::endl;
        } else {
                std::cout << "already mkdir" << dir << std::endl;
        }
*/
// ******************************* ROOT FILES CREATION **************************************
	ifstream fin;
	TString FName = file;
	// Extract RunNumber (e.g., 1710 from list_1710.asc)
	Int_t RunNumber = stoi(FName(FName.Last('_') + 1, FName.Last('.') - FName.Last('_') - 1));
	if (PrintInpFileON == 1) {
		cout << Form("PARSING the %d run: %s", RunNumber, FName.Data()) << endl;
	}
	
	TString ofile = Form("%s%s/Th%04.0f/%04d.root", MainDir.Data(), RootDir.Data(), Ql_MIN, RunNumber);
	TFile *fout = new TFile(ofile, "recreate");
	
	Int_t ch, tof, interval;
	Double_t ql, qs, psd;
	TTree *tData = new TTree("tData","tData");
	tData->Branch("ch",&ch,"ch/I");
	tData->Branch("tof",&tof,"tof/I");
	tData->Branch("interval",&interval,"interval/I");
	tData->Branch("ql",&ql,"ql/D");
	tData->Branch("qs",&qs,"qs/D");
	tData->Branch("psd",&psd,"psd/D");
	tData->SetAutoSave(0);
	tData->SetAutoFlush(0);
	
	fin.open(file);
	while (fin >> ch >> tof >> interval >> ql >> qs >> psd) // Filling the ROOT file
	{
		tof=tof*2;
		if (PrintReadingON == 1) {cout << Form("%2d  %10d  %5.0f  %5.0f  %10.6f", ch, tof, ql, qs, psd) << endl;}
		if (psd == PSDartifact) {if (PrintReadingON == 1) {cout << "Found it! ";}}
		else {tData->Fill();} // else if (ql >= Ql_MIN && ql <= Ql_MAX) {tData->Fill();}
	}
	tData->Write();
	fin.close();
	fout->Close();

}