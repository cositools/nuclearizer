/*
 * MModuleDepthCalibration2024.cxx
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
// MModuleDepthCalibration2024
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleDepthCalibration2024.h"
#include "MGUIOptionsDepthCalibration2024.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"
#include "TH1.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleDepthCalibration2024)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleDepthCalibration2024::MModuleDepthCalibration2024() : MModule()
{
  // Construct an instance of MModuleDepthCalibration2024

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Depth calibration 2024"; // - Determining the depth of each event (by Sean);

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibration2024";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader, true);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration, true);
  AddPreceedingModuleType(MAssembly::c_StripPairing, true);
//  AddPreceedingModuleType(MAssembly::c_CrosstalkCorrection, false); // Soft requirement

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

  m_NoError = 0;
  m_Error1 = 0;
  m_Error2 = 0;
  m_Error3 = 0;
  m_Error4 = 0;
  m_Error5 = 0;
  m_ErrorSH = 0;
}


////////////////////////////////////////////////////////////////////////////////


MModuleDepthCalibration2024::~MModuleDepthCalibration2024()
{
  // Delete this instance of MModuleDepthCalibration2024
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::Initialize()
{

  if( LoadCoeffsFile(m_CoeffsFile) == false ){
    return false;
  }
  if( LoadSplinesFile(m_SplinesFile) == false ){
    return false;
  }

  // The detectors need to be in the same order as DetIDs.
  // ie DetID=0 should be the 0th detector in m_Detectors, DetID=1 should the 1st, etc.
  m_Detectors = m_Geometry->GetDetectorList();

  if( LoadTACCalFile(m_TACCalFile) == false ){
    cout << "No TAC Calibration file loaded. Proceeding without TAC Calibration." << endl;
  }

  // Look through the Geometry and get the names and thicknesses of all the detectors.
  for(unsigned int i = 0; i < m_Detectors.size(); ++i){
    // TODO: need to fix DetID, but leaving as 11 for debugging purposes
    unsigned int DetID = i;
    if ( m_UCSDOverride ){
      DetID = 11;
    }
    MDDetector* det = m_Detectors[i];
    // MString det_name = (det->GetDetectorVolume())->GetNamedDetectorName(0);
    if (det->GetNNamedDetectors() > 0){
      // TODO: determine thickness of each detector using the geometry file
      // cout << "Trying to get thickness from the geometry file..." << endl;
      // cout << "step 1" << endl;
      // MDVolume* vol = det->GetSensitiveVolume(0); 
      // cout << "step 2" << endl;
      // MDShapeBRIK* shape = dynamic_cast<MDShapeBRIK*>(vol->GetShape());
      // cout << "step 3" << endl;
      // double thickness = (shape->GetSize()).GetZ();
      // cout << "Success, the thickness is " << thickness << " cm" << endl;
      // m_Thicknesses[DetID] = thickness;
      MString det_name = det->GetNamedDetectorName(0);
      m_DetectorNames[DetID] = det_name;
      MDStrip3D* strip = dynamic_cast<MDStrip3D*>(det);
      m_XPitches[DetID] = strip->GetPitchX();
      m_YPitches[DetID] = strip->GetPitchY();
      m_NXStrips[DetID] = strip->GetNStripsX();
      m_NYStrips[DetID] = strip->GetNStripsY();
      cout << "Found detector " << det_name << " corresponding to DetID=" << DetID << "." << endl;
    }
  }

  MSupervisor* S = MSupervisor::GetSupervisor();
  m_EnergyCalibration = (MModuleEnergyCalibrationUniversal*) S->GetAvailableModuleByXmlTag("EnergyCalibrationUniversal");
  if (m_EnergyCalibration == nullptr) {
    cout << "MModuleDepthCalibration2024: couldn't resolve pointer to Energy Calibration Module... need access to this module for energy resolution lookup!" << endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::AnalyzeEvent(MReadOutAssembly* Event) 
{
  
  for( unsigned int i = 0; i < Event->GetNHits(); ++i ){
    // Each event represents one photon. It contains Hits, representing interaction sites.
    // H is a pointer to an instance of the MHit class. Each Hit has activated strips, represented by
    // instances of the MStripHit class.
    MHit* H = Event->GetHit(i);

    int Grade = GetHitGrade(H);

    // cout << "got a hit with grade " << Grade << endl;

    // Handle different grades differently    
    // GRADE=-1 is an error. Break from the loop and continue.
    if ( Grade == -1 ){
      H->SetNoDepth();
      Event->SetDepthCalibrationIncomplete();
      ++m_ErrorSH;
    }

    // GRADE=5 is some complicated geometry with multiple hits on a single strip. 
    // GRADE=4 means there are more than 2 strip hits on one or both sides.
    // TODO: handle GRADE
    else if( Grade > 3 ){
      H->SetNoDepth();
      Event->SetDepthCalibrationIncomplete();
      if(Grade==4){
        ++m_Error4;
      }
      else if(Grade==5){
        ++m_Error5;
      }
      else if(Grade==6){
        ++m_Error6;
      }
    }

    // If the Grade is 0-3, we can handle it.
    else {

      MVector LocalPosition, PositionResolution, GlobalPosition, GlobalResolution, LocalOrigin;

      // Calculate the position. If error is thrown, record and no depth.
      // Take a Hit and separate its activated X- and Y-strips into separate vectors.
      std::vector<MStripHit*> XStrips;
      std::vector<MStripHit*> YStrips;
      // cout << "looping over strip hits..." << endl;
      for( unsigned int j = 0; j < H->GetNStripHits(); ++j){
        // cout << "strip hit " << j << endl;
        MStripHit* SH = H->GetStripHit(j);
        if( SH->IsXStrip() ) XStrips.push_back(SH); else YStrips.push_back(SH);
      }

      // cout << "finished looping over strip hits" << endl;
      double XEnergyFraction;
      double YEnergyFraction;
      MStripHit* XSH = GetDominantStrip(XStrips, XEnergyFraction); 
      MStripHit* YSH = GetDominantStrip(YStrips, YEnergyFraction); 

      // cout << "found the dominant strips" << endl;

      double CTD_s = 0.0;

      //now try and get z position
      int DetID = XSH->GetDetectorID();
      int XStripID = XSH->GetStripID();
      int YStripID = YSH->GetStripID();
      int pixel_code = 10000*DetID + 100*XStripID + YStripID;

      // TODO: Calculate X and Y positions more rigorously using charge sharing.
      double Xpos = m_XPitches[DetID]*((m_NXStrips[DetID]/2.0) - ((double)XStripID));
      double Ypos = m_YPitches[DetID]*((m_NYStrips[DetID]/2.0) - ((double)YStripID));
      // cout << "X position " << Xpos << endl;
      // cout << "Y position " << Ypos << endl;
      double Zpos = 0.0;

      double Xsigma = m_XPitches[DetID]/sqrt(12.0);
      double Ysigma = m_YPitches[DetID]/sqrt(12.0);
      double Zsigma = m_Thicknesses[DetID]/sqrt(12.0);

      // cout << "looking up the coefficients" << endl;
      vector<double>* Coeffs = GetPixelCoeffs(pixel_code);

      double XTiming = XSH->GetTiming();
      double YTiming = YSH->GetTiming();

      // cout << "Got the coefficients: " << Coeffs << endl;

      // If there aren't coefficients loaded, then calibration is incomplete.
      if( Coeffs == nullptr ){
        //set the bad flag for depth
        H->SetNoDepth();
        Event->SetDepthCalibrationIncomplete();
        ++m_Error1;
      }
      // If there isn't timing information, set no depth.
      // Alex's old comments suggest assigning the event to the middle of the detector and the position resolution to be large.
      else if( (XTiming < 1.0E-6) || (YTiming < 1.0E-6) ){
          // cout << "no timing info" << endl;
          ++m_Error3;
          H->SetNoDepth();
          Event->SetDepthCalibrationIncomplete();
      }

      // If there are coefficients and timing information is loaded, try calculating the CTD and depth
      else {

        vector<double> ctdvec = GetCTD(DetID, Grade);
        vector<double> depthvec = GetDepth(DetID);

      	if ( ctdvec.size() == 0){
      	  cout << "Empty CTD vector" << endl;
      	  H->SetNoDepth();
      	  Event->SetDepthCalibrationIncomplete();
      	}

        // TODO: For Card Cage, may need to add noise
        double CTD;
        if ( XSH->IsPositiveStrip() ){
          if ( m_TACCalFileIsLoaded ){
            XTiming = XTiming*m_LVTACCal[DetID][XStripID][0] + m_LVTACCal[DetID][XStripID][1];
            YTiming = YTiming*m_HVTACCal[DetID][YStripID][0] + m_HVTACCal[DetID][YStripID][1];
          }
          CTD = (YTiming - XTiming);
        }
        else {
          if ( m_TACCalFileIsLoaded ){
            XTiming = XTiming*m_HVTACCal[DetID][XStripID][0] + m_HVTACCal[DetID][XStripID][1];
            YTiming = YTiming*m_LVTACCal[DetID][YStripID][0] + m_LVTACCal[DetID][YStripID][1];
          }
          CTD = (XTiming - YTiming);
        }

        // cout << "Got the CTD: " << CTD << endl;

        // Confirmed that this matches SP's python code.
        CTD_s = (CTD - Coeffs->at(1))/(Coeffs->at(0)); //apply inverse stretch and offset

        // cout << "Transformed CTD: " << CTD_s << endl;

        double Xmin = * std::min_element(ctdvec.begin(), ctdvec.end());
        double Xmax = * std::max_element(ctdvec.begin(), ctdvec.end());

        // cout << "Got the min and max ctd values: " << Xmin << "; " << Xmax << endl;

        double noise = GetTimingNoiseFWHM(pixel_code, H->GetEnergy());

        // cout << "Got the timing noise: " << noise << endl;

        //if the CTD is out of range, check if we should reject the event.
        if( (CTD_s < (Xmin - 2.0*noise)) || (CTD_s > (Xmax + 2.0*noise)) ){
          H->SetNoDepth();
          Event->SetDepthCalibrationIncomplete();
          ++m_Error2;
        }

        // If the CTD is in range, calculate the depth
        else {
          // cout << "Calculating depth" << endl;
          // Calculate the probability given timing noise of CTD_s corresponding to the values of depth in depthvec
          // Utlize symmetry of the normal distribution.
          // TODO: Get the energy of the event and pass it to GetTimingNoiseFWHM
          vector<double> prob_dist = norm_pdf(ctdvec, CTD_s, noise/2.355);
          
          // Weight the depth by probability
	  double prob_sum = 0.0;
	  for( unsigned int k=0; k < prob_dist.size(); ++k ){
	    prob_sum += prob_dist[k];
	  }
          //double prob_sum = std::accumulate(prob_dist.begin(), prob_dist.end(), 0);
	  // cout << "summed probability: " << prob_sum << endl;
          double weighted_depth = 0.0;
          for( unsigned int k = 0; k < depthvec.size(); ++k ){
            weighted_depth += prob_dist[k]*depthvec[k];
          }
          // Calculate the expectation value of the depth
          double mean_depth = weighted_depth/prob_sum;

          // Calculate the standard deviation of the depth
          double depth_var = 0.0;
          for( unsigned int k=0; k<depthvec.size(); ++k ){
            depth_var += prob_dist[k]*pow(depthvec[k]-mean_depth, 2.0);
          }

          Zsigma =  sqrt(depth_var/prob_sum);
          // Zpos = m_Thicknesses[DetID]/2.0 - mean_depth;
          Zpos = mean_depth;
	  // cout << "calculated depth: " << Zpos << endl;
          m_NoError+=1;
        }
      }

    LocalPosition.SetXYZ(Xpos, Ypos, Zpos);
    LocalOrigin.SetXYZ(0.0,0.0,0.0);
    // TODO: Don't use the detector name
    // cout << m_DetectorNames[DetID] << endl;
    GlobalPosition = m_Geometry->GetGlobalPosition(LocalPosition, m_DetectorNames[DetID]);
    // cout << "Found the GlobalPosition" << endl;

    // Make sure XYZ resolution are correctly mapped to the global coord system.
    PositionResolution.SetXYZ(Xsigma, Ysigma, Zsigma);
    GlobalResolution = (m_Geometry->GetGlobalPosition(PositionResolution, m_DetectorNames[DetID]) - m_Geometry->GetGlobalPosition(LocalOrigin, m_DetectorNames[DetID])).Abs();
    
    // cout << "Set the PositionResolution vector" << endl;

    H->SetPosition(GlobalPosition); 

    // cout << "Set the global position for the strip hit" << endl;

    H->SetPositionResolution(GlobalResolution);

    // cout << "Set the position resolution for the strip hit" << endl;


    }
  }
  
  Event->SetAnalysisProgress(MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation);

  return true;
}

MStripHit* MModuleDepthCalibration2024::GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction)
{
  double MaxEnergy = -numeric_limits<double>::max(); // AZ: When both energies are zero (which shouldn't happen) we still pick one
  double TotalEnergy = 0.0;
  MStripHit* MaxStrip = nullptr;

  // Iterate through strip hits and get the strip with highest energy
  for (const auto SH : Strips) {
    double Energy = SH->GetEnergy();
    TotalEnergy += Energy;
    if (Energy > MaxEnergy) {
      MaxStrip = SH;
      MaxEnergy = Energy;
    }
  }
  if (TotalEnergy == 0) {
    EnergyFraction = 0;
  } else {
    EnergyFraction = MaxEnergy/TotalEnergy;
  }
  return MaxStrip;
}

double MModuleDepthCalibration2024::GetTimingNoiseFWHM(int pixel_code, double Energy)
{
  // Placeholder for determining the timing noise with energy, and possibly even on a pixel-by-pixel basis.
  // Should follow 1/E relation
  // TODO: Check that this makes sense
  if ( m_Coeffs_Energy != NULL ){
    return m_Coeffs[pixel_code][2] * m_Coeffs_Energy/Energy;
  }
  else{
    return 6.0*2.355;
  }
}

bool MModuleDepthCalibration2024::LoadCoeffsFile(MString FName)
{
  // Read in the stretch and offset file, which should have a header line with information on the measurements:
  // ### 800 V 80 K 59.5 keV
  // And which should contain for each pixel:
  // Pixel code (10000*det + 100*Xchannel + Ychannel), Stretch, Offset, Timing/CTD noise, Chi2 for the CTD fit (for diagnostics mainly)
  MFile F;
  if( F.Open(FName) == false ){
    cout << "MModuleDepthCalibration2024: failed to open coefficients file..." << endl;
    return false;
  } else {
    MString Line;
    while( F.ReadLine( Line ) ){
      if ( Line.BeginsWith('#') ){
        std::vector<MString> Tokens = Line.Tokenize(" ");
        m_Coeffs_Energy = Tokens[5].ToDouble();
        cout << "The stretch and offset were calculated for " << m_Coeffs_Energy << " keV." << endl;
      }
      else {
        std::vector<MString> Tokens = Line.Tokenize(",");
        if( Tokens.size() == 5 ){
          int pixel_code = Tokens[0].ToInt();
          double Stretch = Tokens[1].ToDouble();
          double Offset = Tokens[2].ToDouble();
          double CTD_FWHM = Tokens[3].ToDouble() * 2.355;
          double Chi2 = Tokens[4].ToDouble();
          // Previous iteration of depth calibration read in "Scale" instead of ctd resolution.
          vector<double> coeffs;
          coeffs.push_back(Stretch); coeffs.push_back(Offset); coeffs.push_back(CTD_FWHM); coeffs.push_back(Chi2);
          m_Coeffs[pixel_code] = coeffs;
        }
      }
    }
    F.Close();
  }

  m_CoeffsFileIsLoaded = true;

  return true;

}

bool MModuleDepthCalibration2024::LoadTACCalFile(MString FName)
{
  // Read in the TAC Calibration file, which should contain for each strip:
  //  DetID, h or l for high or low voltage, TAC cal, TAC cal error, TAC cal offset, TAC offset error
  MFile F;
  if( F.Open(FName) == false ){
    cout << "MModuleDepthCalibration2024: failed to open TAC Calibration file." << endl;
    m_TACCalFileIsLoaded = false;
    return false;
  } else {
    for(unsigned int i = 0; i < m_Detectors.size(); ++i){
      unordered_map<int, vector<double>> temp_map_HV;
      m_HVTACCal[i] = temp_map_HV;
      unordered_map<int, vector<double>> temp_map_LV;
      m_LVTACCal[i] = temp_map_LV;
    }
    MString Line;
    while( F.ReadLine( Line ) ){
      if( !Line.BeginsWith("#") ){
        std::vector<MString> Tokens = Line.Tokenize(",");
        if( Tokens.size() == 7 ){
          int DetID = Tokens[0].ToInt();
          int StripID = Tokens[2].ToInt();
          double taccal = Tokens[3].ToDouble();
          double taccal_err = Tokens[4].ToDouble();
          double offset = Tokens[5].ToDouble();
          double offset_err = Tokens[6].ToDouble();
          vector<double> cal_vals;
          cal_vals.push_back(taccal); cal_vals.push_back(offset); cal_vals.push_back(taccal_err); cal_vals.push_back(offset_err);
          if ( Tokens[1] == "l" or Tokens[1] == "p" ){
            m_LVTACCal[DetID][StripID] = cal_vals;
          }
          else if ( Tokens[1] == "h" or Tokens[1] == "n" ){
            m_HVTACCal[DetID][StripID] = cal_vals;
          }
        }
      }
    }
    F.Close();
  }

  m_TACCalFileIsLoaded = true;

  return true;

}


std::vector<double>* MModuleDepthCalibration2024::GetPixelCoeffs(int pixel_code)
{
  // Check to see if the stretch and offset have been loaded. If so, try to get the coefficients for the specified pixel.
  if( m_CoeffsFileIsLoaded ){
    if( m_Coeffs.count(pixel_code) > 0 ){
      return &m_Coeffs[pixel_code];
    } else {
      cout << "MModuleDepthCalibration2024::GetPixelCoeffs: cannot get stretch and offset; pixel code " << pixel_code << " not found." << endl;
      return nullptr;
    }
  } else {
    cout << "MModuleDepthCalibration2024::GetPixelCoeffs: cannot get stretch and offset; file has not yet been loaded." << endl;
    return nullptr;
  }

}

vector<double> MModuleDepthCalibration2024::norm_pdf(vector<double> x, double mu, double sigma)
{
  vector<double> result;
  for( unsigned int i=0; i<x.size(); ++i ){
    double prob = 1.0 / (sigma * sqrt(2.0 * M_PI)) * exp(-(pow((x[i] - mu)/sigma, 2.0)/2.0));
    // cout << "Probability: " << prob << endl;
    result.push_back(prob);
  }
  return result;
}

bool MModuleDepthCalibration2024::LoadSplinesFile(MString FName)
{
  //when invert flag is set to true, the splines returned are CTD->Depth
  // Previously saved cathode and anode timing in addition to CTD. This may be redundant, commenting out for now.
  // Input spline files should have the following format:
  // ### DetID, HV, Temperature, Photopeak Energy (TODO: More? Fewer?)
  // depth, ctd0, ctd1, ctd2.... (Basically, allow for CTDs for different subpixel regions)
  // '' '' ''
  MFile F; 
  if( F.Open(FName) == false ){
    return false;
  }
  // vector<double> depthvec, ctdvec, anovec, catvec;
  vector<double> depthvec;
  vector<vector<double>> ctdarr;
  for( unsigned int i=0; i < 5; ++i ){
    vector<double> temp_vec;
    ctdarr.push_back(temp_vec);
  }
  MString line;
  int DetID, NewDetID;
  while( F.ReadLine(line) ){
    if( line.Length() != 0 ){
      if( line.BeginsWith("#") ){
        // If we've reached a new ctd spline then record the previous one in the m_SplineMaps and start a new one.
        vector<MString> tokens = line.Tokenize(" ");
        NewDetID = tokens[1].ToInt();
        if( depthvec.size() > 0 ) {
          AddDepthCTD(depthvec, ctdarr, DetID, m_DepthGrid, m_CTDMap);        
        }
        depthvec.clear(); ctdarr.clear(); 
        for( unsigned int i=0; i < 5; ++i ){
            vector<double> temp_vec;
            ctdarr.push_back(temp_vec);
        }
        DetID = NewDetID;
      } else {
        vector<MString> tokens = line.Tokenize(",");
        depthvec.push_back(tokens[0].ToDouble());

        // Multiple CTDs allowed.
        for( unsigned int i = 0; i < (tokens.size() - 1); ++i ){
          ctdarr[i].push_back(tokens[1+i].ToDouble());
        }
        // Fill in the higher grades with the GRADE=0 CTD if there are none listed in the file.
        for(unsigned int i=tokens.size()-1; i<5; ++i){
          ctdarr[i].push_back(tokens[1].ToDouble());
        }
      }
    }
  }
  //make last spline
  if( depthvec.size() > 0 ){
    AddDepthCTD(depthvec, ctdarr, DetID, m_DepthGrid, m_CTDMap);
  }

  m_SplinesFileIsLoaded = true;
  return true;

}

int MModuleDepthCalibration2024::GetHitGrade(MHit* H){
  // Function for choosing which Depth-to-CTD relation to use for a given event.
  // At time of writing, intention is to choose a CTD based on sub-pixel region determined via charge sharing (Event "grade").
  // 5 possible grades, and one Error Grade, -1. GRADE 4 is as yet uncategorized complicated geometry. GRADE 5 means multiple, presumably separated strip hits.

  //organize x and y strips into vectors
  if( H == NULL ){
    return -1;
  }
  if( H->GetNStripHits() == 0 ){
    // Error if no strip hits listed. Bad grade is returned
    cout << "ERROR in MModuleDepthCalibration2024: HIT WITH NO STRIP HITS" << endl;
    return -1;
  }
   
  // Take a Hit and separate its activated p and n strips into separate vectors.
  std::vector<MStripHit*> PStrips;
  std::vector<MStripHit*> NStrips;
  vector<int> PStripIDs;
  vector<int> NStripIDs;
  for( unsigned int j = 0; j < H->GetNStripHits(); ++j){
    MStripHit* SH = H->GetStripHit(j);
    if( SH == NULL ) { cout << "ERROR in MModuleDepthCalibration2024: Depth Calibration: got NULL strip hit :( " << endl; return -1;}
    if( SH->GetEnergy() == 0 ) { cout << "ERROR in MModuleDepthCalibration2024: Depth Calibration: got strip without energy :( " << endl; return -1;}
    if( SH->IsPositiveStrip() ){
      PStrips.push_back(SH); 
      PStripIDs.push_back(SH->GetStripID());
    }
    else {
      NStrips.push_back(SH);
      NStripIDs.push_back(SH->GetStripID());
    }
  }

  // if the same strip has multiple hits, this is a bad grade.
  bool MultiHitX = H->GetStripHitMultipleTimesX();
  bool MultiHitY = H->GetStripHitMultipleTimesY();
  if( MultiHitX || MultiHitY ){
    return 5;  
  }

  if( PStrips.size()>0 && NStrips.size()>0 ){
    int Nmin = * std::min_element(NStripIDs.begin(), NStripIDs.end());
    int Nmax = * std::max_element(NStripIDs.begin(), NStripIDs.end());

    int Pmin = * std::min_element(PStripIDs.begin(), PStripIDs.end());
    int Pmax = * std::max_element(PStripIDs.begin(), PStripIDs.end());

    // If the strip hits are not all adjacent, it's a bad grade.
    if ( ((Nmax - Nmin) >= (NStrips.size())) || ((Pmax - Pmin) >= (PStrips.size())) ){
      return 6;
    }
  }
  else{
    // cout << "ERROR in MModuleDepthCalibration2024: HIT with no strip hits on one side" << endl;
    return -1;
  }

  int return_value;
  // If 1 strip on each side, GRADE=0
  // This represents the center of the pixel
  if( (PStrips.size() == 1) && (NStrips.size() == 1) ){
    return_value = 0;
  } 
  // If 2 hits on N side and 1 on P, GRADE=1
  // This represents the middle of the edges of the pixel
  else if( (PStrips.size() == 1) && (NStrips.size() == 2) ){
    return_value = 1;
  } 

  // If 2 hits on P and 1 on N, GRADE=2
  // This represents the middle of the edges of the pixel
  else if( (PStrips.size() == 2) && (NStrips.size() == 1) ){
    return_value = 2;
  } 
  
  // If 2 strip hits on both sides, GRADE=3
  // This represents the corners the pixel
  else if( (PStrips.size() == 2) && (NStrips.size() == 2) ){
    return_value = 3;
  } 

  // If 3 hits on N side and 1 on P, GRADE=0
  // This represents the middle of the pixel, near the p (LV) side of the detector.
  else if( (PStrips.size() == 1) && (NStrips.size() == 3) ){
    return_value = 0;
  } 

  // If 3 hits on P and 1 on N, GRADE=0
  // This represents the middle of the pixel, near the n (HV) side of the detector.
  else if( (PStrips.size() == 3) && (NStrips.size() == 1) ){
    return_value = 0;
  } 

  // If 3 hits on N side and 2 on P, GRADE=0
  // This represents the middle of the edge of the pixel, near the p (LV) side of the detector.
  else if( (PStrips.size() == 2) && (NStrips.size() == 3) ){
    return_value = 2;
  } 

  // If 3 hits on P and 2 on N, GRADE=0
  // This represents the middle of the edge of the pixel, near the n (HV) side of the detector.
  else if( (PStrips.size() == 3) && (NStrips.size() == 2) ){
    return_value = 1;
  } 

  else {
    // If more complicated than the above cases, return 4 for now.
    // TODO: Handle more complicated charge distributions.
    return_value = 4;
  }

  return return_value;
}

bool MModuleDepthCalibration2024::AddDepthCTD(vector<double> depthvec, vector<vector<double>> ctdarr, int DetID, unordered_map<int, vector<double>>& DepthGrid, unordered_map<int,vector<vector<double>>>& CTDMap){

  // Saves a CTD array, basically allowing for multiple CTDs as a function of depth 
  // depthvec: list of simulated depth values
  // ctdarr: vector of vectors of simulated CTD values. Each vector of CTDs must be the same length as depthvec
  // DetID: An integer which specifies which detector.
  // CTDMap: unordered map into which the array of CTDs should be placed

  // TODO: Possible energy dependence of CTD?
  // TODO: Depth values need to be evenly spaced. Check this when reading the files in.

  // Check to make sure things look right.
  // First check that the CTDs all have the right length.
  for( unsigned int i = 0; i < ctdarr.size(); ++i ){
    if( (ctdarr[i].size() != depthvec.size()) && (ctdarr[i].size() > 0) ){
      cout << "MModuleDepthCalibration2024::AddDepthCTD: The number of values in the CTD list is not equal to the number of depth values." << endl;
      return false;
    }
  }
  // Now make sure the values for the depth start with 0.0.
  // if( * std::min_element(depthvec.begin(), depthvec.end()) != 0.0){
  //     cout << "MModuleDepthCalibration2024::AddDepthCTD: The minimum depth is not zero." << endl;
  //     return false;
  // }

  CTDMap[DetID] = ctdarr;
  DepthGrid[DetID] = depthvec;
  double maxdepth = * std::max_element(depthvec.begin(), depthvec.end());
  double mindepth = * std::min_element(depthvec.begin(), depthvec.end());
  m_Thicknesses[DetID] = maxdepth-mindepth;
  cout << "MModuleDepthCalibration2024::AddDepthCTD: The thickness of detector " << DetID << " is " << m_Thicknesses[DetID] << endl;
  return true;
}


vector<double> MModuleDepthCalibration2024::GetCTD(int DetID, int Grade)
{
  // Retrieves the appropriate CTD vector given the Detector ID and Event Grade passed

  if( !m_SplinesFileIsLoaded ){
    cout << "MModuleDepthCalibration2024::GetCTD: cannot return Depth to CTD relation because the file was not loaded." << endl;
    return vector<double> ();
  }
  // If there is a CTD array for the given detector, return it.
  // If the Grade is larger than the number of CTD vectors stored, then just return Grade 0 vector.
  if( m_CTDMap.count(DetID) > 0 ){
    if ( ((int)m_CTDMap[DetID].size()) > Grade) {
      return (m_CTDMap[DetID][Grade]);
    }
    else {
      cout << "MModuleDepthCalibration2024::GetCTD: No CTD map is loaded for Grade " << Grade << ". Returning Grade 0 CTD." << endl;
      return (m_CTDMap[DetID][0]);
    }
  } else {
    cout << "MModuleDepthCalibration2024::GetCTD: No CTD map is loaded for Det " << DetID << "." << endl;
    return vector<double> ();
  }
}

vector<double> MModuleDepthCalibration2024::GetDepth(int DetID)
{
  // Retrieves the appropriate CTD vector given the Detector ID and Event Grade passed

  if( !m_SplinesFileIsLoaded ){
    cout << "MModuleDepthCalibration2024::GetDepth: cannot return Depth grid because the file was not loaded." << endl;
    return vector<double> ();
  }

  // If there is a CTD array for the given detector, return it.
  // If the Grade is larger than the number of CTD vectors stored, then just return Grade 0 vector.
  if( m_DepthGrid.count(DetID) > 0 ){
    return m_DepthGrid[DetID];
    } else {
      cout << "MModuleDepthCalibration2024::GetDepth: No Depth grid is loaded for Det " << DetID << "." << endl;
      return vector<double> ();
  }
} 

void MModuleDepthCalibration2024::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
  MGUIOptionsDepthCalibration2024* Options = new MGUIOptionsDepthCalibration2024(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


bool MModuleDepthCalibration2024::ReadXmlConfiguration(MXmlNode* Node)
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

  MXmlNode* TACCalFileNameNode = Node->GetNode("TACCalFileName");
  if (TACCalFileNameNode != 0) {
    m_TACCalFile = TACCalFileNameNode->GetValue();
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////

MXmlNode* MModuleDepthCalibration2024::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0,m_XmlTag);
  new MXmlNode(Node, "CoeffsFileName", m_CoeffsFile);
  new MXmlNode(Node, "SplinesFileName", m_SplinesFile);
  new MXmlNode(Node, "TACCalFileName", m_TACCalFile);

  return Node;
}

void MModuleDepthCalibration2024::Finalize()
{

  MModule::Finalize();
  cout << "###################" << endl;
  cout << "AWL depth cal stats" << endl;
  cout << "###################" << endl;
  cout << "Good hits: " << m_NoError << endl;
  cout << "Number of hits missing calibration coefficients: " << m_Error1 << endl;
  cout << "Number of hits too far outside of detector: " << m_Error2 << endl;
  cout << "Number of hits missing timing information: " << m_Error3 << endl;
  cout << "Number of hits with strips hit multiple times: " << m_Error5 << endl;
  cout << "Number of hits with non-adjacent strip hits: " << m_Error6 << endl;
  cout << "Number of hits with too many strip hits: " << m_Error4 << endl;
  cout << "Number of hits with no strip hits on one or both sides: " << m_ErrorSH << endl;
  /*
  TFile* rootF = new TFile("EHist.root","recreate");
  rootF->WriteTObject( EHist );
  rootF->Close();
  */

}



// MModuleDepthCalibration2024.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
