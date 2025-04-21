// This macro was created by Rossi Fabiana on 2024-07-04
// Last modified on 2025-04-21
//
//	How to run this macro:
//		Single file -> root -b -q Read_RootCompass_250421.C
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

const TString MainDir = "/home/ndg/Fabi/JFY2025/202504_Optimization/Compass";
const TString RunDir = "/run_";
const TString RawDir = "/RAW";
const TString PlotDir = "/Plot";

const Bool_t PrintMemoON = 1;
const Bool_t PrintJunkON = 0;
const Bool_t PrintINPFileON = 1;
const Bool_t PrintINPReadON = 1;
const Bool_t PrintOUTFileON = 0;
const Bool_t PrintOUTReadON = 0;
const Bool_t PrintErrMessageON = 0;
const Bool_t SaveFigureON = 1; // = 1 to save png images

const Int_t MAXRunNumber = 2; // this is the max number of files per hour
const Int_t MAXnumChannel = 10; // this is the number of channel
const Double_t MeasTime = 3600.; // seconds
const Int_t MAXTimeSelect = 2;

void Read_RootCompass_250421()
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
		cout << " PSD_E  = TDirectoryFile with the PSD vs Energy TH2I" << endl;
		cout << "ChannelNumber =  1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 10" << endl << endl;
		cout << "Output: Data for the 10 channels" << endl;
		cout << "        BG for the 10 channels" << endl;
		cout << "        Data-BG for the 10 channels" << endl;
		cout << "**********************************************************************" << endl << endl;
	}
	
	Int_t TimeSelect;
	cout << "Do you want count rate (0) or total counts in 3600s (1) ?" << endl;
	cin >> TimeSelect;
	Double_t YMin[MAXRunNumber+1][MAXTimeSelect], YMax[MAXRunNumber+1][MAXTimeSelect];

// *************************************************************************************************************************************************** //
//                                                                   FILE DEFINITION                                                                   //
// *************************************************************************************************************************************************** //

// run_1 22Na   run_2 BG
	TString FName[MAXRunNumber];
	Int_t RunNum[MAXRunNumber] = {20, 19};
	FName[0] = Form("%s%s%d%s/HcompassR_run_20_20250421_140616.root", MainDir.Data(), RunDir.Data(), RunNum[0], RawDir.Data());
	FName[1] = Form("%s%s%d%s/HcompassR_run_19_20250421_124600.root", MainDir.Data(), RunDir.Data(), RunNum[1], RawDir.Data());
	
	YMin[0][0] = 0; YMax[0][0] = 30;
	YMin[0][1] = 0; YMax[0][1] = 1.00e5;
	YMin[1][0] = 0; YMax[1][0] = 5;
	YMin[1][1] = 0; YMax[1][1] = 1.6e4;
	YMin[MAXRunNumber][0] = 0; YMax[MAXRunNumber][0] = 30;
	YMin[MAXRunNumber][1] = 0; YMax[MAXRunNumber][1] = 1.00e5;
	
	Double_t RealTime[MAXRunNumber][MAXnumChannel] = {
		{3600.065, 3600.058, 3600.065, 3600.063, 3600.064, 3600.065, 3600.066, 3600.065, 3600.063, 3600.065},
		{3600.068, 3600.070, 3600.067, 3600.059, 3600.069, 3600.055, 3600.066, 3600.070, 3600.070, 3600.069}
	}; // seconds
	Double_t LiveTime[MAXRunNumber][MAXnumChannel] = {
		{3597.729, 3597.905, 3597.820, 3597.840, 3595.210, 3595.355, 3597.656, 3597.919, 3597.691, 3597.562},
		{3599.298, 3599.441, 3599.283, 3599.363, 3599.325, 3599.328, 3599.369, 3599.238, 3599.245, 3599.202}
	}; // seconds
	
	TString LegHead[MAXRunNumber+1] = {"run_20: ^{22}Na", "run_19: BG", "run_20 - run_19: ^{22}Na - BG"};
	TString LegCanvas[MAXRunNumber+1] = {"run20_22Na", "run19_BG", "run20-19_22Na-BG"};
	TFile *file[MAXRunNumber];

// *************************************************************************************************************************************************** //
//                                                                 READING AND PLOTTING                                                                //
// *************************************************************************************************************************************************** //

	TH1D *h[MAXRunNumber+1][MAXnumChannel]; // data   BG
	Double_t ColFile[MAXnumChannel] = {kBlack, kRed, kBlue, kGreen+2, kMagenta, kCyan, kViolet+1, kOrange+1, kAzure+1, kSpring+9};
	TCanvas *canv[MAXRunNumber+1];
	TLegend *leg[MAXRunNumber+1];
/*	Double_t ScaleTime[MAXRunNumber][MAXnumChannel]; // seconds
	for (int i=0; i<MAXRunNumber; i++)
	{
		for (int j=0; j<MAXnumChannel; j++)
		{
			ScaleTime[i][j] = (MeasTime/LiveTime[i][j]);
			if (PrintJunkON == 1) {cout << Form("%.10f",ScaleTime[i][j]) << endl;}
		}
	}*/
	
	for (int i=0; i<MAXRunNumber; i++)
	{
		file[i] = TFile::Open(FName[i]);
		if (PrintINPFileON == 1) {cout << "READING the file: " << FName[i] << endl;}
		canv[i] = new TCanvas(Form("canv%d",i), Form("canv%d",i), 1500,1000);
		canv[i]->SetMargin(0.15,0.025,0.15,0.08);
		canv[i]->SetGridx(); canv[i]->SetGridy();
		leg[i] = new TLegend(0.50, 0.80, 0.99, 0.99);
		leg[i]->SetMargin(0.40);
		leg[i]->SetTextSize(0.05);
		leg[i]->SetNColumns(5);
		
		for (int j=0; j<MAXnumChannel; j++)
		{
			if (PrintINPReadON == 1) {cout << Form("_R_EnergyCH%d@V1730SB_27412",j+1) << endl;}
			h[i][j] = (TH1D*) gDirectory->Get(Form("Energy/_R_EnergyCH%d@V1730SB_27412",j+1));
			h[i][j]->GetXaxis()->SetTitleOffset(1.50);
			h[i][j]->GetXaxis()->SetTitleSize(0.05);
			h[i][j]->GetXaxis()->SetLabelSize(0.05);
			h[i][j]->GetXaxis()->CenterTitle();
			h[i][j]->GetXaxis()->SetTitle("ADC Channel");
			
			h[i][j]->GetYaxis()->SetTitleOffset(1.50);
			h[i][j]->GetYaxis()->SetTitleSize(0.05);
			h[i][j]->GetYaxis()->SetLabelSize(0.05);
			h[i][j]->GetYaxis()->CenterTitle();
			if (TimeSelect == 1)
			{
//				h[i][j]->Scale(ScaleTime[i][j]);
				h[i][j]->GetYaxis()->SetTitle("Counts (t_{m} = 3600s)");
			}
			else if (TimeSelect == 0)
			{
				h[i][j]->Scale(1/MeasTime); //LiveTime[i][j]);
				h[i][j]->GetYaxis()->SetTitle("CountRate [/s]");
			}
			
			h[i][j]->SetLineColor(ColFile[j]);
			h[i][j]->SetLineWidth(3);
			h[i][j]->SetFillColor(kWhite);
			h[i][j]->GetYaxis()->SetRangeUser(YMin[i][TimeSelect], YMax[i][TimeSelect]);
			h[i][j]->GetXaxis()->SetRangeUser(0, 400);
			
			leg[i]->SetHeader(LegHead[i],"c");
			leg[i]->AddEntry(h[i][j],Form("H%02d",j+1),"l");
			
			if (j==0) {h[i][j]->Draw("hist");}
			else {h[i][j]->Draw("hist,same");}
		}
		leg[i]->Draw();
	}
	
	canv[MAXRunNumber] = new TCanvas(Form("canv%d", MAXRunNumber), Form("canv%d", MAXRunNumber), 1500,1000);
	canv[MAXRunNumber]->SetMargin(0.15,0.025,0.15,0.08);
	canv[MAXRunNumber]->SetGridx(); canv[MAXRunNumber]->SetGridy();
	leg[MAXRunNumber] = new TLegend(0.50, 0.80, 0.99, 0.99);
	leg[MAXRunNumber]->SetMargin(0.40);
	leg[MAXRunNumber]->SetNColumns(5);
	leg[MAXRunNumber]->SetTextSize(0.05);
	leg[MAXRunNumber]->SetHeader(LegHead[MAXRunNumber],"c");

	for (int j=0; j<MAXnumChannel; j++)
	{
		h[MAXRunNumber][j] = (TH1D*)h[0][j]->Clone(Form("s%d",j));
		h[MAXRunNumber][j]->Add(h[1][j],-1);
		h[MAXRunNumber][j]->GetYaxis()->SetRangeUser(YMin[MAXRunNumber][TimeSelect],YMax[MAXRunNumber][TimeSelect]);
		if (j==0) {h[MAXRunNumber][j]->Draw("hist");}
		else {h[MAXRunNumber][j]->Draw("hist,same");}
		leg[MAXRunNumber]->AddEntry(h[MAXRunNumber][j],Form("H%01d",j+1),"l");
	}
	
	leg[MAXRunNumber]->Draw();
	canv[MAXRunNumber]->Update();
	
	for (int i=0; i<MAXRunNumber+1; i++)
	{
		if (SaveFigureON == 1)
		{
			if (TimeSelect == 1)
			{
				canv[i]->SaveAs(Form("%s/%s/%s_Counts.png", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
				canv[i]->SaveAs(Form("%s/%s/%s_Counts.root", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
			}
			else if (TimeSelect == 0)
			{
				canv[i]->SaveAs(Form("%s/%s/%s_CountRate.png", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
				canv[i]->SaveAs(Form("%s/%s/%s_CountRate.root", MainDir.Data(), PlotDir.Data(), LegCanvas[i].Data()));
			}
		}
	}
}
