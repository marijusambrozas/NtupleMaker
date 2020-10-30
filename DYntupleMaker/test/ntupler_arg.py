#########################################################
#-- usage: cmsRun ntupler_arg.py globalTag=<global tag> inputFile=<inputFile> nEvent=<number> isMC=<1 or 0> isSignalMC=<1 or 0> -- #
#########################################################

import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')

options.register('globalTag',
                  "80X_dataRun2_2016SeptRepro_v7", # default value
                  VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.varType.string,         # string, int, or float
                  "Global tag used for the ntuple production")

# -- GT for DATA (03Feb2017): 80X_dataRun2_2016SeptRepro_v7 (Run2016Hv2,v3: 80X_dataRun2_Prompt_v16)
# -- GT for MC (Moriond17): 80X_mcRun2_asymptotic_2016_TrancheIV_v6

options.register('inputFile',
                  "root://cms-xrootd.gridpp.ac.uk//store/data/Run2016B/SinglePhoton/MINIAOD/03Feb2017_ver2-v2/100000/D430BD98-58EB-E611-9DD9-008CFA1974DC.root", # default value
                  VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.varType.string,         # string, int, or float
                  "input EDM file location (MINIAOD). Don't forget to add 'file:' for the local file ")

# -- example file for DATA: /u/user/kplee/scratch/ROOTFiles_Test/80X/MINIAOD_SingleMuon_Run2016G_03Feb2017.root
# -- example file for MC: /u/user/kplee/scratch/ROOTFiles_Test/80X/MINIAOD_DYLL_M50toInf_Morind17.root

options.register('nEvent',
                  100,#-1, # default value
                  VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.varType.int,         # string, int, or float
                  "number of events to run")

options.register('isMC',
                  0, # default value
                  VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.varType.int,         # string, int, or float
                  "isMC")

options.register('isSignalMC',
                  0, # default value
                  VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.varType.int,         # string, int, or float
                  "save LHE information related to PDF systematics. It is always false if isMC = false.")

options.parseArguments()

if not options.isMC: options.isSignalMC = 0

print "input global tag           = ", options.globalTag
print "input file                 = ", options.inputFile
print "number of events to run    = ", options.nEvent
print "isMC                       = ", options.isMC
print "isSignalMC                 = ", options.isSignalMC

process = cms.Process("ntupler")

## MessageLogger
process.load("FWCore.MessageLogger.MessageLogger_cfi")

## Options and Output Report
process.options   = cms.untracked.PSet( 
  wantSummary = cms.untracked.bool(True),
  SkipEvent = cms.untracked.vstring('ProductNotFound') # -- a few events have no GenEventInfoProduct or LHEInfoProduct: ignore them
)
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

## Source
process.source = cms.Source("PoolSource",
	fileNames = cms.untracked.vstring( options.inputFile )
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.nEvent) )

# -- Global Tags -- #
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff")
process.GlobalTag.globaltag = cms.string(options.globalTag)

outputName = ""
if options.isMC: outputName = "ntuple_mc.root"
else:            outputName = "ntuple_data.root"

process.TFileService = cms.Service("TFileService",
  fileName = cms.string(outputName)
)

# -- L1 trigger test -- #
process.load("EventFilter.L1TRawToDigi.gtStage2Digis_cfi")
process.gtStage2Digis.InputLabel = cms.InputTag( "hltFEDSelectorL1" )
process.load('PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff')

# -- DY Tree -- #
from Phys.DYntupleMaker.DYntupleMaker_cfi import *

process.recoTree = DYntupleMaker.clone()
process.recoTree.isMC = bool(options.isMC)

from Phys.DYntupleMaker.HLTList import *
process.recoTree.InputHLTList = cms.untracked.vstring(GetList_HLT())

####################
# -- Let it run -- #
####################
process.p = cms.Path(process.recoTree)
