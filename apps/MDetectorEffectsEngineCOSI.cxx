/* 
 * MNCTDetectorEffectsEngineCOSI.cxx
 *
 *
 * Copyright (C) by Alex Lowell, Cori Gerrity, Carolyn Kierans, Clio Sleator, Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * the copyright holders.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <map>
#include <fstream>
#include <cstring>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <algorithm>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>
#include <MString.h>
#include <TRandom3.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MReadOutElementDoubleStrip.h"

// Nuclearizer
#include "MNCTDepthCalibrator.h"

/******************************************************************************/


/******************************************************************************/

class MNCTDetectorEffectsEngineCOSI
{
	public:
		//! Default constructor
		MNCTDetectorEffectsEngineCOSI();
		//! Default destructor
		~MNCTDetectorEffectsEngineCOSI();

		//! Parse the command line
		bool ParseCommandLine(int argc, char** argv);
		//! Analyze whatever needs to be analyzed...
		bool Analyze();
		//! Interrupt the analysis
		void Interrupt() { m_Interrupt = true; }
		//! Read in and parse energy calibration file
		void ParseEnergyCalibrationFile();
		//! Read in and parse thresholds file
		void ParseThresholdFile();
		//! Read in and parse dead strip file
		void ParseDeadStripFile();
		//! noise shield energy
		double NoiseShieldEnergy(double energy, MString ShieldName);

	private:
		//! True, if the analysis needs to be interrupted
		bool m_Interrupt;

		//! Simulation file name
		MString m_FileName;
		//! Geometry file name
		MString m_GeometryFileName;
		//! Thresholds file name
		MString m_ThresholdFilename;
		//! Depth calibration coefficients file name
		MString m_DepthCalibrationCoeffsFileName;
		//! Depth calibration splines file name
		MString m_DepthCalibrationSplinesFileName;

		//! Calibration map between read-out element and fitted function for energy calibration
		map<MReadOutElementDoubleStrip, TF1*> m_EnergyCalibration;
		//! Calibration map between read-out element and fitted function for energy resolution calibration
		map<MReadOutElementDoubleStrip, TF1*> m_ResolutionCalibration;
		//! Energy calibration file name
		MString m_EnergyCalibrationFilename;

		//! Dead strip file name
		MString m_DeadStripFilename;
		//! List of dead strips
		int m_DeadStrips[12][2][37];

		//! Depth calibrator class
		MNCTDepthCalibrator* m_DepthCalibrator;

		//! Tiny helper class for MNCTDetectorEffectsEngineCOSI describing a special strip hit
		class MNCTDEEStripHit
		{
			public:
				//! The read-out element
				MReadOutElementDoubleStrip m_ROE;
				//! The ADC value
				double m_ADC;
				//! The timing value;
				double m_Timing;

				//! The simulated position
				MVector m_Position;
				//! The simulated energy deposit
				double m_Energy;

				//! True if this is a guard ring
				bool m_IsGuardRing;

				vector<MNCTDEEStripHit> m_OppositeStrips;

				//! list of origins of strip hits from cosima output
				list<int> m_Origins;

				//! A list of original strip hits making up this strip hit
				vector<MNCTDEEStripHit> m_SubStripHits;

				//! lists indices of other substriphits that have same IA origin
				vector<int> m_SharedOrigin;

		};

	public:
		//! Convert Energy to ADC value
		int EnergyToADC(MNCTDEEStripHit& Hit, double energy);


};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
MNCTDetectorEffectsEngineCOSI::MNCTDetectorEffectsEngineCOSI() : m_Interrupt(false)
{
	gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
MNCTDetectorEffectsEngineCOSI::~MNCTDetectorEffectsEngineCOSI()
{
	// Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool MNCTDetectorEffectsEngineCOSI::ParseCommandLine(int argc, char** argv)
{
	ostringstream Usage;
	Usage<<"cosidee - The COSI detector effects engine"<<endl;
	Usage<<endl;
	Usage<<"  Usage: cosidee <options>"<<endl;
	Usage<<"    General options:"<<endl;
	Usage<<"         -f:   simulation file name"<<endl;
	Usage<<"         -g:   geometry file name"<<endl;
	Usage<<"         -e:   energy calibration file name"<<endl;
	Usage<<"         -t:   thresholds file name"<<endl;
	Usage<<"         -d:   dead strip file name"<<endl;
	Usage<<"         -h:   print this help"<<endl;
	Usage<<"         -D:   depth calibration coefficients filename"<<endl;
	Usage<<"         -s:   depth calibration splines file"<<endl;
	Usage<<endl;

	string Option;

	// Check for help
	for (int i = 1; i < argc; i++) {
		Option = argv[i];
		if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
			cout<<Usage.str()<<endl;
			return false;
		}
	}


	// Now parse the command line options:
	for (int i = 1; i < argc; i++) {
		Option = argv[i];

		// First check if each option has sufficient arguments:
		// Single argument
		if (Option == "-f" || Option == "-o") {
			if (!((argc > i+1) && argv[i+1][0] != '-')){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage.str()<<endl;
				return false;
			}
		} 
		// Multiple arguments_
		//else if (Option == "-??") {
		//  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
		//    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
		//    cout<<Usage.str()<<endl;
		//    return false;
		//  }
		//}

		// Then fulfill the options:
		if (Option == "-f") {
			m_FileName = argv[++i];
			cout<<"Accepting file name: "<<m_FileName<<endl;
		} else if (Option == "-g") {
			m_GeometryFileName = argv[++i];
			cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
		} else if (Option == "-e") {
			m_EnergyCalibrationFilename = argv[++i];
			cout << "Accepting energy calibration file name: " << m_EnergyCalibrationFilename << endl;
		} else if (Option == "-t") {
			m_ThresholdFilename = argv[++i];
			cout << "Accepting threshold file name: " << m_ThresholdFilename << endl;
			cout << "Accepting energy calibration file name: " << m_EnergyCalibrationFilename << endl;
		} else if (Option == "-d") {
			m_DeadStripFilename = argv[++i];
			cout << "Accepting dead strip file name: " << m_DeadStripFilename << endl;
		} else if (Option == "-D"){
			m_DepthCalibrationCoeffsFileName = argv[++i];
			cout << "Accepting depth calibration coefficients file name: " << m_DepthCalibrationCoeffsFileName << endl;
		} else if (Option == "-s"){
			m_DepthCalibrationSplinesFileName = argv[++i];
			cout << "Accepting depth calibration splines file name: " << m_DepthCalibrationSplinesFileName << endl;
		} else {
			cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			cout<<Usage.str()<<endl;
			return false;
		}
	}

	if (m_FileName == "") {
		cout<<"Error: Need a simulation file name!"<<endl;
		cout<<Usage.str()<<endl;
		return false;
	}

	if (m_GeometryFileName == "") {
		cout<<"Error: Need a geometry file name!"<<endl;
		cout<<Usage.str()<<endl;
		return false;
	}

	if (m_ThresholdFilename == "") {
		cout<<"Error: Need a threshold file name!"<<endl;
		cout<<Usage.str()<<endl;
		return false;
	}


	if (m_FileName.EndsWith(".sim") == false) {
		cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
		cout<<Usage.str()<<endl;
		return false;
	}

	return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool MNCTDetectorEffectsEngineCOSI::Analyze()
{

	static TRandom3 r(0);

	// Load geometry:
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
		Geometry->ActivateNoising(false);
		Geometry->SetGlobalFailureRate(0.0);
		cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
	} else {
		cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
		return false;
	}  

	//load energy calibration information
	ParseEnergyCalibrationFile();
	//load dead strip information
	ParseDeadStripFile();
	//load threshold information
	ParseThresholdFile();

  m_DepthCalibrator = new MNCTDepthCalibrator();
	if( m_DepthCalibrator->LoadCoeffsFile(m_DepthCalibrationCoeffsFileName) == false ){
		cout << "Unable to load depth calibration coefficients file - Aborting!" << endl;
		return false;
	}

	if( m_DepthCalibrator->LoadSplinesFile(m_DepthCalibrationSplinesFileName) == false ){
		cout << "Unable to load depth calibration splines file - Aborting!" << endl;
		return false;
	}


	MFileEventsSim* Reader = new MFileEventsSim(Geometry);
	if (Reader->Open(m_FileName) == false) {
		cout<<"Unable to open sim file "<<m_FileName<<" - Aborting!"<<endl; 
		return false;
	}
	Reader->ShowProgress();

	MString RoaFileName = m_FileName;
	RoaFileName.ReplaceAllInPlace(".sim", ".roa");
	cout << "Output File: " << RoaFileName << endl;

	ofstream out;
	out.open(RoaFileName);
	out<<endl;
	out<<"TYPE   ROA"<<endl;
	out<<"UF     doublesidedstrip adc-timing-origins"<<endl;
	out<<endl;

	//  TCanvas specCanvas("c","c",600,400); 
	//  Threshold1D* spectrum = new Threshold1D("spec","spectrum",40,640,680);

	//count how many events have multiple hits per strip
	int mult_hits_counter = 0;
	int charge_loss_counter = 0;

	// for shield veto: shield pulse duration and card cage delay: constant for now
	double shield_pulse_duration = 1.e-6;
	double cc_delay = 700.e-9;
	// for shield veto: gets updated with shield event times
	// start at -10s so that it doesn't veto beginning events by accident
	double shield_time = -10;

	MSimEvent* Event = 0;
	//int RunningID = 0;
  
	while ((Event = Reader->GetNextEvent(false)) != 0) {

    // Hitting Ctrl-C raises this flag
		if (m_Interrupt == true) return false;

		if (Event->GetNHTs() == 0) {
      cout<<Event->GetID()<<": No hits"<<endl;
			delete Event;
			continue;
		}

		
		// Step (0): Check whether events should be vetoed
		double evt_time = Event->GetTime().GetAsSeconds();
    
		// first check if there's another shield hit above the threshold
		// if so, veto event
		for (unsigned int h=0; h<Event->GetNHTs(); h++){
			MSimHT* HT = Event->GetHTAt(h);
			if (HT->GetDetectorType() == 4) {
				MDVolumeSequence* VS = HT->GetVolumeSequence();
				MDDetector* Detector = VS->GetDetector();
				MString DetName = Detector->GetName();

				double energy = HT->GetEnergy();
				energy = NoiseShieldEnergy(energy,DetName);
				HT->SetEnergy(energy);

				if (energy > 80.){
					shield_time = evt_time;
				}
			}
		}

		if (evt_time + cc_delay < shield_time + shield_pulse_duration){
			delete Event;
      cout<<Event->GetID()<<": Vetoed"<<endl;
			continue;
		}


		//get interactions to look for ionization in hits
		vector<MSimIA*> IAs;
		for (unsigned int i=0; i<Event->GetNIAs(); i++){
			MSimIA* ia = Event->GetIAAt(i);
			IAs.push_back(ia);
		}


		// Step (1): Convert positions into strip hits
		list<MNCTDEEStripHit> StripHits;

		vector<vector<MNCTDEEStripHit> > SimHits;
		vector<MNCTDEEStripHit> tempVec;

		// (1a) The real strips
		for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
			MSimHT* HT = Event->GetHTAt(h);

			MDVolumeSequence* VS = HT->GetVolumeSequence();
			MDDetector* Detector = VS->GetDetector();
			MString DetectorName = Detector->GetName();
			if(!DetectorName.BeginsWith("Detector")){
				continue; //probably a shield hit.  this can happen if the veto flag is off for the shields
			}
			DetectorName.RemoveAllInPlace("Detector");
			int DetectorID = DetectorName.ToInt();

			MNCTDEEStripHit pSide;
			MNCTDEEStripHit nSide;

			pSide.m_ROE.IsPositiveStrip(true);
			nSide.m_ROE.IsPositiveStrip(false);

			// Convert detector name in detector ID
			pSide.m_ROE.SetDetectorID(DetectorID);
			nSide.m_ROE.SetDetectorID(DetectorID);

			// Convert position into
			MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
			MDGridPoint GP = Detector->GetGridPoint(PositionInDetector);
			double Depth_ = PositionInDetector.GetZ();
			double Depth = -(Depth_ - (m_DepthCalibrator->GetThickness(DetectorID)/2.0)); // change the depth coordinates so that one side is 0.0 cm and the other side is ~1.5cm

			// Not sure about if p or n-side is up, but we can debug this later
			pSide.m_ROE.SetStripID(38-(GP.GetYGrid()+1));
			nSide.m_ROE.SetStripID(38-(GP.GetXGrid()+1));

//			cout << pSide.m_ROE.GetStripID() << '\t' << nSide.m_ROE.GetStripID() << endl;

			//SetStripID needs to be called before we can look up the depth calibration coefficients
			int PixelCode = DetectorID*10000 + pSide.m_ROE.GetStripID()*100 + nSide.m_ROE.GetStripID();
			std::vector<double>* Coeffs = m_DepthCalibrator->GetPixelCoeffs(PixelCode);
			if( Coeffs == NULL ){
				//pixel is not calibrated! discard this event....
				cout << "pixel " << PixelCode << " has no depth calibration... discarding event" << endl;
				//delete Event;
				continue;
			}

			pSide.m_Timing = (Coeffs->at(0) * m_DepthCalibrator->GetCathodeSpline(DetectorID)->Eval(Depth)) + (Coeffs->at(1)/2.0);
			nSide.m_Timing = (Coeffs->at(0) * m_DepthCalibrator->GetAnodeSpline(DetectorID)->Eval(Depth)) - (Coeffs->at(1)/2.0);

			pSide.m_Energy = HT->GetEnergy();
			nSide.m_Energy = HT->GetEnergy();

			pSide.m_Position = PositionInDetector;
			nSide.m_Position = PositionInDetector;

			//check for ionization if it's in the sim file
			vector<int> origin = HT->GetOrigins();
			pSide.m_Origins = list<int>(origin.begin(), origin.end());
			nSide.m_Origins = list<int>(origin.begin(), origin.end());

			StripHits.push_back(pSide);
			StripHits.push_back(nSide);

			//set the m_Opposite pointers to each other

			//add striphits to SimHits vector
			tempVec.clear();
			tempVec.push_back(pSide);
			tempVec.push_back(nSide);
			SimHits.push_back(tempVec);

		}


		// (1b) The guard ring hits
		for (unsigned int h = 0; h < Event->GetNGRs(); ++h) {
			MSimGR* GR = Event->GetGRAt(h);
			MDVolumeSequence* VS = GR->GetVolumeSequence();
			MDDetector* Detector = VS->GetDetector();
			MString DetectorName = Detector->GetName();
			DetectorName.RemoveAllInPlace("Detector_");
			int DetectorID = DetectorName.ToInt();

			MNCTDEEStripHit GuardRingHit;
			GuardRingHit.m_ROE.IsPositiveStrip(true); // <-- not important
			GuardRingHit.m_ROE.SetDetectorID(DetectorID);
			GuardRingHit.m_ROE.SetStripID(38); // ?

			GuardRingHit.m_Energy = GR->GetEnergy();
			GuardRingHit.m_Position = MVector(0, 0, 0); // <-- not important
		}


		// (1c): Merge strip hits
		list<MNCTDEEStripHit> MergedStripHits;
		while (StripHits.size() > 0) {
			MNCTDEEStripHit Start;
			Start.m_SubStripHits.push_back(StripHits.front());
			StripHits.pop_front();
			Start.m_ROE = Start.m_SubStripHits.front().m_ROE;

			list<MNCTDEEStripHit>::iterator i = StripHits.begin();
			while (i != StripHits.end()) {
				if ((*i).m_ROE == Start.m_ROE) {
					Start.m_SubStripHits.push_back(*i);
					i = StripHits.erase(i);
				} else {
					++i;
				}
			}
			MergedStripHits.push_back(Start);
		}

		bool fromSameInteraction = true;
		for (MNCTDEEStripHit& Hit: MergedStripHits){
			int nSubHits = Hit.m_SubStripHits.size();
			if (nSubHits > 1){
				for (int i=0; i<nSubHits-1; i++){
					for (int j=i+1; j<nSubHits; j++){
						MNCTDEEStripHit& SubHit1 = Hit.m_SubStripHits.at(i);
						MNCTDEEStripHit& SubHit2 = Hit.m_SubStripHits.at(j);
						fromSameInteraction = false;
	          
            for (int o1: SubHit1.m_Origins) {
              for (int o2: SubHit1.m_Origins) {
								if (o1 == o2){
									fromSameInteraction = true;
								}
							}
						}
						if (fromSameInteraction){
							//have to mark it in the strip hits so that
							// charge loss effect can be applied
							SubHit1.m_SharedOrigin.push_back(j);
							SubHit2.m_SharedOrigin.push_back(i);
						}

						//these counters are not actually counting the right thing right now...
						if (fromSameInteraction){ charge_loss_counter++; }
						else{	mult_hits_counter++; }
					}
				}
			}
		}

		// Merge origins
		for (MNCTDEEStripHit& Hit: MergedStripHits) {
      Hit.m_Origins.clear();
      for (MNCTDEEStripHit& SubHit: Hit.m_SubStripHits) {
        for (int& Origin: SubHit.m_Origins) {
          Hit.m_Origins.push_back(Origin);
        }
      }
      Hit.m_Origins.sort();
      Hit.m_Origins.unique();      
    }
		
		// Step (2): Calculate and noise timing
		const double TimingNoise = 3.76; //ns//I have been assuming 12.5 ns FWHM on the CTD... so the 1 sigma error on the timing value should be (12.5/2.35)/sqrt(2)
		for (MNCTDEEStripHit& Hit: MergedStripHits) {
			
			//find lowest timing value 
			double LowestNoisedTiming = Hit.m_SubStripHits.front().m_Timing + gRandom->Gaus(0,TimingNoise);
			for(size_t i = 1; i < Hit.m_SubStripHits.size(); ++i){
				double Timing = Hit.m_SubStripHits.at(i).m_Timing + gRandom->Gaus(0,TimingNoise);
				//SubHit.m_Timing += gRandom->Gaus(0,TimingNoise);
				if( Timing < LowestNoisedTiming ) LowestNoisedTiming = Timing;
			}
			LowestNoisedTiming -= fmod(LowestNoisedTiming,5.0); //round down to nearest multiple of 5
			Hit.m_Timing = LowestNoisedTiming;

		}


		// Step (3): Calculate and noise ADC values including charge loss, charge sharing, ADC overflow!

		// (3a) Do charge sharing
		// (3b) Do charge loss

		// (3c) Noise energy deposit
		for (MNCTDEEStripHit& Hit: MergedStripHits) {
			double Energy = 0;
			for (MNCTDEEStripHit SubHit: Hit.m_SubStripHits) {
				Energy += SubHit.m_Energy;
			}

			Hit.m_Energy = Energy;
			Hit.m_ADC = EnergyToADC(Hit,Energy);
			//      spectrum->Fill(Energy);
			//      cout << "Energy: " << Energy << '\t';
			//      cout << "ADC: " << Hit.m_ADC << endl;
			// Let's do a dummy energy:
			//      Hit.m_ADC = int(10*Energy);
		}

		// (3d) Handle ADC overflow


		// Step (4): Apply thresholds and triggers including guard ring hits
		//           * use the trigger threshold calibration and invert it here 
		//           * take care of guard ring hits with their special thresholds
		//           * take care of hits in dead strips
		//           * throw out hits which did not trigger

		// (4a) Take care of dead strips:
		list<MNCTDEEStripHit>::iterator j = MergedStripHits.begin();
		while (j != MergedStripHits.end()) {
			int det = (*j).m_ROE.GetDetectorID();
			int stripID = (*j).m_ROE.GetStripID();
			bool side_b = (*j).m_ROE.IsPositiveStrip();
			int side = 0;
			if (side_b) {side = 1;}

			//if strip has been flagged as dead, erase strip hit
			if (m_DeadStrips[det][side][stripID-1] == 1){
				j = MergedStripHits.erase(j);
			}
			else {
				++j;
			}
		}

		// (4b) Handle trigger thresholds make sure we throw out timing too!
		list<MNCTDEEStripHit>::iterator i = MergedStripHits.begin();
		while (i != MergedStripHits.end()) {
			if ((*i).m_Energy < 20) { // Dummy threshold of 0 keV sharp
				i = MergedStripHits.erase(i);
			} else {
				if((*i).m_Energy < 55){
					(*i).m_Timing = 0.0;
				}
				++i;
			}
		}

		// (4c) Take care of guard ring vetoes


		// Step (5): Split into card cage events - i.e. split by detector
/*		vector<vector<MNCTDEEStripHit>> CardCagedStripHits;
		for (MNCTDEEStripHit Hit: MergedStripHits) {
			bool Found = false;
			for (vector<MNCTDEEStripHit>& V: CardCagedStripHits) {
				if (V[0].m_ROE.GetDetectorID() == Hit.m_ROE.GetDetectorID()) {
					V.push_back(Hit);
					Found = true;
				}
			}
			if (Found == false) {
				vector<MNCTDEEStripHit> New;
				New.push_back(Hit);
				CardCagedStripHits.push_back(New);
			}
		}


		// Step (6): Determine and noise the global event time
		vector<double> CardCageTiming(CardCagedStripHits.size());
		for (double& T: CardCageTiming) {
			T = Event->GetTime().GetAsSeconds();
		}
*/


		// Step (7): Dump event to file in ROA format
		out<<"SE"<<endl;
		out<<"ID "<<Event->GetID()<<endl;
		//out<<"ID "<<++RunningID<<endl;
    for (unsigned int i = 0; i < IAs.size(); ++i) {
      out<<IAs[i]->ToSimString()<<endl;
    }
		out<<"TI "<<Event->GetTime().GetAsSeconds() << endl;
		for (MNCTDEEStripHit Hit: MergedStripHits){
			out<<"UH "<<Hit.m_ROE.GetDetectorID()<<" "<<Hit.m_ROE.GetStripID()<<" "<<(Hit.m_ROE.IsPositiveStrip() ? "p" : "n")<<" "<<Hit.m_ADC<<" "<<Hit.m_Timing;
      
      MString Origins;
      for (int Origin: Hit.m_Origins) {
        if (Origins != "") Origins += ";";
        Origins += Origin;
      }
      if (Origins == "") Origins += "-";
      out<<" "<<Origins<<endl;
		}
		
		// Never forget to delete the event
		delete Event;
	}

	//  spectrum->Draw();
	//  specCanvas.Print("spectrum_adc.pdf");

//	cout << "number of events with multiple hits per strip: " << mult_hits_counter << endl;
//	cout << "charge loss applies counter: " << charge_loss_counter << endl;

	// Some cleanup
	out<<"EN"<<endl<<endl;
	out.close();
	delete Reader;
	delete Geometry;

	return true;
}


/******************************************************************************
 * Convert energy to ADC value by reversing energy calibration done in 
 * MNCTModuleEnergyCalibrationUniversal.cxx
 */
int MNCTDetectorEffectsEngineCOSI::EnergyToADC(MNCTDEEStripHit& Hit, double mean_energy)
{  
	//first, need to simulate energy spread
	static TRandom3 r(0);
	TF1* FitRes = m_ResolutionCalibration[Hit.m_ROE];
	//resolution is a function of energy
	double EnergyResolution = 3; //default to 3keV...does this make sense?
	if (FitRes != 0){
		EnergyResolution = FitRes->Eval(mean_energy);
	}

	//get energy from gaussian around mean_energy with sigma=EnergyResolution
	//TRandom3 r(0);
	double energy = r.Gaus(mean_energy,EnergyResolution);
	//  spectrum->Fill(energy);

	//  if (fabs(mean_energy-662.) < 5){
	//    cout << mean_energy << '\t' << EnergyResolution << '\t' << energy << endl;
	//  }

	//then, convert energy to ADC
	double ADC_double = 0;

	//get the fit function
	TF1* Fit = m_EnergyCalibration[Hit.m_ROE];

	if (Fit != 0) {
		//find roots
		ADC_double = Fit->GetX(energy,0.,8000.);
	}


	int ADC = int(ADC_double);
	return ADC;

}

/******************************************************************************
 * Noise shield energy with measured resolution
 */
double MNCTDetectorEffectsEngineCOSI::NoiseShieldEnergy(double energy, MString shield_name)
{ 

	double resolution_consts[6] = {3.75,3.74,18.47,4.23,3.07,3.98};

	shield_name.RemoveAllInPlace("Shield");
	int shield_num = shield_name.ToInt();
	shield_num = shield_num - 1;
	double res_constant = resolution_consts[shield_num];

	TF1* ShieldRes = new TF1("ShieldRes","[0]*(x^(1/2))",0,1000); //this is from Knoll
	ShieldRes->SetParameter(0,res_constant);

	double sigma = ShieldRes->Eval(energy);

	TRandom3 r(0);
	double noised_energy = r.Gaus(energy,sigma);

	return noised_energy;

}

/******************************************************************************
 * Read in thresholds
 */
void MNCTDetectorEffectsEngineCOSI::ParseThresholdFile()
{
	MParser Parser;
	if (Parser.Open(m_ThresholdFilename, MFile::c_Read) == false) {
		cout << "Unable to open threshold file " << m_ThresholdFilename << endl;
		return;
	}
	map<MReadOutElementDoubleStrip, double> LowerThreshold;
	map<MReadOutElementDoubleStrip, double> UpperThreshold; 

	for (unsigned int i=0; i<Parser.GetNLines(); i++) {
		unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
		if (NTokens < 2) continue;
		if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"Threshold") == true) {

			if (Parser.GetTokenizerAt(i)->IsTokenAt(1,"(LOWER)") == true ||
					Parser.GetTokenizerAt(i)->IsTokenAt(1,"(UPPER)") == true) {

				MReadOutElementDoubleStrip R;
				R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
				R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(4));
				R.IsPositiveStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(5) == "p");

				if (Parser.GetTokenizerAt(i)->IsTokenAt(1,"(LOWER)") == true) {
					LowerThreshold[R] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(6);
					//cout << LowerThreshold[R] << endl; //to verify lower thresholds
				} 
				else if (Parser.GetTokenizerAt(i)->IsTokenAt(1,"(UPPER)") == true) {
					UpperThreshold[R] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(6);
					//cout << UpperThreshold[R] << endl; //to verify upper thresholds
				}
				else {
					cout << "Error parsing threshold file." << endl;
				}
			}
		}
	}
}


/******************************************************************************
 * Parse ecal file: should be done once at the beginning to save all the poly3 coefficients
 */
void MNCTDetectorEffectsEngineCOSI::ParseEnergyCalibrationFile()
{
	MParser Parser;
	if (Parser.Open(m_EnergyCalibrationFilename, MFile::c_Read) == false){
		cout << "Unable to open calibration file " << m_EnergyCalibrationFilename << endl;
		return;
	}

	map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine; //Energy Calibration Model
	map<MReadOutElementDoubleStrip, unsigned int> CR_ROEToLine; //Energy Resolution Calibration Model
	//used to make sure there are enough data points:
	map<MReadOutElementDoubleStrip, unsigned int> CP_ROEToLine; //peak fits

	for (unsigned int i=0; i<Parser.GetNLines(); i++){
		unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
		if (NTokens < 2) continue;
		if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"CM") == true || 
				Parser.GetTokenizerAt(i)->IsTokenAt(0,"CP") == true ||
				Parser.GetTokenizerAt(i)->IsTokenAt(0,"CR") == true) {

			if (Parser.GetTokenizerAt(i)->IsTokenAt(1,"dss") == true) {
				MReadOutElementDoubleStrip R;
				R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
				R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
				R.IsPositiveStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p");

				if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"CM") == true) {
					CM_ROEToLine[R] = i;
				}
				else if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"CP") == true) {
					CP_ROEToLine[R] = i;
				}
				else {
					CR_ROEToLine[R] = i;
				}
			}
		}
	}

	for (auto CM: CM_ROEToLine){

		//only use calibration if we have 3 data points
		if (CP_ROEToLine.find(CM.first) != CP_ROEToLine.end()){
			unsigned int i = CP_ROEToLine[CM.first];
			if (Parser.GetTokenizerAt(i)->IsTokenAt(5,"pakw") == true){
				if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6) < 3){
					continue;
				}
			}
		}


		//get the fit function from the file
		unsigned int Pos = 5;
		MString CalibratorType = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsString(Pos);
		CalibratorType.ToLower();

		//for now Carolyn just does poly3, so I am only doing that one
		if (CalibratorType == "poly3"){
			double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
			double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
			double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
			double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

			//from fit parameters, define function
			TF1* melinatorfit = new TF1("poly3","[0]+[1]*x+[2]*x^2+[3]*x^3",0.,8000.);
			melinatorfit->FixParameter(0,a0);
			melinatorfit->FixParameter(1,a1);
			melinatorfit->FixParameter(2,a2);
			melinatorfit->FixParameter(3,a3);

			//Define the map by saving the fit function I just created as a map to the current ReadOutElement
			m_EnergyCalibration[CM.first] = melinatorfit;
		}
	}

	for (auto CR: CR_ROEToLine){

		unsigned int Pos = 5;
		MString CalibratorType = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsString(Pos);
		CalibratorType.ToLower();
		if (CalibratorType == "p1"){
			double f0 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
			double f1 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
			TF1* resolutionfit = new TF1("P1","([0]+[1]*x)^(1/2)",0.,2000.);
			resolutionfit->FixParameter(0,f0);
			resolutionfit->FixParameter(1,f1);

			m_ResolutionCalibration[CR.first] = resolutionfit;
		}
	}

}


/******************************************************************************
 * Parse the dead strip file
 */
void MNCTDetectorEffectsEngineCOSI::ParseDeadStripFile()
{  
	//initialize m_DeadStrips: set all values to 0
	for (int i=0; i<12; i++) {
		for (int j=0; j<2; j++) {
			for (int k=0; k<37; k++) {
				m_DeadStrips[i][j][k] = 0;
			}
		}
	}

	ifstream deadStripFile;
	deadStripFile.open(m_DeadStripFilename);

	if (!deadStripFile.is_open()) {
		cout << "Error opening dead strip file" << endl;
		return;
	}

	string line;
	vector<int> lineVec;
	while (deadStripFile.good() && !deadStripFile.eof()) {
		getline(deadStripFile,line);
		stringstream sLine(line);
		string sub;
		int sub_int;

		while (sLine >> sub){
			sub_int = atoi(sub.c_str());
			lineVec.push_back(sub_int);
		}

		if (lineVec.size() != 3) {
			continue;
		}

		int det = lineVec.at(0);
		int side = lineVec.at(1);
		int strip = lineVec.at(2)-1; //in file, strips go from 1-37; in m_DeadStrips they go from 0-36
		lineVec.clear();

		//any dead strips have their value in m_DeadStrips set to 1 
		m_DeadStrips[det][side][strip] = 1;
	}
}


/******************************************************************************/

MNCTDetectorEffectsEngineCOSI* g_Prg = 0;
int g_NInterrupts = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
	cout<<"Catched signal Ctrl-C:"<<endl;

	--g_NInterrupts;
	if (g_NInterrupts <= 0) {
		cout<<"Aborting..."<<endl;
		abort();
	} else {
		cout<<"Trying to cancel the analysis..."<<endl;
		if (g_Prg != 0) {
			g_Prg->Interrupt();
		}
		cout<<"If you hit "<<g_NInterrupts<<" more times, then I will abort immediately!"<<endl;
	}
}

/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
	// Set a default error handler and catch some signals...
	signal(SIGINT, CatchSignal);

	// Initialize global MEGAlib variables, especially mgui, etc.
	MGlobal::Initialize();

	TApplication MNCTDetectorEffectsEngineCOSIApp("MNCTDetectorEffectsEngineCOSIApp", 0, 0);

	g_Prg = new MNCTDetectorEffectsEngineCOSI();

	if (g_Prg->ParseCommandLine(argc, argv) == false) {
		cerr<<"Error during parsing of command line!"<<endl;
		return -1;
	} 
	if (g_Prg->Analyze() == false) {
		cerr<<"Error during analysis!"<<endl;
		return -2;
	} 

	//MNCTDetectorEffectsEngineCOSIApp.Run();

	cout<<"Program exited normally!"<<endl;

	return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
