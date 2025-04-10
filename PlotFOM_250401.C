//	This macro was created by Rossi Fabiana on 2024-08-09
//	Last modified on 2025-03-27
/*
	How to run this macro:
		Single file -> root PlotFOM_250401.C\(\"ROOT/Th####/FOM2025.txt\",Ql_MIN\)
					-> root PlotFOM_250401.C\(\"ROOT/Th0100/FOM2025.txt\",100\)
					-> root PlotFOM_250401.C\(\"ROOT/Th0500/FOM2025.txt\",500\)
					-> root PlotFOM_250401.C\(\"ROOT/Th1000/FOM2025.txt\",1000\)
					-> root PlotFOM_250401.C\(\"ROOT/Th1500/FOM2025.txt\",1500\)
					-> root PlotFOM_250401.C\(\"ROOT/Th2000/FOM2025.txt\",2000\)
		
		**********************************************************************
		1) Parsing the file  where the FOM are stored
		2) Plot all the FOM with different Ql and Qs
		
		**********************************************************************
*/

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

const TString MainDir = "/home/ndg/Fabi/JFY2024/01_ROOT/202504_Optimization";
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
const Int_t MAXparameter = 8;
const Int_t MAXvalue = 2; // value and error
const Int_t MaxSCV = 3;
// const Double_t gSCV = 3; // Sigma confidence value (99.7%)
// const Double_t nSCV = 3; // Sigma confidence value (99.7%)
// const Double_t gSCV = 4; // Sigma confidence value (99.9937%)
// const Double_t nSCV = 4; // Sigma confidence value (99.9937%)

void PlotFOM_250401(string file, Double_t Ql_MIN)
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
	gStyle->SetOptFit(0);//10111);
	gStyle->SetOptTitle(0);
	gStyle->SetPalette(1,0);

	if (PrintMemoON == 1)
	{
		cout << "**********************************************************************" << endl;
		cout << "1) Parsing the file  where the FOM are stored" << endl;
		cout << "2) Plot all the FOM with different Ql and Qs" << endl;
		cout << "**********************************************************************" << endl << endl;
	}
	
// ******************************* PARSE FOM FILE **************************************
	
	ifstream fin;
	TString FName = file;
	TString ofile = Form("%s%s/Th%04.0f/FOM2025.root", MainDir.Data(), RootDir.Data(), Ql_MIN);
	TFile *fout = new TFile(ofile, "recreate");
	
	Int_t rn, ch, inQl, inQs;
	Double_t tmeas, S, S_err, FWHMg, FWHMg_err, FWHMn, FWHMn_err, FOM, FOM_err, chi;
	TTree *tData = new TTree("tData","tData");
	tData->Branch("rn",&rn,"rn/I");
	tData->Branch("ch",&ch,"ch/I");
	tData->Branch("tmeas",&tmeas,"tmeas/D");
	tData->Branch("inQs",&inQs,"inQs/I");
	tData->Branch("inQl",&inQl,"inQl/I");
	tData->Branch("S",&S,"S/D");
	tData->Branch("S_err",&S_err,"S_err/D");
	tData->Branch("FWHMg",&FWHMg,"FWHMg/D");
	tData->Branch("FWHMg_err",&FWHMg_err,"FWHMg_err/D");
	tData->Branch("FWHMn",&FWHMn,"FWHMn/D");
	tData->Branch("FWHMn_err",&FWHMn_err,"FWHMn_err/D");
	tData->Branch("FOM",&FOM,"FOM/D");
	tData->Branch("FOM_err",&FOM_err,"FOM_err/D");
	tData->Branch("chi",&chi,"chi/D");
	tData->SetAutoSave(0);
	tData->SetAutoFlush(0);
	
	fin.open(file);
	while (fin >> rn >> ch >> tmeas >> inQs >> inQl >> S >> S_err >> FWHMg >> FWHMg_err >> FWHMn >> FWHMn_err >> FOM >> FOM_err >> chi) // Filling the ROOT file
	{
		tData->Fill();
	}
	tData->Write();
	fin.close();
	fout->Close();
	
// ******************************* PLOT: FOM vs Qs / FOM vs Ql **************************************
	
	FName = ofile;
	TFile *finFOM;
//	TCanvas *canvQs, *canvQl; // fixed inQs // fixed inQl
	
	finFOM = TFile::Open(FName);
	tData = (TTree*)finFOM->Get("tData");
	tData->SetBranchAddress("rn",&rn);
	tData->SetBranchAddress("ch",&ch);
	tData->SetBranchAddress("tmeas",&tmeas);
	tData->SetBranchAddress("inQs",&inQs);
	tData->SetBranchAddress("inQl",&inQl);
	tData->SetBranchAddress("S",&S);
	tData->SetBranchAddress("S_err",&S_err);
	tData->SetBranchAddress("FWHMg",&FWHMg);
	tData->SetBranchAddress("FWHMg_err",&FWHMg_err);
	tData->SetBranchAddress("FWHMn",&FWHMn);
	tData->SetBranchAddress("FWHMn_err",&FWHMn_err);
	tData->SetBranchAddress("FOM",&FOM);
	tData->SetBranchAddress("FOM_err",&FOM_err);
	tData->SetBranchAddress("chi",&chi);
	
	TCanvas *canvQs = new TCanvas(Form("FOMvsQs_%04.0f", Ql_MIN), Form("FOMvsQs_%04.0f", Ql_MIN), 1500, 1000);
	canvQs->SetMargin(0.10, 0.025, 0.10, 0.03); // Left, Right, Bottom, Top
	canvQs->SetGridx();
	canvQs->SetGridy();
	TMultiGraph *mg = new TMultiGraph();
	TLegend *legend = new TLegend(0.75, 0.80, 0.99, 0.99);
	legend->SetNColumns(2);
	legend->SetHeader(Form("Threshold = %4.0f", Ql_MIN), "c");
	int colors[] = {kBlack, kRed, kBlue, kGreen+2, kViolet, kMagenta};
	
	for (int ch = 1; ch <= 6; ch++)
	{
		// Select points from TTree
		TString selection = Form("ch == %d && inQl == 600", ch);
		tData->Draw("FOM:inQs:FOM_err", selection, "goff");
		int nPoints = tData->GetSelectedRows();
		if (nPoints == 0) continue; // Skip if no points for this channel
		double *x = tData->GetV2();       // inQs (X-axis)
		double *y = tData->GetV1();       // FOM (Y-axis)
		double *ey = tData->GetV3();      // FOM_err (Y-axis error)
		double *ex = new double[nPoints]; // X-axis errors (assumed 0)
		for (int i = 0; i < nPoints; i++) ex[i] = 0;
		TGraphErrors *grErr = new TGraphErrors(nPoints, x, y, ex, ey);
		grErr->SetMarkerColor(colors[ch - 1]);
		grErr->SetLineColor(colors[ch - 1]);
		grErr->SetMarkerStyle(20);
//		grErr->SetMarkerSize(0.8);
		mg->Add(grErr);
		legend->AddEntry(grErr, Form("H%02d", ch), "p");
	}
	// Draw everything
	mg->Draw("AP");
	mg->GetXaxis()->SetTitle("Q_{S} [ns]");
	mg->GetYaxis()->SetTitle("Figure of Merit (FOM)");
	mg->GetXaxis()->CenterTitle();
	mg->GetYaxis()->CenterTitle();
	mg->GetXaxis()->SetTitleOffset(1.2);
	mg->GetYaxis()->SetTitleOffset(1.3);
	mg->GetYaxis()->SetDecimals();
	legend->Draw();
	canvQs->Update();
	canvQs->SaveAs(Form("%s%s/Th%04.0f%s/FOM_Qs_Th%04.0f.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), Ql_MIN));
	
	TCanvas *canvQl = new TCanvas(Form("FOMvsQl_%04.0f", Ql_MIN), Form("FOMvsQl_%04.0f", Ql_MIN), 1500, 1000);
	canvQl->SetMargin(0.10, 0.025, 0.10, 0.03); // Left, Right, Bottom, Top
	canvQl->SetGridx();
	canvQl->SetGridy();
	mg = new TMultiGraph();
	legend = new TLegend(0.75, 0.15, 0.99, 0.34);
	legend->SetNColumns(2);
	legend->SetHeader(Form("Threshold = %4.0f", Ql_MIN), "c");
	
	for (int ch = 1; ch <= 6; ch++)
	{
		// Select points from TTree
		TString selection = Form("ch == %d && inQs == 42", ch);
		tData->Draw("FOM:inQl:FOM_err", selection, "goff");
		int nPoints = tData->GetSelectedRows();
		if (nPoints == 0) continue; // Skip if no points for this channel
		double *x = tData->GetV2();       // inQs (X-axis)
		double *y = tData->GetV1();       // FOM (Y-axis)
		double *ey = tData->GetV3();      // FOM_err (Y-axis error)
		double *ex = new double[nPoints]; // X-axis errors (assumed 0)
		for (int i = 0; i < nPoints; i++) ex[i] = 0;
		TGraphErrors *grErr = new TGraphErrors(nPoints, x, y, ex, ey);
		grErr = new TGraphErrors(nPoints, x, y, ex, ey);
		grErr->SetMarkerColor(colors[ch - 1]);
		grErr->SetLineColor(colors[ch - 1]);
		grErr->SetMarkerStyle(20);
//		grErr->SetMarkerSize(0.8);
		mg->Add(grErr);
		legend->AddEntry(grErr, Form("H%02d", ch), "p");
	}
	// Draw everything
	mg->Draw("AP");
	mg->GetXaxis()->SetTitle("Q_{L} [ns]");
	mg->GetYaxis()->SetTitle("Figure of Merit (FOM)");
	mg->GetXaxis()->CenterTitle();
	mg->GetYaxis()->CenterTitle();
	mg->GetXaxis()->SetTitleOffset(1.2);
	mg->GetYaxis()->SetTitleOffset(1.3);
	mg->GetYaxis()->SetDecimals();
	legend->Draw();
	canvQl->Update();
	canvQl->SaveAs(Form("%s%s/Th%04.0f%s/FOM_Ql_Th%04.0f.png", MainDir.Data(), RootDir.Data(), Ql_MIN, RootPlotDir.Data(), Ql_MIN));
}