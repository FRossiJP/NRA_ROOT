// This macro was created by Rossi Fabiana on 2024-07-04
// Last modified on 2025-04-22
//
//	How to run this macro:
//		Single file -> root -b -q Read_RootCompass_250421.C
//
//	Improvements that can be done:
//		1) Read the run# automatically
//

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

const TString MainDir = "/home/ndg/Fabi/JFY2025/202504_Optimization/Compass"; // "/home/nra/Lee_ROOT/CoMPASS/20250421/DAQ"
const TString RunDir = "/run_";
const TString RawDir = "/RAW";
const TString PlotDir = "/Plot";

const Bool_t PrintMemoON = 1;
const Bool_t PrintJunkON = 0;
const Bool_t PrintINPFileON = 0;
const Bool_t PrintINPReadON = 0;
const Bool_t PrintOUTFileON = 0;
const Bool_t PrintOUTReadON = 0;
const Bool_t PrintErrMessageON = 0;
const Bool_t SaveFigureON = 1; // = 1 to save png images

const Int_t MAXRunNumber = 2; // this is the max number of files per hour
const Int_t MAXnumChannel = 10; // this is the number of channel
const Double_t MeasTime = 3600.; // seconds
const Int_t MAXTimeSelect = 2;

// Function to parse a line of format "Key = H:M:S.sss" into seconds
	Double_t parseTime(const string &line)
	{
		// Find the equals sign
		size_t pos = line.find('=');
		if (pos == string::npos) return 0;
		// Extract time string and trim leading spaces
		string timestr = line.substr(pos + 1);
		while (!timestr.empty() && isspace(timestr.front())) {timestr.erase(0, 1);}
	
		// Parse hours, minutes, seconds
		int h = 0, m = 0;
		double s = 0.0;
		char delim;
		istringstream iss(timestr);
		iss >> h >> delim >> m >> delim >> s;
	
		return h * 3600. + m * 60. + s;
	}

void Read_RootCompass_250422()
{
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
	gStyle->SetOptFit(1);//10111);
	gStyle->SetOptTitle(0);
	gStyle->SetPalette(1,0);

	if (PrintMemoON == 1)
	{
		cout << "**********************************************************************" << endl;
		cout << "1) Read from HcompassR_run_#_YYYYMMDD_hhmmss.root file" << endl;
		cout << " Energy = TDirectoryFile with the Energy TH1D" << endl;
		cout << " Time   = TDirectoryFile with the Time TH1D" << endl;
		cout << " PSD    = TDirectoryFile with the PSD TH1D" << endl;
		cout << " PSD_E  = TDirectoryFile with the PSD vs Energy TH2I" << endl << endl;
		cout << "2) Read Real-time and Live-time from CH*@V1730SB_27412_EspectrumR_run_#_YYYYMMDD_hhmmss.txt3" << endl;
		cout << "ChannelNumber =  1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 10" << endl << endl;
		cout << "Output:" << endl;
		cout << "        a. TH1D for the counts in the 10 channels of each run" << endl;
		cout << "        b. TH1D for the BG subtracted in the 10 channels (on demand)" << endl;
		cout << "**********************************************************************" << endl << endl;
	}
	
	Int_t RunNumber;
	cout << "How many files do you want to parse ?" << endl;
	cin >> RunNumber;
	Int_t RunNum[RunNumber];
	TString RunMemo[RunNumber];
	TString LegHead[RunNumber], LegCanvas[RunNumber];
	for (int i=0; i<RunNumber; i++)
	{
		cout << Form("Please write the run number of the %d file ", i+1) << endl;
		cin >> RunNum[i];
		cout << Form("Write a short description for the run %02d", RunNum[i]) << endl;
		TString line;
		line.ReadLine(cin);
		RunMemo[i] = line;
		LegHead[i] = Form("RUN%3d: %s", RunNum[i], RunMemo[i].Data());
		LegCanvas[i] = Form("run_%02d", RunNum[i]);
	}
	cout << endl;
	
	// Find ROOT and TXT3 files
	TString FNameROOT[RunNumber], FNameTXT[RunNumber][MAXnumChannel];
	TSystemDirectory baseDir(".", ".");
	TList* entries = baseDir.GetListOfFiles();
	TIter nextEntry(entries);
	TSystemFile* entry;
	while ((entry = (TSystemFile*)nextEntry()))
	{
		TString dirName = entry->GetName();
		if (entry->IsDirectory() && dirName.BeginsWith("run_"))
		{
			TString rawPath = dirName + "/RAW/";
			TSystemDirectory subDir("subdir", rawPath);
			TList* files = subDir.GetListOfFiles();
			if (!files) continue;
	
			TIter nextFile(files);
			TSystemFile* file;
	
			while ((file = (TSystemFile*)nextFile()))
			{
				if (file->IsDirectory()) continue;
				TString fname = file->GetName();
	
				// Find ROOT files with "HcompassR_run_*_..."
				if (fname.BeginsWith("HcompassR_run_") && fname.EndsWith(".root"))
				{
					for (int i = 0; i < RunNumber; i++)
					{
						TString pattern = Form("HcompassR_run_%d_", RunNum[i]);
						if (fname.Contains(pattern))
						{
							FNameROOT[i] = rawPath + fname;
							break;
						}
					}
				}
	
				// Find TXT3 files with "CH*@V1730SB..."
				if (fname.EndsWith(".txt3"))
				{
					for (int ch = 0; ch < MAXnumChannel; ch++)
					{
						TString chPattern = Form("CH%d@", ch + 1);
						if (!fname.Contains(chPattern)) continue;
	
						for (int i = 0; i < RunNumber; i++)
						{
							TString runPattern = Form("run_%d_", RunNum[i]);
							if (fname.Contains(runPattern))
							{
								FNameTXT[i][ch] = rawPath + fname;
								break;
							}
						}
					}
				}
			}
		}
	}
	// If PrintINPFileON = 1, list the ROOT and TXT3 files' names
	if (PrintINPFileON)
	{
		for (int i = 0; i < RunNumber; i++)
		{
			if (FNameROOT[i].IsNull()) {cout << Form("Run %d ROOT file not found!", RunNum[i]) << endl;}
			else {cout << Form("ROOT file for run %d: %s", RunNum[i], FNameROOT[i].Data()) << endl;}
		
			for (int ch = 0; ch < MAXnumChannel; ch++)
			{
				if (FNameTXT[i][ch].IsNull()) {cout << Form("  CH%d TXT3 file not found!", ch + 1) << endl;}
				else {cout << Form("    CH%d TXT3 file: %s", ch + 1, FNameTXT[i][ch].Data()) << endl;}
			}
		}
		cout << endl;
	}
	
	// Get information about Real and Live time using the TXT3 files
	Double_t RealTime[RunNumber][MAXnumChannel];
	Double_t LiveTime[RunNumber][MAXnumChannel];
	ifstream fin;
	for (int i = 0; i < RunNumber; i++)
	{
		if (PrintINPReadON) {cout << Form("Run%3d:",RunNum[i]) << endl;}
		for (int ch = 0; ch < MAXnumChannel; ch++)
		{
			fin.open(FNameTXT[i][ch]);
			if (!fin.is_open())
			{
				cerr << Form("Cannot open file: %s", FNameTXT[i][ch].Data()) << endl << endl;
				RealTime[i][ch] = 0;
				LiveTime[i][ch] = 0;
				continue;
			}
			string line;
			getline(fin, line); // Skip the first line
			getline(fin, line); // Read RealTime
			RealTime[i][ch] = parseTime(line);
			getline(fin, line); // Read LiveTime
			LiveTime[i][ch] = parseTime(line);
			fin.close();
			if (PrintINPReadON) {cout << Form("          CH%02d: RealTime = %.3f, LiveTime = %.3f", ch+1, RealTime[i][ch], LiveTime[i][ch]) << endl;}
		}
		if (PrintINPReadON) {cout << endl;}
	}
	
	Int_t TimeSelect;
	cout << "Do you want count rate (0) or total counts in 3600s (1) ?" << endl;
	cin >> TimeSelect;
	
	Double_t YMin[MAXTimeSelect], YMax[MAXTimeSelect];
	YMin[0] = 0; YMax[0] = 25;
	YMin[1] = 0; YMax[1] = 8.5e4;
	
	TFile *file[RunNumber];
	TH1D *h[RunNumber][MAXnumChannel];
	Double_t ColFile[MAXnumChannel] = {kBlue, kViolet+1, kBlack, kGreen+2, kRed, kOrange+1, kAzure+1, kMagenta, kCyan, kSpring+9};
	TCanvas *canv[RunNumber];
	TLegend *leg[RunNumber];
	
	for (int i=0; i<RunNumber; i++)
	{
		file[i] = TFile::Open(FNameROOT[i]);
		if (PrintINPFileON == 1) {cout << "READING the file: " << FNameROOT[i] << endl;}
		canv[i] = new TCanvas(Form("canv%d",i), Form("canv%d",i), 1500,1000);
		canv[i]->SetMargin(0.15,0.025,0.15,0.08);
		canv[i]->SetGridx(); canv[i]->SetGridy();
		leg[i] = new TLegend(0.50, 0.80, 0.99, 0.99);
		leg[i]->SetMargin(0.40);
		leg[i]->SetTextSize(0.05);
		leg[i]->SetNColumns(5);
		for (int ch = 0; ch < MAXnumChannel; ch++)
		{
			if (PrintJunkON == 1) {cout << Form("_R_EnergyCH%d@V1730SB_27412", ch+1) << endl;}
			h[i][ch] = (TH1D*) gDirectory->Get(Form("Energy/_R_EnergyCH%d@V1730SB_27412", ch+1));
			h[i][ch]->GetXaxis()->SetTitleOffset(1.50);
			h[i][ch]->GetXaxis()->SetTitleSize(0.05);
			h[i][ch]->GetXaxis()->SetLabelSize(0.05);
			h[i][ch]->GetXaxis()->CenterTitle();
			h[i][ch]->GetXaxis()->SetTitle("ADC Channel");
			h[i][ch]->GetYaxis()->SetTitleOffset(1.50);
			h[i][ch]->GetYaxis()->SetTitleSize(0.05);
			h[i][ch]->GetYaxis()->SetLabelSize(0.05);
			h[i][ch]->GetYaxis()->CenterTitle();
			if (TimeSelect == 1)
			{
				h[i][ch]->GetYaxis()->SetTitle("Counts (t_{meas} = 3600s)");
			}
			else if (TimeSelect == 0)
			{
				h[i][ch]->Scale(1/LiveTime[i][ch]);
				h[i][ch]->GetYaxis()->SetTitle("CountRate [/s]");
			}
			
			h[i][ch]->SetLineColor(ColFile[ch]);
			h[i][ch]->SetLineWidth(3);
			h[i][ch]->SetFillColor(kWhite);
			h[i][ch]->GetYaxis()->SetRangeUser(YMin[TimeSelect], YMax[TimeSelect]);
			h[i][ch]->GetXaxis()->SetRangeUser(0, 400);
			
			leg[i]->SetHeader(LegHead[i],"c");
			leg[i]->AddEntry(h[i][ch],Form("H%02d", ch+1),"l");
			
			if (ch==0) {h[i][ch]->Draw("hist");}
			else {h[i][ch]->Draw("hist,same");}
		}
		leg[i]->Draw();
	}
	
	cout << "\nAvailable runs:\n";
	for (int i = 0; i < RunNumber; i++)
	{
		cout << Form("  [%d] RUN %d: %s", i, RunNum[i], RunMemo[i].Data()) << endl;
	}
	cout << endl;
	
	char doBGSub;
	cout << "Do you want to perform background subtraction? (y/n): ";
	cin >> doBGSub;
	
	while (doBGSub == 'y' || doBGSub == 'Y')
	{
		Int_t dataRunNum, bgRunNum;
		cout << "Enter the DATA run number: ";
		cin >> dataRunNum;
		cout << "Enter the BACKGROUND run number: ";
		cin >> bgRunNum;
		
		Int_t dataIdx = -1, bgIdx = -1;
		for (int i = 0; i < RunNumber; i++)
		{
			if (RunNum[i] == dataRunNum) dataIdx = i;
			if (RunNum[i] == bgRunNum) bgIdx = i;
		}
		
		if (dataIdx == -1 || bgIdx == -1)
		{
			cerr << "❌ One or both run numbers not found! Try again." << endl;
			continue;
		}
		TCanvas *cBGSub = new TCanvas(Form("cBGSub_%d_%d", dataIdx, bgIdx), "Background Subtraction", 1500, 1000);
		cBGSub->SetMargin(0.15, 0.025, 0.15, 0.08);
		cBGSub->SetGridx(); cBGSub->SetGridy();
		
		TLegend *legBG = new TLegend(0.50, 0.80, 0.99, 0.99);
		legBG->SetMargin(0.40);
		legBG->SetTextSize(0.05);
		legBG->SetNColumns(5);
		legBG->SetHeader(Form("RUN%d - RUN%d", RunNum[dataIdx], RunNum[bgIdx]), "c");
		
		for (int ch = 0; ch < MAXnumChannel; ch++) {
			if (!h[dataIdx][ch] || !h[bgIdx][ch]) continue;
		
			// Clone and subtract
			TH1D* hSub = (TH1D*)h[dataIdx][ch]->Clone(Form("hSub_CH%d", ch));
			hSub->Add(h[bgIdx][ch], -1);
			hSub->SetLineColor(ColFile[ch]);
			hSub->SetLineWidth(3);
			hSub->GetXaxis()->SetTitle("ADC Channel");
			hSub->GetYaxis()->SetTitle("Subtracted Counts");
			hSub->GetXaxis()->SetRangeUser(0, 400);
			hSub->GetYaxis()->SetRangeUser(YMin[TimeSelect], YMax[TimeSelect]);
		
			legBG->AddEntry(hSub, Form("H%02d", ch+1), "l");
			if (ch == 0) hSub->Draw("hist");
			else hSub->Draw("hist same");
		}
		
		legBG->Draw();
		cBGSub->SaveAs(Form("%s/%s/BGSub_Run%d_minus_Run%d.png", MainDir.Data(), PlotDir.Data(), RunNum[dataIdx], RunNum[bgIdx]));
		
		cout << "Do you want to subtract another background? (y/n): ";
		cin >> doBGSub;
	}
	
	for (int i=0; i<RunNumber; i++)
	{
		if (SaveFigureON == 1)
		{
			if (TimeSelect == 1)
			{
				canv[i]->SaveAs(Form("%s/%s/%s_Counts.png", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
//				canv[i]->SaveAs(Form("%s/%s/%s_Counts.root", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
			}
			else if (TimeSelect == 0)
			{
				canv[i]->SaveAs(Form("%s/%s/%s_CountRate.png", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
//				canv[i]->SaveAs(Form("%s/%s/%s_CountRate.root", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
			}
		}
	}
}
