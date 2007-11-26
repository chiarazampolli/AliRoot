/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Produces the data needed to calculate the quality assurance.           //
// All data must be mergeable objects.                                    //
// S.Radomski Uni-Heidelberg October 2007                                 //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

// --- ROOT system ---
#include <TClonesArray.h>
#include <TFile.h> 
#include <TH1D.h> 
#include <TH2D.h>
#include <TProfile.h>

// --- AliRoot header files ---
#include "AliESDEvent.h"
#include "AliLog.h"
#include "AliTRDdigit.h"
#include "AliTRDhit.h"
#include "AliTRDcluster.h"
#include "AliTRDQADataMaker.h"
#include "AliTRDdigitsManager.h"
#include "AliTRDgeometry.h"
#include "AliTRDdataArrayI.h"
#include "AliTRDRawStreamV2.h"

ClassImp(AliTRDQADataMaker)
           
//____________________________________________________________________________ 
  AliTRDQADataMaker::AliTRDQADataMaker() : 
  AliQADataMaker(AliQA::GetDetName(AliQA::kTRD), "TRD Quality Assurance Data Maker")
{
  // ctor
}

//____________________________________________________________________________ 
AliTRDQADataMaker::AliTRDQADataMaker(const AliTRDQADataMaker& qadm) :
  AliQADataMaker()
{
  //copy ctor 
  SetName((const char*)qadm.GetName()) ; 
  SetTitle((const char*)qadm.GetTitle()); 
}

//__________________________________________________________________
AliTRDQADataMaker& AliTRDQADataMaker::operator = (const AliTRDQADataMaker& qadm )
{
  // Equal operator.
  this->~AliTRDQADataMaker();
  new(this) AliTRDQADataMaker(qadm);
  return *this;
}
 
//____________________________________________________________________________ 
void AliTRDQADataMaker::EndOfDetectorCycle()
{
  //Detector specific actions at end of cycle

  // Rec points
  
}

//____________________________________________________________________________ 
void AliTRDQADataMaker::InitESDs()
{
  //create ESDs histograms in ESDs subdir
  const Int_t nhist = 19;
  TH1 *hist[nhist];
 
  hist[0] = new TH1D("qaTRD_esd_ntracks", ":Number of tracks", 300, -0.5, 299.5);
  hist[1] = new TH1D("qaTRD_esd_sector", ":Sector", 18, -0.5, 17.7);
  hist[2] = new TH1D("qaTRD_esd_bits", ";Bits", 64, -0.5, 63.5);

  const Int_t knbits = 6;
  const char *suf[knbits] = {"TPCi", "TPCo", "TPCz", "TRDo", "TRDr", "TRDz"};
  
  // 3
  for(Int_t i=0; i<knbits; i++) {
    hist[2*i+3] = new TH1D(Form("qaTRD_esd_pt%s",suf[i]), ";p_{T} (GeV/c);", 50, 0, 10);
    hist[2*i+4] = new TH1D(Form("qaTRD_esd_trdz%s", suf[i]), ";z (cm)", 200, -400, 400); 
  }
  
  // 3 + 12 = 15
  hist[15] = new TH1D("qaTRD_esd_clsTRDo", "TRDo;number of clusters", 130, -0.5, 129.5);;
  hist[16] = new TH1D("qaTRD_esd_clsTRDr", "TRDr;number of clusters", 130, -0.5, 129.5);;
  hist[17] = new TH1D("qaTRD_esd_clsTRDz", "TRDz;number of clusters", 130, -0.5, 129.5);;
  //  hist[18] = new TH1D("qaTRD_esd_clsRatio", ";cluster ratio", 100, 0., 1.3);;

  hist[18] = new TH2D("qaTRD_esd_sigMom", ";momentum (GeV/c);signal", 100, 0, 5, 200, 0, 1e3);

  for(Int_t i=0; i<nhist; i++) {
    //hist[i]->Sumw2();
    Add2ESDsList(hist[i], i);
  }
}

//____________________________________________________________________________ 
void AliTRDQADataMaker::InitHits()
{
  // create Hits histograms in Hits subdir
  const Int_t nhist = 4;
  TH1D *hist[nhist];
  
  hist[0] = new TH1D("qaTRD_hits_det", ";Detector Id of the hit", 540, -0.5, 539.5) ; 
  
  hist[1] = new TH1D("qaTRD_hist_Qdrift", ";Charge from tracks", 100, 0, 100);
  hist[2] = new TH1D("qaTRD_hist_Qamp", ";Charge from TRD photon", 100, 0, 100);
  hist[3] = new TH1D("qaTRD_hist_Qphoton", ";Charge from TRD photon", 100, 0, 100);

  for(Int_t i=0; i<nhist; i++) {
    //hist[i]->Sumw2();
    Add2HitsList(hist[i], i);
  }
}

//____________________________________________________________________________ 
void AliTRDQADataMaker::InitDigits()
{
  // create Digits histograms in Digits subdir
  
  const Int_t nhist = 3;
  TH1D *hist[nhist];
  
  hist[0] = new TH1D("qaTRD_digits_det", ";Detector Id of the digit", 540, -0.5, 539.5);
  hist[1] = new TH1D("qaTRD_digits_time", ";Time bin", 40, -0.5, 39.5);
  hist[2] = new TH1D("qaTRD_digits_amp", ";Amplitude", 100, 0, 100.);

  for(Int_t i=0; i<nhist; i++) {
    hist[i]->Sumw2();
    Add2DigitsList(hist[i], i);
  }

}

//____________________________________________________________________________ 
void AliTRDQADataMaker::InitRecPoints()
{
  // create Reconstructed Points histograms in RecPoints subdir
  const Int_t nhist = 12;
  TH1 *hist[nhist];

  hist[0] = new TH1D("qaTRD_recPoints_det", ";Detector ID of the cluster", 540, -0.5, 539.5);
  hist[1] = new TH2D("qaTRD_recPoints_amp", ";Amplitude", 540, -0.5, 539, 200, -0.5, 199.5);
  hist[2] = new TH1D("qaTRD_recPoints_npad", ";Number of Pads", 12, -0.5, 11.5);
 
  hist[3] = new TH1D("qaTRD_recPoints_dist2", ";residuals [2pad]", 100, -1, 1);
  hist[4] = new TH1D("qaTRD_recPoints_dist3", ";residuals [3pad]", 100, -1, 1);
  hist[5] = new TH1D("qaTRD_recPoints_dist4", ";residuals [4pad]", 100, -1, 1);
  hist[6] = new TH1D("qaTRD_recPoints_dist5", ";residuals [5pad]", 100, -1, 1);

  hist[7] = new TH2D("qaTRD_recPoints_rowCol", ";row;col", 16, -0.5, 15.5, 145, -0.5, 144.5);
  hist[8] = new TH1D("qaTRD_recPoints_time", ";time bin", 35, -0.5, 34.5);
  hist[9] = new TH1D("qaTRD_recPoints_nCls", ";number of clusters", 500, -0.5, 499.5);

  hist[10] = new TProfile("qaTRD_recPoints_sigTime", ";time bin;signal", 35, -0.5, 34.5, 0, 200, "");
  hist[11] = new TProfile("qaTRD_recPoints_prf", ";distance;center of gravity", 120, -0.6, 0.6, -1.2, 1.2, "");

  hist[12] = new TH1D("qaTRD_recPoints_ampDist", ";amplitude MPV", 100, 0, 100);


  for(Int_t i=0; i<nhist; i++) {
    //hist[i]->Sumw2();
    Add2RecPointsList(hist[i], i);
  }
}

//____________________________________________________________________________ 
void AliTRDQADataMaker::InitRaws()
{
  // create Raws histograms in Raws subdir
  const Int_t kSM = 18;
  const Int_t nhist = 6+kSM;
  TH1D *hist[nhist];
 
  hist[0] = new TH1D("qaTRD_raws_det", ";detector", 540, -0.5, 539.5);
  hist[1] = new TH1D("qaTRD_raws_sig", ";signal", 100, -0.5, 99.5);
  hist[2] = new TH1D("qaTRD_raws_sigCentral", "; signal central bin", 100, -0.5, 99.5);
  hist[3] = new TH1D("qaTRD_raws_sigTail", ";signal cluster", 100, -0.5, 99.5);
  hist[4] = new TH1D("qaTRD_raws_tiemBin", ";time bin", 40, -0.5, 39.5); 
  hist[5] = new TH1D("qaTRD_rows_smId", ";supermodule", 18, -0.5, 17.5);

  // one char per ADC chanell
  const Int_t nADC = 30 * 8 * 16 * 22;
  for(Int_t i=0; i<kSM; i++)
    hist[6+i] = new TH1D(Form("qaTRD_raws_sm%d",i),"",nADC, -0.5, nADC-0.5); 

  for(Int_t i=0; i<nhist; i++) {
    //hist[i]->Sumw2();
    Add2RawsList(hist[i], i);
  }
}

//____________________________________________________________________________ 
void AliTRDQADataMaker::InitSDigits()
{
  // create SDigits histograms in SDigits subdir
  
  const Int_t nhist = 3;
  TH1D *hist[nhist];
  
  hist[0] = new TH1D("qaTRD_sdigits_det", ";Detector Id of the digit", 540, -0.5, 539.5);
  hist[1] = new TH1D("qaTRD_sdigits_time", ";Time bin", 40, -0.5, 39.5);
  hist[2] = new TH1D("qaTRD_sdigits_amp", ";Amplitude", 100, 0, 1e7);

  for(Int_t i=0; i<nhist; i++) {
    hist[i]->Sumw2();
    Add2SDigitsList(hist[i], i);
  }
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeESDs(AliESDEvent * esd)
{
  // make QA data from ESDs
  Int_t nTracks = esd->GetNumberOfTracks();
  GetESDsData(0)->Fill(nTracks);
  
  // track loop
  for(Int_t i=0; i<nTracks; i++) {
    
    AliESDtrack *track = esd->GetTrack(i);
    const AliExternalTrackParam *paramOut = track->GetOuterParam();
    const AliExternalTrackParam *paramIn = track->GetInnerParam();

    // long track ..
    if (!paramIn) continue;
    if (!paramOut) continue;

    // not a kink
    if (track->GetKinkIndex(0) > 0) continue; 

    Double_t extZ = GetExtZ(paramIn);
    if (TMath::Abs(extZ) > 320) continue; // acceptance cut

    // .. in the acceptance
    Int_t sector = GetSector(paramOut->GetAlpha());
    GetESDsData(1)->Fill(sector);

    UInt_t u = 1;
    UInt_t status = track->GetStatus();
    for(Int_t bit=0; bit<32; bit++) 
      if (u<<bit & status) GetESDsData(2)->Fill(bit);

    const Int_t knbits = 6; 
    Int_t bit[6] = {0,0,0,0,0,0};    
    bit[0] = status & AliESDtrack::kTPCin;
    bit[1] = status & AliESDtrack::kTPCout;
    bit[2] = (status & AliESDtrack::kTPCout) && !(status & AliESDtrack::kTRDout);
    bit[3] = status & AliESDtrack::kTRDout;
    bit[4] = status & AliESDtrack::kTRDrefit;
    bit[5] = (status & AliESDtrack::kTRDout) && !(status & AliESDtrack::kTRDrefit);

    // transverse momentum
    //const Double_t *val = paramOut->GetParameter(); // parameters at the Outer plane
    Double_t pt = paramOut->Pt(); //1./TMath::Abs(val[4]);

    for(Int_t b=0; b<knbits; b++) {
      if (bit[b]) {
	GetESDsData(2*b+3)->Fill(pt); 
	GetESDsData(2*b+4)->Fill(extZ);
      }
    }

    // clusters
    for(Int_t b=0; b<3; b++) 
      if (bit[3+b]) GetESDsData(b+15)->Fill(track->GetTRDncls());

    // refitted only
    if (!bit[4]) continue;

    //fQuality->Fill(track->GetTRDQuality());
    //fBudget->Fill(track->GetTRDBudget());
    //fSignal->Fill(track->GetTRDsignal());
	
    GetESDsData(18)->Fill(track->GetP(), track->GetTRDsignal());

    /*
    // PID only
    if (status & AliESDtrack::kTRDpid) {
      
      for(Int_t l=0; l<6; l++) fTime->Fill(track->GetTRDTimBin(l));

      // fill pid histograms
      Double_t trdr0 = 0; //, tpcr0 = 0;
      Int_t trdBestPid = 5; //, tpcBestPid = 5;  // charged
      const Double_t kminPidValue = 0.9;

      //Double_t pp[5];
      //track->GetTPCpid(pp); // ESD inconsequence

      for(Int_t pid=0; pid<5; pid++) {
	
	trdr0 += track->GetTRDpid(pid);
	//tpcr0 += pp[pid];
	
	fTrdPID[pid]->Fill(track->GetTRDpid(pid));
	//fTpcPID[pid]->Fill(pp[pid]);
	
	if (track->GetTRDpid(pid) > kminPidValue) trdBestPid = pid;
	//if (pp[pid] > kminPidValue) tpcBestPid = pid;
      }
      
      fTrdPID[5]->Fill(trdr0); // check unitarity
      fTrdSigMomPID[trdBestPid]->Fill(track->GetP(), track->GetTRDsignal());
      
      //fTpcPID[5]->Fill(tpcr0); // check unitarity
      //fTpcSigMomPID[tpcBestPid]->Fill(track->GetP(), track->GetTPCsignal());
    }
    */
    
  }



}

//______________________________________________________________________________
Int_t AliTRDQADataMaker::GetSector(const Double_t alpha) const {
  // Gets the sector number 

  Double_t size = TMath::DegToRad() * 20.; // shall use TRDgeo
  Int_t sector = (Int_t)((alpha + TMath::Pi())/size);
  return sector;
}

//______________________________________________________________________________
Double_t AliTRDQADataMaker::GetExtZ(const AliExternalTrackParam *in) const {
  //
  // returns the Z position at the entry to TRD
  // using parameters from the TPC in
  //

  const Double_t x0 = 300;

  Double_t x = in->GetX();
  const Double_t *par = in->GetParameter();
  Double_t theta = par[3];
  Double_t z = in->GetZ();
  
  Double_t zz = z + (x0-x) * TMath::Tan(theta);
  return zz;
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeHits(TClonesArray * hits)
{
  //make QA data from Hits
  //printf("making QA for TRD hits from an array %d\n", hits->GetEntriesFast());

  TIter next(hits); 
  AliTRDhit * hit; 
  
  while ( (hit = dynamic_cast<AliTRDhit *>(next())) ) {
    GetHitsData(0)->Fill(hit->GetDetector());
    Double_t q = TMath::Abs(hit->GetCharge());
    
    if (hit->FromDrift()) GetHitsData(1)->Fill(q);
    if (hit->FromAmplification()) GetHitsData(2)->Fill(q);
    if (hit->FromTRphoton()) GetHitsData(3)->Fill(q);
  }

}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeHits(TTree * hitTree)
{
  //make QA data from Hits
  //printf("making QA for TRD hits from a tree\n");
  
  if (!CheckPointer(hitTree, "TRD hits tree")) return;
 
  TBranch *branch = hitTree->GetBranch("TRD");
  if (!CheckPointer(branch, "TRD hits branch")) return;
 
  Int_t nhits = (Int_t)(hitTree->GetTotBytes()/sizeof(AliTRDhit));
  TClonesArray *hits = new TClonesArray("AliTRDhit", nhits+1000);
  TClonesArray *tmp = new TClonesArray("AliTRDhit", 1000);
  branch->SetAddress(&tmp);
  
  Int_t index = 0;
  Int_t nEntries = (Int_t)branch->GetEntries();
  for(Int_t i = 0; i < nEntries; i++) {
    branch->GetEntry(i);
    Int_t nHits = (Int_t)tmp->GetEntries();
    for(Int_t j=0; j<nHits; j++) {
      AliTRDhit *hit = (AliTRDhit*)tmp->At(j);
      new((*hits)[index++]) AliTRDhit(*hit);
    }
  }

  tmp->Delete();
  delete tmp;
  MakeHits(hits);
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeDigits(TClonesArray * digits)
{
  // makes data from Digits
  
  TIter next(digits) ; 
  AliTRDdigit * digit ; 
  while ( (digit = dynamic_cast<AliTRDdigit *>(next())) ) {
    GetDigitsData(0)->Fill(digit->GetDetector());
    GetDigitsData(1)->Fill(digit->GetTime());
    GetDigitsData(2)->Fill(digit->GetAmp());
  }  
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeDigits(TTree * digits)
{

  AliTRDdigitsManager *digitsManager = new AliTRDdigitsManager();
  digitsManager->CreateArrays();
  digitsManager->ReadDigits(digits);

  for (Int_t i = 0; i < AliTRDgeometry::kNdet; i++) {
    
    AliTRDdataArrayI *digitsIn = digitsManager->GetDigits(i);      
    
    // This is to take care of switched off super modules
    if (digitsIn->GetNtime() == 0) continue;
    
    digitsIn->Expand();
    
    //AliTRDSignalIndex* indexes = digitsManager->GetIndexes(i);
    //if (indexes->IsAllocated() == kFALSE) digitsManager->BuildIndexes(i);
    
    Int_t nRows = digitsIn->GetNrow();
    Int_t nCols = digitsIn->GetNcol();
    Int_t nTbins = digitsIn->GetNtime();

    for(Int_t row = 0; row < nRows; row++) 
      for(Int_t col = 0; col < nCols; col++) 
	for(Int_t time = 0; time < nTbins; time++) {

	  Float_t signal = digitsIn->GetDataUnchecked(row,col,time);
	  GetDigitsData(0)->Fill(i);
	  GetDigitsData(1)->Fill(time);
	  GetDigitsData(2)->Fill(signal);
      	}
    
    //delete digitsIn;
  }
  delete digitsManager;
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeSDigits(TClonesArray * sdigits)
{
  // makes data from Digits
  
  TIter next(sdigits) ; 
  AliTRDdigit * digit ; 
  while ( (digit = dynamic_cast<AliTRDdigit *>(next())) ) {
    GetDigitsData(0)->Fill(digit->GetDetector());
    GetDigitsData(1)->Fill(digit->GetTime());
    GetDigitsData(2)->Fill(digit->GetAmp());
  }  
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeSDigits(TTree * digits)
{

  AliTRDdigitsManager *digitsManager = new AliTRDdigitsManager();
  digitsManager->CreateArrays();
  digitsManager->ReadDigits(digits);

  for (Int_t i = 0; i < AliTRDgeometry::kNdet; i++) {
    
    AliTRDdataArrayI *digitsIn = digitsManager->GetDigits(i);      
    
    // This is to take care of switched off super modules
    if (digitsIn->GetNtime() == 0) continue;
    
    digitsIn->Expand();
    
    //AliTRDSignalIndex* indexes = digitsManager->GetIndexes(i);
    //if (indexes->IsAllocated() == kFALSE) digitsManager->BuildIndexes(i);
    
    Int_t nRows = digitsIn->GetNrow();
    Int_t nCols = digitsIn->GetNcol();
    Int_t nTbins = digitsIn->GetNtime();

    for(Int_t row = 0; row < nRows; row++) 
      for(Int_t col = 0; col < nCols; col++) 
	for(Int_t time = 0; time < nTbins; time++) {

	  Float_t signal = digitsIn->GetDataUnchecked(row,col,time);
	  if (signal <= 0) continue;
	  GetSDigitsData(0)->Fill(i);
	  GetSDigitsData(1)->Fill(time);
	  GetSDigitsData(2)->Fill(signal);
      	}
    
    // delete digitsIn;
  }

  delete digitsManager;
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeRaws(AliRawReader* rawReader)
{
  // 157
  // T9 -- T10

  //const Int_t kSM  = 18;
  //const Int_t kROC = 30;
  //const Int_t kLayer = 6;
  //const Int_t kStack = 5;
  const Int_t kROB = 8;
  const Int_t kMCM = 16;
  const Int_t kADC = 22;

  AliTRDRawStreamV2 *raw = new AliTRDRawStreamV2(rawReader);
  
  raw->SetRawVersion(3);
  raw->Init();
  
  while (raw->Next()) {
    
    GetRawsData(0)->Fill(raw->GetDet());
    
    Int_t *sig = raw->GetSignals();
    for(Int_t i=0; i<3; i++) GetRawsData(1)->Fill(sig[i]);
    
    GetRawsData(2)->Fill(sig[1]);
    GetRawsData(3)->Fill(sig[0]);
    GetRawsData(3)->Fill(sig[2]);
    
    GetRawsData(4)->Fill(raw->GetTimeBin());
    
    // calculate the index;
    Int_t sm = raw->GetSM();
    Int_t roc = raw->GetROC();
    Int_t rob = raw->GetROB();
    Int_t mcm = raw->GetMCM();
    Int_t adc = raw->GetADC();
    
    Int_t index = roc * (kROB*kMCM*kADC) + rob * (kMCM*kADC) + mcm * kADC + adc;
    GetRawsData(5)->Fill(sm);
    //printf("SM = %d\n", sm);
    GetRawsData(6+sm)->Fill(index);
  }
}

//____________________________________________________________________________
void AliTRDQADataMaker::MakeRecPoints(TTree * clustersTree)
{
  
  // makes data from RecPoints
  // 
  
  Int_t nsize = Int_t(clustersTree->GetTotBytes() / (sizeof(AliTRDcluster))); 
  TObjArray *clusterArray = new TObjArray(nsize+1000); 
  
  TBranch *branch = clustersTree->GetBranch("TRDcluster");
  if (!branch) {
    AliError("Can't get the branch !");
    return;
  }
  branch->SetAddress(&clusterArray); 
  
  // Loop through all entries in the tree
  Int_t nEntries   = (Int_t) clustersTree->GetEntries();
  Int_t nbytes     = 0;
  AliTRDcluster *c = 0;
  Int_t nDet[540];
  for (Int_t i=0; i<540; i++) nDet[i] = 0;
  
  for (Int_t iEntry = 0; iEntry < nEntries; iEntry++) {    
    
    // Import the tree
    nbytes += clustersTree->GetEvent(iEntry);  
    
    // Get the number of points in the detector
    Int_t nCluster = clusterArray->GetEntriesFast();  
    
    // Loop through all TRD digits
    for (Int_t iCluster = 0; iCluster < nCluster; iCluster++) { 
      c = (AliTRDcluster *) clusterArray->UncheckedAt(iCluster);
      
      Int_t iDet = c->GetDetector();
      nDet[iDet]++;
      GetRecPointsData(0)->Fill(iDet);
      GetRecPointsData(1)->Fill(iDet, c->GetQ());
      GetRecPointsData(2)->Fill(c->GetNPads());
      if (c->GetNPads() < 6)
	GetRecPointsData(1+c->GetNPads())->Fill(c->GetCenter());

      //if (c->GetPadTime() < 5)
      ((TH2D*)GetRecPointsData(7))->Fill(c->GetPadRow(), c->GetPadCol());
      GetRecPointsData(8)->Fill(c->GetPadTime());
      
      ((TProfile*)GetRecPointsData(10))->Fill(c->GetPadTime(), c->GetQ());
      
      // PRF for 2pad
      //if (c->GetNPads() == 2) {
      Short_t *sig = c->GetSignals();
      Double_t frac = -10;
      
      if (sig[0] == 0 && sig[1] == 0 && sig[2] == 0 && sig[5] == 0 && sig[6] == 0) 
	frac = 1. * sig[4] / (sig[3] + sig[4]);
      
      
      if (sig[0] == 0 && sig[1] == 0 && sig[4] == 0 && sig[5] == 0 && sig[6] == 0)
	frac = -1. * sig[2] / (sig[2] + sig[3]);
      
      if (frac > -10)  ((TProfile*)GetRecPointsData(11))->Fill(c->GetCenter(), frac);
	
      //}
    }
  }
  
  for(Int_t i=0; i<540; i++) 
    if (nDet[i] > 0) GetRecPointsData(9)->Fill(nDet[i]);


  delete clusterArray;
}

//____________________________________________________________________________ 
void AliTRDQADataMaker::StartOfDetectorCycle()
{
  //Detector specific actions at start of cycle

}

//__________________________________________________________________________
Int_t AliTRDQADataMaker::CheckPointer(TObject *obj, const char *name) {

  if (!obj) AliWarning(Form("null pointer: %s", name));
  return !!obj;
}
//__________________________________________________________________________
