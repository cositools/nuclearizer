/*
 * MNCTModuleStripPairingGreedy_b.cxx
 *
 *
 * Copyright (C) by Clio Sleator & Daniel Perez-Becker.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Clio Sleator & Daniel Perez-Becker.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleStripPairingGreedy_b
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleStripPairingGreedy_b.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTMath.h"
#include "MGUIOptionsStripPairing.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleStripPairingGreedy_b)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairingGreedy_b::MNCTModuleStripPairingGreedy_b() : MModule()
{
  // Construct an instance of MNCTModuleStripPairingGreedy_a
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Strip pairing - Clio's \"Greedy\" version";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "StripPairingGreedy_b";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_StripPairing);
  
  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;

	m_Mode = 1;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  m_NBadMatches = 0;
  m_NMatches = 0;
  m_TotalMatches = 0;
  
  // Set the histogram display
  m_ExpoStripPairing = new MGUIExpoStripPairing(this);
  m_ExpoStripPairing->SetEnergiesHistogramParameters(1500, 0, 1500);
  m_Expos.push_back(m_ExpoStripPairing);
  
  m_NAllowedWorkerThreads = 1;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairingGreedy_b::~MNCTModuleStripPairingGreedy_b()
{
  // Delete this instance of MNCTModuleStripPairingGreedy_b
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairingGreedy_b::Initialize()
{
  // Initialize the module 
  
  Eth = 30;
  
  // Add all initializations which are global to all events
  // and have member variables here
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////

//main data analysis routine, which updates the event to a new level
bool MNCTModuleStripPairingGreedy_b::AnalyzeEvent(MReadOutAssembly* Event){
  
  const int nDetectors = 12;

	bool newAlg;
	if (m_Mode == 0) { newAlg = true; }
	else { newAlg = false; }

  for (int detector = 0; detector < nDetectors; detector++){
    bool runRest = GetEventInfo(Event, detector);

    if (nHits.at(0)>0 && nHits.at(1)>0 && runRest==true){
			bool print = false;

//			if (nHits.at(0)>1 && nHits.at(1)>1){print = true;}

			if (print) {
				cout << endl << endl << endl <<"---------------------------------------------------------" << endl;
			}
			if (print) {cout << "nx: " << nHits.at(0) << '\t' << "ny: " << nHits.at(1) << endl;}

			if (newAlg){
//				PrintXYStripsHit();
				InitializeKillMatrices();
				CheckForAdjacentStrips();
				FindFinalPairsCorrected();
				WriteHits(Event, detector);
				vector<vector<int> > strips = GetStripsHit();
			}
			else {
	    	InitializeKillMatrices();
	//			if (print) {PrintKillMatrix();}
				if (print) {PrintXYStripsHit();}
	      CheckForAdjacentStrips();
	//			if (print) {
	//				cout << "checked adjacent strips..." << endl;
	//				PrintXYStripsHit();
	//				PrintKillMatrix();
	//			}
	      CheckMultipleHits();
	//			if (print){
	//				cout << "checked multiple hits..." << endl;
	//				PrintXYStripsHit();
	//				PrintKillMatrix();
	//			}
	      CheckForBadCombinations();
	      FindFinalPairs();
	      CalculateDetectorQuality();
	      CheckInitialEnergyDifference();
	      WriteHits(Event, detector);
	    }
		}
	}
/*	    else {
	      detectorQualityFactors.push_back(0);
	    }
    //		cout << "size: " << detectorQualityFactors.size() << endl;
	  }
//	}
	  //	cout << "done with loops" << endl;
	  //	cout << "size: " << detectorQualityFactors.size() << endl;
	  CalculateEventQuality(Event, nDetectors);
	  detectorQualityFactors.clear();
	}
*/
 
/* 
  // Flag events with poorly matched strips...
  for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
    int pNStrips = 0;
    double pEnergy = 0.0;
    double pUncertainty = 0.0;
    int nNStrips = 0;
    double nEnergy = 0.0;
    double nUncertainty = 0.0;
    
    for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); ++s) {
      if (Event->GetHit(h)->GetStripHit(s)->IsXStrip() == true) {
        ++pNStrips;
        pEnergy += Event->GetHit(h)->GetStripHit(s)->GetEnergy(); 
        pUncertainty += pow(Event->GetHit(h)->GetStripHit(s)->GetEnergyResolution(), 2);
      } else {
        ++nNStrips;
        nEnergy += Event->GetHit(h)->GetStripHit(s)->GetEnergy(); 
        nUncertainty += pow(Event->GetHit(h)->GetStripHit(s)->GetEnergyResolution(), 2);
      }
    }
    if (pNStrips > 0 && nNStrips > 0) { // Just a place holder atthe moment...
      m_ExpoStripPairing->AddEnergies(pEnergy, nEnergy);
    }
    
    pUncertainty = sqrt(pUncertainty);
    nUncertainty = sqrt(nUncertainty);
    
    double Difference = fabs(pEnergy - nEnergy);
    // Difference must be more than 10 keV for cross talk + 2 sigma energy resolution on *both* sides
    if (Difference > 2*pUncertainty + 2*nUncertainty + 10) {
      Event->SetStripPairingIncomplete(true);
      if (g_Verbosity >= c_Warning) cout<<"Bad strip pairing: p: E="<<pEnergy<<" dE="<<pUncertainty<<" n: E="<<nEnergy<<" dE="<<nUncertainty<<endl; 
    } else {
      if (g_Verbosity >= c_Info) cout<<"Good strip pairing: p: E="<<pEnergy<<" dE="<<pUncertainty<<" n: E="<<nEnergy<<" dE="<<nUncertainty<<endl; 
    }
  }  
  
  if (g_Verbosity >= c_Info) {
    cout<<"After strip pairing..."<<endl;
    for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
      cout<<"Hit "<<h<<endl;
      Event->GetHit(h)->StreamDat(cout); 
      for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); ++s) {
        Event->GetHit(h)->GetStripHit(s)->StreamDat(cout); 
      }
    }
  }
 */ 
  Event->SetAnalysisProgress(MAssembly::c_StripPairing);
  
  return true;
};

//this function takes the MReadOutAssembly and get all the info from it.
bool MNCTModuleStripPairingGreedy_b::GetEventInfo(MReadOutAssembly* Event, int detector) {
  
  ClearMembers();  
  
  //clear nHits (from the previous detector)
  int n_x = 0;
  int n_y = 0;
  
  //Find the number of hits per side for this detector
  for (unsigned int i = 0; i < Event->GetNStripHits(); i++){
    if (detector == Event->GetStripHit(i)->GetDetectorID()){
      if (Event->GetStripHit(i)->IsXStrip() == true){
        n_x += 1;
      }
      else {
        n_y += 1;
      }
      
    }
  }
  
  //	cout << "detector: " << detector << endl;
  //	cout << "n_x: " << n_x << endl;
  //	cout << "n_y: " << n_y << endl;
  
  
  //IMPORTANT: THE IF STATEMENT BELOW SETS CAPS ON THE KIND OF EVENTS ANALIZED BY THE CODE!!!!!!!!
  
  if( (n_x > 0) && (n_y > 0) && (fabs(n_x - n_y) < 5) && (n_x < 8) && (n_y < 8)) {
    
    vector<int> xStripsHit, yStripsHit;
    vector<float> xEnergy, yEnergy, xSigma, ySigma;
    int stripID;
    float stripEnergy, stripSigma;
    double inf = numeric_limits<double>::infinity();
    
		//skip events where initial n and p energies are not close together 
/*
		float xTotalEnergy = 0;
		float yTotalEnergy = 0;
    for (unsigned int i=0; i<Event->GetNStripHits(); i++){
      if (detector == Event->GetStripHit(i)->GetDetectorID()){
        if (Event->GetStripHit(i)->IsXStrip() == true){
          xTotalEnergy += Event->GetStripHit(i)->GetEnergy();
        }    
        else {
			    yTotalEnergy += Event->GetStripHit(i)->GetEnergy();
        }
			}
		}
    if (fabs(xTotalEnergy-yTotalEnergy) > 20){ 
	     nHits.push_back(0);
  	   nHits.push_back(0);
       return false;
   	 }    
*/

		//skip events that aren't single-site events
		int xCounter = 0;
		int yCounter = 0;
		for (unsigned int i=0; i<Event->GetNStripHits(); i++){
			if (detector == Event->GetStripHit(i)->GetDetectorID()){
				if (Event->GetStripHit(i)->IsXStrip() == true){
					xCounter += 1;
				}
				else {
					yCounter += 1;
				}
			}
		}
		if (xCounter != 1 || yCounter != 1){
			nHits.push_back(0);
			nHits.push_back(0);
			return false;
		}


    for (unsigned int i=0; i<Event->GetNStripHits(); i++){
      if (detector == Event->GetStripHit(i)->GetDetectorID()){
        if (Event->GetStripHit(i)->IsXStrip() == true){
          stripID = Event->GetStripHit(i)->GetStripID();
          stripEnergy = Event->GetStripHit(i)->GetEnergy();
          stripSigma = Event->GetStripHit(i)->GetEnergyResolution();
          //make sure energy and error are NOT inf, 0, nan
          if (stripEnergy!=0 && stripEnergy!=inf && !isnan(stripEnergy) && stripSigma!=0 && stripSigma!=inf && !isnan(stripSigma)){
            xStripsHit.push_back(stripID);
            xEnergy.push_back(stripEnergy);
            xSigma.push_back(stripSigma);
          }
        }
        else {
          stripID = Event->GetStripHit(i)->GetStripID();
          stripEnergy = Event->GetStripHit(i)->GetEnergy();
          stripSigma = Event->GetStripHit(i)->GetEnergyResolution();
          if (stripEnergy!=0 && stripEnergy!=inf && !isnan(stripEnergy) && stripSigma!=0 && stripSigma!=inf && !isnan(stripSigma)){
            yStripsHit.push_back(stripID);
            yEnergy.push_back(stripEnergy);
            ySigma.push_back(stripSigma);
        }
        }
      }
    }	

    stripsHit.push_back(xStripsHit);
    stripsHit.push_back(yStripsHit);
    energy.push_back(xEnergy);
    energy.push_back(yEnergy);
    sig.push_back(xSigma);
    sig.push_back(ySigma);
    
    //change n_x and n_y in case some strips had bad values for energy / error
    n_x = stripsHit.at(0).size();
    n_y = stripsHit.at(1).size();
    nHits.push_back(n_x);
    nHits.push_back(n_y);
    nHitsOrig = nHits;
    
    
    
    
    //sort strips (and energy and resolution) in numerical order by strip ID
    //do this for each axis
    for (int axis=0; axis<2; axis++){
      for (int i=0; i<nHits.at(axis)-1; i++){
				int min=i;
        for (int j=i+1; j<nHits.at(axis); j++){
          if (stripsHit.at(axis).at(j) < stripsHit.at(axis).at(min)){
            min = j;
					}
				}
				if (min != i){
          int tempStrip = stripsHit.at(axis).at(i);
          stripsHit.at(axis).at(i) = stripsHit.at(axis).at(min);
          stripsHit.at(axis).at(min) = tempStrip;
            
          float tempEnergy = energy.at(axis).at(i);
          energy.at(axis).at(i) = energy.at(axis).at(min);
          energy.at(axis).at(min) = tempEnergy;
            
          float tempSig = sig.at(axis).at(i);
          sig.at(axis).at(i) = sig.at(axis).at(min);
          sig.at(axis).at(min) = tempSig;
        }
      }
    }
    return true;
  }
  else {
    nHits.push_back(0);
    nHits.push_back(0);
    return false;
  }
  
};      

void MNCTModuleStripPairingGreedy_b::CalculateDetectorQuality(){
  
  float detectorQuality = 0;
  int counter = 0;
  
  for (unsigned int i=0; i<hitQualityFactor.size(); i++){
    detectorQuality = detectorQuality + hitQualityFactor.at(i);
    counter += 1;
  }
  
  detectorQuality = detectorQuality / counter;
  
  detectorQualityFactors.push_back(detectorQuality);
  
};

void MNCTModuleStripPairingGreedy_b::CalculateEventQuality(MReadOutAssembly* Event, int nDetectors){
  
  float eventQuality = 0;
  int counter = 0;
  //	cout << detectorQualityFactors.at(2) << endl;
  
  for (int i=0; i<nDetectors; i++){
    eventQuality = eventQuality + detectorQualityFactors.at(i);
    counter += 1;
  }
  
  Event->SetEventQuality(eventQuality);
  
};

//output stuff
void MNCTModuleStripPairingGreedy_b::WriteHits(MReadOutAssembly* Event, int detector){
  
  
  vector<vector<vector<int> > > decodedFinalPairs = DecodeFinalPairs();
 
  //pair indexes over the pairs (hits) -- for each pair, there is a new MNCTHit
  //strip indexes over the strips in each pair on the x or y side
  //n indexes over the list of strip hits in the MReadOutAssembly
  
  //this loop iterates over the pairs, and for each pair creates a new MNCTHit
  //then, it iterates over each strip on the x side of that pair
  //it then iterates over each stripHit in the MReadOutAssembly, checks that the detector is right,
  //	that it's an x hit, and if it matches the strip, it is added to the MNCTHit
  //the same is done in the second sub-loop for the y side
  
  //hit quality, energy, energy resolution only needs to be added once,
  // (it's the same for x and y), so it is done on the x side
  
  bool addHit = false;
  
  for (unsigned int pair=0; pair<decodedFinalPairs.size(); pair++){
    MNCTHit* Hit = new MNCTHit();
    //x side
    for (unsigned int strip=0; strip<decodedFinalPairs.at(pair).at(0).size(); strip++){
      for (unsigned int n = 0; n<Event->GetNStripHits(); n++){
        if (detector == Event->GetStripHit(n)->GetDetectorID()){
          if (Event->GetStripHit(n)->IsXStrip() == true){
            if (Event->GetStripHit(n)->GetStripID() == decodedFinalPairs.at(pair).at(0).at(strip)){
              Hit->AddStripHit(Event->GetStripHit(n));
              Hit->SetHitQuality(hitQualityFactor.at(pair));
              Hit->SetEnergyResolution(energyResolution.at(pair));
              Hit->SetEnergy(hitEnergy.at(pair));
              addHit = true;
            }
          }
        }
      }
    }
    //y side
    for (unsigned int strip=0; strip<decodedFinalPairs.at(pair).at(1).size(); strip++){
      for (unsigned int n=0; n<Event->GetNStripHits(); n++){
        if (detector == Event->GetStripHit(n)->GetDetectorID()){
          if (Event->GetStripHit(n)->IsXStrip() == false){
            if (Event->GetStripHit(n)->GetStripID() == decodedFinalPairs.at(pair).at(1).at(strip)){
              Hit->AddStripHit(Event->GetStripHit(n));
            }
          }
        }
      }
    }
    if (addHit){
      Event->AddHit(Hit);
    }
    addHit = false;


		  //carolyn's addition for debugging CrossTalkOffset
 // int DetectorID = 0;
//  DetectorID = Event->GetHit(0)->GetStripHit(0)->GetDetectorID(); 
  //cout<<"Got DetectorID: "<<DetectorID<<endl;
//  if (DetectorID == 1) {
//    for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
/*		cout << "NStripHits(): " << Hit->GetNStripHits() << endl;
    if (Hit->GetNStripHits() == 4) {
      int nXhits = 0;
      int nYhits = 0;
      int xhit_strip[3] = {0, 0, 0};
      double xhit_energy[3] = {0.0, 0.0, 0.0};;
      int yhit_strip[3] = {0, 0, 0};
      double yhit_energy[3] = {0.0, 0.0, 0.0};
      for (unsigned int s = 0; s < 4; ++s){
        if (Hit->GetStripHit(s)->IsXStrip() == true){
          xhit_strip[nXhits] = Hit->GetStripHit(s)->GetStripID();
          xhit_energy[nXhits] = Hit->GetStripHit(s)->GetEnergy();
          nXhits = nXhits + 1;
        } else if (Hit->GetStripHit(s)->IsXStrip() == false){
          yhit_strip[nYhits] = Hit->GetStripHit(s)->GetStripID();
          yhit_energy[nYhits] = Hit->GetStripHit(s)->GetEnergy();
           }
      }
    cout<<"Clio found number of x hits: "<<nXhits<<", & number of y hits: "<<nYhits<<endl;
    cout<<"xHit1: "<<xhit_strip[0]<<", "<<xhit_energy[0]<<endl;
    cout<<"xHit2: "<<xhit_strip[1]<<", "<<xhit_energy[1]<<endl;
    cout<<"xHit3: "<<xhit_strip[2]<<", "<<xhit_energy[2]<<endl;
    cout<<"yHit1: "<<yhit_strip[0]<<", "<<yhit_energy[0]<<endl;
    cout<<"yHit2: "<<yhit_strip[1]<<", "<<yhit_energy[1]<<endl;
    cout<<"yHit3: "<<yhit_strip[2]<<", "<<yhit_energy[2]<<endl;
      }
//  }
 // }
*/

  }
  
};

//clears members in between each detector
void MNCTModuleStripPairingGreedy_b::ClearMembers(){
  
  stripsHit.clear();
  energy.clear();
  sig.clear();
  nHits.clear();
  nHitsOrig.clear();
  badCombinations.clear();
  killMatrix.clear();
  weightMatrix.clear();
  finalPairs.clear();
  
  hitQualityFactor.clear();
  energyResolution.clear();
  hitEnergy.clear();
  //	detectorQualityFactors.clear();
  
};

//defines information about event, will be replaced once code is integrated into Nuclearizer
void MNCTModuleStripPairingGreedy_b::DefineEventInfo() {
  
  //	const int n_detectors = 12;
  vector<int> xStripsHit, yStripsHit;
  vector<float> xEnergy, yEnergy;
  vector<float> xSig, ySig;
  
  //	double seed = time(NULL);
  //	srand(seed);
  //	cout << "SEED: " << seed << endl;
  srand(12);
  
  //set n_x and n_y to be between 3 and 5
  int n_x = rand() % 3 + 3;
  int n_y = rand() % 3 + 3;
  int n_xOrig = n_x;
  int n_yOrig = n_y;
  
  int strip, energyInt;
  float sigInt;
  
  //fill x and y strip, energy, and significance using random numbers
  for(int i=0; i<n_x; i++){
    strip = rand() % 37 + 1;
    energyInt = rand() % 1000 + 1;
    sigInt = static_cast <float> (rand() % 9 + 1)/10;
    
    xStripsHit.push_back(strip);
    xEnergy.push_back(energyInt);
    //	if (sig == 1){sig = sig - 0.1;}
    xSig.push_back(sigInt);
    
    //	cout << xStripsHit.at(i) << endl;
  }
  
  //make sure no strips are repeated, and if they are, change them
  bool checkAgain = false;
  do{ for(int i=0; i<n_x; i++){
    for(int j=0; j<n_x; j++){
      if(xStripsHit.at(i)==xStripsHit.at(j) && i!=j){
        xStripsHit.at(i) = rand() % 37 + 1;
        checkAgain = true;}
        else{checkAgain = false;}
    }
  } }
  while(checkAgain == true);
  
  //	cout << endl;
  
  for(int i=0; i<n_y; i++){	
    strip = rand() % 37 + 1;
    energyInt = rand() % 1000 + 1;
    sigInt = static_cast <float> (rand() % 9 + 1)/10;
    yStripsHit.push_back(strip);
    yEnergy.push_back(energyInt);
    //	if (sig == 1){sig = sig - 0.1;}
    ySig.push_back(sigInt);
    
    //	cout << yStripsHit.at(i) << endl;
  }
  
  checkAgain = false;
  do{ for(int i=0; i<n_y; i++){
    for(int j=0; j<n_y; j++){
      if(yStripsHit.at(i)==yStripsHit.at(j) && i!=j){
        yStripsHit.at(i) = rand() % 37 + 1;
        checkAgain = true;}
        else{checkAgain = false;}
    }
  } }
  while(checkAgain == true);
  
  
  //	cout << "------------------" << endl;
  //sort strip numbers. they will get different energies and significances than originally but that doesn't matter
  sort(xStripsHit.begin(), xStripsHit.end());
  sort(yStripsHit.begin(), yStripsHit.end());
  if (g_Verbosity >= 3) {
    for (int i = 0; i < n_x; ++i) { cout << xStripsHit.at(i) << endl; } cout << endl;
    for (int i = 0; i < n_x; ++i) { cout << xEnergy.at(i) << endl; } cout << endl;
    for (int i = 0; i < n_x; ++i) { cout << xSig.at(i) << endl; } cout<<endl;
  }
  
  stripsHit.push_back(xStripsHit);
  stripsHit.push_back(yStripsHit);
  
  energy.push_back(xEnergy);
  energy.push_back(yEnergy);
  
  sig.push_back(xSig);
  sig.push_back(ySig);
  
  nHits.push_back(n_x);
  nHits.push_back(n_y);
  nHitsOrig.push_back(n_xOrig);
  nHitsOrig.push_back(n_yOrig);
  
  //	for (int i=0; i<n_y; i++){cout << yStripsHit.at(i) << endl;}
  
  
}; 



//calculate average sigma (delta E)
//avg sigma = sigma(x) + sigma(y)
//sigma(x) = sqrt(sigma(hit 1)^2 + sigma(hit 2)^2 + ... )
float MNCTModuleStripPairingGreedy_b::CalculateSigma(){
  
  vector<float> sigTotalSquared, sigTotal;
  
  sigTotalSquared.push_back(0);
  sigTotalSquared.push_back(0);
  
  //find sigma(x)^2 and sigma(y)^2
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHitsOrig.at(axis); i++){
      sigTotalSquared.at(axis) = sigTotalSquared.at(axis) + sig.at(axis).at(i)*sig.at(axis).at(i);
    }
  }
  
  //find sigma(x) and sigma(y), store in vector sigTotal
  sigTotal.push_back(sqrt(sigTotalSquared.at(0)));
  sigTotal.push_back(sqrt(sigTotalSquared.at(1)));
  
  //find average sigma
  float avgSigma = 0.5*(sigTotal.at(0)+sigTotal.at(1));
  
  //	cout << "avgSigma: " << avgSigma << endl;
  
  return avgSigma;
  
};

//checks size of energy difference between x and y by comparing to the average sigma calculated in CalculateSigma()
//returns true if difference is smaller than 3*(avg sigma), false if difference is larger than 3*(avg sigma)
bool MNCTModuleStripPairingGreedy_b::CheckInitialEnergyDifference(){
  
  int avgSigma = CalculateSigma();
  
  vector<float> eTotal;
  eTotal.push_back(0);
  eTotal.push_back(0);
  
  //find total x energy and total  y energy
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHitsOrig.at(axis); i++){
      eTotal.at(axis) = eTotal.at(axis) + energy.at(axis).at(i);
    }
  }
  
  //compare energy difference to 3*(avg sigma)	
  if (fabs(eTotal.at(0)-eTotal.at(1)) < 3*avgSigma){
    //		cout << "E difference less than 3 sigma" << endl;
    return true;
  }
  else {
    //		cout << "E difference greater than 3 sigma" << endl;
    return false;
  }
  
};

//if energy difference is larger than 3sigma, tries to determine what could have caused the difference
//options are dead strip, charge loss, noise hit, and a photon with energy very near the threshold, so it was only noticed on one side
void MNCTModuleStripPairingGreedy_b::DetermineOption(bool adjStripsHit){
  
  //define map of all possible options
  map<string, bool> options;
  
  options["dead strip"] = false;
  options["charge loss"] = false;
  options["threshold"] = false;
  options["noise hit"] = false;
  
  int deltaE = CalculateSigma();
  float Eth = GetEth();
  
  if (deltaE < 10 && adjStripsHit == true){
    options["charge loss"] = true;
  }
  
  if (deltaE < Eth){
    options["threshold"] = true;
  }
  
  if (deltaE > Eth){
    //how to tell the difference between > and >>??
    options["noise hit"] = true;
    options["dead strip"] = true;
  }
  
  
};

//checks if any hits are on adjacent strips, and return true if so
//appends any hits on adjacent strips to stripsHit vector
//if strip n and n+1 are hit, the "strip number" for the combined strips is 50+n
//for example, if strips 3 and 4 are hit, 53 is appended to the end of stripsHit
bool MNCTModuleStripPairingGreedy_b::CheckForAdjacentStrips(){
 
  //define and initialize variables
  int adjStripLabel;
  float adjStripEnergy, adjStripSig;
  int counter = 0;
  bool adjStrips = false;
  
  //iterate over x and y axes
  for (int axis=0; axis<2; axis++){
    //iterate over all but last hit on each axis
    for (int i=0; i<nHits.at(axis)-1; i++){
      //check for adjacent strips
      if (stripsHit.at(axis).at(i)+1 == stripsHit.at(axis).at(i+1)){
        adjStrips = true;
        
        counter += 1;
        //label combined strip by adding 50 to the lower strip number
        //append combined strip number to stripsHit
        adjStripLabel = stripsHit.at(axis).at(i) + 50;
        stripsHit.at(axis).push_back(adjStripLabel);
        
        //add row and column to kill matrix
        ExpandKillMatrix(axis);
        
        //fill kill matrix
        //each element of the kill matrix is the vector sum of the constituent strips
        for (int k=0; k<nHits.at(axis)+counter-1; k++){
          killMatrix.at(axis).at(nHits.at(axis)+counter-1).at(k) = killMatrix.at(axis).at(i).at(k) + killMatrix.at(axis).at(i+1).at(k);
          killMatrix.at(axis).at(k).at(nHits.at(axis)+counter-1) = killMatrix.at(axis).at(k).at(i) + killMatrix.at(axis).at(k).at(i+1);
        }
        
        //add the energies of the adjacent strips and append them to the energy vector
        adjStripEnergy = energy.at(axis).at(i) + energy.at(axis).at(i+1);
        energy.at(axis).push_back(adjStripEnergy);
        
        //add the errors of the adjacent strips and append them to the error vector
        adjStripSig = sqrt(sig.at(axis).at(i)*sig.at(axis).at(i)+sig.at(axis).at(i+1)*sig.at(axis).at(i+1));
        sig.at(axis).push_back(adjStripSig);
      }
    }
    //change the number of hits
    nHits.at(axis) = nHits.at(axis) + counter;
    
    counter = 0;
  }
  
  return adjStrips;
  
};

//check if multiple hits need to be considered
//multiple hits need to be considered when there are more hits on one side than the other
bool MNCTModuleStripPairingGreedy_b::CheckMultipleHits(){
  
  //	DefineEventInfo();
  
  int n_x = nHits.at(0);
  int n_y = nHits.at(1);
  
  //	cout << "nx: " << nHits.at(0) << endl;
  //	cout << "ny: " << nHits.at(1) << endl << endl;
  
  if (n_x == n_y) {return false;}
  
  else if (n_x > n_y) {  //y strips have multiple hits, so want to make pairs from x: ie two x strips for one y strip
    AddMultipleHits(0);
    //		for (int i=0; i<n_x; i++){cout << stripsHit.at(0).at(i) << endl;}
    return true;
  }
  
  else {	//x strips have multiple hits, so want to make pairs from y: ie two y strips for one x strip
    AddMultipleHits(1);
    //		for (int i=0; i<n_y; i++){cout << stripsHit.at(1).at(i) << endl;}
    return true;
  }
};


//add possibilities of multiple hits to vector of hits
void MNCTModuleStripPairingGreedy_b::AddMultipleHits(int axis){
  
  //initialize variables
  int x1, x2, pair;
  float pairE, pairSig;
  int nPairs = 0;
  vector<int> pairHitsVec;
  vector<float> pairEnergyVec, pairSigVec;
  
  //goal is to make all possible combinations of strips on one axis
  for (int i=0; i<nHits.at(axis); i++){
    for (int j=i+1; j<nHits.at(axis); j++){
      x1 = stripsHit.at(axis).at(i);
      x2 = stripsHit.at(axis).at(j);
      
      //label combinations by multiplying the lower strip number by 100 and adding it to the higher strip number
      //append combined strip to pairHitsVec
      pair = x1*100+x2;
      pairHitsVec.push_back(pair);
      
      //count number of pairs
      nPairs+=1;
      //			cout << "nPairs: " << nPairs << endl;
      
      //add row and column to kill matrix
      ExpandKillMatrix(axis);
      //fill kill matrix
      //each element is the vector sum of its constituent strips
      for (int k=0; k<nHits.at(axis)+nPairs-1; k++){
        killMatrix.at(axis).at(nHits.at(axis)+nPairs-1).at(k) = killMatrix.at(axis).at(i).at(k) + killMatrix.at(axis).at(j).at(k);
        killMatrix.at(axis).at(k).at(nHits.at(axis)+nPairs-1) = killMatrix.at(axis).at(k).at(i) + killMatrix.at(axis).at(k).at(j);
      }
      
      //print kill matrix
      /*
      for (int m=0; m<nHits.at(axis)+nPairs-1; m++){
        for (int n=0; n<nHits.at(axis)+nPairs-1; n++){
          cout << killMatrix.at(axis).at(m).at(n) << '\t';
        }
        cout << endl;
      }
      */
      //add to energy and significance vectors
      pairE = energy.at(axis).at(i)+energy.at(axis).at(j);
      pairEnergyVec.push_back(pairE);
      
      pairSig = sqrt(sig.at(axis).at(i)*sig.at(axis).at(i)+sig.at(axis).at(j)*sig.at(axis).at(j));
      pairSigVec.push_back(pairSig);
      
    }
  }
  /*
  cout << "-----checking order-----" << endl;
  for (int i=0; i<nPairs; i++){
    cout << pairHitsVec.at(i) << endl;
  }
  */
  
  //add strip combinations info to stripsHit, energy, and sig
  for (int i=0; i<nPairs; i++){
    stripsHit.at(axis).push_back(pairHitsVec.at(i));
    energy.at(axis).push_back(pairEnergyVec.at(i));
    sig.at(axis).push_back(pairSigVec.at(i));
  }
  
  //change number of hits to account for multiple strips being hit
  nHits.at(axis) = nHits.at(axis) + nPairs;
  
  //print final kill matrix
  //	cout << "printing final kill matrix..." << endl;
  //	for (int i=0; i<nHits.at(axis); i++){
  //		for (int j=0; j<nHits.at(axis); j++){
  //			cout << killMatrix.at(axis).at(i).at(j) << '\t';
  //		}
  //		cout << endl;
  //	}
  
  
  
  //	cout << "--------------------//------------------------" << endl;
  //	for(int i=0; i<xStripsHit.size(); i++){cout << xStripsHit.at(i) << endl;}
  
  
};

//initialize badCombinations vector and fill with 0s
void MNCTModuleStripPairingGreedy_b::InitializeBadCombinations(){
  
  badCombinations.clear();
  
  vector<int> badCombX, badCombY;
  for (int i=0; i<nHits.at(0); i++){
    badCombX.push_back(0);
  }
  for (int i=0; i<nHits.at(1); i++){
    badCombY.push_back(0);
  }
  badCombinations.push_back(badCombX);
  badCombinations.push_back(badCombY);
  
};

//check for combinations that take a strip into account more than once
//for example, 251 is the combination of strip 2 and charge sharing between strips 1 and 2
void MNCTModuleStripPairingGreedy_b::CheckForBadCombinations(){
  
  InitializeBadCombinations();
  
  //from the way the kill matrix is defined, if killMatrix[i][j]>1, than strip i is a bad combination
  for (int axis=0; axis<2; axis++){
//		cout << axis << ": ";
    for (int i=0; i<nHits.at(axis); i++){
      for (int j=0; j<nHitsOrig.at(axis); j++){
        //add bad combinations to badCombinations vector
        if (killMatrix.at(axis).at(i).at(j) > 1){
          badCombinations.at(axis).at(i) = 1;
        }
      }
  //    			cout << badCombinations.at(axis).at(i) << "  ";

    }
   // 		cout << endl;
  }
};

//calculates weight for one x-y pair
float MNCTModuleStripPairingGreedy_b::CalculateWeight(int xIndex, int yIndex){
  
  float weight;
  float xE = energy.at(0).at(xIndex);
  float yE = energy.at(1).at(yIndex);
  float xS = sig.at(0).at(xIndex);
  float yS = sig.at(1).at(yIndex);
  
  weight = (xE-yE)*(xE-yE)/((xS*xS)+(yS*yS));
  // cout << "weight: x=" << xIndex << ", y=" << yIndex << ": "<< weight << endl;
  return weight;
  
};

//makes a matrix of the correct size and fills it with 0s
void MNCTModuleStripPairingGreedy_b::InitializeWeightMatrix(){
  
  weightMatrix.clear();
  
  vector<float> col;
  
  
  for (int i=0; i<nHits.at(1); i++){
    col.push_back(0);
  }
  for (int j=0; j<nHits.at(0); j++){
    weightMatrix.push_back(col);
  }
  
};

//initializes kill matrices
//makes matrices the right size, fills them with 0s, stores them in killMatrix vector
void MNCTModuleStripPairingGreedy_b::InitializeKillMatrices(){
  
  killMatrix.clear();
  
  vector<int> col_x, col_y;
  vector<vector<int> > kill_x, kill_y;
  
  for (int i=0; i<nHits.at(0); i++){
    col_x.push_back(0);
  }
  for (int i=0; i<nHits.at(1); i++){
    col_y.push_back(0);
  }
  for (int i=0; i<nHits.at(0); i++){
    kill_x.push_back(col_x);
  }
  for (int i=0; i<nHits.at(1); i++){
    kill_y.push_back(col_y);
  }
  
  killMatrix.push_back(kill_x);
  killMatrix.push_back(kill_y);
  
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHits.at(axis); i++){
      killMatrix.at(axis).at(i).at(i) = 1;
    }
  }
  
};

//add row and column of 0s to kill matrix
void MNCTModuleStripPairingGreedy_b::ExpandKillMatrix(int axis){
  
  int size = killMatrix.at(axis).size();
  vector<int> col;
  //make a column that's 1 bigger than the size of the kill matrix
  for (int i=0; i<size+1; i++){
    col.push_back(0);
  }
  //add the column to the kill matrix
  killMatrix.at(axis).push_back(col);
  
  //add a 0 to every row?
  for (int j=0; j<size; j++){
    killMatrix.at(axis).at(j).push_back(0);
  }
  
  //set new diagonal term equal to 1
  killMatrix.at(axis).at(size).at(size) = 1;
  
  // cout << kill_x.size() << endl;
  // for (int i=0; i<kill_x.size(); i++){
  //   cout << "size: " << kill_x.at(i).size() << endl;
  // }
  
  //	for (int i=0; i<kill_x.size(); i++){
  //    for (int j=0; j<kill_x.size(); j++){
  //      cout << kill_x.at(i).at(j) << '\t';
  //    }
  //    cout << endl;
  //  }
  //
};

//calculates weight matrix
//fills weight matrtix of 0s with the weights of each x-y pair
void MNCTModuleStripPairingGreedy_b::CalculateWeightMatrix(){
  
  int n_x = nHits.at(0);
  int n_y = nHits.at(1);
  int n_xOrig = nHitsOrig.at(0);
  int n_yOrig = nHitsOrig.at(1);
  
  InitializeWeightMatrix();
  float weight;
  
  //for each x-y pair, fill in weight matrix with the weight of the pair
  //if it's a bad pair, set weight to -1
  for (int i=0; i<n_x; i++){
    for (int j=0; j<n_y; j++){
      if (badCombinations.at(0).at(i) == 0 && badCombinations.at(1).at(j) == 0){
        if (i<n_xOrig || j<n_yOrig){
          weight = CalculateWeight(i,j);
          weightMatrix.at(i).at(j) = weight;
        }
        else {weightMatrix.at(i).at(j) = -1;}
      }
      else {weightMatrix.at(i).at(j) = -1;}
    }
  }
  
  //	cout << "printing weight matrix: first time" << endl;
  //	PrintWeightMatrix();
  
};

//go through all elements of weight matrix and find minimum weight
vector<int> MNCTModuleStripPairingGreedy_b::FindMinWeight(){
  
  int n_x = nHits.at(0);
  int n_y = nHits.at(1);
  
  //set min_weight equal to the maximum float value that can possibly exist
  float min_weight = numeric_limits<float>::max();
  float weight;
  int xIndex, yIndex;
  //for each weight in the weight matrix, if it's lower than min_weight,
  //set min_weight to that weight
  for (int i=0; i<n_x; i++){
    for (int j=0; j<n_y; j++){
      weight = weightMatrix.at(i).at(j);
      if (weight < min_weight && weight >= 0){
        min_weight = weight;
        xIndex = i;
        yIndex = j;
        //	cout << "minWeight: " << min_weight << endl;
        //	cout << "xIndex: " << xIndex << endl;
        //	cout << "yIndex: " << yIndex << endl;
      }
    }
  }
  
  //set the final minimum weight to -1, so that this search can be re-done
  weightMatrix.at(xIndex).at(yIndex) = -1;
  
  //	cout << "xIndex: " << xIndex << endl;
  //	cout << "yIndex: " << yIndex << endl;
  
  //return the indices of the minimum weight
  vector<int> pairIndex;
  pairIndex.push_back(xIndex);
  pairIndex.push_back(yIndex);
  return pairIndex;
  
};

//once a pair is chosen (indexed by xIndex, yIndex), prevents program from repeating a strip
void MNCTModuleStripPairingGreedy_b::ConflictingStrips(int xIndex, int yIndex){
  
  vector<int> indices;
  indices.push_back(xIndex);
  indices.push_back(yIndex);
  
  //loop sets weight matrix to -1 at any place where a strip could be repeated
  //for example, if pair (x=1, y=2) is chosen, the weight matrix at (x=1, y=anything)
  //or (x=51, y=anything), or (x=105, y=anything), etc, must be set to -1
  //that way, strips don't repeat
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHits.at(axis); i++){
      //			cout << i << '\t' << killMatrix.at(axis).at(indices.at(axis)).at(i) << endl;
      if (killMatrix.at(axis).at(indices.at(axis)).at(i) != 0){
        //				cout << "bad x: " << i << '\t' << stripsHit.at(axis).at(i) << endl;
        //				cout << killMatrix.at(axis).at(indices.at(axis)).at(i) << endl;
        
        int j_max;
        if (axis==1){j_max = 0;}
        else {j_max = 1;}
        for (int j=0; j<nHits.at(j_max); j++){
          if (axis==0){weightMatrix.at(i).at(j) = -1;}
          if (axis==1){weightMatrix.at(j).at(i) = -1;}
        }
      }
    }
  }
  
  //	cout << "printing weight matrix: second time" << endl;
  //	PrintWeightMatrix();
  
};

//count the number of elements in the weight matrix that are set to -1
int MNCTModuleStripPairingGreedy_b::CountNegativeElements(){
  
  int counter = 0;
  for (int i=0; i<nHits.at(0); i++){
    for (int j=0; j<nHits.at(1); j++){
      if (weightMatrix.at(i).at(j) == -1){
        counter += 1;
      }
    }
  }
  
  return counter;
  
};

//find the list of final pairs by finding the minimum weights
void MNCTModuleStripPairingGreedy_b::FindFinalPairs(){
  
  //initialize variables
  int n_x = nHits.at(0);
  int n_y = nHits.at(1);
  
  CalculateWeightMatrix();
  if (g_Verbosity >= c_Info) PrintXYStripsHit();
  if (g_Verbosity >= c_Info) PrintWeightMatrix();
  int nElements = n_x*n_y;
  int nNegElem = 0;
  int xIndex, yIndex;
  int finalXStrip, finalYStrip;
  vector<int> pairVec;
  float weight, hitE, eRes;
  
  //	cout << "nElements: " << nElements << endl;
  //find minimum weight while the number of elements that equal -1 in the weight matrix
  //is less than the number of elements in the weight matrix
  do{
    //find indices of the minimum weight
    vector<int> indices = FindMinWeight();
    xIndex = indices.at(0);
    yIndex = indices.at(1);
    //add the strip numbers at those indices to finalPairs
    finalXStrip = stripsHit.at(0).at(xIndex);
    finalYStrip = stripsHit.at(1).at(yIndex);
    pairVec.push_back(finalXStrip);
    pairVec.push_back(finalYStrip);
    finalPairs.push_back(pairVec);
    pairVec.clear();
    
    //fill hit quality factor vector
    weight = weightMatrix.at(xIndex).at(yIndex);
    hitQualityFactor.push_back(weight);
    
    //fill hit energy resolution vector
    float eResX = sig.at(0).at(xIndex);
    float eResY = sig.at(1).at(yIndex);
    eRes = (sqrt(2)*eResX*eResY)/(eResX*eResX+eResY*eResY);
    energyResolution.push_back(eRes);
    
    //fill hit energy vector
    float eX = energy.at(0).at(xIndex);
    float eY = energy.at(1).at(yIndex);
    double inf = numeric_limits<double>::infinity();
    //sometimes the energy resolution is infinite (not sure why)
    //if it is, then take the average of the energies
    //otherwise use correct formula
    if (eResX != inf && eResY != inf){
      hitE = (eX/(eResX*eResX)+eY/(eResY*eResY))/(1/(eResX*eResX)+1/(eResY*eResY));
    }
    else {
      hitE = (eX+eY)/2;
    }
    // At this stage we can only use ONE side, the above formula we can only use AFTER charge charging/cross talk correction!
    hitEnergy.push_back(eY); //hitEnergy.push_back(hitE);
    
    //		PrintWeightMatrix();
    //call ConflictingStrips to avoid repeating strips
    ConflictingStrips(xIndex, yIndex);
    nNegElem = CountNegativeElements();
    //		cout << "nNegElem: " << nNegElem << endl;
  }
  while (nNegElem != nElements);
  
  //print final pairs
  if (g_Verbosity >= c_Info) {
    cout << "final pairs: " << endl;
    for (unsigned int i=0; i<finalPairs.size(); i++){
      cout << finalPairs.at(i).at(0) << '\t' << finalPairs.at(i).at(1) << endl;
    }
  }
};

//puts list of final pairs back into strip numbers
//for example, if strip 51 is selected, it gets separated into strips 1 and 2
//if 104 is selected, it gets separated into strips 1 and 4
//if 256 is selected, it gets separated into strip 2 and charge sharing between 6 and 7
vector<vector<vector<int> > > MNCTModuleStripPairingGreedy_b::DecodeFinalPairs(){
 
  vector<int> xVec;
  vector<int> yVec;
  vector<vector<int> > pair;
  vector<vector<vector<int> > > decodedFinalPairs;

	//if there are multiple hits per strip, they should be separate hits
	bool xTwoHits, yTwoHits;
	bool xChargeSharing, yChargeSharing;
	//need to add energy to hitEnergy vector for the additional hit
	int twoHitsCounter = 0;
 
  int nPairs = finalPairs.size();
  //	cout << "nPairs: " << nPairs << endl;
  for (int i=0; i<nPairs; i++){
		xTwoHits = false;
		yTwoHits = false;
		xChargeSharing = false;
		yChargeSharing = false;

    if (finalPairs.at(i).at(0) > 50 && finalPairs.at(i).at(0) < 100){
      xVec.push_back(finalPairs.at(i).at(0)-50);
      xVec.push_back(finalPairs.at(i).at(0)+1-50);
    }
    else if (finalPairs.at(i).at(0) > 100){
			xTwoHits = true;
      if (finalPairs.at(i).at(0)-(int)(finalPairs.at(i).at(0)/100)*100 > 50){
				xChargeSharing = true;
        xVec.push_back(finalPairs.at(i).at(0)/100);
        xVec.push_back(finalPairs.at(i).at(0)-(int)(finalPairs.at(i).at(0)/100)*100-50);
        xVec.push_back(finalPairs.at(i).at(0)-(int)(finalPairs.at(i).at(0)/100)*100+1-50);
      }
      else {
        xVec.push_back(finalPairs.at(i).at(0)/100);
        xVec.push_back(finalPairs.at(i).at(0) - (int)(finalPairs.at(i).at(0)/100)*100);
      }
    }
    else if (finalPairs.at(i).at(0) < 38) {xVec.push_back(finalPairs.at(i).at(0));}
    
    if (finalPairs.at(i).at(1) > 50 && finalPairs.at(i).at(1) < 100){
      yVec.push_back(finalPairs.at(i).at(1)-50);
      yVec.push_back(finalPairs.at(i).at(1)+1-50);
    }
    else if (finalPairs.at(i).at(1) > 100){
			yTwoHits = true;
      if (finalPairs.at(i).at(1)-(int)(finalPairs.at(i).at(1)/100)*100 > 50){
				yChargeSharing = true;
        yVec.push_back(finalPairs.at(i).at(1)/100);
        yVec.push_back(finalPairs.at(i).at(1)-(int)(finalPairs.at(i).at(1)/100)*100-50);
        yVec.push_back(finalPairs.at(i).at(1)-(int)(finalPairs.at(i).at(1)/100)*100+1-50);
      }
      else {
        yVec.push_back(finalPairs.at(i).at(1)/100);
        yVec.push_back(finalPairs.at(i).at(1) - (int)(finalPairs.at(i).at(1)/100)*100);
      }
    }
    else if (finalPairs.at(i).at(1) < 38) {yVec.push_back(finalPairs.at(i).at(1));}

		if (xTwoHits){
			twoHitsCounter += 1;

			vector<int> xVecNew;
			//first hit
			xVecNew.push_back(xVec.at(0));
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			pair.clear();
			xVecNew.clear();
			//change energy and resolution for first hit
			//hitEnergy.at(i+twoHitsCounter-1) = 

			//second hit
			if (xChargeSharing){
				xVecNew.push_back(xVec.at(1));
				xVecNew.push_back(xVec.at(2));
				pair.push_back(xVecNew);
				pair.push_back(yVec);
				decodedFinalPairs.push_back(pair);
			}
			else {
				xVecNew.push_back(xVec.at(1));
				pair.push_back(xVecNew);
				pair.push_back(yVec);
  	  	decodedFinalPairs.push_back(pair);
			}
			xVecNew.clear();
			//change energy and resolution for second hit
			//also add hit quality, but keep it the same for now
			vector<float>::iterator itOne = hitEnergy.begin();
			vector<float>::iterator itTwo = energyResolution.begin();
			vector<float>::iterator itThree = hitQualityFactor.begin();

			hitEnergy.insert(itOne+i+twoHitsCounter, hitEnergy.at(i+twoHitsCounter-1));
			energyResolution.insert(itTwo+i+twoHitsCounter, energyResolution.at(i+twoHitsCounter-1)); 
			hitQualityFactor.insert(itThree+i+twoHitsCounter, hitQualityFactor.at(i+twoHitsCounter-1));
		}
		else if (yTwoHits){
			twoHitsCounter += 1;

			vector<int> yVecNew;
			//first hit
			pair.push_back(xVec);
			yVecNew.push_back(yVec.at(0));
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			pair.clear();
			yVecNew.clear();
			//second hit
			if (yChargeSharing){
				yVecNew.push_back(yVec.at(1));
				yVecNew.push_back(yVec.at(2));
				pair.push_back(xVec);
				pair.push_back(yVecNew);
			}
			else {
				pair.push_back(xVec);
				yVecNew.push_back(yVec.at(1));
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
			}
			yVecNew.clear();
			//change energy and resolution for second hit
			//also add element to hit quality, need to do properly later
			vector<float>::iterator itOne = hitEnergy.begin();
			vector<float>::iterator itTwo = energyResolution.begin();
			vector<float>::iterator itThree = hitQualityFactor.begin();

			hitEnergy.insert(itOne+i+twoHitsCounter, hitEnergy.at(i+twoHitsCounter-1));
			energyResolution.insert(itTwo+i+twoHitsCounter, energyResolution.at(i+twoHitsCounter-1));
			hitQualityFactor.insert(itThree+i+twoHitsCounter, hitQualityFactor.at(i+twoHitsCounter-1));
		}
		else if (!xTwoHits && !yTwoHits){
			pair.push_back(xVec);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
		}
    
   /* cout << "pair: "  << endl;
    for (int j=0; j<pair.size(); j++){
      for (int k=0; k<pair.at(j).size(); k++){
        cout << pair.at(j).at(k) << '\t' ;
        cout << pair.at(j).at(k) << endl;
      }
    }
   */ 
    xVec.clear();
    yVec.clear();
    pair.clear();
    //		pair.at(0).clear();
    //	pair.at(1).clear();
    
    
  }

  
  //print decodedFinalPairs
  /*	
  cout << decodedFinalPairs.size() << endl;
  cout << "decoded final pairs" << endl;
  for (int i=0; i<decodedFinalPairs.size(); i++){
    for (int j=0; j<2; j++){
      for (int k=0; k<decodedFinalPairs.at(i).at(j).size(); k++){
        cout << decodedFinalPairs.at(i).at(j).at(k) << '\t';
      }
      cout << '\t' << '\t';
    }
    cout << endl << endl;
  }
  */
  return decodedFinalPairs;
  
};

//checks whether all strips were paired, returns true if so
bool MNCTModuleStripPairingGreedy_b::CheckAllStripsWerePaired(){
  
  vector<vector<vector<int> > > decodedFinalPairs = DecodeFinalPairs();
  int counter = 0;
  
  //count how many strips in decodedFinalPairs are equal to original strips
  for (int axis=0; axis<2; axis++){
    //n indexes original hits
    for (int n=0; n<nHitsOrig.at(axis); n++){
      //i, j, k index decodedFinalPairs
      for (unsigned int i=0; i<decodedFinalPairs.size(); i++){
        for (unsigned int k=0; k<decodedFinalPairs.at(i).at(axis).size(); k++){
          if (stripsHit.at(axis).at(n) == decodedFinalPairs.at(i).at(axis).at(k)){
            counter += 1;
          }
        }
      }
    }
  }
  
  //compare counter to total number of strips hit
  if (counter > nHitsOrig.at(0)+nHitsOrig.at(1)){
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": There's something wrong with the code!" << endl;
    return false;
  }
  else if (counter == nHitsOrig.at(0)+nHitsOrig.at(1)){
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": All strips were paired successfully!" << endl;
    return true;
  }
  else {
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Alert! Not all strips were paired!" << endl;
    return false;
  }
  
};

void MNCTModuleStripPairingGreedy_b::FindFinalPairsCorrected(){

	vector<vector<vector<vector<int> > > > combinations;
	vector<vector<vector<int> > > tempVec;

	vector<int> possible_numHits = GetNumbers();

	CheckForBadCombinations();
	RemoveBadStrips();
	//PrintXYStripsHit();

	//cout << "numbers: " << endl;
	//for (int i=0; i<possible_numHits.size(); i++){
	//	cout << possible_numHits.at(i) << '\t';
	//}
	//cout << endl << "---------------" << endl << endl;

	bool dontContinue = false;

	for (int i=0; i<possible_numHits.size(); i++){
		int nFinalHits = possible_numHits.at(i);
		dontContinue = false;
		if (nFinalHits >= 4){
			//cout << "STOPPING?!" << endl;
			dontContinue = true;
			continue;
		}


//	int nFinalHits = possible_numHits.at(0);

		//initialize combo vector
		vector<int> combo;
		for (int i=0; i<nFinalHits; i++){
			combo.push_back(0);
		}
		//cout << "1450" << endl;

		//clear members xCombos and yCombos
		xCombos.clear();
		yCombos.clear();


		//fill xCombos recursively
		GetAllCombinations(0,combo,0, stripsHit.at(0).size()-1, 0, nFinalHits);
		//fill yCombos recursively
		GetAllCombinations(1,combo,0, stripsHit.at(1).size()-1, 0, nFinalHits);
		//cout << "1461" << endl;
		//permute all y combinations
		PermuteCombinations();
		//cout << "1466" << endl;

		//check to make sure there are enough combinations for x and y
		if (xCombos.size()==0 || yCombos.size()==0){
			//cout << "1470" << endl;
			continue;
		}
		else {
			if (xCombos.at(0).size()<nFinalHits || yCombos.at(0).size()<nFinalHits){
				//cout << "HERE" << endl;
				continue;
			}
		}

		//cout << "1474" << endl;
		//get rid of bad combinations
		if (nFinalHits > 1){
			RemoveBadCombinations(0,nFinalHits);
			RemoveBadCombinations(1,nFinalHits);
		}
		//cout << "1480" << endl;

		//add xCombos and yCombos for this specific value of nFinalHits to combinations
		tempVec.push_back(xCombos);
		tempVec.push_back(yCombos);
		combinations.push_back(tempVec);
		tempVec.clear();

/*	cout << "printing combinations.." << endl;	
	for (int i=0; i<xCombos.size(); i++){
			for (int j=0; j<xCombos.at(i).size(); j++){
				cout << xCombos.at(i).at(j) << '\t';
			}
			cout << endl;
		}	
*/
	}

	//find chi^2 of each
	//find min chi^2
	if (!dontContinue){
		//cout << "1491" << endl;
		FindMinChiSquared(combinations);

		//cout << "final pairs: " << endl;
	  //for (unsigned int i=0; i<finalPairs.size(); i++){
	    //cout << finalPairs.at(i).at(0) << '\t' << finalPairs.at(i).at(1) << endl;
	  //}
	}

	//cout << endl << endl << endl << endl;


};

void MNCTModuleStripPairingGreedy_b::FindMinChiSquared(vector<vector<vector<vector<int> > > > combinations){

	int nHitsIndex = -1;
	int xComboIndex = -1;
	int yComboIndex = -1;
	float chiSq;
	float minChiSq = numeric_limits<float>::max();

	//cout << "1519" << endl;
	//cout << combinations.at(0).at(0).size() << endl;
	//cout << "1521" << endl;

	bool something;
	//PUT CONTINUE STATEMENTS SO THAT IF COMBOS VECTOR IS EMPTY, IT DOESN'T CRASH!!
	for (int n=0; n<combinations.size(); n++){
		something = true;
//		if (combinations.at(n).at(0).size()==0 || combinations.at(n).at(1).size()==1){
//			something = false;
//			continue;
//		}
		//cout << "n: " << n << endl;
		//cout << combinations.at(n).at(0).size() << '\t' << combinations.at(n).at(1).size() << endl;
		for (int xC=0; xC<combinations.at(n).at(0).size(); xC++){
			//cout << "xC: " << xC << endl;
			for (int yC=0; yC<combinations.at(n).at(1).size(); yC++){
//				cout << ".";
				chiSq = CalculateChiSquared(combinations.at(n).at(0).at(xC), combinations.at(n).at(1).at(yC));
				//cout << "1499" << endl;
				if (chiSq < minChiSq){
					//cout << "1501" << endl;
					bool canSetMin = EnoughStripsPaired(combinations.at(n).at(0).at(xC), combinations.at(n).at(1).at(yC));
					//cout << "1503" << endl;
					if (canSetMin){
						minChiSq = chiSq;
						nHitsIndex = n;
						xComboIndex = xC;
						yComboIndex = yC;
					}
				}
			}
		}
	}

	//cout << "something: " << something << endl;

	//cout << "1537" << endl;
	//cout << "nHitsIndex: " << nHitsIndex << endl;
	//cout << "xComboIndex: " << xComboIndex << endl;
	//cout << "yComboIndex: " << yComboIndex << endl;

	if (nHitsIndex != -1 && xComboIndex != -1 && yComboIndex != -1){
		//make final pairs vector
		vector<int> pair;
		int xStripIDIndex, yStripIDIndex;
		int nPairs = combinations.at(nHitsIndex).at(0).at(xComboIndex).size();
		for (int i=0; i<nPairs; i++){
			xStripIDIndex = combinations.at(nHitsIndex).at(0).at(xComboIndex).at(i);
			yStripIDIndex = combinations.at(nHitsIndex).at(1).at(yComboIndex).at(i);
			pair.push_back(stripsHit.at(0).at(xStripIDIndex));
			pair.push_back(stripsHit.at(1).at(yStripIDIndex));
			finalPairs.push_back(pair);
			pair.clear();

			hitEnergy.push_back(energy.at(1).at(yStripIDIndex));
			energyResolution.push_back(sig.at(1).at(yStripIDIndex));
			hitQualityFactor.push_back(1);
		}
	}

};

float MNCTModuleStripPairingGreedy_b::CalculateChiSquared(vector<int> xVec, vector<int> yVec){

	int nPairs = xVec.size();


	int xIndex, yIndex;
	float Qs = 0;
	for (int i=0; i<nPairs; i++){
		xIndex = xVec.at(i);
		yIndex = yVec.at(i);	
		Qs = Qs + pow((energy.at(0).at(xIndex) - energy.at(1).at(yIndex)),2) / (pow(sig.at(0).at(xIndex),2) + pow(sig.at(1).at(yIndex),2));
	}

	Qs = Qs / nPairs;

/*	if (stripsHit.at(0).size()==2 && stripsHit.at(1).size()==2){
		if (stripsHit.at(0).at(0)==6 && stripsHit.at(0).at(1)==8 && stripsHit.at(1).at(0)==1 && stripsHit.at(1).at(1)==3){
			cout << "xVec: " << '\t';
			for (int i=0; i<nPairs; i++){
				cout << xVec.at(i) << '\t';
			}
			cout << endl << "yVec: " << '\t';
			for (int i=0; i<nPairs; i++){
				cout << yVec.at(i) << '\t';
			}
			cout << endl;
			cout << "Qs: " << Qs << endl;
		}
	}*/

	return Qs;

};

bool MNCTModuleStripPairingGreedy_b::EnoughStripsPaired(vector<int> xVec, vector<int> yVec){

	//count number of strips in the combination, make sure there's enough
	int nXStrips = 0;
	int nYStrips = 0;

	int xIndex, yIndex;
	for (int i=0; i<xVec.size(); i++){
		xIndex = xVec.at(i);
		if (stripsHit.at(0).at(xIndex) < 50){
			nXStrips++;
		}
		else if (stripsHit.at(0).at(xIndex) > 50 && stripsHit.at(0).at(xIndex) < 100){
			nXStrips += 2;
		}
		else if (stripsHit.at(0).at(xIndex) > 100 && (stripsHit.at(0).at(xIndex)%100) < 50){
			nXStrips += 2;
		}
		else if (stripsHit.at(0).at(xIndex) > 100 && stripsHit.at(0).at(xIndex) < 5000){
			nXStrips += 3;
		}
		else {
			nXStrips += 4;
		}
	}
	for (int i=0; i<yVec.size(); i++){
		yIndex = yVec.at(i);
		if (stripsHit.at(1).at(yIndex) < 50){
			nYStrips++;
		}
		else if (stripsHit.at(1).at(yIndex) > 50 && stripsHit.at(1).at(yIndex) < 100){
			nYStrips += 2;
		}
		else if (stripsHit.at(1).at(yIndex) > 100 && (stripsHit.at(1).at(yIndex)%100) < 50){
			nYStrips += 2;
		}
		else if (stripsHit.at(1).at(yIndex) > 150 && stripsHit.at(1).at(yIndex) < 5000){
			nYStrips += 3;
		}
		else {
			nYStrips += 4;
		}

	}

//	cout << "nXStrips: " << nXStrips << '\t' << "nYStrips: " << nYStrips << endl;
//	cout << "nOrigx: " << nHitsOrig.at(0) << '\t' << "nOrigY: " << nHitsOrig.at(1) << endl;

	if (nHitsOrig.at(0)+nHitsOrig.at(1) > (nXStrips+nYStrips + 1)){
		return false;
	}
	else if (nHitsOrig.at(0)+nHitsOrig.at(1) < nXStrips+nYStrips){
		return false;
	}
	else {
		return true;
	}



};

void MNCTModuleStripPairingGreedy_b::PermuteCombinations(){

//	cout << "size before: " << yCombos.size() << endl;

	vector<vector<int> > yCombosTemp;
	for (int c=0; c<yCombos.size(); c++){
		yCombosTemp.push_back(yCombos.at(c));
		while (next_permutation(yCombos.at(c).begin(), yCombos.at(c).end())){
			yCombosTemp.push_back(yCombos.at(c));
		}
	}

	yCombos.clear();
	yCombos = yCombosTemp;
//	cout << "size after: " << yCombos.size() << endl;

};

void MNCTModuleStripPairingGreedy_b::GetAllCombinations(int axis, vector<int> combo, int start, int end, int index, int r){

	if (index==r){
		if (axis == 0){
			xCombos.push_back(combo);
		}
		else {
			yCombos.push_back(combo);
		}
		return;
	}

	for (int i=start; i<=end && (end-i+1) >= (r-index); i++){
//		cout << stripsHit.at(axis).at(i) << endl;
//		cout << combo.at(index) << endl;
//		combo.at(index) = stripsHit.at(axis).at(i);
		combo.at(index) = i;	//just want index of stripHit, not stripID
		GetAllCombinations(axis, combo, i+1, end, index+1, r);
	}

};

void MNCTModuleStripPairingGreedy_b::RemoveBadStrips(){

	int xCounter = 0;
	int yCounter = 0;

	vector<int> xTempStrips, yTempStrips;
	vector<float> xTempEnergy, yTempEnergy;
	vector<float> xTempSig, yTempSig;

//	for (int axis=0; axis<2; axis++){
//		for (int i=nHits.at(axis)-1; i>=0; i--){
		for (int i=0; i<nHits.at(0); i++){
			if (badCombinations.at(0).at(i) == 1){
				xCounter++;
			}
			else {
				xTempStrips.push_back(stripsHit.at(0).at(i));
				xTempEnergy.push_back(energy.at(0).at(i));
				xTempSig.push_back(sig.at(0).at(i));
			}
		}
		for (int i=0; i<nHits.at(1); i++){
			if (badCombinations.at(1).at(i) == 1){
				yCounter++;
			}
			else {
				yTempStrips.push_back(stripsHit.at(1).at(i));
				yTempEnergy.push_back(energy.at(1).at(i));
				yTempSig.push_back(sig.at(1).at(i));
			}
		}

		stripsHit.clear();
		energy.clear();
		sig.clear();

		stripsHit.push_back(xTempStrips);
		stripsHit.push_back(yTempStrips);
		energy.push_back(xTempEnergy);
		energy.push_back(yTempEnergy);
		sig.push_back(xTempSig);
		sig.push_back(yTempSig);

//				stripsHit.at(axis).erase(stripsHit.at(axis).begin()+i);
//				energy.at(axis).erase(energy.at(axis).begin()+i);
//				sig.at(axis).erase(sig.at(axis).begin()+i);
//				i--;
//	}

	nHits.at(0) = nHits.at(0)-xCounter;
	nHits.at(1) = nHits.at(1)-yCounter;


};

void MNCTModuleStripPairingGreedy_b::RemoveBadCombinations(int axis, int r){

	vector<int> indicesToKeep;
	vector<vector<int> > xCombosTemp, yCombosTemp;

//	if (axis==0){	PrintKillMatrix();}

//	cout << "xCombos.size() : " << xCombos.size() << '\t' << "yCombos.size(): " << yCombos.size() << '\t' << "....." << endl;

	if (axis==0){
		if (xCombos.size() == 1){
//			cout << "xCombos.size() == 1" << endl;
			return;
		}
		else {
			for (int c=0; c<xCombos.size(); c++){
				//cout << "----------------" << endl;
				for (int i=0; i<xCombos.at(c).size()-1; i++){
					for (int j=i+1; j<xCombos.at(c).size(); j++){
						int indexOne = xCombos.at(c).at(i);
						int indexTwo = xCombos.at(c).at(j);
						//cout << xCombos.at(c).at(i) << '\t' << xCombos.at(c).at(j) << endl;
						if (killMatrix.at(0).at(indexOne).at(indexTwo) == 0){
							//cout << xCombos.at(c).at(i) << '\t' << xCombos.at(c).at(j) << endl;
							indicesToKeep.push_back(c);
							xCombosTemp.push_back(xCombos.at(c));
						}
					}
				}
			}
			xCombos.clear();
			xCombos = xCombosTemp;
		}
	}
	else {
		if (yCombos.size() == 1){
			//cout << "yCombos.size() == 1" << endl;
			return;
		}
		else {
			for (int c=0; c<yCombos.size(); c++){
				for (int i=0; i<yCombos.at(c).size()-1; i++){
					for (int j=i+1; j<yCombos.at(c).size(); j++){
						int indexOne = yCombos.at(c).at(i);
						int indexTwo = yCombos.at(c).at(j);
						if (killMatrix.at(axis).at(indexOne).at(indexTwo) == 0){
							indicesToKeep.push_back(c);
							yCombosTemp.push_back(yCombos.at(c));
						}
					}
				}
			}
			yCombos.clear();
			yCombos = yCombosTemp;
		}
	}
	//cout << xCombosTemp.size() << '\t' << yCombosTemp.size() << endl;

	//cout << "xCombo.size(): " << xCombos.size() << '\t' << "yCombos.size() : " << yCombos.size() << endl;
	//cout << "done with function" << endl;

};

vector<int> MNCTModuleStripPairingGreedy_b::GetNumbers(){

	int nOrigx = nHitsOrig.at(0);
	int nOrigy = nHitsOrig.at(1);

	//find number of adjacent strips
	//nAdj = nTotal - nOriginal UNLESS there are three adjacent strips hit
	int nAdjx = nHits.at(0) - nOrigx;
	int nAdjy = nHits.at(1) - nOrigy;

	//check for three adjacent strips hit
	for (int i=0; i<nHits.at(0)-1; i++){
		if (stripsHit.at(0).at(i) > 50){
			if (stripsHit.at(0).at(i+1) - stripsHit.at(0).at(i) == 1){
				nAdjx = nAdjx-1;
			}
		}
	}
	for (int i=0; i<nHits.at(1)-1; i++){
		if (stripsHit.at(1).at(i) > 50){
			if (stripsHit.at(1).at(i+1) - stripsHit.at(1).at(i) == 1){
				nAdjy = nAdjy-1;
			}
		}
	}

	//add multiple hits (conditional?)
	//for now, add multiple hits on both as long as
	//	there's more than 2 strip hits per side
	//if there's 2 per side or less:
	//	add multiple hits UNLESS nOx = nOy and nAx = nAy
	//	if nAx > nAy OR nOx > nOy, add mult hits on y strip
	if (nOrigx <= 2 || nOrigy <= 2){
		if (nOrigx > nOrigy || nAdjx < nAdjy){
			AddMultipleHits(0);
		}
		else if (nOrigy > nOrigx || nAdjy < nAdjx){
			AddMultipleHits(1);
		}
	}
	else {
		AddMultipleHits(0);
		AddMultipleHits(1);
	}


	//figure out number of hits that can possibly be there
	int max_nCombis, min_nCombis;

	if (nOrigx == nOrigy){
		max_nCombis = nOrigx;
	}
	else if (nOrigx > nOrigy){
		max_nCombis = nOrigx;
	}
	else {
		max_nCombis = nOrigy;
	}

	if ( (nOrigx-nAdjx) == (nOrigy-nAdjy) ){
		min_nCombis = nOrigx-nAdjx;
	}
	else if ( (nOrigx-nAdjx) == (nOrigy-nAdjy)+1 ){
		min_nCombis = nOrigy-nAdjy;
	}
	else if ( (nOrigy-nAdjy) == (nOrigx-nAdjx)+1 ){
		min_nCombis = nOrigx-nAdjx;
	}
	else if ( (nOrigx-nAdjx) > (nOrigy-nAdjy) ){
		min_nCombis = (nOrigx-nAdjx) - 1;
	}
	else {
		min_nCombis = (nOrigy-nAdjy) - 1;
	}

	vector<int> possibleNHits;
	possibleNHits.push_back(min_nCombis);
	if (max_nCombis - min_nCombis == 0){
		; //empty statement (like pass in python)
	}
	else if (max_nCombis - min_nCombis == 1){
		possibleNHits.push_back(max_nCombis);
	}	
	else {
		int diff = max_nCombis - min_nCombis;
		for (int i=1; i<diff; i++){
			possibleNHits.push_back(min_nCombis + i);
		}
		possibleNHits.push_back(max_nCombis);
	}

	//SHOULD FIX THIS LATER!!
	//problem arises because multiple hits per strip count as "one" hit
	//even though they are 2
	if ((nOrigx==1 && nOrigy>=3) || (nOrigx>=3 && nOrigy==1)){
		possibleNHits.clear();
		possibleNHits.push_back(1);
		possibleNHits.push_back(2);
	}

	return possibleNHits;


};


////////////////////////////////////////////////////////////////////////////////

void MNCTModuleStripPairingGreedy_b::ShowOptionsGUI(){
  
  // Show the options GUI - or do nothing
	MGUIOptionsStripPairing* Options = new MGUIOptionsStripPairing(this);
	Options->Create();
	gClient->WaitForUnmap(Options);


};

bool MNCTModuleStripPairingGreedy_b::ReadXmlConfiguration(MXmlNode* Node){

	//! Read the configuration data from an XML node

	MXmlNode* ModeNode = Node->GetNode("Mode");
	if (ModeNode != 0){
		m_Mode = ModeNode->GetValueAsUnsignedInt();
	}

	return true;

};

MXmlNode* MNCTModuleStripPairingGreedy_b::CreateXmlConfiguration(){

	//! Create an XML node tree from the configuration

	MXmlNode* Node = new MXmlNode(0, m_XmlTag);
	new MXmlNode(Node, "Mode", m_Mode);

	return Node;

};

/////////////////////////////////////////////////////////////////////////////////

//print the stripsHit vector
void MNCTModuleStripPairingGreedy_b::PrintXYStripsHit(){
  
//  cout << "stripsHit.size(): " << stripsHit.size() << endl;
  
  cout << "--------------------------" << endl << "Printing xStripsHit...." << endl;
  for(int i=0; i<nHits.at(0); i++){
    cout << stripsHit.at(0).at(i) << '\t' << energy.at(0).at(i) << endl;
  }
  
  cout << "--------------------------" << endl << "Printing yStripsHit...." << endl;
  for(int i=0; i<nHits.at(1); i++){
    cout << stripsHit.at(1).at(i) << '\t' << energy.at(1).at(i) << endl;
  }
  
};

//print the weight matrix
void MNCTModuleStripPairingGreedy_b::PrintWeightMatrix(){
  
  cout << "-------------------" << endl;
  
  cout << nHits.at(1) << endl;
  cout << nHits.at(0) << endl;
  cout << weightMatrix.size() << endl;
  cout << weightMatrix.at(0).size() << endl;
  //	cout << weightMatrix.at(1).size() << endl;
  
  cout << "printing matrix" << endl;
  for (int i=0; i<nHits.at(1); i++){
    for (int j=0; j<nHits.at(0); j++){
      cout << weightMatrix.at(j).at(i) << '\t';
    }
    cout << endl;
  }
  
  
  cout << "-------------------" << endl;
  
};

void MNCTModuleStripPairingGreedy_b::PrintKillMatrix(){

	for (int axis=0; axis<2; axis++){
		cout << "axis: " << axis << endl;
		for (int m=0; m<nHits.at(axis); m++){
			for (int n=0; n<nHits.at(axis); n++){
				cout << killMatrix.at(axis).at(m).at(n) << '\t';
			}
			cout << endl;
		}
		cout << "-------" << endl;
	} 

};

float MNCTModuleStripPairingGreedy_b::GetEth(){
  
  return Eth;
  
};

vector<vector<int> > MNCTModuleStripPairingGreedy_b::GetStripsHit(){
  
  return stripsHit;
  
};

void MNCTModuleStripPairingGreedy_b::SetStripsHit(vector<vector<int> > inputVec){
  
  nHits.clear();
  stripsHit.clear();
  
  stripsHit = inputVec;
  nHits.push_back(inputVec.at(0).size());
  nHits.push_back(inputVec.at(1).size());
  nHitsOrig = nHits;
  
  //	cout << "stripsHit size: " << stripsHit.size() << '\t' <<  stripsHit.at(0).size() << '\t' <<stripsHit.at(1).size() << endl;
  
};

vector<vector<float> > MNCTModuleStripPairingGreedy_b::GetEnergy(){
  
  return energy;
  
};

void MNCTModuleStripPairingGreedy_b::SetEnergy(vector<vector<float> > inputVec){
  
  energy.clear();
  energy = inputVec;
  
};

vector<vector<float> > MNCTModuleStripPairingGreedy_b::GetSigma(){
  
  return sig;
  
};

void MNCTModuleStripPairingGreedy_b::SetSigma(vector<vector<float> > inputVec){
  
  sig.clear();
  sig = inputVec;
  
};

void MNCTModuleStripPairingGreedy_b::SetFinalPairs(vector<vector<int> > inputVec){
  
  finalPairs.clear();
  finalPairs = inputVec;
  /*
  cout << "printing final pairs: " << endl;
  cout << finalPairs.size() << endl;
  cout << finalPairs.at(0).size() << endl;
  cout << finalPairs.at(1).size() << endl;
  for (int i=0; i<finalPairs.at(0).size(); i++){
    cout << finalPairs.at(0).at(i) << '\t' << finalPairs.at(1).at(i) << endl;
  }
  */
};

vector<vector<float> > MNCTModuleStripPairingGreedy_b::GetWeightMatrix(){
  
  return weightMatrix;
  
};

vector<vector<int> > MNCTModuleStripPairingGreedy_b::GetBadCombinations(){
  
  return badCombinations;
  
};

void MNCTModuleStripPairingGreedy_b::SetBadCombinations(vector<vector<int> > inputVec){
  
  badCombinations.clear();
  badCombinations = inputVec;
  
};

int MNCTModuleStripPairingGreedy_b::GetNBadCombinations(int axis){

	int counter = 0;

	for (int i=0; i<nHits.at(axis); i++){
		if (badCombinations.at(axis).at(i) == 1){
			counter += 1;
		}
	}

	return counter;
};
