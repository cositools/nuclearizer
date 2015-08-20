//TODO:
//-implement detector name vector for global position lookup
//-ckech for bad flags from Clio's strip pairing
//-handle events that get mapped out of detector
//-check timing values before computing CTD, could be that some strips have zero timing because they didn't trigger FLD




/*
 * MNCTModuleDepthCalibration.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTModuleDepthCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibration.h"
#include "MGUIOptionsDepthCalibration.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"
#include "TH1.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleDepthCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////

TH1D* EHist;

MNCTModuleDepthCalibration::MNCTModuleDepthCalibration() : MModule()
{
  // Construct an instance of MNCTModuleDepthCalibration

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Depth calibration";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibration";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_StripPairing);

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

	for(unsigned int i = 0; i < 12; ++i){
		char det_name[32]; sprintf(det_name,"Detector%d",i);
		m_DetectorNames.push_back( MString(det_name) );
	}

	m_Thicknesses.resize(12);
	m_Thicknesses[0] = 1.49;
	m_Thicknesses[1] = 1.45;
	m_Thicknesses[2] = 1.50;
	m_Thicknesses[3] = 1.51;
	m_Thicknesses[4] = 1.50;
	m_Thicknesses[5] = 1.47;
	m_Thicknesses[6] = 1.48;
	m_Thicknesses[7] = 1.47;
	m_Thicknesses[8] = 1.49;
	m_Thicknesses[9] = 1.47;
	m_Thicknesses[10] = 1.42;
	m_Thicknesses[11] = 1.45;

	SetTimingNoiseFWHM(12.5); //use 12.5 ns for FWHM noise on CTD measurements
	EHist = new TH1D("","",50,0.0,200.0);

}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibration::~MNCTModuleDepthCalibration()
{
  // Delete this instance of MNCTModuleDepthCalibration
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration::Initialize()
{

	//read in coeffs
	MFile CoeffsFile;
	if( CoeffsFile.Open(m_CoeffsFile) == false ){
		cout << "Depth calibration: couldn't open coefficients file..." << endl;
		return false;
	}
	MString Line;
	while( CoeffsFile.ReadLine( Line ) ){
		if( !Line.BeginsWith("#") ){
			std::vector<MString> Tokens = Line.Tokenize(" ");
			if( Tokens.size() == 5 ){
				int pixel_code = Tokens[0].ToInt();
				double Stretch = Tokens[1].ToDouble();
				double Offset = Tokens[2].ToDouble();
				double Scale = Tokens[3].ToDouble();
				double Chi2 = Tokens[4].ToDouble();
				//last two tokens are amplitude and chi2, not really needed here
				std::vector<double>* coeffs = new std::vector<double>();
				coeffs->push_back(Stretch); coeffs->push_back(Offset); coeffs->push_back(Scale); coeffs->push_back(Chi2);
				m_Coeffs[pixel_code] = coeffs;
			}
		}
	}
	CoeffsFile.Close();

	//read in splines, true as 3rd arg gives CTD->Depth, false would give Depth->CTD
	bool SplinesGood = GetDepthSplines(m_SplinesFile, m_Splines, true);
	if( SplinesGood == false ){
		cout << "Depth Calibration: couldn't read in spline file..." << endl;
		return false;
	} else {
		TFile* rootF = new TFile("new_splines.root","recreate");
		for(unsigned int i = 0; i < 12; ++i){
			TSpline3* Sp = m_Splines[i];
			if( Sp != NULL ){
				rootF->WriteTObject( Sp );
			} else {
				cout << "@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
			}
		}
		rootF->Close();
	}


	return true;

}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

	for( unsigned int i = 0; i < Event->GetNHits(); ++i ){
		MNCTHit* H = Event->GetHit(i);
		//check here if the hit has a bad flag from the pairing!!!
		if( H == NULL ) continue;
		std::vector<MNCTStripHit*> XStrips;
		std::vector<MNCTStripHit*> YStrips;
		for( unsigned int j = 0; j < H->GetNStripHits(); ++j){
			MNCTStripHit* SH = H->GetStripHit(j);
			if( SH == NULL ) { cout << "Depth Calibration: got NULL strip hit :( " << endl; continue;}
			if( SH->IsXStrip() ) XStrips.push_back(SH); else YStrips.push_back(SH);
		}

		MVector GlobalPosition, PositionResolution;
		int PosError;
		int DetID = XStrips[0]->GetDetectorID();

		//check for each of the four cases:
		if( (XStrips.size() == 1) && (YStrips.size() == 1) ){
			MNCTStripHit* XSH = XStrips[0]; MNCTStripHit* YSH = YStrips[0]; 
			PosError = CalculatePosition(XSH, YSH, GlobalPosition, PositionResolution);
		} else if( (XStrips.size() == 2) && (YStrips.size() == 1) ){
			//should i be checking whether or not x strips are neighboring? strip pairing should already be doing this 
			//perform a check on energy fraction... eventually need to include error here based on energy fraction and depth
			double EnergyFraction;
			MNCTStripHit* XSH = GetDominantStrip(XStrips, EnergyFraction);
			MNCTStripHit* YSH = YStrips[0];
			PosError = CalculatePosition(XSH, YSH, GlobalPosition, PositionResolution);
		} else if( (XStrips.size() == 1) && (YStrips.size() == 2) ){
			double EnergyFraction;
			MNCTStripHit* XSH = XStrips[0];
			MNCTStripHit* YSH = GetDominantStrip(YStrips, EnergyFraction);
			PosError = CalculatePosition(XSH, YSH, GlobalPosition, PositionResolution);
		} else if( (XStrips.size() == 2) && (YStrips.size() == 2) ){
			double EnergyFractionX, EnergyFractionY;
			MNCTStripHit* XSH = GetDominantStrip(XStrips, EnergyFractionX);
			MNCTStripHit* YSH = GetDominantStrip(YStrips, EnergyFractionY);
			PosError = CalculatePosition(XSH, YSH, GlobalPosition, PositionResolution);
		} else {
			//set too many SH bad flag
			PosError = -1;
		}

		if( PosError == 0 ){
			//good
			H->SetPosition(GlobalPosition); H->SetPositionResolution(PositionResolution);
			++m_NoError;
		} else if( PosError == 1 ){
			Event->SetDepthCalibrationIncomplete();
			++m_Error1;
		} else if( PosError == 2 ){
			Event->SetDepthCalibrationIncomplete();
			++m_Error2;
		} else if( PosError == 3){
			EHist->Fill(H->GetEnergy());
			Event->SetDepthCalibrationIncomplete();
			++m_Error3;
		} else if( PosError == -1){
			Event->SetDepthCalibrationIncomplete();
			++m_ErrorSH;
		}
	}


	return true;
}

MNCTStripHit* MNCTModuleDepthCalibration::GetDominantStrip(std::vector<MNCTStripHit*>& Strips, double& EnergyFraction){
	double MaxEnergy = 0.0;
	double TotalEnergy = 0.0;
	MNCTStripHit* MaxStrip = NULL;
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
		

//return zero if the position calculation is error-free
int MNCTModuleDepthCalibration::CalculatePosition(MNCTStripHit* XSH, MNCTStripHit* YSH, MVector& GlobalPosition, MVector& PositionResolution){

	int DetID = XSH->GetDetectorID();
	int pixel_code = 10000*DetID + 100*XSH->GetStripID() + YSH->GetStripID();
	std::vector<double>* Coeffs = m_Coeffs[pixel_code];
	if( Coeffs == NULL ){
		//set the bad flag for depth
		return 1;
	} else {

		if( (XSH->GetTiming() < 1.0E-6) || (YSH->GetTiming() < 1.0E-6) ){
			//we don't have timing on one or both of the strips..... return with an error
			//better yet, assign the event to the middle of the detector and set the position resolution to be large
			return 3;
		}

		double CTD = (XSH->GetTiming() - YSH->GetTiming())*10.0;
		double CTD_s = (CTD - Coeffs->at(1))/Coeffs->at(0); //apply inverse stretch and offset
		double Xmin = m_Splines[DetID]->GetXmin();
		double Xmax = m_Splines[DetID]->GetXmax();

		//if the CTD is out of range, check if we should reject the event or assume it was an edge event
		if(CTD_s < Xmin){
			if(fabs(CTD_s - Xmin) <= (2.0*m_TimingNoiseFWHM)) CTD_s = Xmin; else return 2;
		}
		if( CTD_s > Xmax){
			if(fabs(CTD_s - Xmax) <= (2.0*m_TimingNoiseFWHM)) CTD_s = Xmax; else return 2;
		}

		double Depth = m_Splines[DetID]->Eval(CTD_s);
		double Zpos = m_Thicknesses[DetID] - Depth; //convert back to mass model coordinates [-thickness/2,+thickness/2]
		double Xpos = ((double)YSH->GetStripID() - 19.0)*(-0.2);
		double Ypos = ((double)XSH->GetStripID() - 19.0)*(-0.2);
		GlobalPosition = m_Geometry->GetGlobalPosition( MVector(Xpos, Ypos, Zpos), m_DetectorNames[DetID]);
		//to get position resolution, assume FWHM noise of 12.5 ns, say CTD is 70 ns, find the difference in depth between (70-6.25) and (70+6.25) and then call that the depth FWHM error.
		double Z_FWHM = GetZFWHM( CTD_s, DetID, m_TimingNoiseFWHM );
		PositionResolution.SetXYZ(0.2/sqrt(12.0), 0.2/sqrt(12.0), Z_FWHM/2.35);
		return 0;
	}
}


double MNCTModuleDepthCalibration::GetZFWHM(double CTD_s, int DetID, double Noise){

	TSpline3* Sp = m_Splines[DetID];
	double xmax = Sp->GetXmax(); double xmin = Sp->GetXmin();
	double HalfNoise = Noise/2.0;
	if( (CTD_s - HalfNoise) < xmin ){
		return fabs(Sp->Eval(xmin) - Sp->Eval(xmin+Noise));
	} else if( (CTD_s + HalfNoise) > xmax ){
		return fabs(Sp->Eval(xmax) - Sp->Eval(xmax-Noise));
	} else {
		return fabs(Sp->Eval(CTD_s - HalfNoise) - Sp->Eval(CTD_s + HalfNoise));
	}

}

void MNCTModuleDepthCalibration::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
	MGUIOptionsDepthCalibration* Options = new MGUIOptionsDepthCalibration(this);
	Options->Create();
	gClient->WaitForUnmap(Options);
}

bool MNCTModuleDepthCalibration::GetDepthSplines(MString fname, std::unordered_map<int, TSpline3*>& SplineMap, bool invert){
	//when invert flag is set to true, the splines returned are CTD->Depth
	MFile F; 
	if( F.Open(fname) == false ){
		return false;
	}
	vector<double> xvec, yvec;
	MString line;
	int DetID, NewDetID;
	while( F.ReadLine(line) ){
		if( line.Length() != 0 ){
			if( line.BeginsWith("#") ){
				vector<MString> tokens = line.Tokenize(" ");
				NewDetID = tokens[1].ToInt();
				if( xvec.size() > 0 ) AddSpline(xvec, yvec, SplineMap, DetID, invert);
				DetID = NewDetID;
			} else {
				vector<MString> tokens = line.Tokenize(" ");
				xvec.push_back(tokens[0].ToDouble()); yvec.push_back(tokens[1].ToDouble());

			}
		}
	}
	//make last spline
	if( xvec.size() > 0 ) AddSpline(xvec, yvec, SplineMap, DetID, invert);
	return true;
}

void MNCTModuleDepthCalibration::AddSpline(vector<double>& xvec, vector<double>& yvec, unordered_map<int, TSpline3*>& SplineMap, int DetID, bool invert){
	//add one more point to the start and end, corresponding to the detector edges so that the spline covers the
	//entire detector. just use a linear interpolation to get the edge values.

	//first extrapolate the lower side
	double dx, dy, m, b, newx, newy;
	dx = xvec[1] - xvec[0];
	dy = yvec[1] - yvec[0];
	m = dy / dx;
	b = yvec[0] - m*xvec[0];
	newx = xvec[0] - (dx/2.0);
	newy = m*newx + b;
	//	xvec.push_front(newx); yvec.push_front(newy);
	xvec.insert(xvec.begin(), newx); yvec.insert(yvec.begin(), newy);


	//next extrapolate the upper side
	size_t N = xvec.size();
	dx = xvec[N-1] - xvec[N-2];
	dy = yvec[N-1] - yvec[N-2];
	m = dy / dx;
	b = yvec[N-1] - m*xvec[N-1];
	newx = xvec[N-1] + (dx/2.0);
	newy = m*newx + b;
	xvec.push_back(newx); yvec.push_back(newy);

	double* x = &xvec[0]; double* y = &yvec[0];
	if( invert ){
		//need to make sure that none of the y elements is equal to the previous element... otherwise the inversion will give a nan
		for( unsigned int i = 1; i < yvec.size(); ++i ){
			//if( fabs(yvec[i] - yvec[i-1]) < 1.0E-6 ) yvec[i] += 2.0E-6;
			if( yvec[i] <= yvec[i-1] ) yvec[i] = yvec[i-1] + 1.0E-6; 

		}
		SplineMap[DetID] = new TSpline3("",y,x,xvec.size());
	} else {
		SplineMap[DetID] = new TSpline3("",x,y,xvec.size());
	}
	xvec.clear(); yvec.clear();
	return;
}

bool MNCTModuleDepthCalibration::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* CoeffsFileNameNode = Node->GetNode("CoeffsFileName");
  if (CoeffsFileNameNode != 0) {
	m_CoeffsFile = CoeffsFileNameNode->GetValue();
  }

  MXmlNode* SplinesFileNameNode = Node->GetNode("SplinesFileName");
  if (SplinesFileNameNode != 0) {
	m_SplinesFile = SplinesFileNameNode->GetValue();
  }


  return true;
}


/////////////////////////////////////////////////////////////////////////////////

MXmlNode* MNCTModuleDepthCalibration::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0,m_XmlTag);
  new MXmlNode(Node, "CoeffsFileName", m_CoeffsFile);
  new MXmlNode(Node, "SplinesFileName", m_SplinesFile);

  return Node;
}

void MNCTModuleDepthCalibration::Finalize()
{

	MModule::Finalize();
	cout << "###################" << endl;
	cout << "AWL depth cal stats" << endl;
	cout << "###################" << endl;
	cout << "Good hits: " << m_NoError << endl;
	cout << "Number of hits missing calibration coefficients: " << m_Error1 << endl;
	cout << "Number of hits too far outside of detector: " << m_Error2 << endl;
	cout << "Number of hits missing timing information: " << m_Error3 << endl;
	cout << "Number of hits with too many strip hits: " << m_ErrorSH << endl;
	TFile* rootF = new TFile("EHist.root","recreate");
	rootF->WriteTObject( EHist );
	rootF->Close();

}

// MNCTModuleDepthCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
