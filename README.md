# NtupleMaker
Ntuple maker for differental DY cross section measurements with full Run-2 data

**Simplified version that demonstrates only L1 and HLT information writing into ntuples**

* Problem: all L1 prescales obtained are == 1

### How to get started:
	export SCRAM_ARCH=slc6_amd64_gcc530
	export VO_CMS_SW_DIR=/cvmfs/cms.cern.ch
	source $VO_CMS_SW_DIR/cmsset_default.sh
	cmsrel CMSSW_8_0_32
	cd CMSSW_8_0_32/src
	cmsenv
	git cms-init
	git clone https://github.com/marijusambrozas/NtupleMaker.git Phys -b 80X_L1_demo_branch # this demo branch
	scram b -j 4

	voms-proxy-init --voms cms --valid 168:00
	cd Phys/DYntupleMaker/test
	cmsRun ntupler_arg.py
	# by default it will run on one of SinglePhoton MINIAOD data files from Run2016B (03Feb2017_ver2-v2)

To run `ntupler_arg.py` with more specific settings, several different arguments can be provided:
  * ```globalTag```: global tag to use
  * ```inputFile```: input EDM file (MINIAOD). ```file:``` prefix is needed in front of the path of the file if it is local file
  * ```nEvent```: number of event to run
  * ```isMC```: true if it is MC
  * ```isSignalMC```: true if it is signal MC (DY). Then it will save LHE information, which is used to estimate PDF systematics

* Usage:
  ```
  cmsRun ntupler_arg.py globalTag=<global tag> inputFile=<inputFile> useSinglePhotonTrigger=<1 or 0> nEvent=<number> isMC=<1 or 0> isSignalMC=<1 or 0>
  ```

* Example (signal MC):
    ```
    cmsRun ntupler_arg.py \
    globalTag=80X_mcRun2_asymptotic_2016_TrancheIV_v6 \
    inputFile=file:/u/user/kplee/scratch/ROOTFiles_Test/80X/MINIAOD_DYLL_M50toInf_Morind17.root \
    nEvent=3000 \
    isMC=1 \
    isSignalMC=1 >&ntupler_arg_signalMC.log&
    ```
