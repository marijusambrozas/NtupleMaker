import FWCore.ParameterSet.Config as cms

from Phys.DYntupleMaker.HLTList import GetList_HLT
from Phys.DYntupleMaker.L1SeedList import GetL1SeedList

DYntupleMaker = cms.EDAnalyzer("DYntupleMaker",
	isMC = cms.untracked.bool(True),
	processName = cms.untracked.string("HLT"),

	# -- Trigger -- #
	TriggerResults = cms.untracked.InputTag("TriggerResults", "", "HLT"),
	TriggerResultsPAT = cms.untracked.InputTag("TriggerResults", "", "PAT"),
	TriggerObject = cms.untracked.InputTag("selectedPatTrigger"),
	globalAlgBlk = cms.untracked.InputTag("gtStage2Digis"),
	L1SeedList = cms.untracked.vstring(GetL1SeedList()),
	l1tAlgBlkInputTag = cms.InputTag("gtStage2Digis"),
	l1tExtBlkInputTag = cms.InputTag("gtStage2Digis"),
	ReadPrescalesFromFile = cms.bool(False),

	# -- Filters -- #
	ApplyFilter = cms.untracked.bool(False),
	FilterType = cms.untracked.int32(0),

	# -- HLT list -- #
	InputHLTList = cms.untracked.vstring(GetList_HLT()),

	# -- Trigger test from https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2016#Trigger -- #
	prescales = cms.InputTag("patTrigger"),
	prescales_l1min = cms.InputTag("patTrigger:l1min"),
	prescales_l1max = cms.InputTag("patTrigger:l1max"),
)
