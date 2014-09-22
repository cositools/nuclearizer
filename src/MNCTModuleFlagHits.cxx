/*
 * MNCTModuleFlagHits.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleFlagHits
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleFlagHits.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleFlagHits)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleFlagHits::MNCTModuleFlagHits() : MModule()
{
  // Construct an instance of MNCTModuleFlagHits

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Flag Cross Talk and Charge Sharing";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "FlagHits";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_DetectorEffectsEngine);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_StripPairing);

  // Set all types this modules handles
	AddModuleType(MAssembly::c_FlagHits);

  // Set all modules, which can follow this module
	AddModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsFlagHits)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleFlagHits::~MNCTModuleFlagHits()
{
  // Delete this instance of MNCTModuleFlagHits	
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleFlagHits::Initialize()
{
  // Initialize the module 

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleFlagHits::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

	int nHits = Event->GetNHits();
	for (int h=0; h<nHits; h++){
		MNCTHit* Hit = Event->GetHit(h);
		ReadHit(Hit);
	}

  return true;
};

void MNCTModuleFlagHits::ReadHit(MNCTHit* Hit){

	vector<double> nStripIDs, pStripIDs;
	double nEnergy = 0;
	double pEnergy = 0;
	double nEnRes = 0;
	double pEnRes = 0;;
	double stripID, energy, enRes;
	int detector;

	//get energy and resolution of each strip hit
	int nStripHits = Hit->GetNStripHits();
	for (int i=0; i<nStripHits; i++){
		MNCTStripHit* stripHit = Hit->GetStripHit(i);
		detector = stripHit->GetDetectorID();
		stripID = stripHit->GetStripID();
		energy = stripHit->GetEnergy();
		enRes = stripHit->GetEnergyResolution();
		if (stripHit->IsXStrip()){
			pStripIDs.push_back(stripID);
			pEnergy = pEnergy + energy;
			pEnRes = sqrt(pow(pEnRes,2) + pow(enRes,2));
		}
		else {
			nStripIDs.push_back(stripID);
			nEnergy = nEnergy + energy;
			nEnRes = sqrt(pow(nEnRes,2) + pow(enRes,2));
		}
	}

	bool pAdjacent = false;
	bool nAdjacent = false;
	//check if hit has adjacent strips
	int num_pStrips = pStripIDs.size();
	if (num_pStrips > 1){
		sort(pStripIDs.begin(), pStripIDs.end());
		for (int i=0; i<num_pStrips-1; i++){
			if (pStripIDs.at(i) == pStripIDs.at(i+1)){
				pAdjacent = true;
			}
		}
	}
	int num_nStrips = nStripIDs.size();
	if (num_nStrips > 1){
		sort(nStripIDs.begin(), nStripIDs.end());
		for (int i=0; i<num_nStrips-1; i++){
			if (nStripIDs.at(i) == nStripIDs.at(i+1)){
				nAdjacent = true;
			}
		}
	}

	bool flagCrossTalk = false;
	bool flagChargeLoss = false;

	if (!pAdjacent && !nAdjacent){
		return;
	}

	else if (pAdjacent && !nAdjacent){
		flagCrossTalk = CheckCrossTalk(detector,1,pEnergy,nEnergy,nEnRes);
		if (!flagCrossTalk){
			flagChargeLoss = CheckChargeLoss(1,pEnergy,nEnergy,nEnRes);
		}
	}
	else if (nAdjacent && !pAdjacent){
		flagCrossTalk = CheckCrossTalk(detector,0,pEnergy,nEnergy,pEnRes);
		if (!flagCrossTalk){
			flagChargeLoss = CheckChargeLoss(0,pEnergy,nEnergy,pEnRes);
		}
	}

	Hit->SetCrossTalkFlag(flagCrossTalk);
	Hit->SetChargeLossFlag(flagChargeLoss);

};

bool MNCTModuleFlagHits::CheckCrossTalk(int det, bool pSide, double pEnergy, double nEnergy, double eRes){

	bool flagCT = false;

	TF1* ctFunction = GetCrossTalkEnergyDiff(det);

	double ctDiff;
	//if pSide has adjacent strips, pEnergy must be larger than nEnergy
	if (pSide && pEnergy > nEnergy){
		ctDiff = ctFunction->Eval(pEnergy);
		if (pEnergy-nEnergy < ctDiff+eRes && pEnergy-nEnergy > ctDiff-eRes){
			flagCT = true;
		}
	}
	else if (!pSide && nEnergy > pEnergy){
		ctDiff = ctFunction->Eval(nEnergy);
		if (nEnergy-pEnergy < ctDiff+eRes && nEnergy-pEnergy > ctDiff-eRes){
			flagCT = true;
		}
	}

	return flagCT;

};

TF1* MNCTModuleFlagHits::GetCrossTalkEnergyDiff(int det){

	//fit data points to a line
//	int ctDiff[4];
	int trueEnergy[] = {511,662,898,1275};
	int ctDiff[4];
	if (det == 0){
		ctDiff[0] = 10;
		ctDiff[1] = 10;
		ctDiff[2] = 14;
		ctDiff[3] = 21;
	}
	else if (det==1 || det==2 || det==4 || det==8){
		ctDiff[0] = 8;
		ctDiff[1] = 10;
		ctDiff[2] = 14;
		ctDiff[3] = 20;
	}
	else if (det == 3){
		ctDiff[0] = 10;
		ctDiff[1] = 10;
		ctDiff[2] = 13;
		ctDiff[3] = 20;
	}
	else if (det==5 || det==9){
		ctDiff[0] = 9;
		ctDiff[1] = 10;
		ctDiff[2] = 14;
		ctDiff[3] = 20;
	}
	else if (det==6 || det==10){
		ctDiff[0] = 9;
		ctDiff[1] = 10;
		ctDiff[2] = 13;
		ctDiff[3] = 18;
	}
	else if (det == 7){
		ctDiff[0] = 8;
		ctDiff[1] = 10;
		ctDiff[2] = 13;
		ctDiff[3] = 20;
	}
	else if (det == 11){
		ctDiff[0] = 8;
		ctDiff[1] = 13;
		ctDiff[2] = 12;
		ctDiff[3] = 20;
	}

	TGraph* graph = new TGraph(4,trueEnergy,ctDiff);
	TF1* fit = new TF1("fit", "[0]*x+[1]", 0, 1400);
	graph->Fit("fit","Q");

//	double slope = fit->GetParameter(0);
//	double constant = fit->GetParameter(1);

	return fit;


};

bool MNCTModuleFlagHits::CheckChargeLoss(bool pSide, double pEnergy, double nEnergy, double eRes){

	bool flagCL = false;

	if (pSide && pEnergy < nEnergy){
		if (nEnergy-pEnergy < 3*eRes && nEnergy-pEnergy > eRes){
			flagCL = true;
		}
	}
	else if (!pSide && nEnergy < pEnergy){
		if (pEnergy-nEnergy < 3*eRes && pEnergy-nEnergy > eRes){
			flagCL = true;
		}
	}

	return true;

};

////////////////////////////////////////////////////////////////////////////////


void MNCTModuleFlagHits::ShowOptionsGUI()
{

	// Show the options GUI - or do nothing
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleFlagHits::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode->GetValue();
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleFlagHits::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


////////////////////////////////////////////////////////////////////////////////
