/*
 * MModuleStripPairingGreedy.cxx
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
// MModuleStripPairingGreedy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleStripPairingGreedy.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
//#include "MMath.h"
#include "MGUIOptionsStripPairing.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleStripPairingGreedy)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleStripPairingGreedy::MModuleStripPairingGreedy() : MModule()
{
  // Construct an instance of MModuleStripPairingGreedy_a
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Strip pairing - Clio's \"Greedy\" version";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "StripPairingGreedy_b";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_TACcut);
  //AddPreceedingModuleType(MAssembly::c_CrosstalkCorrection);
  
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
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = true;

  m_MagicNumberA = 65; //38;
  m_MagicNumberB = 100; //50;
  m_MagicNumberC = 200; //100;
  m_MagicNumberD = 5000;
  m_MagicNumberE = 10000;

}


////////////////////////////////////////////////////////////////////////////////


MModuleStripPairingGreedy::~MModuleStripPairingGreedy()
{
  // Delete this instance of MModuleStripPairingGreedy
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingGreedy::CreateExpos()
{
  // Create all expos
  
  if (HasExpos() == true) return;
  
  // Set the histogram display
  m_ExpoStripPairing = new MGUIExpoStripPairing(this);
  m_ExpoStripPairing->SetEnergiesHistogramParameters(1500, 0, 1500);
  m_Expos.push_back(m_ExpoStripPairing);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleStripPairingGreedy::Initialize()
{
  // Initialize the module 
  
  Eth = 30;
  
  // Add all initializations which are global to all events
  // and have member variables here
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////

//main data analysis routine, which updates the event to a new level
bool MModuleStripPairingGreedy::AnalyzeEvent(MReadOutAssembly* Event){

//	usleep(100);

  const int nDetectors = 12;

	//bool newAlg;
	//if (m_Mode == 0) { newAlg = true; }
	//else { newAlg = false; }

	vector<vector<int> > pairs_temp1, pairs_temp2, pairs_temp3;
	vector<int> xmult_temp1, xmult_temp2, xmult_temp3;
	vector<int> ymult_temp1, ymult_temp2, ymult_temp3;
	vector<int> share_temp1, share_temp2, share_temp3;
	float firstChiSq,secondChiSq,thirdChiSq,fourthChiSq;

	//to see if hits with multiple strips are well PAIRED
	//need to look at Chi-Sq, not final hit n and p energies
	float chi_sq[nDetectors];

	//to keep track of what flags to give bad hits
	int notEnoughStrips[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

  for (int detector = 0; detector < nDetectors; detector++){
		firstChiSq = secondChiSq = thirdChiSq = fourthChiSq = -1;

    int doAnalysis = GetEventInfo(Event, detector);

    if (doAnalysis == 1){
	    InitializeKillMatrices();
	    CheckForAdjacentStrips();
//			AddMultipleHits(0);
//			AddMultipleHits(1);
			CheckForBadCombinations();
			firstChiSq = FindFinalPairs();

			chi_sq[detector] = firstChiSq;

			if (firstChiSq > 25){
				pairs_temp1 = finalPairs;
				xmult_temp1 = xStripHitMultipleTimes;
				ymult_temp1 = yStripHitMultipleTimes;
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
					xmult_temp2 = xStripHitMultipleTimes;
					ymult_temp2 = yStripHitMultipleTimes;
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
						xmult_temp3 = xStripHitMultipleTimes;
						ymult_temp3 = yStripHitMultipleTimes;
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
						if (firstChiSq<=secondChiSq && firstChiSq<=thirdChiSq && firstChiSq<=fourthChiSq){
							finalPairs = pairs_temp1;
							xStripHitMultipleTimes = xmult_temp1;
							yStripHitMultipleTimes = ymult_temp1;
							chargeSharing = share_temp1;
						}
						else if (secondChiSq<=firstChiSq && secondChiSq<=thirdChiSq && secondChiSq<=fourthChiSq){
							finalPairs = pairs_temp2;
							xStripHitMultipleTimes = xmult_temp2;
							yStripHitMultipleTimes = ymult_temp2;
							chargeSharing = share_temp2;
						}
						else if (thirdChiSq<=firstChiSq && thirdChiSq<=secondChiSq && thirdChiSq<=fourthChiSq){
							finalPairs = pairs_temp3;
							xStripHitMultipleTimes = xmult_temp3;
							yStripHitMultipleTimes = ymult_temp3;
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

					if (firstChiSq<=secondChiSq && firstChiSq<=thirdChiSq){
						finalPairs = pairs_temp1;
						xStripHitMultipleTimes = xmult_temp1;
						yStripHitMultipleTimes = ymult_temp1;
						chargeSharing = share_temp1;
					}
					else if (secondChiSq<=firstChiSq && secondChiSq<=thirdChiSq){
						finalPairs = pairs_temp2;
						xStripHitMultipleTimes = xmult_temp2;
						yStripHitMultipleTimes = ymult_temp2;
						chargeSharing = share_temp2;
					}
				}

				if (firstChiSq<=secondChiSq){
					finalPairs = pairs_temp1;
					xStripHitMultipleTimes = xmult_temp1;
					yStripHitMultipleTimes = ymult_temp1;
					chargeSharing = share_temp1;
				}
			}


//			PrintFinalPairs();

     	CalculateDetectorQuality();
//			if (Event->GetID() == 33){ PrintFinalPairs(); }
			WriteHits(Event, detector);

/*			if (thirdChiSq != -1){
				cout << "CHISQ(2): " << chi_sq[detector] << endl;
				PrintXYStripsHitOrig();
				PrintFinalPairs();
				dummy_func();
			}
*/		}
    else if (doAnalysis == -1) {
	    detectorQualityFactors.push_back(0);
			notEnoughStrips[detector] = 1;
	  }
		else if (doAnalysis == -2) {
			detectorQualityFactors.push_back(0);
			notEnoughStrips[detector] = 2;
		}
		else if (doAnalysis == -3) {
			detectorQualityFactors.push_back(0);
			notEnoughStrips[detector] = 3;
		}
	}
	CalculateEventQuality(Event, nDetectors);
	detectorQualityFactors.clear();

	//flag hits without enough strips
	for (int det=0; det<12; det++){
		if (notEnoughStrips[det] == 1 || notEnoughStrips[det] == 2){
			Event->SetStripPairingIncomplete(true,"bad number of strip hits");
			break;
		}
	}


	for (unsigned int h = 0; h < Event->GetNHits(); h++){
		if (Event->GetHit(h)->GetStripHitMultipleTimesX() == true || Event->GetHit(h)->GetStripHitMultipleTimesY() == true){
			Event->SetStripPairingIncomplete(true,"multiple hits per strip");
		}
/*
		if (Event->GetHit(h)->GetChargeSharing() == true){
			Event->SetStripPairingIncomplete(true,"charge sharing");
		}
*/
/*			int detID = Event->GetHit(h)->GetStripHit(0)->GetDetectorID();
			if (chi_sq[detID] <= 25){
				Event->SetStripPairingIncomplete(true,"good pairing, multiple hits per strip");
			}
			else {
				Event->SetStripPairingIncomplete(true,"bad pairing, multiple hits per strip");
			}
*/		
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
      if (Event->GetHit(h)->GetStripHit(s)->IsLowVoltageStrip() == true) {
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
			if (Event->GetHit(h)->GetStripHitMultipleTimesX() == false && Event->GetHit(h)->GetStripHitMultipleTimesY() == false) {
        if (HasExpos() == true) {
          m_ExpoStripPairing->AddEnergies(pEnergy, nEnergy);
        }
			}
    }
    
    pUncertainty = sqrt(pUncertainty);
    nUncertainty = sqrt(nUncertainty);
    
    double Difference = fabs(pEnergy - nEnergy);
/*		if (Difference > m_MagicNumberC){
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
			if (Event->GetHit(h)->GetStripHitMultipleTimesX() == false && Event->GetHit(h)->GetStripHitMultipleTimesY() == false){
		  	Event->SetStripPairingIncomplete(true, "bad pairing");
			}
/*			if (nHits[0] != 0 && nHits[1] != 0){
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

/*
	if (Event->GetNHits()==0 && Event->IsStripPairingIncomplete() == false){
		for (unsigned int sh=0; sh<Event->GetNStripHits(); sh++){
			MStripHit* striphit = Event->GetStripHit(sh);
			cout << striphit->GetDetectorID() << '\t';
			cout << striphit->GetStripID() << '\t';
			cout << striphit->IsLowVoltageStrip() << '\t';
			cout << striphit->GetEnergy() << endl;
		}
		dummy_func();
	}
*/

  if (m_StripPairingFailed != "") {
    Event->SetStripPairingIncomplete(true, m_StripPairingFailed);
  }

  Event->SetAnalysisProgress(MAssembly::c_StripPairing);
  /*
  if(Event->GetNHits() == 0){
	  cout << "++++" << endl;
	  for(int z = 0; z < Event->GetNStripHits(); ++z){
		  Event->GetStripHit(z)->StreamDat(cout);
	  }
	  cout << "----" << endl;
  }
  */
  
  return true;
};

//this function takes the MReadOutAssembly and get all the info from it.
int MModuleStripPairingGreedy::GetEventInfo(MReadOutAssembly* Event, int detector) {
  
  ClearMembers();  
  
  //clear nHits (from the previous detector)
  int n_x = 0;
  int n_y = 0;
  
  //Find the number of hits per side for this detector
  for (unsigned int i = 0; i < Event->GetNStripHits(); i++){
    if (detector == Event->GetStripHit(i)->GetDetectorID()){
      if (Event->GetStripHit(i)->IsLowVoltageStrip() == true){
        n_x += 1;
      }
      else {
        n_y += 1;
      }
      
    }
  }
 
	//no strip hits in detector: different return value
	// (don't want to flag as bad if there weren't any strip hits...)
	if (n_x == 0 && n_y == 0){ return -3; }
 
  
  
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
        if (Event->GetStripHit(i)->IsLowVoltageStrip() == true){
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

    for (unsigned int i=0; i<Event->GetNStripHits(); i++){
      if (detector == Event->GetStripHit(i)->GetDetectorID()){
        if (Event->GetStripHit(i)->IsLowVoltageStrip() == true){
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
    n_x = stripsHit[0].size();
    n_y = stripsHit[1].size();
    nHits.push_back(n_x);
    nHits.push_back(n_y);
//    nHitsOrig = nHits;
  	nHitsOrig.push_back(n_x);
		nHitsOrig.push_back(n_y);  
    
		//check strip numbers again: some strips could have bad energy or resolution
		// causing there not to be enough strip hits
		if(!( (n_x > 0) && (n_y > 0) && (fabs(n_x - n_y) < 5) && (n_x < 8) && (n_y < 8))) {
			return -2;
		}
    
    
    //sort strips (and energy and resolution) in numerical order by strip ID
    //do this for each axis
    for (int axis=0; axis<2; axis++){
      for (int i=0; i<nHits[axis]-1; i++){
				int min=i;
        for (int j=i+1; j<nHits[axis]; j++){
          if (stripsHit[axis][j] < stripsHit[axis].at(min)){
            min = j;
					}
				}
				if (min != i){
          int tempStrip = stripsHit[axis][i];
          stripsHit[axis][i] = stripsHit[axis].at(min);
          stripsHit[axis].at(min) = tempStrip;
            
          float tempEnergy = energy[axis][i];
          energy[axis][i] = energy[axis].at(min);
          energy[axis].at(min) = tempEnergy;
            
          float tempSig = sig[axis][i];
          sig[axis][i] = sig[axis].at(min);
          sig[axis].at(min) = tempSig;
        }
      }
    }
//		if (Event->IsStripPairingIncomplete()){
//			PrintXYStripsHit();
//			dummy_func();
//		}
		return 1;
  }

  else {
    nHits.push_back(0);
    nHits.push_back(0);
    return -1;
  }
  
};      

void MModuleStripPairingGreedy::CalculateDetectorQuality(){
  
  float detectorQuality = 0;
  int counter = 0;
  
  for (unsigned int i=0; i<hitQualityFactor.size(); i++){
    detectorQuality = detectorQuality + hitQualityFactor[i];
    counter += 1;
  }
  
  detectorQuality = detectorQuality / counter;
  
  detectorQualityFactors.push_back(detectorQuality);
  
};

void MModuleStripPairingGreedy::CalculateEventQuality(MReadOutAssembly* Event, int nDetectors){
  
  float eventQuality = 0;
  int counter = 0;
  //	cout << detectorQualityFactors[2] << endl;
  
  for (int i=0; i<nDetectors; i++){
    eventQuality = eventQuality + detectorQualityFactors[i];
    counter += 1;
  }
  
  Event->SetEventQuality(eventQuality);
  
};

//output stuff
void MModuleStripPairingGreedy::WriteHits(MReadOutAssembly* Event, int detector){


	vector<vector<vector<int> > > decodedFinalPairs = DecodeFinalPairs();

  if (m_StripPairingFailed != "") return;


	//pair indexes over the pairs (hits) -- for each pair, there is a new MHit
	//strip indexes over the strips in each pair on the x or y side
	//n indexes over the list of strip hits in the MReadOutAssembly

	//this loop iterates over the pairs, and for each pair creates a new MHit
	//then, it iterates over each strip on the x side of that pair
	//it then iterates over each stripHit in the MReadOutAssembly, checks that the detector is right,
	//	that it's an x hit, and if it matches the strip, it is added to the MHit
	//the same is done in the second sub-loop for the y side

	//hit quality, energy, energy resolution only needs to be added once,
	// (it's the same for x and y), so it is done on the x side

	bool addHit = false;

	for (unsigned int pair=0; pair<decodedFinalPairs.size(); pair++) {
    addHit = false;
		MHit* Hit = new MHit();
		//x side
		for (unsigned int strip=0; strip<decodedFinalPairs.at(pair)[0].size(); strip++){
			for (unsigned int n = 0; n<Event->GetNStripHits(); n++){
				if (detector == Event->GetStripHit(n)->GetDetectorID()){
					if (Event->GetStripHit(n)->IsLowVoltageStrip() == true){
						if (Event->GetStripHit(n)->GetStripID() == decodedFinalPairs.at(pair)[0].at(strip)){
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
		for (unsigned int strip=0; strip<decodedFinalPairs.at(pair)[1].size(); strip++){
			for (unsigned int n=0; n<Event->GetNStripHits(); n++){
				if (detector == Event->GetStripHit(n)->GetDetectorID()){
					if (Event->GetStripHit(n)->IsLowVoltageStrip() == false){
						if (Event->GetStripHit(n)->GetStripID() == decodedFinalPairs.at(pair)[1].at(strip)){
							Hit->AddStripHit(Event->GetStripHit(n));
						}
					}
				}
			}
		}
		if (addHit) {
			Event->AddHit(Hit);
			if( Hit->GetNStripHits() == 0 ){
				cout << "STRIP PAIRING BAD HIT" << endl;
			}

      if (xStripHitMultipleTimes.at(pair) == 1){
        Hit->SetStripHitMultipleTimesX(true);
  		}
      else {
        Hit->SetStripHitMultipleTimesX(false);
      }
			if (yStripHitMultipleTimes.at(pair) == 1){
				Hit->SetStripHitMultipleTimesY(true);
			}
			else { Hit->SetStripHitMultipleTimesY(false); }

      if (chargeSharing.at(pair) == 1){
        Hit->SetChargeSharing(true);
      }
      else {
        Hit->SetChargeSharing(false);
      }
    } else {
      delete Hit; 
    }


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
				if (Hit->GetStripHit(s)->IsLowVoltageStrip() == true){
				xhit_strip[nXhits] = Hit->GetStripHit(s)->GetStripID();
				xhit_energy[nXhits] = Hit->GetStripHit(s)->GetEnergy();
				nXhits = nXhits + 1;
				} else if (Hit->GetStripHit(s)->IsLowVoltageStrip() == false){
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

	//CCS (8/8/19): choosing the hit energy -- taken from the crosstalk correction module
  for (unsigned int sh = 0; sh < Event->GetNHits(); sh++) {
    double Energy = 0;
    double Resolution = 0.0;
    
    // Handle what happens if strips have been hit multiple times:
    
    // If both have been hit multiple times, we don't do anything - we keep the result from the strip pairing
    if (Event->GetHit(sh)->GetStripHitMultipleTimesX() == true && 
        Event->GetHit(sh)->GetStripHitMultipleTimesY() == true) {
      continue;
    }
    // If y strip was hit multiple times, need to use the x energy
    else if (Event->GetHit(sh)->GetStripHitMultipleTimesY() == true) {
      for (unsigned int sh_i = 0; sh_i < Event->GetHit(sh)->GetNStripHits(); sh_i++){
        if (Event->GetHit(sh)->GetStripHit(sh_i)->IsLowVoltageStrip() == true){
          Energy += Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergy();
          Resolution += pow(Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergyResolution(), 2);
        }
      }
      Resolution = sqrt(Resolution);
      //cout<<"Using X (my): "<<Energy<<endl;
    
    } 
    // If x strip was hit multiple times, need to use the y energy
    else if (Event->GetHit(sh)->GetStripHitMultipleTimesX() == true) {
      for (unsigned int sh_i = 0; sh_i < Event->GetHit(sh)->GetNStripHits(); sh_i++){
        if (Event->GetHit(sh)->GetStripHit(sh_i)->IsLowVoltageStrip() == false){
          Energy += Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergy();
          Resolution += pow(Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergyResolution(), 2);
        }
      }
      Resolution = sqrt(Resolution);
      //cout<<"Using Y (mx): "<<Energy<<endl;
    } 
    // Best case: Can do all corrections:
    else {
      double EnergyX = 0.0;
      double SigmaXSquared = 0.0;
      double EnergyY = 0.0;
      double SigmaYSquared = 0.0;
      for (unsigned int sh_i = 0; sh_i < Event->GetHit(sh)->GetNStripHits(); sh_i++) {  
        //for now, just define the hit energy as the sum of the y strip hits. This could later be modifided to take an average of the two sides.
        
        if (Event->GetHit(sh)->GetStripHit(sh_i)->IsLowVoltageStrip() == false) {
          EnergyY += Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergy();
          SigmaYSquared += pow(Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergyResolution(), 2);
        } else {
          EnergyX += Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergy();
          SigmaXSquared += pow(Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergyResolution(), 2);
        }
      }
      
      if (SigmaXSquared > 0 && SigmaYSquared > 0) {
        //
        double EnergyDiff = fabs(EnergyX - EnergyY);
        double MinSigmaSquared = min(SigmaXSquared, SigmaYSquared);
        
        const double DecisionUsingHigherMeasurement = 4.0;
        
        if (EnergyDiff > DecisionUsingHigherMeasurement*sqrt(MinSigmaSquared)) {
          if (EnergyX > EnergyY) {
            Energy = EnergyX;
            Resolution = sqrt(SigmaXSquared);
            //cout<<"Using X (Y too small): "<<EnergyX<<endl;
          } else {
            Energy = EnergyY;
            Resolution = sqrt(SigmaYSquared);
            //cout<<"Using Y (X too small): "<<EnergyY<<endl;            
          }
        } else {
          Energy = (EnergyX/SigmaXSquared + EnergyY/SigmaYSquared) / (1.0/SigmaXSquared + 1.0/SigmaYSquared);
          Resolution = sqrt( 1.0 / (1.0/SigmaXSquared + 1.0/SigmaYSquared) );
          //cout<<"Corrected: "<<Energy<<" vs. "<<EnergyX<<":"<<EnergyY<<endl;
        }
      } else if (SigmaXSquared > 0) {
        Energy = EnergyX;
        Resolution = sqrt(SigmaXSquared);
        //cout<<"Using X: "<<EnergyX<<endl;
      } else if (SigmaYSquared > 0) {
        Energy = EnergyY;
        Resolution = sqrt(SigmaYSquared);
        //cout<<"Using Y: "<<EnergyY<<endl;
      }
      /*
      ofstream out;
      out.open("energy.txt", ios::app);
      out<<EnergyX<<" "<<EnergyY<<endl;
      out.close();
      */
    }
    Event->GetHit(sh)->SetEnergy(Energy);
    Event->GetHit(sh)->SetEnergyResolution(Resolution);
  }

};

//clears members in between each detector
void MModuleStripPairingGreedy::ClearMembers(){
  
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

  
  hitQualityFactor.clear();
  energyResolution.clear();
  hitEnergy.clear();
  //	detectorQualityFactors.clear();
  
  xStripHitMultipleTimes.clear();
	yStripHitMultipleTimes.clear();
  chargeSharing.clear();
};


//calculate average sigma (delta E)
//avg sigma = sigma(x) + sigma(y)
//sigma(x) = sqrt(sigma(hit 1)^2 + sigma(hit 2)^2 + ... )
float MModuleStripPairingGreedy::CalculateSigma(){
  
  vector<float> sigTotalSquared, sigTotal;
  
  sigTotalSquared.push_back(0);
  sigTotalSquared.push_back(0);
  
  //find sigma(x)^2 and sigma(y)^2
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHitsOrig[axis]; i++){
      sigTotalSquared[axis] = sigTotalSquared[axis] + sig[axis][i]*sig[axis][i];
    }
  }
  
  //find sigma(x) and sigma(y), store in vector sigTotal
  sigTotal.push_back(sqrt(sigTotalSquared[0]));
  sigTotal.push_back(sqrt(sigTotalSquared[1]));
  
  //find average sigma
  float avgSigma = 0.5*(sigTotal[0]+sigTotal[1]);
  
  //	cout << "avgSigma: " << avgSigma << endl;
  
  return avgSigma;
  
};

//checks size of energy difference between x and y by comparing to the average sigma calculated in CalculateSigma()
//returns true if difference is smaller than 3*(avg sigma), false if difference is larger than 3*(avg sigma)
bool MModuleStripPairingGreedy::CheckInitialEnergyDifference(){
  
  int avgSigma = CalculateSigma();
  
  vector<float> eTotal;
  eTotal.push_back(0);
  eTotal.push_back(0);
  
  //find total x energy and total  y energy
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHitsOrig[axis]; i++){
      eTotal[axis] = eTotal[axis] + energy[axis][i];
    }
  }
  
  //compare energy difference to 3*(avg sigma)	
  if (fabs(eTotal[0]-eTotal[1]) < 3*avgSigma){
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
void MModuleStripPairingGreedy::DetermineOption(bool adjStripsHit){
  
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
//if strip n and n+1 are hit, the "strip number" for the combined strips is m_MagicNumberB+n
//for example, if strips 3 and 4 are hit, 53 is appended to the end of stripsHit
bool MModuleStripPairingGreedy::CheckForAdjacentStrips(){
 
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
    for (int i=0; i<nHits[axis]-1; i++){
      //check for adjacent strips
      if (stripsHit[axis][i]+1 == stripsHit[axis][i+1]){
        adjStrips = true;
        
        counter += 1;
        //label combined strip by adding m_MagicNumberB to the lower strip number
        //append combined strip number to stripsHit
        adjStripLabel = stripsHit[axis][i] + m_MagicNumberB;
        stripsHit[axis].push_back(adjStripLabel);
        
        //add row and column to kill matrix
        ExpandKillMatrix(axis);
        
        //fill kill matrix
        //each element of the kill matrix is the vector sum of the constituent strips
        int k_max = nHits[axis]+counter-1;
        for (int k=0; k<k_max; k++){
          killMatrix[axis][k_max][k] = killMatrix[axis][i][k] + killMatrix[axis][i+1][k];
          killMatrix[axis][k][k_max] = killMatrix[axis][k][i] + killMatrix[axis][k][i+1];
        }
        
        //add the energies of the adjacent strips and append them to the energy vector
        adjStripEnergy = energy[axis][i] + energy[axis][i+1];
        energy[axis].push_back(adjStripEnergy);
        
        //add the errors of the adjacent strips and append them to the error vector
        adjStripSig = sqrt(sig[axis][i]*sig[axis][i]+sig[axis][i+1]*sig[axis][i+1]);
        sig[axis].push_back(adjStripSig);
      }
    }
    //change the number of hits
    nHits[axis] = nHits[axis] + counter;
		nHitsAdj[axis] = nHits[axis] - nHitsOrig[axis];

    counter = 0;
  }

 
  return adjStrips;
  
};

//check if multiple hits need to be considered
//multiple hits need to be considered when there are more hits on one side than the other
bool MModuleStripPairingGreedy::CheckMultipleHits(){
  
  //	DefineEventInfo();
  
  int n_x = nHitsOrig[0];
  int n_y = nHitsOrig[1];
  
  //	cout << "nx: " << nHits[0] << endl;
  //	cout << "ny: " << nHits[1] << endl << endl;
  
  if (n_x == n_y) {return false;}
  
  else if (n_x > n_y) {  //y strips have multiple hits, so want to make pairs from x: ie two x strips for one y strip
    AddMultipleHits(0);
    //		for (int i=0; i<n_x; i++){cout << stripsHit[0][i] << endl;}
    return true;
  }
  
  else {	//x strips have multiple hits, so want to make pairs from y: ie two y strips for one x strip
    AddMultipleHits(1);
    //		for (int i=0; i<n_y; i++){cout << stripsHit[1][i] << endl;}
    return true;
  }
};


//add possibilities of multiple hits to vector of hits
void MModuleStripPairingGreedy::AddMultipleHits(int axis){
  
  //initialize variables
  int x1, x2, pair;
  float pairE, pairSig;
  int nPairs = 0;
  vector<int> pairHitsVec;
  vector<float> pairEnergyVec, pairSigVec;
  
  //goal is to make all possible combinations of strips on one axis
  for (int i=0; i<nHits[axis]-nThreeHitsAdj[axis]; i++){
    for (int j=i+1; j<nHits[axis]-nThreeHitsAdj[axis]; j++){
      x1 = stripsHit[axis][i];
      x2 = stripsHit[axis][j];
      
      //label combinations by multiplying the lower strip number by m_MagicNumberC and adding it to the higher strip number
      //append combined strip to pairHitsVec
      pair = x1*m_MagicNumberC+x2;
      pairHitsVec.push_back(pair);
      
      //count number of pairs
      nPairs+=1;
      //			cout << "nPairs: " << nPairs << endl;
      
      //add row and column to kill matrix
      ExpandKillMatrix(axis);
      //fill kill matrix
      //each element is the vector sum of its constituent strips
      int k_max = nHits[axis]+nPairs-1;
      for (int k=0; k<nHits[axis]+nPairs-1; k++){
        killMatrix[axis][k_max][k] = killMatrix[axis][i][k] + killMatrix[axis][j][k];
        killMatrix[axis][k][k_max] = killMatrix[axis][k][i] + killMatrix[axis][k][j];
      }
      
      //print kill matrix
      /*
      for (int m=0; m<nHits[axis]+nPairs-1; m++){
        for (int n=0; n<nHits[axis]+nPairs-1; n++){
          cout << killMatrix[axis][m][n] << '\t';
        }
        cout << endl;
      }
      */
      //add to energy and significance vectors
      pairE = energy[axis][i]+energy[axis][j];
      pairEnergyVec.push_back(pairE);
      
      pairSig = sqrt(sig[axis][i]*sig[axis][i]+sig[axis][j]*sig[axis][j]);
      pairSigVec.push_back(pairSig);
      
    }
  }
  /*
  cout << "-----checking order-----" << endl;
  for (int i=0; i<nPairs; i++){
    cout << pairHitsVec[i] << endl;
  }
  */
  
  //add strip combinations info to stripsHit, energy, and sig
  for (int i=0; i<nPairs; i++){
    stripsHit[axis].push_back(pairHitsVec[i]);
    energy[axis].push_back(pairEnergyVec[i]);
    sig[axis].push_back(pairSigVec[i]);
  }
  
  //change number of hits to account for multiple strips being hit
  nHits[axis] = nHits[axis] + nPairs;
  
  //print final kill matrix
  //	cout << "printing final kill matrix..." << endl;
  //	for (int i=0; i<nHits[axis]; i++){
  //		for (int j=0; j<nHits[axis]; j++){
  //			cout << killMatrix[axis][i][j] << '\t';
  //		}
  //		cout << endl;
  //	}
  
  
  
  //	cout << "--------------------//------------------------" << endl;
  //	for(int i=0; i<xStripsHit.size(); i++){cout << xStripsHit[i] << endl;}
  
  
};

//add possibility of charge sharing on 3 strips to vector of hits
void MModuleStripPairingGreedy::ChargeSharingThreeStrips(int axis){

	int counter = 0;

	//check pairs of adjacent strips
	for (int i=nHitsOrig[axis]; i<nHitsOrig[axis]+nHitsAdj[axis]-1; i++){
		if ( fabs(stripsHit[axis][i+1] - stripsHit[axis][i]) == 1){
			int sID_one = stripsHit[axis][i]-m_MagicNumberB;

			//get strip IDs to check energies
			int indOne = GetStripIndex(axis,sID_one);

			//check energies
			float eOne = energy[axis][indOne];
			float eTwo = energy[axis][indOne+1];
			float eThree = energy[axis][indOne+2];

			//if energy makes sense, get new "strip ID"
			if (eTwo > eOne && eTwo > eThree){
				int newStripID = m_MagicNumberD+sID_one;
				stripsHit[axis].push_back(newStripID);
				float newEnergy = eOne+eTwo+eThree;
				energy[axis].push_back(newEnergy);

				//get energy resolution
				float eResOne = sig[axis][indOne];
				float eResTwo = sig[axis][indOne+1];
				float eResThree = sig[axis][indOne+2];
				float newERes = sqrt(eResOne*eResOne + eResTwo*eResTwo + eResThree*eResThree);
				sig[axis].push_back(newERes);

				counter += 1;

				//add things to kill matrix
				ExpandKillMatrix(axis);

//				cout << "killMatrix[0].size(): " << killMatrix[0].size() << endl;

	      //each element is the vector sum of its constituent strips
        int k_max = nHits[axis]+counter-1;
	      for (int k=0; k<k_max; k++){
	        killMatrix[axis][k_max][k] = killMatrix[axis][indOne][k] + killMatrix[axis][indOne+1][k] + killMatrix[axis][indOne+2][k];
	        killMatrix[axis][k][k_max] = killMatrix[axis][k][indOne] + killMatrix[axis][k][indOne+1] + killMatrix[axis][k][indOne+2];
	      }

			}
		}
	}

	nHits[axis] = nHits[axis] + counter;
	nThreeHitsAdj[axis] = nThreeHitsAdj[axis] + counter;

};


//add possibilities of three hits to vector of hits
void MModuleStripPairingGreedy::AddThreeHits(int axis){
  
  //initialize variables
  int x1, x2, pair;
  float pairE, pairSig;
  int nPairs = 0;
  vector<int> pairHitsVec;
  vector<float> pairEnergyVec, pairSigVec;
  
  //goal is to make all possible combinations of strips on one axis
  for (int i=0; i<nHitsOrig[axis]+nHitsAdj[axis]; i++){
    for (int j=nHitsOrig[axis]+nHitsAdj[axis]+nThreeHitsAdj[axis]; j<nHits[axis]; j++){
      x1 = stripsHit[axis][i];
      x2 = stripsHit[axis][j];
      
      //label combinations by multiplying the lower strip number by m_MagicNumberC and adding it to the higher strip number
      //append combined strip to pairHitsVec
      pair = x1*m_MagicNumberE+x2;
      pairHitsVec.push_back(pair);
      
      //count number of pairs
      nPairs+=1;
      //			cout << "nPairs: " << nPairs << endl;
      
      //add row and column to kill matrix
      ExpandKillMatrix(axis);
      //fill kill matrix
      //each element is the vector sum of its constituent strips
      /*
      for (int k=0; k<nHits[axis]+nPairs-1; k++){
        killMatrix[axis].at(nHits[axis]+nPairs-1)[k] = killMatrix[axis][i][k] + killMatrix[axis][j][k];
        killMatrix[axis][k].at(nHits[axis]+nPairs-1) = killMatrix[axis][k][i] + killMatrix[axis][k][j];
      }
      */
      int k_max = nHits[axis] + nPairs - 1;
      vector<vector<int> >& SubMatrix = killMatrix[axis];
      for (int k = 0; k < k_max; ++k){
        SubMatrix[k_max][k] = SubMatrix[i][k] + SubMatrix[j][k];
        SubMatrix[k][k_max] = SubMatrix[k][i] + SubMatrix[k][j];
      }
      
      //print kill matrix
      /*
      for (int m=0; m<nHits[axis]+nPairs-1; m++){
        for (int n=0; n<nHits[axis]+nPairs-1; n++){
          cout << killMatrix[axis][m][n] << '\t';
        }
        cout << endl;
      }
      */
      //add to energy and significance vectors
      pairE = energy[axis][i]+energy[axis][j];
      pairEnergyVec.push_back(pairE);
      
      pairSig = sqrt(sig[axis][i]*sig[axis][i]+sig[axis][j]*sig[axis][j]);
      pairSigVec.push_back(pairSig);
      
    }
  }
  /*
  cout << "-----checking order-----" << endl;
  for (int i=0; i<nPairs; i++){
    cout << pairHitsVec[i] << endl;
  }
  */
  
  //add strip combinations info to stripsHit, energy, and sig
  for (int i=0; i<nPairs; i++){
    stripsHit[axis].push_back(pairHitsVec[i]);
    energy[axis].push_back(pairEnergyVec[i]);
    sig[axis].push_back(pairSigVec[i]);
  }
  
  //change number of hits to account for multiple strips being hit
  nHits[axis] = nHits[axis] + nPairs;
  
  //print final kill matrix
  //	cout << "printing final kill matrix..." << endl;
  //	for (int i=0; i<nHits[axis]; i++){
  //		for (int j=0; j<nHits[axis]; j++){
  //			cout << killMatrix[axis][i][j] << '\t';
  //		}
  //		cout << endl;
  //	}
  
  
  
  //	cout << "--------------------//------------------------" << endl;
  //	for(int i=0; i<xStripsHit.size(); i++){cout << xStripsHit[i] << endl;}
  
  
};



//initialize badCombinations vector and fill with 0s
void MModuleStripPairingGreedy::InitializeBadCombinations(){
  
  badCombinations.clear();
  badCombinations.push_back(vector<int>(nHits[0], 0));
  badCombinations.push_back(vector<int>(nHits[1], 0));
  
  /*
  vector<int> badCombX, badCombY;
  for (int i=0; i<nHits[0]; i++){
    badCombX.push_back(0);
  }
  for (int i=0; i<nHits[1]; i++){
    badCombY.push_back(0);
  }
  badCombinations.push_back(badCombX);
  badCombinations.push_back(badCombY);
  */
};

//check for combinations that take a strip into account more than once
//for example, 251 is the combination of strip 2 and charge sharing between strips 1 and 2
void MModuleStripPairingGreedy::CheckForBadCombinations(){
  
  InitializeBadCombinations();
  
  //from the way the kill matrix is defined, if killMatrix[i][j]>1, than strip i is a bad combination
  for (int axis=0; axis<2; axis++){
//		cout << axis << ": ";
    for (int i=0; i<nHits[axis]; i++){
      for (int j=0; j<nHitsOrig[axis]; j++){
        //add bad combinations to badCombinations vector
        if (killMatrix[axis][i][j] > 1){
          badCombinations[axis][i] = 1;
        }
      }
//     			cout << badCombinations[axis][i] << "  ";

    }
//    		cout << endl;
  }

};

//calculates weight for one x-y pair
float MModuleStripPairingGreedy::CalculateWeight(int xIndex, int yIndex){
  
  float weight;
  float xE = energy[0][xIndex];
  float yE = energy[1][yIndex];
  float xS = sig[0][xIndex];
  float yS = sig[1][yIndex];
  
  weight = (xE-yE)*(xE-yE)/((xS*xS)+(yS*yS));
  // cout << "weight: x=" << xIndex << ", y=" << yIndex << ": "<< weight << endl;
  return weight;
  
};

//makes a matrix of the correct size and fills it with 0s
void MModuleStripPairingGreedy::InitializeWeightMatrix(){
  
  weightMatrix.clear();
  weightMatrix.resize(nHits[0], vector<float>(nHits[1], 0));
  
  /*
  vector<float> col;
  
  for (int i=0; i<nHits[1]; i++){
    col.push_back(0);
  }
  for (int j=0; j<nHits[0]; j++){
    weightMatrix.push_back(col);
  }
  */
};

//initializes kill matrices
//makes matrices the right size, fills them with 0s, stores them in killMatrix vector
void MModuleStripPairingGreedy::InitializeKillMatrices(){
  
  killMatrix.clear();
  killMatrix.push_back(vector<vector<int>>(nHits[0], vector<int>(nHits[0], 0)));
  killMatrix.push_back(vector<vector<int>>(nHits[1], vector<int>(nHits[1], 0)));
  
  /*
  vector<int> col_x, col_y;
  vector<vector<int> > kill_x, kill_y;
  
  for (int i=0; i<nHits[0]; i++){
    col_x.push_back(0);
  }
  for (int i=0; i<nHits[1]; i++){
    col_y.push_back(0);
  }
  for (int i=0; i<nHits[0]; i++){
    kill_x.push_back(col_x);
  }
  for (int i=0; i<nHits[1]; i++){
    kill_y.push_back(col_y);
  }
  
  killMatrix.push_back(kill_x);
  killMatrix.push_back(kill_y);
  */
  
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHits[axis]; i++){
      killMatrix[axis][i][i] = 1;
    }
  }
  
};

//add row and column of 0s to kill matrix
void MModuleStripPairingGreedy::ExpandKillMatrix(int axis){
  
  int size = killMatrix[axis].size();

  //make a column that's 1 bigger than the size of the kill matrix
  //add the column to the kill matrix
  killMatrix[axis].push_back(vector<int>(size+1, 0));
  
  //add a 0 to every row?
  for (int j=0; j<size; j++){
    killMatrix[axis][j].push_back(0);
  }
  
  //set new diagonal term equal to 1
  killMatrix[axis][size][size] = 1;
  
  // cout << kill_x.size() << endl;
  // for (int i=0; i<kill_x.size(); i++){
  //   cout << "size: " << kill_x[i].size() << endl;
  // }
  
  //	for (int i=0; i<kill_x.size(); i++){
  //    for (int j=0; j<kill_x.size(); j++){
  //      cout << kill_x[i][j] << '\t';
  //    }
  //    cout << endl;
  //  }
  //
};

//calculates weight matrix
//fills weight matrtix of 0s with the weights of each x-y pair
void MModuleStripPairingGreedy::CalculateWeightMatrix(){
  
  int n_x = nHits[0];
  int n_y = nHits[1];
  int n_xOrig = nHitsOrig[0];
  int n_yOrig = nHitsOrig[1];
  
  InitializeWeightMatrix();
  float weight;
 
//	PrintXYStripsHit();
 
  //for each x-y pair, fill in weight matrix with the weight of the pair
  //if it's a bad pair, set weight to -1
  for (int i=0; i<n_x; i++){
    for (int j=0; j<n_y; j++){
      if (badCombinations[0][i] == 0 && badCombinations[1][j] == 0){
//        if (i<n_xOrig || j<n_yOrig){
//				if (i<n_x-nHitsAdj[0] || j<n_y-nHitsAdj[1]){ 
				if (i<n_xOrig + nHitsAdj[0] || j<n_yOrig + nHitsAdj[1]){
          weight = CalculateWeight(i,j);
          weightMatrix[i][j] = weight;
        }
        else {weightMatrix[i][j] = -1;}
      }
      else {weightMatrix[i][j] = -1;}
    }
  }
  
  //	cout << "printing weight matrix: first time" << endl;
  //	PrintWeightMatrix();
  
};

//go through all elements of weight matrix and find minimum weight
vector<int> MModuleStripPairingGreedy::FindMinWeight(){
 
 	int n_x = nHits[0];
  int n_y = nHits[1];
  
  //set min_weight equal to the maximum float value that can possibly exist
  float min_weight = numeric_limits<float>::max();
  float weight = 0;
  int xIndex = 0, yIndex = 0;
  //for each weight in the weight matrix, if it's lower than min_weight,
  //set min_weight to that weight
  for (int i=0; i<n_x; i++){
    for (int j=0; j<n_y; j++){
      weight = weightMatrix[i][j];
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
  weightMatrix[xIndex][yIndex] = -1;
  
  //	cout << "xIndex: " << xIndex << endl;
  //	cout << "yIndex: " << yIndex << endl;
  
  //return the indices of the minimum weight
  vector<int> pairIndex(2);
  pairIndex[0] = xIndex;
  pairIndex[1] = yIndex;
  return pairIndex;
  
};


//once a pair is chosen (indexed by xIndex, yIndex), prevents program from repeating a strip
void MModuleStripPairingGreedy::ConflictingStrips(int xIndex, int yIndex){
  
  vector<int> indices = { xIndex, yIndex };
  
  //loop sets weight matrix to -1 at any place where a strip could be repeated
  //for example, if pair (x=1, y=2) is chosen, the weight matrix at (x=1, y=anything)
  //or (x=51, y=anything), or (x=105, y=anything), etc, must be set to -1
  //that way, strips don't repeat
  for (int axis=0; axis<2; axis++){
    for (int i=0; i<nHits[axis]; i++){
      //			cout << i << '\t' << killMatrix[axis].at(indices[axis])[i] << endl;
      if (killMatrix[axis][indices[axis]][i] != 0){
        //				cout << "bad x: " << i << '\t' << stripsHit[axis][i] << endl;
        //				cout << killMatrix[axis].at(indices[axis])[i] << endl;
        
        int j_max;
        if (axis==1){j_max = 0;}
        else {j_max = 1;}
        
        // --> time critical
        /* slow version
        for (int j=0; j<nHits.at(j_max); j++){
          if (axis==0){weightMatrix[i][j] = -1;}
          if (axis==1){weightMatrix[j][i] = -1;}
        }
        */
        int max = nHits[j_max];
        for (int j=0; j<max; ++j){
          if (axis==0) {
            weightMatrix[i][j] = -1;
          } else if (axis==1) {
            weightMatrix[j][i] = -1;
          }
        }
        //<-- time critial end
      }
    }
  }
  
  //	cout << "printing weight matrix: second time" << endl;
  //	PrintWeightMatrix();
  
};

//count the number of elements in the weight matrix that are set to -1
int MModuleStripPairingGreedy::CountNegativeElements(){
  
  int counter = 0;
  int i_max = nHits[0];
  for (int i=0; i<i_max; i++){
    int j_max = nHits[1];
    for (int j=0; j<j_max; j++){
      if (weightMatrix[i][j] == -1){
        counter += 1;
      }
    }
  }
  
  return counter;
  
};

//find the list of final pairs by finding the minimum weights
float MModuleStripPairingGreedy::FindFinalPairs(){

  //initialize variables
  int n_x = nHits[0];
  int n_y = nHits[1];

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
//  float eRes;
	float greedyChiSq = 0;

  //	cout << "nElements: " << nElements << endl;
  //find minimum weight while the number of elements that equal -1 in the weight matrix
  //is less than the number of elements in the weight matrix
  do{
//		PrintWeightMatrix();
    //find indices of the minimum weight
    vector<int> indices = FindMinWeight();
    xIndex = indices[0];
    yIndex = indices[1];
    //add the strip numbers at those indices to finalPairs
    finalXStrip = stripsHit[0][xIndex];
    finalYStrip = stripsHit[1][yIndex];
    pairVec.push_back(finalXStrip);
    pairVec.push_back(finalYStrip);
    finalPairs.push_back(pairVec);
    pairVec.clear();

		energyPair.push_back(energy[0][xIndex]);
		energyPair.push_back(energy[1][yIndex]);
		finalPairEnergy.push_back(energyPair);
    energyPair.clear();

		resPair.push_back(sig[0][xIndex]);
		resPair.push_back(sig[1][yIndex]);
		finalPairRes.push_back(resPair);
		resPair.clear();

		greedyChiSq += pow((energy[0][xIndex]-energy[1][yIndex]),2) / (pow(sig[0][xIndex],2) + pow(sig[1][yIndex],2));

    //fill hit quality factor vector
    weight = weightMatrix[xIndex][yIndex];
    hitQualityFactor.push_back(weight);
    
    //fill hit energy resolution vector
    //float eResX = sig[0][xIndex];
    //float eResY = sig[1][yIndex];
    //eRes = (sqrt(2)*eResX*eResY)/(eResX*eResX+eResY*eResY);
    //energyResolution.push_back(eRes);
    
    //fill hit energy vector
    //float eX = energy[0][xIndex];
    //float eY = energy[1][yIndex];
    //double inf = numeric_limits<double>::infinity();
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
    
    float eY = energy[1][yIndex];
    hitEnergy.push_back(eY);
    float eResY = sig[1][yIndex];
    energyResolution.push_back(eResY);
  
    
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
      cout << finalPairs[i][0] << '\t' << finalPairs[i][1] << endl;
    }
		cout << "chi sq: " << greedyChiSq << endl;
  }


	return greedyChiSq;
};

//puts list of final pairs back into strip numbers
//for example, if strip 51 is selected, it gets separated into strips 1 and 2
//if 104 is selected, it gets separated into strips 1 and 4
//if 256 is selected, it gets separated into strip 2 and charge sharing between 6 and 7
vector<vector<vector<int> > > MModuleStripPairingGreedy::DecodeFinalPairs(){

/*
	for (unsigned int pair=0; pair<finalPairs.size(); pair++){
		if (finalPairs.at(pair)[0] > m_MagicNumberC && finalPairs.at(pair)[1] > m_MagicNumberC){
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
    if (finalPairs[i][0] > m_MagicNumberB && finalPairs[i][0] < m_MagicNumberC){
      xVec.push_back(finalPairs[i][0]-m_MagicNumberB);
      xVec.push_back(finalPairs[i][0]+1-m_MagicNumberB);
			xChargeSharing = true;
    }
		//charge sharing between three strips
		else if (finalPairs[i][0] > m_MagicNumberD && finalPairs[i][0] < m_MagicNumberD + m_MagicNumberC){
			xVec.push_back(finalPairs[i][0]-m_MagicNumberD);
			xVec.push_back(finalPairs[i][0]+1-m_MagicNumberD);
			xVec.push_back(finalPairs[i][0]+2-m_MagicNumberD);
			xChargeSharing = true;
		}
		//two hits on x: y strip hit twice
    else if (finalPairs[i][0] > m_MagicNumberC && finalPairs[i][0] < m_MagicNumberE){
			xTwoHits = true;
      if (finalPairs[i][0]-(int)(finalPairs[i][0]/m_MagicNumberC)*m_MagicNumberC > m_MagicNumberB){
				xChargeSharing = true;
        xVec.push_back(finalPairs[i][0]/m_MagicNumberC);
        xVec.push_back(finalPairs[i][0]-(int)(finalPairs[i][0]/m_MagicNumberC)*m_MagicNumberC-m_MagicNumberB);
        xVec.push_back(finalPairs[i][0]-(int)(finalPairs[i][0]/m_MagicNumberC)*m_MagicNumberC+1-m_MagicNumberB);
      }
      else {
        xVec.push_back(finalPairs[i][0]/m_MagicNumberC);
        xVec.push_back(finalPairs[i][0] - (int)(finalPairs[i][0]/m_MagicNumberC)*m_MagicNumberC);
      }
    }
		//three hits on x: y strip hit three times
		else if (finalPairs[i][0] > m_MagicNumberE){
			xThreeHits = true;
			xVec.push_back(finalPairs[i][0]/m_MagicNumberE);
			int lowerFourDigits = finalPairs[i][0]-(int)(finalPairs[i][0]/m_MagicNumberE)*m_MagicNumberE;
	    if (lowerFourDigits-(int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC > m_MagicNumberB){
				xChargeSharing = true;
        xVec.push_back(lowerFourDigits/m_MagicNumberC);
        xVec.push_back(lowerFourDigits-(int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC-m_MagicNumberB);
        xVec.push_back(lowerFourDigits-(int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC+1-m_MagicNumberB);
      }
      else {
        xVec.push_back(lowerFourDigits/m_MagicNumberC);
        xVec.push_back(lowerFourDigits - (int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC);
      }
 		}
		//simplest case: one hit on x, one on y
    else if (finalPairs[i][0] < m_MagicNumberA) {xVec.push_back(finalPairs[i][0]);}
    
    if (finalPairs[i][1] > m_MagicNumberB && finalPairs[i][1] < m_MagicNumberC){
      yVec.push_back(finalPairs[i][1]-m_MagicNumberB);
      yVec.push_back(finalPairs[i][1]+1-m_MagicNumberB);
			yChargeSharing = true;
    }
		else if (finalPairs[i][1] > m_MagicNumberD && finalPairs[i][1] < m_MagicNumberD + m_MagicNumberC){
			yVec.push_back(finalPairs[i][1]-m_MagicNumberD);
			yVec.push_back(finalPairs[i][1]+1-m_MagicNumberD);
			yVec.push_back(finalPairs[i][1]+2-m_MagicNumberD);
			yChargeSharing = true;
		}
    else if (finalPairs[i][1] > m_MagicNumberC && finalPairs[i][1] < m_MagicNumberE){
			yTwoHits = true;
      if (finalPairs[i][1]-(int)(finalPairs[i][1]/m_MagicNumberC)*m_MagicNumberC > m_MagicNumberB){
				yChargeSharing = true;
        yVec.push_back(finalPairs[i][1]/m_MagicNumberC);
        yVec.push_back(finalPairs[i][1]-(int)(finalPairs[i][1]/m_MagicNumberC)*m_MagicNumberC-m_MagicNumberB);
        yVec.push_back(finalPairs[i][1]-(int)(finalPairs[i][1]/m_MagicNumberC)*m_MagicNumberC+1-m_MagicNumberB);
      }
      else {
        yVec.push_back(finalPairs[i][1]/m_MagicNumberC);
        yVec.push_back(finalPairs[i][1] - (int)(finalPairs[i][1]/m_MagicNumberC)*m_MagicNumberC);
      }
    }
		else if (finalPairs[i][1] > m_MagicNumberE){
			yThreeHits = true;
			yVec.push_back(finalPairs[i][1]/m_MagicNumberE);
			int lowerFourDigits = finalPairs[i][1]-(int)(finalPairs[i][1]/m_MagicNumberE)*m_MagicNumberE;
	    if (lowerFourDigits-(int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC > m_MagicNumberB){
				yChargeSharing = true;
        yVec.push_back(lowerFourDigits/m_MagicNumberC);
        yVec.push_back(lowerFourDigits-(int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC-m_MagicNumberB);
        yVec.push_back(lowerFourDigits-(int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC+1-m_MagicNumberB);
      }
      else {
        yVec.push_back(lowerFourDigits/m_MagicNumberC);
        yVec.push_back(lowerFourDigits - (int)(lowerFourDigits/m_MagicNumberC)*m_MagicNumberC);
      }
 		}
    else if (finalPairs[i][1] < m_MagicNumberA) {yVec.push_back(finalPairs[i][1]);}

		if (xTwoHits){
			twoHitsCounter += 1;
			int indexOne, indexTwo;

			vector<int> xVecNew;
			//first hit
			xVecNew.push_back(xVec[0]);
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			pair.clear();
			xVecNew.clear();
			yStripHitMultipleTimes.push_back(1);
			xStripHitMultipleTimes.push_back(0);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(0,xVec[0]);

			//second hit
			if (xChargeSharing){
				xVecNew.push_back(xVec[1]);
				xVecNew.push_back(xVec[2]);
				pair.push_back(xVecNew);
				pair.push_back(yVec);
				decodedFinalPairs.push_back(pair);
				yStripHitMultipleTimes.push_back(1);
				xStripHitMultipleTimes.push_back(0);
				chargeSharing.push_back(1);
				indexTwo = GetStripIndex(0, (m_MagicNumberB+xVec[1]));
			}
			else {
				xVecNew.push_back(xVec[1]);
				pair.push_back(xVecNew);
				pair.push_back(yVec);
  	  	decodedFinalPairs.push_back(pair);
				yStripHitMultipleTimes.push_back(1);
				xStripHitMultipleTimes.push_back(0);
				chargeSharing.push_back(0);
				indexTwo = GetStripIndex(0, xVec[1]);
			}
			xVecNew.clear();
			//change energy and resolution for second hit
			//also add hit quality, but keep it the same for now
			//because there are multiple hits on a y strip, hit energy should be the energy of the x strip
			float energyOne = energy[0][indexOne];
			float energyTwo = energy[0][indexTwo];
			float eResOne = sig[0][indexOne];
			float eResTwo = sig[0][indexTwo];

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

      if (hitEnergy.size() <= i+twoHitsCounter-1) {
        m_StripPairingFailed = "Programming error: Array out of bounds";
        cout<<m_StripPairingFailed<<endl;
        return decodedFinalPairs;
      }
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
			yVecNew.push_back(yVec[0]);
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			pair.clear();
			yVecNew.clear();
			xStripHitMultipleTimes.push_back(1);
			yStripHitMultipleTimes.push_back(0);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(1,yVec[0]);

			//second hit
			if (yChargeSharing){
				yVecNew.push_back(yVec[1]);
				yVecNew.push_back(yVec[2]);
				pair.push_back(xVec);
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(1);
				indexTwo = GetStripIndex(1, (m_MagicNumberB+yVec[1]));
			}
			else {
				pair.push_back(xVec);
				yVecNew.push_back(yVec[1]);
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(0);
				indexTwo = GetStripIndex(1,yVec[1]);
			}
			xStripHitMultipleTimes.push_back(1);
			yStripHitMultipleTimes.push_back(0);
			yVecNew.clear();
			//change energy and resolution for second hit
			//also add element to hit quality, need to do properly later
			float energyOne = energy[1][indexOne];
			float energyTwo = energy[1][indexTwo];
			float eResOne = sig[1][indexOne];
			float eResTwo = sig[1][indexTwo];

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

      if (hitEnergy.size() <= i+twoHitsCounter-1) {
        m_StripPairingFailed = "Programming error: Array out of bounds";
        cout<<m_StripPairingFailed<<endl;
        return decodedFinalPairs;
      }

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
			xVecNew.push_back(xVec[0]);
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			yStripHitMultipleTimes.push_back(1);
			xStripHitMultipleTimes.push_back(0);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(0,xVec[0]);
			pair.clear();
			xVecNew.clear();
			//change energy and resolution for first hit
			//hitEnergy.at(i+twoHitsCounter-1) = 

			//second hit
			xVecNew.push_back(xVec[1]);
			pair.push_back(xVecNew);
			pair.push_back(yVec);
			decodedFinalPairs.push_back(pair);
			yStripHitMultipleTimes.push_back(1);
			xStripHitMultipleTimes.push_back(0);
			chargeSharing.push_back(0);
			indexTwo = GetStripIndex(0,xVec[1]);
			pair.clear();
			xVecNew.clear();

			//third hit
			if (xChargeSharing){
				xVecNew.push_back(xVec[2]);
				xVecNew.push_back(xVec[3]);
				pair.push_back(xVecNew);
				pair.push_back(yVec);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(1);
				indexThree = GetStripIndex(0,(m_MagicNumberB+xVec[2]));
			}
			else {
				xVecNew.push_back(xVec[2]);
				pair.push_back(xVecNew);
				pair.push_back(yVec);
  	  	decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(0);
				indexThree = GetStripIndex(0,xVec[2]);
			}
			yStripHitMultipleTimes.push_back(1);
			xStripHitMultipleTimes.push_back(0);
			xVecNew.clear();
			//change energy and resolution for second hit
			//also add hit quality, but keep it the same for now
			float energyOne = energy[0][indexOne];
			float energyTwo = energy[0][indexTwo];
			float energyThree = energy[0][indexThree];
			float eResOne = sig[0][indexOne];
			float eResTwo = sig[0][indexTwo];
			float eResThree = sig[0][indexThree];

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

      if (hitEnergy.size() <= i+threeHitsCounter-1) {
        m_StripPairingFailed = "Programming error: Array out of bounds";
        cout<<m_StripPairingFailed<<endl;
        return decodedFinalPairs;
      }

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
			yVecNew.push_back(yVec[0]);
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			xStripHitMultipleTimes.push_back(1);
			yStripHitMultipleTimes.push_back(0);
			chargeSharing.push_back(0);
			indexOne = GetStripIndex(1,yVec[0]);
			pair.clear();
			yVecNew.clear();
			//second hit
			pair.push_back(xVec);
			yVecNew.push_back(yVec[1]);
			pair.push_back(yVecNew);
			decodedFinalPairs.push_back(pair);
			xStripHitMultipleTimes.push_back(1);
			yStripHitMultipleTimes.push_back(0);
			chargeSharing.push_back(0);
			indexTwo = GetStripIndex(1,yVec[1]);
			pair.clear();
			yVecNew.clear();
			//third hit
			if (yChargeSharing){
				yVecNew.push_back(yVec[2]);
				yVecNew.push_back(yVec[3]);
				pair.push_back(xVec);
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(1);
				indexThree = GetStripIndex(1,(m_MagicNumberB+yVec[2]));
			}
			else {
				pair.push_back(xVec);
				yVecNew.push_back(yVec[2]);
				pair.push_back(yVecNew);
				decodedFinalPairs.push_back(pair);
				chargeSharing.push_back(0);
				indexThree = GetStripIndex(1,yVec[2]);
			}
			xStripHitMultipleTimes.push_back(1);
			yStripHitMultipleTimes.push_back(0);
			yVecNew.clear();
			//change energy and resolution for second hit
			//also add element to hit quality, need to do properly later
			float energyOne = energy[1][indexOne];
			float energyTwo = energy[1][indexTwo];
			float energyThree = energy[1][indexThree];
			float eResOne = sig[1][indexOne];
			float eResTwo = sig[1][indexTwo];
			float eResThree = sig[1][indexThree];

			//vector<float>::iterator itOne = hitEnergy.begin();
			//vector<float>::iterator itTwo = energyResolution.begin();
			//vector<float>::iterator itThree = hitQualityFactor.begin();

      if (hitEnergy.size() <= i+threeHitsCounter-1) {
        m_StripPairingFailed = "Programming error: Array out of bounds";
        cout<<m_StripPairingFailed<<endl;
        return decodedFinalPairs;
      }

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
			xStripHitMultipleTimes.push_back(0);
			yStripHitMultipleTimes.push_back(0);
			if (!xChargeSharing && !yChargeSharing){
				chargeSharing.push_back(0);
			}
			else { chargeSharing.push_back(1); }
		}
/*   
   cout << "pair: "  << endl;
    for (int j=0; j<pair.size(); j++){
      for (int k=0; k<pair[j].size(); k++){
        cout << pair[j][k] << '\t' ;
        cout << pair[j][k] << endl;
      }
    }
*/

    xVec.clear();
    yVec.clear();
    pair.clear();
    //		pair[0].clear();
    //	pair[1].clear();
    
  }
 
 
  //print decodedFinalPairs
  
//  cout << decodedFinalPairs.size() << endl;
/*	cout << decodedFinalPairs[0][1].size() << endl;
  cout << "decoded final pairs" << endl;
  for (int i=0; i<decodedFinalPairs.size(); i++){
    for (int j=0; j<2; j++){
      for (int k=0; k<decodedFinalPairs[i][j].size(); k++){
        cout << decodedFinalPairs[i][j][k] << '\t';
      }
      cout << '\t' << '\t';
    }
    cout << endl << endl;
  }
 */
	 return decodedFinalPairs;
  
};

//checks whether all strips were paired, returns true if so
bool MModuleStripPairingGreedy::CheckAllStripsWerePaired(){
  
  vector<vector<vector<int> > > decodedFinalPairs = DecodeFinalPairs();

  if (m_StripPairingFailed != "") return false;

  int counter = 0;
  
  //count how many strips in decodedFinalPairs are equal to original strips
  for (int axis=0; axis<2; axis++){
    //n indexes original hits
    for (int n=0; n<nHitsOrig[axis]; n++){
      //i, j, k index decodedFinalPairs
      for (unsigned int i=0; i<decodedFinalPairs.size(); i++){
        for (unsigned int k=0; k<decodedFinalPairs[i][axis].size(); k++){
          if (stripsHit[axis][n] == decodedFinalPairs[i][axis][k]){
            counter += 1;
          }
        }
      }
    }
  }

/*	if (counter < nHitsOrig[0] + nHitsOrig[1]){
		PrintXYStripsHit();
		cout << "final pairs: " << endl;
		for (int i=0; i<finalPairs.size(); i++){
			cout << finalPairs[i][0] << '\t' << finalPairs[i][1] << endl;
		}
		dummy_func();
	}
*/  
  //compare counter to total number of strips hit
  if (counter > nHitsOrig[0]+nHitsOrig[1]){
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": There's something wrong with the code!" << endl;
    return false;
  }
  else if (counter == nHitsOrig[0]+nHitsOrig[1]){
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": All strips were paired successfully!" << endl;
    return true;
  }
  else {
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Alert! Not all strips were paired!" << endl;
    return false;
  }
  
};


////////////////////////////////////////////////////////////////////////////////

void MModuleStripPairingGreedy::ShowOptionsGUI(){
  
  // Show the options GUI - or do nothing
	MGUIOptionsStripPairing* Options = new MGUIOptionsStripPairing(this);
	Options->Create();
	gClient->WaitForUnmap(Options);


};

bool MModuleStripPairingGreedy::ReadXmlConfiguration(MXmlNode* Node){

	//! Read the configuration data from an XML node

	MXmlNode* ModeNode = Node->GetNode("Mode");
	if (ModeNode != 0){
		m_Mode = ModeNode->GetValueAsUnsignedInt();
	}

	return true;

};

MXmlNode* MModuleStripPairingGreedy::CreateXmlConfiguration(){

	//! Create an XML node tree from the configuration

	MXmlNode* Node = new MXmlNode(0, m_XmlTag);
	new MXmlNode(Node, "Mode", m_Mode);

	return Node;

};

/////////////////////////////////////////////////////////////////////////////////

//print the stripsHit vector
void MModuleStripPairingGreedy::PrintXYStripsHit(){
  
//  cout << "stripsHit.size(): " << stripsHit.size() << endl;
  
  cout << "--------------------------" << endl << "Printing xStripsHit...." << endl;
  for(int i=0; i<nHits[0]; i++){
    cout << stripsHit[0][i] << '\t' << energy[0][i];
		cout << '\t' << sig[0][i] << endl;
  }
 
	cout << "total X energy: " << '\t';
	float xE = 0;
	for (int i=0; i<nHitsOrig[0]; i++){
		xE += energy[0][i];
	}
 	cout << xE << endl;


  cout << "--------------------------" << endl << "Printing yStripsHit...." << endl;
  for(int i=0; i<nHits[1]; i++){
    cout << stripsHit[1][i] << '\t' << energy[1][i];
		cout << '\t' << sig[1][i] << endl;
  }

	cout << "total Y energy: " << '\t';
	float yE = 0;
	for (int i=0; i<nHitsOrig[1]; i++){
		yE += energy[1][i];
	}
	cout << yE << endl;  

};

void MModuleStripPairingGreedy::PrintXYStripsHitOrig(){
  
//  cout << "stripsHit.size(): " << stripsHit.size() << endl;
 
	if (nHitsOrig.size() < 2){
		return;
	}
 
  cout << "--------------------------" << endl << "Printing xStripsHit...." << endl;
  for(int i=0; i<nHitsOrig[0]; i++){
    cout << stripsHit[0][i] << '\t' << energy[0][i];
		cout << '\t' << sig[0][i] << endl;
  }
 
	cout << "total X energy: " << '\t';
	float xE = 0;
	for (int i=0; i<nHitsOrig[0]; i++){
		xE += energy[0][i];
	}
 	cout << xE << endl;


  cout << "--------------------------" << endl << "Printing yStripsHit...." << endl;
  for(int i=0; i<nHitsOrig[1]; i++){
    cout << stripsHit[1][i] << '\t' << energy[1][i];
		cout << '\t' << sig[1][i] << endl;
  }

	cout << "total Y energy: " << '\t';
	float yE = 0;
	for (int i=0; i<nHitsOrig[1]; i++){
		yE += energy[1][i];
	}
	cout << yE << endl;  

};

void MModuleStripPairingGreedy::PrintFinalPairs(){

	cout << "----------------------" << endl << "Printing final pairs" << endl;

	for (unsigned int p=0; p<finalPairs.size(); p++){
		for (int axis=0; axis<2; axis++){
			cout << finalPairs[p][axis] << '\t';
		}
		cout << endl;
	}

};

//print the weight matrix
void MModuleStripPairingGreedy::PrintWeightMatrix(){
  
  cout << "-------------------" << endl;
  
  cout << nHits[1] << endl;
  cout << nHits[0] << endl;
  cout << weightMatrix.size() << endl;
  cout << weightMatrix[0].size() << endl;
  //	cout << weightMatrix[1].size() << endl;
  
  cout << "printing matrix" << endl;
  for (int i=0; i<nHits[1]; i++){
    for (int j=0; j<nHits[0]; j++){
      cout << weightMatrix[j][i] << '\t';
    }
    cout << endl;
  }
  
  
  cout << "-------------------" << endl;
  
};

void MModuleStripPairingGreedy::PrintKillMatrix(){

	for (int axis=0; axis<2; axis++){
		cout << "axis: " << axis << endl;
		for (int m=0; m<nHits[axis]; m++){
			for (int n=0; n<nHits[axis]; n++){
				cout << killMatrix[axis][m][n] << '\t';
			}
			cout << endl;
		}
		cout << "-------" << endl;
	} 

};

float MModuleStripPairingGreedy::GetEth(){
  
  return Eth;
  
};

vector<vector<int> > MModuleStripPairingGreedy::GetStripsHit(){
  
  return stripsHit;
  
};

void MModuleStripPairingGreedy::SetStripsHit(vector<vector<int> > inputVec){
  
  nHits.clear();
  stripsHit.clear();
  
  stripsHit = inputVec;
  nHits.push_back(inputVec[0].size());
  nHits.push_back(inputVec[1].size());
  nHitsOrig = nHits;
  
  //	cout << "stripsHit size: " << stripsHit.size() << '\t' <<  stripsHit[0].size() << '\t' <<stripsHit[1].size() << endl;
  
};

vector<vector<float> > MModuleStripPairingGreedy::GetEnergy(){
  
  return energy;
  
};

void MModuleStripPairingGreedy::SetEnergy(vector<vector<float> > inputVec){
  
  energy.clear();
  energy = inputVec;
  
};

vector<vector<float> > MModuleStripPairingGreedy::GetSigma(){
  
  return sig;
  
};

void MModuleStripPairingGreedy::SetSigma(vector<vector<float> > inputVec){
  
  sig.clear();
  sig = inputVec;
  
};

void MModuleStripPairingGreedy::SetFinalPairs(vector<vector<int> > inputVec){
  
  finalPairs.clear();
  finalPairs = inputVec;
  /*
  cout << "printing final pairs: " << endl;
  cout << finalPairs.size() << endl;
  cout << finalPairs[0].size() << endl;
  cout << finalPairs[1].size() << endl;
  for (int i=0; i<finalPairs[0].size(); i++){
    cout << finalPairs[0][i] << '\t' << finalPairs[1][i] << endl;
  }
  */
};

vector<vector<float> > MModuleStripPairingGreedy::GetWeightMatrix(){
  
  return weightMatrix;
  
};

vector<vector<int> > MModuleStripPairingGreedy::GetBadCombinations(){
  
  return badCombinations;
  
};

void MModuleStripPairingGreedy::SetBadCombinations(vector<vector<int> > inputVec){
  
  badCombinations.clear();
  badCombinations = inputVec;
  
};

int MModuleStripPairingGreedy::GetNBadCombinations(int axis){

	int counter = 0;

	for (int i=0; i<nHits[axis]; i++){
		if (badCombinations[axis][i] == 1){
			counter += 1;
		}
	}

	return counter;
};

int MModuleStripPairingGreedy::GetStripIndex(int axis, int stripID){

	int index = -1;

	for (int i=0; i<nHits[axis]; i++){
		if (stripsHit[axis][i] == stripID){
			index = i;
		}
	}

	return index;
};

void MModuleStripPairingGreedy::dummy_func(void){

	return;
}
