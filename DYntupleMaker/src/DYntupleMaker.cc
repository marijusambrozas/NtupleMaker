#include "Phys/DYntupleMaker/interface/DYntupleMaker.h"

// -- system include files -- //
#include <memory>
#include <iostream>

// -- FrameWorks -- //
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RegexMatch.h"

// -- Triggers -- //
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

// -- Other -- //
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// -- ROOT -- //
#include <TTree.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TFile.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace edm;
using namespace pat;

// -- Constructor -- //
DYntupleMaker::DYntupleMaker(const edm::ParameterSet& iConfig):
TriggerToken		( consumes< edm::TriggerResults >  			(iConfig.getUntrackedParameter<edm::InputTag>("TriggerResults")) ),
TriggerTokenPAT 	( consumes< edm::TriggerResults >  			(iConfig.getUntrackedParameter<edm::InputTag>("TriggerResultsPAT")) ),
TriggerObjectToken	( consumes< std::vector<pat::TriggerObjectStandAlone> > (iConfig.getUntrackedParameter<edm::InputTag>("TriggerObject")) ),
globalAlgBlkToken   	( consumes< BXVector< GlobalAlgBlk > >   		(iConfig.getUntrackedParameter<edm::InputTag>("globalAlgBlk")) ),
// Trigger part from example: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2016#Trigger
triggerPrescales_	( consumes< pat::PackedTriggerPrescales >	(iConfig.getParameter<edm::InputTag>("prescales"))),
triggerPrescales_l1min_	( consumes< pat::PackedTriggerPrescales >  (iConfig.getParameter<edm::InputTag>("prescales_l1min"))),
triggerPrescales_l1max_	( consumes< pat::PackedTriggerPrescales >  (iConfig.getParameter<edm::InputTag>("prescales_l1max")))
{
	nEvt = 0;

	processName = iConfig.getUntrackedParameter<string>("processName", "HLT");

	vec_inputHLTList_ = iConfig.getUntrackedParameter<std::vector<std::string> >("InputHLTList");
	vec_L1Seed_ = iConfig.getUntrackedParameter<std::vector<std::string> >("L1SeedList");
	hltPrescale_ = new HLTPrescaleProvider(iConfig, consumesCollector(), *this);
	L1GtUtils_  = new l1t::L1TGlobalUtil(iConfig, consumesCollector());

}

DYntupleMaker::~DYntupleMaker() { }

void DYntupleMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	_HLT_ntrig = -1;
	_HLT_trigName.clear();
	_HLT_trigFired.clear();
	_HLT_trigPrescale.clear();
	_HLT_trigPrescale_alt.clear();
	_HLT_trigPrescale_l1min.clear();
	_HLT_trigPrescale_l1max.clear();
	_HLT_trigPt.clear();
	_HLT_trigEta.clear();
	_HLT_trigPhi.clear();
	_L1_trigFired.clear();
	_L1_trigPrescale.clear();
	_L1_trigPrescale_alt.clear();

	nEvt++;
	runNum = iEvent.id().run();
	evtNum = iEvent.id().event();
	lumiBlock = iEvent.id().luminosityBlock();
	
	Fill_L1(iEvent, iSetup);
	Fill_HLT(iEvent, iSetup);
	DYTree->Fill();
}

void DYntupleMaker::beginJob()
{
	edm::Service<TFileService> fs;
	DYTree = fs->make<TTree>("DYTree","DYTree");
	DYTree->Branch("runNum",&runNum/*,"runNum/I"*/);
	DYTree->Branch("evtNum",&evtNum/*,"evtNum/l"*/);
	DYTree->Branch("lumiBlock",&lumiBlock);
	DYTree->Branch("HLT_ntrig", &_HLT_ntrig);
	DYTree->Branch("HLT_trigFired", &_HLT_trigFired);
	DYTree->Branch("HLT_trigName", &_HLT_trigName);
	DYTree->Branch("HLT_trigPrescale", &_HLT_trigPrescale);
	DYTree->Branch("HLT_trigPrescale_alt", &_HLT_trigPrescale_alt);
	DYTree->Branch("HLT_trigPrescale_l1min", &_HLT_trigPrescale_l1min);
	DYTree->Branch("HLT_trigPrescale_l1max", &_HLT_trigPrescale_l1max);
	DYTree->Branch("HLT_trigPt", &_HLT_trigPt);
	DYTree->Branch("HLT_trigEta", &_HLT_trigEta);
	DYTree->Branch("HLT_trigPhi", &_HLT_trigPhi);
	DYTree->Branch("L1_trigName", &vec_L1Seed_);
	DYTree->Branch("L1_trigFired", &_L1_trigFired);
	DYTree->Branch("L1_trigPrescale", &_L1_trigPrescale);
	DYTree->Branch("L1_trigPrescale_alt", &_L1_trigPrescale_alt);
}

void DYntupleMaker::beginRun(const Run & iRun, const EventSetup & iSetup)
{
	bool isConfigChanged = true;	
	hltPrescale_->init(iRun, iSetup, processName, isConfigChanged);
	if (isConfigChanged)
		cout << "Prescale config changed." << endl;

	// -- Taking the HLT list from input and comparing with all available triggers; leaving only the matching triggers -- //
	HLT_list.clear();

	for( auto HLTPath : vec_inputHLTList_ )
		HLT_list.push_back( HLTPath );

	int ntrigName = HLT_list.size();
	cout << "# triggers before removing un-available triggers: " << ntrigName << endl;

	int *listRemoval = new int[ntrigName];
	for(int i=0; i<ntrigName; i++)
		listRemoval[i] = -1;

	bool changedConfig;
	if (!hltConfig_.init(iRun, iSetup, processName, changedConfig))
	{
		LogError("HLTMuonVal") << "Initialization of HLTConfigProvider failed!!";
		return;
	}
	else
	{
		std::vector<std::string> triggerNames = hltConfig_.triggerNames();

		// -- iteration for each input triggers -- //
		for( int itrigName = 0; itrigName < ntrigName; itrigName++ )
		{
			listRemoval[itrigName] = 0;

			// -- find triggers in HLT configuration matched with a input trigger using wild card -- //
			std::vector<std::vector<std::string>::const_iterator> matches = edm::regexMatch(triggerNames, HLT_list[itrigName]);

			if( !matches.empty() )
			{	
				// -- iteration for each matched trigger -- //
				BOOST_FOREACH(std::vector<std::string>::const_iterator match, matches)
				{
					// -- find modules corresponding to a trigger in HLT configuration -- //
					std::vector<std::string> moduleNames = hltConfig_.moduleLabels( *match );
					int nsize = moduleNames.size();

					if( nsize-2 >= 0 )
					{
						trigModuleNames.push_back(moduleNames[nsize-2]);
						if( nsize-3 >= 0 )
						{
							trigModuleNames_preFil.push_back(moduleNames[nsize-3]);
						}
						else
						{
							trigModuleNames_preFil.push_back("");
						}
					}
					break; // -- just take into account the case # mathced trigger = 1 -- //
				} // -- end of BOOST_FOREACH(std::vector<std::string>::const_iterator match, matches) -- //
			} // -- end of if( !matches.empty() ) -- //
			else 
				listRemoval[itrigName] = 1;

		} // -- end of for( int itrigName = 0; itrigName < ntrigName; itrigName++ ): trigger iteration -- //
	} // -- end of else of if (!hltConfig_.init(iRun, iSetup, processName, changedConfig)) -- //

	// -- Remove unavailable triggers -- //
	int itmp = 0;
	for( vector<string>::iterator iter = HLT_list.begin(); iter != HLT_list.end(); )
	{
		if( listRemoval[itmp] > 0 )
		{
			iter = HLT_list.erase(iter);
		}
		else
			++iter;

		itmp++;
	}
	delete[] listRemoval; // -- not used anymore

	ntrigName = HLT_list.size();
	cout << "# triggers after removing un-available triggers: " << ntrigName << endl;

	cout << "##### End of Begin Run #####" << endl;
}

// ------------ method called once each job just after ending the event loop  ------------ //
void DYntupleMaker::endJob()
{
	std::cout <<"++++++++++++++++++++++++++++++++++++++" << std::endl;
	std::cout <<"analyzed " << nEvt << " events: " << std::endl;
	std::cout <<"++++++++++++++++++++++++++++++++++++++" << std::endl;
}


// -- Retrieve L1 information -- //
void DYntupleMaker::Fill_L1(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	L1GtUtils_->retrieveL1(iEvent, iSetup, globalAlgBlkToken);
	for(unsigned int i_seed=0; i_seed<vec_L1Seed_.size(); i_seed++)
	{
		bool isFired = false;
		L1GtUtils_->getFinalDecisionByName(string(vec_L1Seed_[i_seed]), isFired);
		_L1_trigFired.push_back(isFired);

		int L1Prescale = -999;
		L1GtUtils_->getPrescaleByName(string(vec_L1Seed_[i_seed]), L1Prescale);
		_L1_trigPrescale.push_back(L1Prescale);
	}
}


// -- Retrieve HLT information -- //
void DYntupleMaker::Fill_HLT(const edm::Event &iEvent, const edm::EventSetup& iSetup)
{
	int ntrigName = HLT_list.size();

	// -- read the whole HLT trigger lists fired in an event -- //
	bool *trigFired = new bool[ntrigName];
	for( int i = 0; i < ntrigName; i++ ) 
		trigFired[i] = false;

	edm::Handle<edm::TriggerResults> trigResult;
	edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;
	edm::Handle<pat::PackedTriggerPrescales> triggerPrescales_l1min;
	edm::Handle<pat::PackedTriggerPrescales> triggerPrescales_l1max;
	iEvent.getByToken(TriggerToken, trigResult);
	iEvent.getByToken(triggerPrescales_, triggerPrescales);
	iEvent.getByToken(triggerPrescales_l1min_, triggerPrescales_l1min);
	iEvent.getByToken(triggerPrescales_l1max_, triggerPrescales_l1max);
	
	std::vector<int> trigPrescale;
	std::vector<int> trigPrescale_alt;
	std::vector<int> trigPrescale_l1min;
	std::vector<int> trigPrescale_l1max;
	for(unsigned int i_seed=0; i_seed<vec_L1Seed_.size(); i_seed++)
        {
		_L1_trigPrescale_alt.push_back(-1);
	}

	if( !trigResult.failedToGet() )
	{
		int ntrigs = trigResult->size();
		const edm::TriggerNames trigName = iEvent.triggerNames(*trigResult);

		for( int itrigName = 0; itrigName < ntrigName; itrigName++ )
		{
			// -- Looking only at matches between our HLT list and available triggers in the event -- //
			std::vector<std::vector<std::string>::const_iterator> matches = edm::regexMatch(trigName.triggerNames(), HLT_list[itrigName]);
			if( !matches.empty() )
			{
				BOOST_FOREACH(std::vector<std::string>::const_iterator match, matches)
				{
					if( trigName.triggerIndex(*match) >= (unsigned int)ntrigs ) continue;
					if( trigResult->accept(trigName.triggerIndex(*match)) ) trigFired[itrigName] = true;
					trigPrescale.push_back(triggerPrescales->getPrescaleForIndex(trigName.triggerIndex(*match)));
					trigPrescale_l1min.push_back(triggerPrescales_l1min->getPrescaleForIndex(trigName.triggerIndex(*match)));
					trigPrescale_l1max.push_back(triggerPrescales_l1max->getPrescaleForIndex(trigName.triggerIndex(*match)));
					// Taking prescales from HLTConfigProvider (also should provide L1 seed prescale)
					std::pair<std::vector<std::pair<std::string,int> >,int> prescalesInDetail = hltPrescale_->prescaleValuesInDetail(iEvent, iSetup, *match);
					trigPrescale_alt.push_back(prescalesInDetail.second);
					for (unsigned int i_seed=0; i_seed<vec_L1Seed_.size(); i_seed++)
					{
						for (unsigned int j_seed=0; j_seed<prescalesInDetail.first.size(); j_seed++)
						{
							if (vec_L1Seed_[i_seed] == prescalesInDetail.first[j_seed].first)
							{
								if (_L1_trigPrescale_alt[i_seed] != -1 && _L1_trigPrescale_alt[i_seed] != prescalesInDetail.first[j_seed].second)
								{
									cout << "L1 seed " << vec_L1Seed_[i_seed] << " has different prescales in the same event: ";
									cout << _L1_trigPrescale_alt[i_seed] << ",  " << prescalesInDetail.first[j_seed].second << endl;
								}
								_L1_trigPrescale_alt[i_seed] = prescalesInDetail.first[j_seed].second;
							}
						}
					}
				}
			}
		} // -- end of for( int itrigName = 0; itrigName < ntrigName; itrigName++ ) -- //
	} // -- end of if( !trigResult.failedToGet() ) -- //

	const bool isRD = iEvent.isRealData();
	if( isRD )
	{
		edm::Handle<edm::TriggerResults> trigResultPAT;
		iEvent.getByToken(TriggerTokenPAT, trigResultPAT);

		if( !trigResultPAT.failedToGet() )
		{
			const edm::TriggerNames trigName = iEvent.triggerNames(*trigResultPAT);
		}
	}

	edm::Handle< std::vector<pat::TriggerObjectStandAlone> > triggerObject;
	iEvent.getByToken(TriggerObjectToken, triggerObject);

	int ntrigTot = 0;

	if( !trigResult.failedToGet() )
	{
		const edm::TriggerNames names = iEvent.triggerNames(*trigResult);

		for (pat::TriggerObjectStandAlone obj : *triggerObject)
		{
			obj.unpackPathNames(names);

			for( size_t i_filter = 0; i_filter < obj.filterLabels().size(); ++i_filter )
			{
				// -- Get the full name of i-th filter -- //
				std::string fullname = obj.filterLabels()[i_filter];

				std::string filterName;

				// -- Find ":" in the full name -- //
				size_t m = fullname.find_first_of(':');

				// -- if ":" exists in the full name, takes the name before ":" as the filter name -- //
				if( m != std::string::npos )
					filterName = fullname.substr(0, m);
				else
					filterName = fullname;

				// -- Loop for the triggers that a user inserted in this code -- //
				for( int itf = 0; itf < ntrigName; itf++ )
				{
					string name = "";

					// -- Store HLT object information only if trigModuleName is equal to this filter name -- //
					if( filterName == trigModuleNames[itf] )
					{
						_HLT_trigFired.push_back(trigFired[itf]);
						_HLT_trigPt.push_back(obj.pt());
						_HLT_trigEta.push_back(obj.eta());
						_HLT_trigPhi.push_back(obj.phi());
						_HLT_trigName.push_back(HLT_list[itf]);
						_HLT_trigPrescale.push_back(trigPrescale[itf]);
						_HLT_trigPrescale_alt.push_back(trigPrescale_alt[itf]);
						_HLT_trigPrescale_l1min.push_back(trigPrescale_l1min[itf]);
						_HLT_trigPrescale_l1max.push_back(trigPrescale_l1max[itf]);
						ntrigTot++;
					}
				} // -- end of for( int itf = 0; itf < ntrigName; itf++ ) -- //
			} // -- end of filter iteration -- //
		} // -- end of trigger object iteration -- //
	} // -- end of !trigResult.failedToGet() -- //

	_HLT_ntrig = ntrigTot;

}


void DYntupleMaker::endRun(const Run & iRun, const EventSetup & iSetup) { }

//define this as a plug-in
DEFINE_FWK_MODULE(DYntupleMaker);
