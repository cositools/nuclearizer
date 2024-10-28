////////////////////////////////////////////////////////////////////////////////
//
// MModuleDepthCalibrationB
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleDepthCalibrationB.h"
#include "MGUIOptionsDepthCalibrationB.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"
#include "TH1.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleDepthCalibrationB)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleDepthCalibrationB::MModuleDepthCalibrationB() : MModule()
{
  // Construct an instance of MModuleDepthCalibrationB

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Depth calibration B"; //

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibrationB";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader, true);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration, true);
  AddPreceedingModuleType(MAssembly::c_StripPairing, true);
  AddPreceedingModuleType(MAssembly::c_CrosstalkCorrection, false); // Soft requirement

  // Set all types this modules handles
  AddModuleType(MAssembly::c_DepthCorrection);
  AddModuleType(MAssembly::c_PositionDetermiation);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

	m_Thicknesses.reserve(12);
	m_Thicknesses[0] = 1.49;
	m_Thicknesses[1] = 1.45;
	m_Thicknesses[2] = 1.50;
	m_Thicknesses[3] = 1.45;
	m_Thicknesses[4] = 1.51;
	m_Thicknesses[5] = 1.50;
	m_Thicknesses[6] = 1.48;
	m_Thicknesses[7] = 1.47;
	m_Thicknesses[8] = 1.49;
	m_Thicknesses[9] = 1.47;
	m_Thicknesses[10] = 1.42;
	m_Thicknesses[11] = 1.45;

	SetTimingNoiseFWHM(12.5); //use 12.5 ns for FWHM noise on CTD measurements

	m_NoError = 0;
	m_Error1 = 0;
	m_Error2 = 0;
	m_Error3 = 0;
	m_Error4 = 0;
	m_ErrorSH = 0;


}


////////////////////////////////////////////////////////////////////////////////


MModuleDepthCalibrationB::~MModuleDepthCalibrationB()
{
  // Delete this instance of MModuleDepthCalibrationB
}


////////////////////////////////////////////////////////////////////////////////

FILE* fdebug;

bool MModuleDepthCalibrationB::Initialize()
{

	m_DepthCalibrator = new MDepthCalibratorB();
	if(!m_DepthCalibrator->LoadLookupTables(m_LookupTableFile)){
		return false;
	}

	MDGeometry* MDG = m_Geometry;

	for(unsigned int i = 0; i < 12; ++i){
		char det_name[32]; sprintf(det_name,"GeWafer_%d",i);
		MString MS(det_name);
		MDVolume* DetVol = MDG->GetVolume(MS);
		//		m_DetectorNames.push_back( MString(det_name) );
    if (DetVol == 0) {
      cout<<"Unable to find detector "<<MS<<endl;
      cout<<"Do you have the correct geometry loaded?"<<endl;
      return false;
    }
		m_DetectorVolumes.push_back(DetVol);
	}

	MSupervisor* S = MSupervisor::GetSupervisor();
	m_EnergyCalibration = (MModuleEnergyCalibrationUniversal*) S->GetAvailableModuleByXmlTag("EnergyCalibrationUniversal");
	if (m_EnergyCalibration == nullptr) {
		cout << "MModuleDepthCalibrationB: couldn't resolve pointer to Energy Calibration Module... need access to this module for energy resolution lookup!" << endl;
		return false;
	}

	fdebug = fopen("fdebug","w");

	return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibrationB::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
	vector<MHit*> NewHits;

	for( unsigned int i = 0; i < Event->GetNHits(); ++i ){
		MHit* H = Event->GetHit(i);

		//organize x and y strips into vectors
		if( H == NULL ) continue;
		if( H->GetNStripHits() == 0 ){
			//have a hit with no strip hits... WTF?
			cout << "HIT WITH NO STRIP HITS" << endl;
			continue;
		}
			
		std::vector<MStripHit*> XStrips;
		std::vector<MStripHit*> YStrips;
		for( unsigned int j = 0; j < H->GetNStripHits(); ++j){
			MStripHit* SH = H->GetStripHit(j);
			if( SH == NULL ) { cout << "Depth Calibration: got NULL strip hit :( " << endl; continue;}
			if( SH->IsLowVoltageStrip() ) XStrips.push_back(SH); else YStrips.push_back(SH);
		}

		MVector LocalPosition, PositionResolution, GlobalPosition;
		int PosError;
		int DetID = H->GetStripHit(0)->GetDetectorID();
		bool MultiHitX = H->GetStripHitMultipleTimesX(); //if true, then depth for this hit is no good
		bool MultiHitY = H->GetStripHitMultipleTimesY();
		bool BadDepth = false;
		if( MultiHitX || MultiHitY ){
			//might want to add more criteria here 
			BadDepth = true;
		}

		//check for each of the four cases:
		int Error = 0;
			
		if( (XStrips.size() == 1) && (YStrips.size() == 1) ){

			MStripHit* XSH = XStrips[0]; MStripHit* YSH = YStrips[0]; 
			PosError = CalculateLocalPosition(XSH, YSH, LocalPosition, PositionResolution, BadDepth);
//			GlobalPosition = m_Geometry->GetGlobalPosition( LocalPosition, m_DetectorNames[DetID]);
			GlobalPosition = m_DetectorVolumes[DetID]->GetPositionInWorldVolume(LocalPosition);
			H->SetPosition(GlobalPosition); H->SetPositionResolution(PositionResolution);
			if( PosError != 0 ) {
				Error = PosError; //record the positioning error 
				H->SetNoDepth();
			} else {
				/*
				int d = XSH->GetDetectorID();
				int pixel_code = (10000*d) + (100*XSH->GetStripID()) + YSH->GetStripID();
				double thick = m_Thicknesses[d];
				double z = LocalPosition.GetZ() + (thick/2.0);
				if(z <= 0.1){
					static int xc = 0;
					fprintf(fdebug,"n=%d px=%d x=%3.3f y=%3.3f ctd=%3.3f z=%3.3f\n",xc,pixel_code,XSH->GetTiming(),YSH->GetTiming(),XSH->GetTiming() - YSH->GetTiming(),z);
					++xc;
				}
				*/
			}

		} else if( ((XStrips.size() == 2) && (YStrips.size() == 1)) || ((XStrips.size() == 1) && (YStrips.size() == 2)) ){
			bool IsNeighborSideX;
			double EnergyFraction;
			vector<MStripHit*>* NeighborSideStrips;
			MStripHit* OtherSideStrip;

			//determine strip situation
			if( XStrips.size() == 2 ) { IsNeighborSideX = true; NeighborSideStrips = &XStrips; OtherSideStrip = YStrips[0];}
			else { IsNeighborSideX = false; NeighborSideStrips = &YStrips; OtherSideStrip = XStrips[0];}
			MStripHit* DominantStrip = GetDominantStrip(*NeighborSideStrips, EnergyFraction);
			MStripHit* NonDominantStrip;
			if( NeighborSideStrips->at(0) == DominantStrip ) NonDominantStrip = NeighborSideStrips->at(1); else NonDominantStrip = NeighborSideStrips->at(0);

			//calculate the position in the detector's coordinate system
			if( IsNeighborSideX ) {
				PosError = CalculateLocalPosition(DominantStrip, OtherSideStrip, LocalPosition, PositionResolution, BadDepth);
			} else {
				PosError = CalculateLocalPosition(OtherSideStrip, DominantStrip, LocalPosition, PositionResolution, BadDepth);
			}

			//calculate the global position and set the hit's position and position resolution
			GlobalPosition = m_DetectorVolumes[DetID]->GetPositionInWorldVolume(LocalPosition);
			H->SetPosition(GlobalPosition); H->SetPositionResolution(PositionResolution);

			if( PosError != 0 ) {
				Error = PosError; //record the positioning error 
				H->SetNoDepth();
			} else {
				H->SetEnergy( DominantStrip->GetEnergy() ); //reset energy to dominant strip energy
				MVector Local2Position, Position2Resolution;
				int Pos2Error = 0;
				if( IsNeighborSideX ) {
					Pos2Error = CalculateLocalPosition(NonDominantStrip, OtherSideStrip, Local2Position, Position2Resolution, BadDepth);
				} else {
					Pos2Error = CalculateLocalPosition(OtherSideStrip, NonDominantStrip, Local2Position, Position2Resolution, BadDepth);
				}
				Local2Position.SetZ( LocalPosition.GetZ() ); Position2Resolution.SetZ( PositionResolution.GetZ() );
				GlobalPosition = m_DetectorVolumes[DetID]->GetPositionInWorldVolume(Local2Position);
				MHit* NH = new MHit();
				NH->SetEnergy(NonDominantStrip->GetEnergy());
				double Eres = m_EnergyCalibration->LookupEnergyResolution(NonDominantStrip, NonDominantStrip->GetEnergy()); NH->SetEnergyResolution(Eres);
				NH->SetPosition( GlobalPosition ); NH->SetPositionResolution( Position2Resolution );
				NH->SetIsNondominantNeighborStrip();
				NH->AddStripHit(NonDominantStrip); NH->AddStripHit(OtherSideStrip);
				NewHits.push_back(NH);
			}

//			Event->SetDepthCalibrationIncomplete(); //for testing purposes!!! filtering out events that are not 1x1 strip AWL1x1
		
		} else if( (XStrips.size() == 2) && (YStrips.size() == 2) ){
			//in this case use depth from dominant strips but use weighted X and Y positions

			double EnergyFractionX, EnergyFractionY;
			MStripHit* XSH = GetDominantStrip(XStrips, EnergyFractionX);
			MStripHit* YSH = GetDominantStrip(YStrips, EnergyFractionY);
			PosError = CalculateLocalPosition(XSH, YSH, LocalPosition, PositionResolution, BadDepth);
			if( PosError != 0 ){
				Error = PosError;
				H->SetNoDepth();
			}
			//determine the weighted x and y positions
			double Xpos1 = ((double)YStrips[0]->GetStripID() - 19.0)*(-0.2);
			double Xpos2 = ((double)YStrips[1]->GetStripID() - 19.0)*(-0.2);
			double Xpos = ((Xpos1)*YStrips[0]->GetEnergy() + (Xpos2)*YStrips[1]->GetEnergy())/(YStrips[0]->GetEnergy() + YStrips[1]->GetEnergy());

			double Ypos1 = ((double)XStrips[0]->GetStripID() - 19.0)*(-0.2);
			double Ypos2 = ((double)XStrips[1]->GetStripID() - 19.0)*(-0.2);
			double Ypos = ((Ypos1)*XStrips[0]->GetEnergy() + (Ypos2)*XStrips[1]->GetEnergy())/(XStrips[0]->GetEnergy() + XStrips[1]->GetEnergy());

			LocalPosition.SetX(Xpos); LocalPosition.SetY(Ypos);
			//GlobalPosition = m_Geometry->GetGlobalPosition( LocalPosition, m_DetectorNames[DetID]);
			GlobalPosition = m_DetectorVolumes[DetID]->GetPositionInWorldVolume(LocalPosition);
			H->SetPosition(GlobalPosition); H->SetPositionResolution(PositionResolution);
//			Event->SetDepthCalibrationIncomplete(); //for testing purposes!!! filtering out events that are not 1x1 strip AWL1X1

		} else {
			//set too many SH bad flag
			Error = -1;
		}

		if( Error == 0 ){
			//good
			++m_NoError;
		} else if( Error == 1 ){
			Event->SetDepthCalibrationIncomplete();
			++m_Error1;
		} else if( Error == 2 ){
			Event->SetDepthCalibrationIncomplete();
			++m_Error2;
		} else if( Error == 3){
			//Hits that were missing timing information
			//EHist->Fill(H->GetEnergy());
			//don't set the globally bad flag
			//Event->SetDepthCalibrationIncomplete();
//			Event->SetDepthCalibrationIncomplete(); //for testing purposes!!! filtering out events that are not 1x1 strip AWL1X1
			++m_Error3;
		} else if( Error == 4){
			//hit was bad because of StripHitMultipleTimes flag from strip pairing
			++m_Error4;
			Event->SetDepthCalibrationIncomplete();
		} else if( Error == -1){
			Event->SetDepthCalibrationIncomplete();
			++m_ErrorSH;
		}

	}

	//add the new hits from the 3-strip events to the event.  Don't do it in the loop above because we don't want to loop back over these new hits
	for( const auto H: NewHits ) Event->AddHit(H);
  
  Event->SetAnalysisProgress(MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation);

	return true;
}

MStripHit* MModuleDepthCalibrationB::GetDominantStrip(std::vector<MStripHit*>& Strips, double& EnergyFraction){
	double MaxEnergy = 0.0;
	double TotalEnergy = 0.0;
	MStripHit* MaxStrip = NULL;
	for(const auto SH : Strips){
		double Energy = SH->GetEnergy();
		TotalEnergy += Energy;
		if( Energy > MaxEnergy ){
			MaxStrip = SH;
			MaxEnergy = Energy;
		}
	}
	EnergyFraction = MaxEnergy/TotalEnergy;
	return MaxStrip;
}

int MModuleDepthCalibrationB::CalculateLocalPosition(MStripHit* XSH, MStripHit* YSH, MVector& LocalPosition, MVector& PositionResolution, bool BadDepth){

	int RetVal = 0;

	//first set the X and Y positions before we try and do anything with the Z position
	double Xpos = ((double)YSH->GetStripID() - 19.0)*(-0.2);
	double Ypos = ((double)XSH->GetStripID() - 19.0)*(-0.2);
	double Zpos = 0.0;
	double CTD_s = 0.0;
	int CTD = 0;

	//now try and get z position
	int DetID = XSH->GetDetectorID();
	int pixel_code = 10000*DetID + 100*XSH->GetStripID() + YSH->GetStripID();
	if( m_DepthCalibrator->PixelTableExists(pixel_code) == false ){
		//set the bad flag for depth
		RetVal = 1;
	} else if( BadDepth == true ){
		RetVal = 4;
	} else {
		if( (XSH->GetTiming() < 1.0E-6) || (YSH->GetTiming() < 1.0E-6) ){
			//we don't have timing on one or both of the strips..... return with an error
			//better yet, assign the event to the middle of the detector and set the position resolution to be large
			RetVal = 3;
		} else {

			double CTD_f = (XSH->GetTiming() - YSH->GetTiming());
			CTD = CTD_f < 0 ? (int)(CTD_f - 0.001) : (int)(CTD_f + 0.001); //to ensure that the cast always gives us the integer we want
			double Z = 0.0;
			bool FoundZ = m_DepthCalibrator->GetZ(pixel_code, CTD, Z);
			/*
			if(!FoundZ){
				//check if we are within 4 entries of the edge of the lookup table
				if(CTD >= 0){
					for(int i = 1; i <= 4; ++i){
						FoundZ = m_DepthCalibrator->GetZ(pixel_code, CTD - (5*i), Z);
						if(FoundZ) break;
					}
				} else {
					for(int i = 1; i <= 4; ++i){
						FoundZ = m_DepthCalibrator->GetZ(pixel_code, CTD + (5*i), Z);
						if(FoundZ) break;
					}
				}
			}
			*/
			if(!FoundZ){
				RetVal = 2;
			} else {
				//cout << "pixel_code = " << pixel_code << ", CTD_f = " << CTD_f << ", CTD = " << CTD << ", Z = " << Z << endl;
			}
			
			if( RetVal == 0 ){

				Zpos = m_Thicknesses[DetID]/2.0 - Z;
			}

		}
	}

	if( RetVal != 0 ) Zpos = 0.0;

	//GlobalPosition = m_Geometry->GetGlobalPosition( MVector(Xpos, Ypos, Zpos), m_DetectorNames[DetID]);
	LocalPosition.SetXYZ(Xpos, Ypos, Zpos);

	//determine position resolution.  if depth could not be looked up, use Thickness/sqrt(12.0)
	if( RetVal == 0 ){
		double Z_FWHM = 0.03; //using a constant value for now
		PositionResolution.SetXYZ(0.2/sqrt(12.0), 0.2/sqrt(12.0), Z_FWHM/2.35);
	} else {
		PositionResolution.SetXYZ(0.2/sqrt(12.0), 0.2/sqrt(12.0), m_Thicknesses[DetID]/sqrt(12.0));
	}

	return RetVal;
}

void MModuleDepthCalibrationB::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
	MGUIOptionsDepthCalibrationB* Options = new MGUIOptionsDepthCalibrationB(this);
	Options->Create();
	gClient->WaitForUnmap(Options);
}


bool MModuleDepthCalibrationB::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
	m_LookupTableFile = FileNameNode->GetValue();
  }


  return true;
}


/////////////////////////////////////////////////////////////////////////////////

MXmlNode* MModuleDepthCalibrationB::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0,m_XmlTag);
  new MXmlNode(Node, "FileName", m_LookupTableFile);

  return Node;
}

void MModuleDepthCalibrationB::Finalize()
{

	MModule::Finalize();
	cout << "###################" << endl;
	cout << "AWL depth cal stats" << endl;
	cout << "###################" << endl;
	cout << "Good hits: " << m_NoError << endl;
	cout << "Number of hits missing calibration coefficients: " << m_Error1 << endl;
	cout << "Number of hits too far outside of detector: " << m_Error2 << endl;
	cout << "Number of hits missing timing information: " << m_Error3 << endl;
	cout << "Number of hits with strips hit multiple times: " << m_Error4 << endl;
	cout << "Number of hits with too many strip hits: " << m_ErrorSH << endl;
	/*
	TFile* rootF = new TFile("EHist.root","recreate");
	rootF->WriteTObject( EHist );
	rootF->Close();
	*/
	fclose(fdebug);

}

// MModuleDepthCalibrationB.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
