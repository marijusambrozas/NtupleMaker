#ifndef DYntupleMaker_H
#define DYntupleMaker_H

// -- system include files -- //
#include <memory>
#include <iostream>

// -- FrameWorks -- //
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"

// -- Triggers -- //
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "L1Trigger/L1TGlobal/interface/L1TGlobalUtil.h"
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"

#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>
#include <TSystem.h>

using namespace std;
using namespace pat;

class DYntupleMaker : public edm::EDAnalyzer
{
public:
	explicit DYntupleMaker(const edm::ParameterSet&);
	~DYntupleMaker();

private:
	virtual void beginJob() ;
	virtual void analyze(const edm::Event&, const edm::EventSetup&);
	virtual void endJob() ;
	virtual void beginRun(const edm::Run &, const edm::EventSetup & );
	virtual void endRun(const edm::Run &, const edm::EventSetup & );

	virtual void Fill_HLT(const edm::Event &iEvent, const edm::EventSetup& iSetup);
	virtual void Fill_L1(const edm::Event &iEvent, const edm::EventSetup &iSetup);

	std::string processName;
	HLTConfigProvider hltConfig_;
	HLTPrescaleProvider *hltPrescale_;
	l1t::L1TGlobalUtil *L1GtUtils_;

	// Tokens
	edm::EDGetTokenT< edm::TriggerResults >	TriggerToken;
	edm::EDGetTokenT< edm::TriggerResults >	TriggerTokenPAT;
	edm::EDGetTokenT< std::vector<pat::TriggerObjectStandAlone> > TriggerObjectToken;
	edm::EDGetTokenT< BXVector<GlobalAlgBlk> > globalAlgBlkToken;
	// Trigger info from example: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2016#Trigger
	edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_;
	edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_l1min_;
	edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_l1max_;

	// For HLT analysis
	std::vector<std::string > HLT_list;
	std::vector<std::string > trigModuleNames;
	std::vector<std::string > trigModuleNames_preFil;
	vector< std::string> vec_inputHLTList_;
	vector< std::string> vec_L1Seed_;

	TTree *DYTree;
	int nEvt;
	// Objects to be saved in ntuple
	int runNum;
	unsigned long long evtNum;
	int lumiBlock;
	int _HLT_ntrig;
	std::vector<std::string> _HLT_trigName;
	std::vector<int> _HLT_trigFired;
	std::vector<int> _HLT_trigPrescale;
	std::vector<int> _HLT_trigPrescale_alt;
	std::vector<int> _HLT_trigPrescale_l1min;
	std::vector<int> _HLT_trigPrescale_l1max;
	std::vector<double> _HLT_trigPt;
	std::vector<double> _HLT_trigEta;
	std::vector<double> _HLT_trigPhi;
	std::vector< bool > _L1_trigFired;
	std::vector< int >  _L1_trigPrescale;
	std::vector< int >  _L1_trigPrescale_alt;

};
#endif
