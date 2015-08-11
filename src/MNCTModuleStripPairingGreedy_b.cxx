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
 // m_HasOptionsGUI = true;
	m_HasOptionsGUI = false;

//	m_Mode = 1;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  m_NBadMatches = 0;
  m_NMatches = 0;
  m_TotalMatches = 0;
  
  // Set the histogram display
  m_ExpoStripPairing = new MGUIExpoStripPairing(this);
  m_ExpoStripPairing->SetEnergiesHistogramParameters(1500, 0, 1500);
  m_Expos.push_back(m_ExpoStripPairing);
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = true;
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

//	usleep(100);

	 
  const int nDetectors = 12;

	//bool newAlg;
	//if (m_Mode == 0) { newAlg = true; }
	//else { newAlg = false; }

	vector<vector<int> > pairs_temp1, pairs_temp2, pairs_temp3;
	vector<int> mult_temp1, mult_temp2, mult_temp3;
	vector<int> share_temp1, share_temp2, share_temp3;
	float firstChiSq,secondChiSq,thirdChiSq,fourthChiSq;

	//to see if hits with multiple strips are well PAIRED
	//need to look at Chi-Sq, not final hit n and p energies
	float chi_sq[nDetectors];

  for (int detector = 0; detector < nDetectors; detector++){
		firstChiSq = secondChiSq = thirdChiSq = fourthChiSq = -1;

    bool runRest = GetEventInfo(Event, detector);

    if (nHits.at(0)>0 && nHits.at(1)>0 && runRest==true){
	    InitializeKillMatrices();
	    CheckForAdjacentStrips();
			CheckForBadCombinations();
			firstChiSq = FindFinalPairs();

			chi_sq[detector] = firstChiSq;

			if (firstChiSq > 25){
				pairs_temp1 = finalPairs;
				mult_temp1 = stripHitMultipleTimes;
				share_temp1 = chargeSharing;

				weightMatrix.clear();
				badCombinations.clear();
				finalPairs.clear();
				finalPairEnergy.clear();
				finalPairRes.clear();

				ChargeSharingThreeStrips(0);
				ChargeSharingThreeStrips(1);	
	
//			CheckMultipleHits();
				CheckForBadCombinations();
				secondChiSq = FindFinalPairs();

				chi_sq[detector] = secondChiSq;

//				PrintFinalPairs();
//				cout << "-----" << endl;
//				cout << "CHISQ(1): " << secondChiSq << endl;


				if (secondChiSq > 25){
					pairs_temp2 = finalPairs;
					mult_temp2 = stripHitMultipleTimes;
					share_temp2 = chargeSharing;

					weightMatrix.clear();
					badCombinations.clear();
					finalPairs.clear();
					finalPairEnergy.clear();
					finalPairRes.clear();

					AddMultipleHits(0);
					AddMultipleHits(1);

					CheckForBadCombinations();
					thirdChiSq = FindFinalPairs();

					chi_sq[detector] = thirdChiSq;


					if (thirdChiSq > 25){
						pairs_temp3 = finalPairs;
						mult_temp3 = stripHitMultipleTimes;
						share_temp3 = chargeSharing;

						weightMatrix.clear();
						badCombinations.clear();
						finalPairs.clear();
						finalPairEnergy.clear();
						finalPairRes.clear();

						AddThreeHits(0);
						AddThreeHits(1);

						CheckForBadCombinations();
						fourthChiSq = FindFinalPairs();

						chi_sq[detector] = fourthChiSq;

						//find min chi_sq
						if (firstChiSq<secondChiSq && firstChiSq<thirdChiSq && firstChiSq<fourthChiSq){
							finalPairs = pairs_temp1;
							stripHitMultipleTimes = mult_temp1;
							chargeSharing = share_temp1;
						}
						else if (secondChiSq<firstChiSq && secondChiSq<thirdChiSq && secondChiSq<fourthChiSq){
							finalPairs = pairs_temp2;
							stripHitMultipleTimes = mult_temp2;
							chargeSharing = share_temp2;
						}
						else if (thirdChiSq<firstChiSq && thirdChiSq<secondChiSq && thirdChiSq<fourthChiSq){
							finalPairs = pairs_temp3;
							stripHitMultipleTimes = mult_temp3;
							chargeSharing = share_temp3;
						}
/*						else {
							PrintXYStripsHitOrig();
							PrintFinalPairs();
							dummy_func();
						}
*/
/*					PrintXYStripsHitOrig();
					PrintFinalPairs();
					dummy_func();
*/

					}

					if (firstChiSq<secondChiSq && firstChiSq<thirdChiSq){
						finalPairs = pairs_temp1;
						stripHitMultipleTimes = mult_temp1;
						chargeSharing = share_temp1;
					}
					else if (secondChiSq<firstChiSq && secondChiSq<thirdChiSq){
						finalPairs = pairs_temp2;
						stripHitMultipleTimes = mult_temp2;
						chargeSharing = share_temp2;
					}
				}

				if (firstChiSq<secondChiSq){
					finalPairs = pairs_temp1;
					stripHitMultipleTimes = mult_temp1;
					chargeSharing = share_temp1;
				}
			}




     	CalculateDetectorQuality();
			WriteHits(Event, detector);

/*			if (thirdChiSq != -1){
				cout << "CHISQ(2): " << chi_sq[detector] << endl;
				PrintXYStripsHitOrig();
				PrintFinalPairs();
				dummy_func();
			}
*/		}
    else {
	    detectorQualityFactors.push_back(0);
	  }
	}
	CalculateEventQuality(Event, nDetectors);
	detectorQualityFactors.clear();

	for (unsigned int h = 0; h < Event->GetNHits(); h++){
		if (Event->GetHit(h)->GetStripHitMultipleTimes() == true){
			Event->SetStripPairingIncomplete(true,"multiple hits per strip");
/*			int detID = Event->GetHit(h)->GetStripHit(0)->GetDetectorID();
			if (chi_sq[detID] <= 25){
				Event->SetStripPairingIncomplete(true,"good pairing, multiple hits per strip");
			}
			else {
				Event->SetStripPairingIncomplete(true,"bad pairing, multiple hits per strip");
			}
*/		}
	}
 

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
			if (Event->GetHit(h)->GetStripHitMultipleTimes() == false){
	      m_ExpoStripPairing->AddEnergies(pEnergy, nEnergy);
			}
    }
    
    pUncertainty = sqrt(pUncertainty);
    nUncertainty = sqrt(nUncertainty);
    
    double Difference = fabs(pEnergy - nEnergy);
/*		if (Difference > 100){
			cout << "Difference: " << Difference << endl;
			cout << "strip hit multiple times?  " << Event->GetHit(h)->GetStripHitMultipleTimes() << endl;
			dummy_func();
		}
*/
    // Difference must be more than 10 keV for cross talk + 2 sigma energy resolution on *both* sides
//		if (Event->GetHit(h)->GetStripHitMultipleTimes() == true){
//			Event->SetStripPairingIncomplete(true,"multiple hits per strip");
//		}

    if (Difference > 2*pUncertainty + 2*nUncertainty + 20) {
			if (Event->GetHit(h)->GetStripHitMultipleTimes() == false){
		  	Event->SetStripPairingIncomplete(true,"bad pairing");
			}
/*			if (nHits.at(0) != 0 && nHits.at(1) != 0){
				PrintXYStripsHitOrig();
				PrintFinalPairs();
				cout << "fourth chisq: " << fourthChiSq << endl;


				cout << "----------" << endl;
				cout << "HIT: " << h << endl;
				cout << "pEnergy: " << pEnergy << endl;
				cout << "nEnergy: " << nEnergy << endl;
				cout << "Difference: " << Difference << endl;
				cout << "pUncertainty: " << pUncertainty << endl;
				cout << "nUncertainty: " << nUncertainty << endl;
				cout << "2*pUnc+2*nUnc+10: " << 2*pUncertainty+2*nUncertainty+10<<endl;
				dummy_func();
			}*/
 


//			}
//			else{
//				Event->SetStripPairingIncomplete(true,"multiple hits per strip");
//			}


     if (g_Verbosity >= c_Warning) cout<<"Bad strip pairing: p: E="<<pEnergy<<" dE="<<pUncertainty<<" n: E="<<nEnergy<<" dE="<<nUncertainty<<endl; 
    }
		else {
      if (g_Verbosity >= c_Info) cout<<"Good strip pairing: p: E="<<pEnergy<<" dE="<<pUncertainty<<" n: E="<<nEnergy<<" dE="<<nUncertainty<<endl; 
    }
		//flag events with charge sharing
//		if (Event->GetHit(h)->GetChargeSharing()){
//			Event->SetStripPairingIncomplete(true,"charge sharing");
//		}
  }
//	}  
  
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
    
 	  float xTotalEnergy = 0; 
 	  float yTotalEnergy = 0; 
 		float xTotalUnc = 0;
		float yTotalUnc = 0;

    for (unsigned int i=0; i<Event->GetNStripHits(); i++){
      if (detector == Event->GetStripHit(i)->GetDetectorID()){
        if (Event->GetStripHit(i)->IsXStrip() == true){
          xTotalEnergy += Event->GetStripHit(i)->GetEnergy();
					xTotalUnc += pow(Event->GetStripHit(i)->GetEnergyResolution(),2);
        }    
        else {
			    yTotalEnergy += Event->GetStripHit(i)->GetEnergy();
					yTotalUnc += pow(Event->GetStripHit(i)->GetEnergyResolution(),2);
        }
			}
		}

		//check if initial n and p energy difference is high
		xTotalUnc = sqrt(xTotalUnc);
		yTotalUnc = sqrt(yTotalUnc);

//		double diff = fabs(xTotalEnergy-yTotalEnergy);

//		if (diff > 2*xTotalUnc + 2*yTotalUnc + 20){
//    if (fabs(xTotalEnergy-yTotalEnergy) > 30.){ 
//	     nHits.push_back(0);
 //	  	 nHits.push_back(0);
	// 	   return false;
//			Event->SetStripPairingIncomplete(true,"mismatched start energies");
 //  	}    

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
//    nHitsOrig = nHits;
  	nHitsOrig.push_back(n_x);
		nHitsOrig.push_back(n_y);  
    
    
    
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
//		if (Event->IsStripPairingIncomplete()){
//			PrintXYStripsHit();
//			dummy_func();
//		}
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
		if (stripHitMultipleTimes.at(pair) == 1){
			Hit->SetStripHitMultipleTimes(true);
/*			PrintXYStripsHitOrig();
			PrintFinalPairs();
			dummy_func();
*/		}
		else {
			Hit->SetStripHitMultipleTimes(false);
		}
		if (chargeSharing.at(pair) == 1){
			Hit->SetChargeSharing(true);
		}
		else {
			Hit->SetChargeSharing(false);
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
	nHitsAdj.clear();
  nHitsOrig.clear();
	nThreeHitsAdj.clear();
  badCombinations.clear();
  killMatrix.clear();
  weightMatrix.clear();
  finalPairs.clear();
	finalPairEnergy.clear();
 	finalPairRes.clear();

	stripHitMultipleTimes.clear();
	chargeSharing.clear();
  hitQualityFactor.clear();
  energyResolution.clear();
  hitEnergy.clear();
  //	detectorQualityFactors.clear();
  
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
 
	nHitsAdj.push_back(0);
	nHitsAdj.push_back(0);

	//initialize here just because?
	nThreeHitsAdj.push_back(0);
	nThreeHitsAdj.push_back(0);
 
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
		nHitsAdj.at(axis) = nHits.at(axis) - nHitsOrig.at(axis);

    counter = 0;
  }

 
  return adjStrips;
  
};

//check if multiple hits need to be considered
//multiple hits need to be considered when there are more hits on one side than the other
bool MNCTModuleStripPairingGreedy_b::CheckMultipleHits(){
  
  //	DefineEventInfo();
  
  int n_x = nHitsOrig.at(0);
  int n_y = nHitsOrig.at(1);
  
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
  for (int i=0; i<nHits.at(axis)-nThreeHitsAdj.at(axis); i++){
    for (int j=i+1; j<nHits.at(axis)-nThreeHitsAdj.at(axis); j++){
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

//add possibility of charge sharing on 3 strips to vector of hits
void MNCTModuleStripPairingGreedy_b::ChargeSharingThreeStrips(int axis){

	int counter = 0;

	//check pairs of adjacent strips
	for (int i=nHitsOrig.at(axis); i<nHitsOrig.at(axis)+nHitsAdj.at(axis)-1; i++){
		if ( fabs(stripsHit.at(axis).at(i+1) - stripsHit.at(axis).at(i)) == 1){
			int sID_one = stripsHit.at(axis).at(i)-50;

			//get strip IDs to check energies
			int indOne = GetStripIndex(axis,sID_one);

			//check energies
			float eOne = energy.at(axis).at(indOne);
			float eTwo = energy.at(axis).at(indOne+1);
			float eThree = energy.at(axis).at(indOne+2);

			//if energy makes sense, get new "strip ID"
			if (eTwo > eOne && eTwo > eThree){
				int newStripID = 5000+sID_one;
				stripsHit.at(axis).push_back(newStripID);
				float newEnergy = eOne+eTwo+eThree;
				energy.at(axis).push_back(newEnergy);

				//get energy resolution
				float eResOne = sig.at(axis).at(indOne);
				float eResTwo = sig.at(axis).at(indOne+1);
				float eResThree = sig.at(axis).at(indOne+2);
				float newERes = sqrt(eResOne*eResOne + eResTwo*eResTwo + eResThree*eResThree);
				sig.at(axis).push_back(newERes);

				counter += 1;

				//add things to kill matrix
				ExpandKillMatrix(axis);

//				cout << "killMatrix.at(0).size(): " << killMatrix.at(0).size() << endl;

	      //each element is the vector sum of its constituent strips
	      for (int k=0; k<nHits.at(axis)+counter-1; k++){
	        killMatrix.at(axis).at(nHits.at(axis)+counter-1).at(k) = killMatrix.at(axis).at(indOne).at(k) + killMatrix.at(axis).at(indOne+1).at(k) + killMatrix.at(axis).at(indOne+2).at(k);
	        killMatrix.at(axis).at(k).at(nHits.at(axis)+counter-1) = killMatrix.at(axis).at(k).at(indOne) + killMatrix.at(axis).at(k).at(indOne+1) + killMatrix.at(axis).at(k).at(indOne+2);
	      }

			}
		}
	}

	nHits.at(axis) = nHits.at(axis) + counter;
	nThreeHitsAdj.at(axis) = nThreeHitsAdj.at(axis) + counter;

};


//add possibilities of three hits to vector of hits
void MNCTModuleStripPairingGreedy_b::AddThreeHits(int axis){
  
  //initialize variables
  int x1, x2, pair;
  float pairE, pairSig;
  int nPairs = 0;
  vector<int> pairHitsVec;
  vector<float> pairEnergyVec, pairSigVec;
  
  //goal is to make all possible combinations of strips on one axis
  for (int i=0; i<nHitsOrig.at(axis)+nHitsAdj.at(axis); i++){
    for (int j=nHitsOrig.at(axis)+nHitsAdj.at(axis)+nThreeHitsAdj.at(axis); j<nHits.at(axis); j++){
      x1 = stripsHit.at(axis).at(i);
      x2 = stripsHit.at(axis).at(j);
      
      //label combinations by multiplying the lower strip number by 100 and adding it to the higher strip number
      //append combined strip to pairHitsVec
      pair = x1*10000+x2;
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
//     			cout << badCombinations.at(axis).at(i) << "  ";

    }
//    		cout << endl;
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
 
//	PrintXYStripsHit();
 
  //for each x-y pair, fill in weight matrix with the weight of the pair
  //if it's a bad pair, set weight to -1
  for (int i=0; i<n_x; i++){
    for (int j=0; j<n_y; j++){
      if (badCombinations.at(0).at(i) == 0 && badCombinations.at(1).at(j) == 0){
//        if (i<n_xOrig || j<n_yOrig){
//				if (i<n_x-nHitsAdj.at(0) || j<n_y-nHitsAdj.at(1)){ 
				if (i<n_xOrig + nHitsAdj.at(0) || j<n_yOrig + nHitsAdj.at(1)){
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
      //  	cout << "minWeight: " << min_weight << endl;
      //	 	cout << "xIndex: " << xIndex << endl;
      //  	cout << "yIndex: " << yIndex << endl;
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
float MNCTModuleStripPairingGreedy_b::FindFinalPairs(){

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
	vector<float> energyPair;
	vector<float> resPair;
  float weight;
  //float hitE;
  float eRes;
	float greedyChiSq = 0;

  //	cout << "nElements: " << nElements << endl;
  //find minimum weight while the number of elements that equal -1 in the weight matrix
  //is less than the number of elements in the weight matrix
  do{
//		PrintWeightMatrix();
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

		energyPair.push_back(energy.at(0).at(xIndex));
		energyPair.push_back(energy.at(1).at(yIndex));
		finalPairEnergy.push_back(energyPair);
    energyPair.clear();

		resPair.push_back(sig.at(0).at(xIndex));
		resPair.push_back(sig.at(1).at(yIndex));
		finalPairRes.push_back(resPair);
		resPair.clear();

		greedyChiSq += pow((energy.at(0).at(xIndex)-energy.at(1).at(yIndex)),2) / (pow(sig.at(0).at(xIndex),2) + pow(sig.at(1).at(yIndex),2));

    //fill hit quality factor vector
    weight = weightMatrix.at(xIndex).at(yIndex);
    hitQualityFactor.push_back(weight);
    
    //fill hit energy resolution vector
    float eResX = sig.at(0).at(xIndex);
    float eResY = sig.at(1).at(yIndex);
    eRes = (sqrt(2)*eResX*eResY)/(eResX*eResX+eResY*eResY);
    energyResolution.push_back(eRes);
    
    //fill hit energy vector
//    float eX = energy.at(0).at(xIndex);
    float eY = energy.at(1).at(yIndex);
//    double inf = numeric_limits<double>::infinity();
    //sometimes the energy resolution is infinite (not sure why)
    //if it is, then take the average of the energies
    //otherwise use correct formula
    //if (eResX != inf && eResY != inf){
    //  hitE = (eX/(eResX*eResX)+eY/(eResY*eResY))/(1/(eResX*eResX)+1/(eResY*eResY));
    //}
    //else {
    //  hitE = (eX+eY)/2;
    //}
    // At this stage we can only use ONE side, the above formula we can only use AFTER charge charging/cross talk correction!
    hitEnergy.push_back(eY); //hitEnergy.push_back(hitE);
    
    //		PrintWeightMatrix();
    //call ConflictingStrips to avoid repeating strips
    ConflictingStrips(xIndex, yIndex);
    nNegElem = CountNegativeElements();
    //		cout << "nNegElem: " << nNegElem << endl;
  }
  while (nNegElem != nElements);

	int nPairs = finalPairs.size();
	greedyChiSq = greedyChiSq / nPairs;
  

  //print final pairs
  if (g_Verbosity >= c_Info) {
    cout << "final pairs: " << endl;
    for (unsigned int i=0; i<finalPairs.size(); i++){
      cout << finalPairs.at(i).at(0) << '\t' << finalPairs.at(i).at(1) << endl;
    }
		cout << "chi sq: " << greedyChiSq << endl;
  }


	return greedyChiSq;
};

//puts list of final pairs back into strip numbers
//for example, if strip 51 is selected, it gets separated into strips 1 and 2
//if 104 is selected, it gets separated into strips 1 and 4
//if 256 is selected, it gets separated into strip 2 and charge sharing between 6 and 7
vector<vector<vector<int> > > MNCTModuleStripPairingGreedy_b::DecodeFinalPairs(){

/*
	for (unsigned int pair=0; pair<finalPairs.size(); pair++){
		if (finalPairs.at(pair).at(0) > 100 && finalPairs.at(pair).at(1) > 100){
			dummy_func();
		}
	}
*/
 
  vector<int> xVec;
  vector<int> yVec;
  vector<vector<int> > pair;
  vector<vector<vector<int> > > decodedFinalPairs;

	//if there are multiple hits per strip, they should be separate hits
	bool xTwoHits, yTwoHits;
	bool xThreeHits, yThreeHits;
	bool xChargeSharing, yChargeSharing;
	//need to add energy to hitEnergy vector for the additional hit
	int twoHitsCounter = 0;
	int threeHitsCounter = 0;
 
  int nPairs = finalPairs.size();
  //	cout << "nPairs: " << nPairs << endl;
  for (int i=0; i<nPairs; i++){
		xTwoHits = false;
		yTwoHits = false;
		xThreeHits = false;
		yThreeHits = false;
		xChargeSharing = false;
		yChargeSharing = false;

		//charge sharing between two strips
    if (finalPairs.at(i).at(0) > 50 && finalPairs.at(i).at(0) < 100){
      xVec.push_back(finalPairs.at(i).at(0)-50);
      xVec.push_back(finalPairs.at(i).at(0)+1-50);
			xChargeSharing = true;
    }
		//charge sharing between three strips
		else if (finalPairs.at(i).at(0) > 5000 && finalPairs.at(i).at(0) < 5100){
			xVec.push_back(finalPairs.at(i).at(0)-5000);
			xVec.push_back(finalPairs.at(i).at(0)+1-5000);
			xVec.push_back(finalPairs.at(i).at(0)+2-5000);
			xChargeSharing = true;
		}
		//two hits on x: y strip hit twice
    else if (finalPairs.at(i).at(0) > 100 && finalPairs.at(i).at(0) < 10000){
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
		//three hits on x: y strip hit three times
		else if (finalPairs.at(i).at(0) > 10000){
			xThreeHits = true;
			xVec.push_back(finalPairs.at(i).at(0)/10000);
			int lowerFourDigits = finalPairs.at(i).at(0)-(int)(finalPairs.at(i).at(0)/10000)*10000;
	    if (lowerFourDigits-(int)(lowerFourDigits/100)*100 > 50){
				xChargeSharing = true;
        xVec.push_back(lowerFourDigits/100);
        xVec.push_back(lowerFourDigits-(int)(lowerFourDigits/100)*100-50);
        xVec.push_back(lowerFourDigits-(int)(lowerFourDigits/100)*100+1-50);
      }
      else {
        xVec.push_back(lowerFourDigits/100);
        xVec.push_back(lowerFourDigits - (int)(lowerFourDigits/100)*100);
      }
 		}
		//simplest case: one hit on x, one on y
    else if (finalPairs.at(i).at(0) < 38) {xVec.push_back(finalPairs.at(i).at(0));}
    
    if (finalPairs.at(i).at(1) > 50 && finalPairs.at(i).at(1) < 100){
      yVec.push_back(finalPairs.at(i).at(1)-50);
      yVec.push_back(finalPairs.at(i).at(1)+1-50);
			yChargeSharing = true;
    }
		else if (finalPairs.at(i).at(1) > 5000 && finalPairs.at(i).at(1) < 5100){
			yVec.push_back(finalPairs.at(i).at(1)-5000);
			yVec.push_back(finalPairs.at(i).at(1)+1-5000);
			yVec.push_back(finalPairs.at(i).at(1)+2-5000);
			yChargeSharing = true;
		}
    else if (finalPairs.at(i).at(1) > 100 && finalPairs.at(i).at(1) < 10000){
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
		else if (finalPairs.at(i).at(1) > 10000){
			yThreeHits = true;
			yVec.push_back(finalPairs.at(i).at(1)/10000);
			int lowerFourDigits = finalPairs.at(i).at(1)-(int)(finalPairs.at(i).at(1)/10000)*10000;
	    if (lowerFourDigits-(int)(lowerFourDigits/100)*100 > 50){
				yChargeSharing = true;
        yVec.push_back(lowerFourDigits/100);
        yVec.push_back(lowerFourDigits-(int)(lowerFourDigits/100)*100-50);
        yVec.push_back(lowerFourDigits-(int)(lowerFourDigits/100)*100+1-50);
      }
      else {
        yVec.push_back(lowerFourDigits/100);
        yVec.push_back(lowerFourDigits - (int)(lowerFourDigits/100)*100);
      }
 		}
    else if (finalPairs.at(i).at(1) < 38) {yVec.push_back(finalPairs.at(i).at(1));}

		if (xTwoHits){
			twoHitsCounter += 1;
			int indexOne, indexTwo;

			vector<int> xVecNew;
			//first hit
			xVecNew.push_back(xVec.at(0));
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			pair.clear();
			xVecNew.clear();
			stripHitMultipleTimes.push_back(1);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(0,xVec.at(0));

			//second hit
			if (xChargeSharing){
				xVecNew.push_back(xVec.at(1));
				xVecNew.push_back(xVec.at(2));
				pair.push_back(xVecNew);
				pair.push_back(yVec);
				decodedFinalPairs.push_back(pair);
				stripHitMultipleTimes.push_back(1);
				chargeSharing.push_back(1);
				indexTwo = GetStripIndex(0, (50+xVec.at(1)));
			}
			else {
				xVecNew.push_back(xVec.at(1));
				pair.push_back(xVecNew);
				pair.push_back(yVec);
  	  	decodedFinalPairs.push_back(pair);
				stripHitMultipleTimes.push_back(1);
				chargeSharing.push_back(0);
				indexTwo = GetStripIndex(0, xVec.at(1));
			}
			xVecNew.clear();
			//change energy and resolution for second hit
			//also add hit quality, but keep it the same for now
			//because there are multiple hits on a y strip, hit energy should be the energy of the x strip
			float energyOne = energy.at(0).at(indexOne);
			float energyTwo = energy.at(0).at(indexTwo);
			float eResOne = sig.at(0).at(indexOne);
			float eResTwo = sig.at(0).at(indexTwo);

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

			hitEnergy.at(i+twoHitsCounter-1) = energyOne;
			hitEnergy.insert(hitEnergy.begin()+i+twoHitsCounter, energyTwo);
			energyResolution.at(i+twoHitsCounter-1) = eResOne;
			energyResolution.insert(energyResolution.begin()+i+twoHitsCounter, eResTwo); 
			hitQualityFactor.insert(hitQualityFactor.begin()+i+twoHitsCounter, hitQualityFactor.at(i+twoHitsCounter-1));
		}
		else if (yTwoHits){
			twoHitsCounter += 1;
			int indexOne, indexTwo;

			vector<int> yVecNew;
			//first hit
			pair.push_back(xVec);
			yVecNew.push_back(yVec.at(0));
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			pair.clear();
			yVecNew.clear();
			stripHitMultipleTimes.push_back(1);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(1,yVec.at(0));

			//second hit
			if (yChargeSharing){
				yVecNew.push_back(yVec.at(1));
				yVecNew.push_back(yVec.at(2));
				pair.push_back(xVec);
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(1);
				indexTwo = GetStripIndex(1, (50+yVec.at(1)));
			}
			else {
				pair.push_back(xVec);
				yVecNew.push_back(yVec.at(1));
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(0);
				indexTwo = GetStripIndex(1,yVec.at(1));
			}
			stripHitMultipleTimes.push_back(1);
			yVecNew.clear();
			//change energy and resolution for second hit
			//also add element to hit quality, need to do properly later
			float energyOne = energy.at(1).at(indexOne);
			float energyTwo = energy.at(1).at(indexTwo);
			float eResOne = sig.at(1).at(indexOne);
			float eResTwo = sig.at(1).at(indexTwo);

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

			hitEnergy.at(i+twoHitsCounter-1) = energyOne;
			hitEnergy.insert(hitEnergy.begin()+i+twoHitsCounter, energyTwo);
			energyResolution.at(i+twoHitsCounter-1) = eResOne;
			energyResolution.insert(energyResolution.begin()+i+twoHitsCounter, eResTwo); 
			hitQualityFactor.insert(hitQualityFactor.begin()+i+twoHitsCounter, hitQualityFactor.at(i+twoHitsCounter-1));
		}
		else if (xThreeHits){
			threeHitsCounter += 1;
			int indexOne, indexTwo, indexThree;

			vector<int> xVecNew;
			//first hit
			xVecNew.push_back(xVec.at(0));
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			stripHitMultipleTimes.push_back(1);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(0,xVec.at(0));
			pair.clear();
			xVecNew.clear();
			//change energy and resolution for first hit
			//hitEnergy.at(i+twoHitsCounter-1) = 

			//second hit
			xVecNew.push_back(xVec.at(1));
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			stripHitMultipleTimes.push_back(1);
			chargeSharing.push_back(0);
			indexTwo = GetStripIndex(0,xVec.at(1));
			pair.clear();
			xVecNew.clear();

			//third hit
			if (xChargeSharing){
				xVecNew.push_back(xVec.at(2));
				xVecNew.push_back(xVec.at(3));
				pair.push_back(xVecNew);
				pair.push_back(yVec);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(1);
				indexThree = GetStripIndex(0,(50+xVec.at(2)));
			}
			else {
				xVecNew.push_back(xVec.at(2));
				pair.push_back(xVecNew);
				pair.push_back(yVec);
  	  	decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(0);
				indexThree = GetStripIndex(0,xVec.at(2));
			}
			stripHitMultipleTimes.push_back(1);
			xVecNew.clear();
			//change energy and resolution for second hit
			//also add hit quality, but keep it the same for now
			float energyOne = energy.at(0).at(indexOne);
			float energyTwo = energy.at(0).at(indexTwo);
			float energyThree = energy.at(0).at(indexThree);
			float eResOne = sig.at(0).at(indexOne);
			float eResTwo = sig.at(0).at(indexTwo);
			float eResThree = sig.at(0).at(indexThree);

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

			hitEnergy.at(i+threeHitsCounter-1) = energyOne;
			hitEnergy.insert(hitEnergy.begin()+i+threeHitsCounter, energyTwo);
			hitEnergy.insert(hitEnergy.begin()+i+threeHitsCounter+1, energyThree);

			energyResolution.at(i+threeHitsCounter-1) = eResOne;
			energyResolution.insert(energyResolution.begin()+i+threeHitsCounter, eResTwo);
			energyResolution.insert(energyResolution.begin()+i+threeHitsCounter+1, eResThree);

			hitQualityFactor.insert(hitQualityFactor.begin()+i+threeHitsCounter, hitQualityFactor.at(i+threeHitsCounter-1));
			hitQualityFactor.insert(hitQualityFactor.begin()+i+threeHitsCounter+1, hitQualityFactor.at(i+threeHitsCounter-1));	
		}
		else if (yThreeHits){
			threeHitsCounter += 1;
			int indexOne, indexTwo, indexThree;

			vector<int> yVecNew;
			//first hit
			pair.push_back(xVec);
			yVecNew.push_back(yVec.at(0));
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			stripHitMultipleTimes.push_back(1);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(1,yVec.at(0));
			pair.clear();
			yVecNew.clear();
			//second hit
			pair.push_back(xVec);
			yVecNew.push_back(yVec.at(1));
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			stripHitMultipleTimes.push_back(1);
			chargeSharing.push_back(0);
			indexTwo = GetStripIndex(1,yVec.at(1));
			pair.clear();
			yVecNew.clear();
			//third hit
			if (yChargeSharing){
				yVecNew.push_back(yVec.at(2));
				yVecNew.push_back(yVec.at(3));
				pair.push_back(xVec);
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(1);
				indexThree = GetStripIndex(1,(50+yVec.at(2)));
			}
			else {
				pair.push_back(xVec);
				yVecNew.push_back(yVec.at(2));
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(0);
				indexThree = GetStripIndex(1,yVec.at(2));
			}
			stripHitMultipleTimes.push_back(1);
			yVecNew.clear();
			//change energy and resolution for second hit
			//also add element to hit quality, need to do properly later
			float energyOne = energy.at(1).at(indexOne);
			float energyTwo = energy.at(1).at(indexTwo);
			float energyThree = energy.at(1).at(indexThree);
			float eResOne = sig.at(1).at(indexOne);
			float eResTwo = sig.at(1).at(indexTwo);
			float eResThree = sig.at(1).at(indexThree);

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

			hitEnergy.at(i+threeHitsCounter-1) = energyOne;
			hitEnergy.insert(hitEnergy.begin()+i+threeHitsCounter, energyTwo);
			hitEnergy.insert(hitEnergy.begin()+i+threeHitsCounter+1, energyThree);

			energyResolution.at(i+threeHitsCounter-1) = eResOne;
			energyResolution.insert(energyResolution.begin()+i+threeHitsCounter, eResTwo);
			energyResolution.insert(energyResolution.begin()+i+threeHitsCounter+1, eResThree);

			hitQualityFactor.insert(hitQualityFactor.begin()+i+threeHitsCounter, hitQualityFactor.at(i+threeHitsCounter-1));
			hitQualityFactor.insert(hitQualityFactor.begin()+i+threeHitsCounter+1, hitQualityFactor.at(i+threeHitsCounter-1));	
		}
		else if (!xTwoHits && !yTwoHits && !xThreeHits && !yThreeHits){
			pair.push_back(xVec);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			stripHitMultipleTimes.push_back(0);
			if (!xChargeSharing && !yChargeSharing){
				chargeSharing.push_back(0);
			}
			else { chargeSharing.push_back(1); }
		}
    
/*    cout << "pair: "  << endl;
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
  
//  cout << decodedFinalPairs.size() << endl;
/*	cout << decodedFinalPairs.at(0).at(1).size() << endl;
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

/*	if (counter < nHitsOrig.at(0) + nHitsOrig.at(1)){
		PrintXYStripsHit();
		cout << "final pairs: " << endl;
		for (int i=0; i<finalPairs.size(); i++){
			cout << finalPairs.at(i).at(0) << '\t' << finalPairs.at(i).at(1) << endl;
		}
		dummy_func();
	}
*/  
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
    cout << stripsHit.at(0).at(i) << '\t' << energy.at(0).at(i);
		cout << '\t' << sig.at(0).at(i) << endl;
  }
 
	cout << "total X energy: " << '\t';
	float xE = 0;
	for (int i=0; i<nHitsOrig.at(0); i++){
		xE += energy.at(0).at(i);
	}
 	cout << xE << endl;


  cout << "--------------------------" << endl << "Printing yStripsHit...." << endl;
  for(int i=0; i<nHits.at(1); i++){
    cout << stripsHit.at(1).at(i) << '\t' << energy.at(1).at(i);
		cout << '\t' << sig.at(1).at(i) << endl;
  }

	cout << "total Y energy: " << '\t';
	float yE = 0;
	for (int i=0; i<nHitsOrig.at(1); i++){
		yE += energy.at(1).at(i);
	}
	cout << yE << endl;  

};

void MNCTModuleStripPairingGreedy_b::PrintXYStripsHitOrig(){
  
//  cout << "stripsHit.size(): " << stripsHit.size() << endl;
 
	if (nHitsOrig.size() < 2){
		return;
	}
 
  cout << "--------------------------" << endl << "Printing xStripsHit...." << endl;
  for(int i=0; i<nHitsOrig.at(0); i++){
    cout << stripsHit.at(0).at(i) << '\t' << energy.at(0).at(i);
		cout << '\t' << sig.at(0).at(i) << endl;
  }
 
	cout << "total X energy: " << '\t';
	float xE = 0;
	for (int i=0; i<nHitsOrig.at(0); i++){
		xE += energy.at(0).at(i);
	}
 	cout << xE << endl;


  cout << "--------------------------" << endl << "Printing yStripsHit...." << endl;
  for(int i=0; i<nHitsOrig.at(1); i++){
    cout << stripsHit.at(1).at(i) << '\t' << energy.at(1).at(i);
		cout << '\t' << sig.at(1).at(i) << endl;
  }

	cout << "total Y energy: " << '\t';
	float yE = 0;
	for (int i=0; i<nHitsOrig.at(1); i++){
		yE += energy.at(1).at(i);
	}
	cout << yE << endl;  

};

void MNCTModuleStripPairingGreedy_b::PrintFinalPairs(){

	cout << "----------------------" << endl << "Printing final pairs" << endl;

	for (unsigned int p=0; p<finalPairs.size(); p++){
		for (int axis=0; axis<2; axis++){
			cout << finalPairs.at(p).at(axis) << '\t';
		}
		cout << endl;
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

int MNCTModuleStripPairingGreedy_b::GetStripIndex(int axis, int stripID){

	int index = -1;

	for (int i=0; i<nHits.at(axis); i++){
		if (stripsHit.at(axis).at(i) == stripID){
			index = i;
		}
	}

	return index;
};

void MNCTModuleStripPairingGreedy_b::dummy_func(void){

	return;
}
