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
#include "TMath.h"
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
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration, true);
  AddPreceedingModuleType(MAssembly::c_StripPairing, true);
  AddPreceedingModuleType(MAssembly::c_TACcut, true);
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

  m_Coeffs_Energy = 0;

  m_NoError = 0;
  m_Error1 = 0;
  m_Error2 = 0;
  m_Error3 = 0;
  m_Error4 = 0;
  m_Error5 = 0;
  m_Error6 = 0;
  m_ErrorSH = 0;
  m_ErrorNullSH=0;
  m_ErrorNoE=0;
}


////////////////////////////////////////////////////////////////////////////////


MModuleDepthCalibration2024::~MModuleDepthCalibration2024()
{
  // Delete this instance of MModuleDepthCalibration2024
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::Initialize()
{

  // The detectors need to be in the same order as DetIDs.
  // ie DetID=0 should be the 0th detector in m_Detectors, DetID=1 should the 1st, etc.
  vector<MDDetector*> DetList = m_Geometry->GetDetectorList();

  // Look through the Geometry and get the names and thicknesses of all the detectors.
  for (unsigned int i = 0; i < DetList.size(); ++i) {
    // For now, DetID is in order of detectors, which puts contraints on how the geometry file should be written.
    // If using the card cage at UCSD, default to DetID=11.
    unsigned int DetID = i;
    if (m_UCSDOverride == true) {
      DetID = 11;
    }

    MDDetector* det = DetList[i];
    vector<string> DetectorNames;
    if (det->GetTypeName() == "Strip3D") {
      if (det->GetNSensitiveVolumes() == 1) {
        MDVolume* vol = det->GetSensitiveVolume(0);
        string det_name = vol->GetName().GetString();
        if (find(DetectorNames.begin(), DetectorNames.end(), det_name) == DetectorNames.end()) {
          DetectorNames.push_back(det_name);
          m_Thicknesses[DetID] = 2 * (det->GetStructuralSize().GetZ());
          MDStrip3D* strip = dynamic_cast<MDStrip3D*>(det);
          m_XPitches[DetID] = strip->GetPitchX();
          m_YPitches[DetID] = strip->GetPitchY();
          m_NXStrips[DetID] = strip->GetNStripsX();
          m_NYStrips[DetID] = strip->GetNStripsY();
          cout << "Found detector " << det_name << " corresponding to DetID=" << DetID << "." << endl;
          cout << "Detector thickness: " << m_Thicknesses[DetID] << endl;
          cout << "Number of X strips: " << m_NXStrips[DetID] << endl;
          cout << "Number of Y strips: " << m_NYStrips[DetID] << endl;
          cout << "X strip pitch: " << m_XPitches[DetID] << endl;
          cout << "Y strip pitch: " << m_YPitches[DetID] << endl;
          m_DetectorIDs.push_back(DetID);
          m_Detectors[DetID] = det;
        } else {
          cout<<"ERROR in MModuleDepthCalibration2024::Initialize: Found a duplicate detector: "<<det_name<<endl;
        }
      } else {
        cout<<"ERROR in MModuleDepthCalibration2024::Initialize: Found a Strip3D detector with "<<det->GetNSensitiveVolumes()<<" Sensitive Volumes."<<endl;
      }
    }
  }

  if (m_DetectorIDs.size() == 0) {
    cout<<"No Strip3D detectors were found."<<endl;
    return false; 
  }

  m_CoeffsFileIsLoaded = LoadCoeffsFile(m_CoeffsFile);
  if (m_CoeffsFileIsLoaded == false) {
    return false;
  }
  m_SplinesFileIsLoaded = LoadSplinesFile(m_SplinesFile);
  if (m_SplinesFileIsLoaded == false) {
    return false;
  }

  if (m_MaskMetrologyEnabled == true) {
    if (g_Verbosity >= c_Info) cout << m_XmlTag << ": !!! Mask Metrology Enabled !!!" << endl;
    m_MaskMetrologyFileIsLoaded = LoadMaskMetrologyFile(m_MaskMetrologyFile);
    if (m_MaskMetrologyFile == false) {
      if (g_Verbosity >= c_Error) cout << m_XmlTag << "Unable to open Metrology file" << endl;
      return false;
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

void MModuleDepthCalibration2024::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoDepthCalibration = new MGUIExpoDepthCalibration2024(this);
  m_ExpoDepthCalibration->SetDepthHistogramArrangement(&m_DetectorIDs);
  for (unsigned int i = 0; i < m_DetectorIDs.size(); ++i){
    unsigned int DetID = m_DetectorIDs[i];
    double thickness = m_Thicknesses[DetID];
    m_ExpoDepthCalibration->SetDepthHistogramParameters(DetID, 120, -thickness/2.0,thickness/2.0);
  }
  m_Expos.push_back(m_ExpoDepthCalibration);
}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::AnalyzeEvent(MReadOutAssembly* Event) 
{
  
  if (Event->GetGuardRingVeto() == true) {
    //TODO: Handle events with GR vetos    

    Event->SetDepthCalibrationIncomplete();
    return false;
  
  } else {
    
    for (unsigned int i = 0; i < Event->GetNHits(); ++i ){
      // Each event represents one photon. It contains Hits, representing interaction sites.
      // H is a pointer to an instance of the MHit class. Each Hit has activated strips, represented by
      // instances of the MStripHit class.
      MHit* H = Event->GetHit(i);

      int Grade = GetHitGrade(H);

      // Handle different grades differently    
      // GRADE=-1 is an error. Break from the loop and continue.
      if (Grade < 0){
        H->SetNoDepth();
        Event->SetDepthCalibrationIncomplete();
        if (Grade == -1) {
          ++m_ErrorSH;
        } else if (Grade == -2) {
          ++m_ErrorNullSH;
        } else if (Grade == -3) {
          ++m_ErrorNoE;
        }
      } else if (Grade > 4) { // GRADE=5 is some complicated geometry with multiple hits on a single strip. GRADE=6 means not all strips are adjacent.
        H->SetNoDepth();
        Event->SetDepthCalibrationIncomplete();
        if (Grade==5) {
          ++m_Error5;
        } else if (Grade==6) {
          ++m_Error6;
        }
      } else { // If the Grade is 0-4, we can handle it.

        // Calculate the position. If error is thrown, record and no depth.
        // Take a Hit and separate its activated X- and Y-strips into separate vectors.
        vector<MStripHit*> LVStrips;
        vector<MStripHit*> HVStrips;

        for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
          MStripHit* SH = H->GetStripHit(j);
          if (SH->IsLowVoltageStrip()) LVStrips.push_back(SH); else HVStrips.push_back(SH);
        }

        double LVEnergyFraction;
        double HVEnergyFraction;
        MStripHit* LVSH = GetDominantStrip(LVStrips, LVEnergyFraction); 
        MStripHit* HVSH = GetDominantStrip(HVStrips, HVEnergyFraction); 

        double CTD_s = 0.0;

        //now try and get z position
        int DetID = LVSH->GetDetectorID();
        int LVStripID = LVSH->GetStripID();
        int HVStripID = HVSH->GetStripID();
        int PixelCode = 10000*DetID + 100*LVStripID + HVStripID;

       //Define the X/Y positions based on the detector pitch and number of strip hits
        // LV strip 0 is in -ve X direction, HV strip 0 is in -ve Y direction.
        // Confusingly, the strips parallel to the Y axis determines the X position, and the "X strips" determine the Y position
        double Xpos = m_YPitches[DetID]*((double)LVStripID - ((m_NYStrips[DetID]-1)/2.0));
        double Ypos = m_XPitches[DetID]*((double)HVStripID - ((m_NXStrips[DetID]-1)/2.0));
        double Zpos = 0.0;

        cout<<"Initial position: Xpos "<<Xpos<<", Ypos: "<<Ypos<<endl;

        // TODO: Confirm X and Y implementation below with Aldo's new metrology files. His old files swapped these.
        if (m_MaskMetrologyEnabled == true) {
          // If we are applying the mask metrology correction, first define two new readout elements to help determine the intersection of these two strips
          MReadOutElementDoubleStrip R_LV = *dynamic_cast<MReadOutElementDoubleStrip*>(LVSH->GetReadOutElement());
          MReadOutElementDoubleStrip R_HV = *dynamic_cast<MReadOutElementDoubleStrip*>(HVSH->GetReadOutElement());

          //find the intercept of the two dominate strips based on the mask metrology, and update Xpos and Ypos	  
          vector<double> inter = GetStripIntersection(R_LV, R_HV);
          Xpos = inter[0];
          Ypos = inter[1];

	}


        // TODO: Calculate X and Y positions more rigorously using charge sharing.

        double Xsigma = m_YPitches[DetID]/sqrt(12.0);
        double Ysigma = m_XPitches[DetID]/sqrt(12.0);
        double Zsigma = m_Thicknesses[DetID]/sqrt(12.0);

        vector<double>* Coeffs = GetPixelCoeffs(PixelCode);

        vector<double> CTDVec = GetCTD(DetID, Grade);
        vector<double> DepthVec = GetDepth(DetID);

        // TODO: For Card Cage, may need to add noise
        double LVTiming = LVSH->GetTiming();
        double HVTiming = HVSH->GetTiming();

        // If there aren't coefficients loaded, then calibration is incomplete.
        if (Coeffs == nullptr){
          //set the bad flag for depth
          H->SetNoDepth();
          Event->SetDepthCalibrationIncomplete();
          ++m_Error1;
        } else if (CTDVec.size() == 0) {
            cout << "Empty CTD vector" << endl;
            H->SetNoDepth();
            Event->SetDepthCalibrationIncomplete();
        } else if (DepthVec.size() == 0) {
            cout << "Empty Depth vector" << endl;
            H->SetNoDepth();
            Event->SetDepthCalibrationIncomplete();
        } else if ((LVTiming < 1.0E-6) || (HVTiming < 1.0E-6)) {
            ++m_Error3;
            H->SetNoDepth();
            Event->SetDepthCalibrationIncomplete();
        } else {
          
          // If there are coefficients and timing information is loaded, try calculating the CTD and depth
          double CTD = (HVTiming - LVTiming);

          // Confirmed that this matches SP's python code.
          CTD_s = (CTD - Coeffs->at(1))/(Coeffs->at(0)); //apply inverse stretch and offset

          double Xmin = * std::min_element(CTDVec.begin(), CTDVec.end());
          double Xmax = * std::max_element(CTDVec.begin(), CTDVec.end());

          double noise = GetTimingNoiseFWHM(PixelCode, H->GetEnergy());

          //if the CTD is out of range, check if we should reject the event.
          if ((CTD_s < (Xmin - 2.0*noise)) || (CTD_s > (Xmax + 2.0*noise))) {
            H->SetNoDepth();
            Event->SetDepthCalibrationIncomplete();
            ++m_Error2;
          }

          // If the CTD is in range, calculate the depth
          // Rather than plugging CTD into a spline to get depth, use the depth-CTD relation to calculate a probability-weighted depth value.
          // This way we can avoid problems like non-monotonicity or assigning depth to events "outside" the detector 
          // Note that this requires that we don't massively overestimate the timing noise
          else {
            // Calculate the probability given timing noise of CTD_s corresponding to the values of depth in DepthVec
            // Utlize symmetry of the normal distribution.
            vector<double> prob_dist = norm_pdf(CTDVec, CTD_s, noise/2.355);
            
            // Weight the depth by probability
        	  double prob_sum = 0.0;
        	  for (unsigned int k=0; k < prob_dist.size(); ++k) {
        	    prob_sum += prob_dist[k];
        	  }
            double weighted_depth = 0.0;

            for (unsigned int k = 0; k < DepthVec.size(); ++k) {
              weighted_depth += prob_dist[k] * DepthVec[k];
            }

            // Calculate the expectation value of the depth
            double mean_depth = weighted_depth/prob_sum;

            // Calculate the standard deviation of the depth
            double depth_var = 0.0;

            for (unsigned int k=0; k < DepthVec.size(); ++k) {
              depth_var += prob_dist[k] * pow(DepthVec[k] - mean_depth, 2.0);
            }

            Zsigma =  sqrt(depth_var/prob_sum);
            Zpos = mean_depth;
            //Zpos = mean_depth - (m_Thicknesses[DetID]/2.0);

            // Add the depth to the GUI histogram.
            if (Event->IsStripPairingIncomplete()==false) {
              if (HasExpos() == true) {
                m_ExpoDepthCalibration->AddDepth(DetID, Zpos);
              }
            }
            m_NoError+=1;
          }
        }

      //cout << "Strip ID :" << LVStripID << " " << HVStripID << endl;
      //cout << "Hit position: "<< Xpos << " " << Ypos << " " << Zpos << endl;

      MVector LocalPosition(Xpos, Ypos, Zpos);
      MVector LocalOrigin(0.0, 0.0, 0.0);
      MVector GlobalPosition = m_Detectors[DetID]->GetSensitiveVolume(0)->GetPositionInWorldVolume(LocalPosition);

      // Make sure XYZ resolution are correctly mapped to the global coord system.
      MVector PositionResolution(Xsigma, Ysigma, Zsigma);
      MVector GlobalResolution = ((m_Detectors[DetID]->GetSensitiveVolume(0)->GetPositionInWorldVolume(PositionResolution)) - (m_Detectors[DetID]->GetSensitiveVolume(0)->GetPositionInWorldVolume(LocalOrigin))).Abs();
      
      H->SetPosition(GlobalPosition); 

      H->SetPositionResolution(GlobalResolution);



      }
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation);

  return true;
}


/////////////////////////////////////////////////////////////////////////////////


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


/////////////////////////////////////////////////////////////////////////////////


double MModuleDepthCalibration2024::GetTimingNoiseFWHM(int PixelCode, double Energy)
{
  // Placeholder for determining the timing noise with energy, and possibly even on a pixel-by-pixel basis.
  // Should follow 1/E relation
  // TODO: Determine real energy dependence and implement it here.
  double noiseFWHM = 0.0;
  if (m_Coeffs_Energy != 0) {
    noiseFWHM = m_Coeffs[PixelCode][2] * m_Coeffs_Energy/Energy;
    if (noiseFWHM < 3.0*2.355) {
      noiseFWHM = 3.0*2.355;
    }
  } else {
    noiseFWHM = 6.0*2.355;
  }
  return noiseFWHM;
}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::LoadCoeffsFile(MString FileName)
{
  // Read in the stretch and offset file, which should have a header line with information on the measurements:
  // ### 800 V 80 K 59.5 keV
  // And which should contain for each pixel:
  // Pixel code (10000*det + 100*LVStrip + HVStrip), Stretch, Offset, Timing/CTD noise, Chi2 for the CTD fit (for diagnostics mainly)

  MFile CoeffsFile;
  if (CoeffsFile.Open(FileName) == false) {
    cout << "ERROR in MModuleDepthCalibration2024::LoadCoeffsFile: failed to open coefficients file." << endl;
    return false;
  }

  MString Line;
  while (CoeffsFile.ReadLine(Line)) {
    if (Line.BeginsWith('#') == true) {
      std::vector<MString> Tokens = Line.Tokenize(" ");
      m_Coeffs_Energy = Tokens[5].ToDouble();
      cout << "The stretch and offset were calculated for " << m_Coeffs_Energy << " keV." << endl;
    } else {
      std::vector<MString> Tokens = Line.Tokenize(",");
      if (Tokens.size() == 5) {
        int PixelCode = Tokens[0].ToInt();
        double Stretch = Tokens[1].ToDouble();
        double Offset = Tokens[2].ToDouble();
        double CTD_FWHM = Tokens[3].ToDouble() * 2.355;
        double Chi2 = Tokens[4].ToDouble();
        // Previous iteration of depth calibration read in "Scale" instead of ctd resolution.
        vector<double> coeffs;
        coeffs.push_back(Stretch); coeffs.push_back(Offset); coeffs.push_back(CTD_FWHM); coeffs.push_back(Chi2);
        m_Coeffs[PixelCode] = coeffs;
      }
    }
  }

  CoeffsFile.Close();

  return true;

}


/////////////////////////////////////////////////////////////////////////////////


std::vector<double>* MModuleDepthCalibration2024::GetPixelCoeffs(int PixelCode)
{
  // Check to see if the stretch and offset have been loaded. If so, try to get the coefficients for the specified pixel.
  if (m_CoeffsFileIsLoaded == true) {
    if (m_Coeffs.count(PixelCode) > 0) {
      return &m_Coeffs[PixelCode];
    } else {
     // cout << "MModuleDepthCalibration2024::GetPixelCoeffs: cannot get stretch and offset; pixel code " << PixelCode << " not found." << endl;
      return nullptr;
    }
  } else {
    cout << "MModuleDepthCalibration2024::GetPixelCoeffs: cannot get stretch and offset; file has not yet been loaded." << endl;
    return nullptr;
  }

}


/////////////////////////////////////////////////////////////////////////////////


vector<double> MModuleDepthCalibration2024::norm_pdf(vector<double> x, double mu, double sigma)
{
  vector<double> result;
  for (unsigned int i=0; i<x.size(); ++i) {
    double prob = 1.0 / (sigma * sqrt(2.0 * M_PI)) * exp(-(pow((x[i] - mu)/sigma, 2.0)/2.0));
    result.push_back(prob);
  }
  return result;
}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::LoadSplinesFile(MString FileName)
{
  // Input spline files should have the following format:
  // ### DetID, HV, Temperature, Photopeak Energy (TODO: More? Fewer?)
  // depth, ctd0, ctd1, ctd2.... (Basically, allow for CTDs for different subpixel regions)
  // '' '' ''
  MFile SplineFile; 
  if (SplineFile.Open(FileName) == false) {
    cout << "ERROR in MModuleDepthCalibration2024::LoadSplinesFile: failed to open splines file." << endl;
    return false;
  }

  vector<double> DepthVec;
  vector<vector<double>> CTDArr;

  bool Result = true;
  MString Line;
  int DetID = 0;
  while (SplineFile.ReadLine(Line)) {
    if (Line.Length() != 0) {
      if (Line.BeginsWith("#") == true) {
        // If we've reached a new ctd spline then record the previous one in the m_SplineMaps and start a new one.
        vector<MString> tokens = Line.Tokenize(" ");

        if (DepthVec.size() > 0) {
          Result &= AddDepthCTD(DepthVec, CTDArr, DetID, m_DepthGrid, m_CTDMap, m_SplineMap, 1000);
        }

        DepthVec.clear(); CTDArr.clear(); 
        DetID = tokens[1].ToInt();

      } else {
        vector<MString> tokens = Line.Tokenize(",");
        DepthVec.push_back(tokens[0].ToDouble());

        // Multiple CTDs allowed.
        for (unsigned int i = 0; i < (tokens.size() - 1); ++i) {
          while (i>=CTDArr.size()) {
            vector<double> TempVec;
            CTDArr.push_back(TempVec);
          }
          CTDArr[i].push_back(tokens[1+i].ToDouble());
        }
      }
    }
  }

  SplineFile.Close();

  //make last spline
  if (DepthVec.size() > 0) {
    Result &= AddDepthCTD(DepthVec, CTDArr, DetID, m_DepthGrid, m_CTDMap, m_SplineMap, 1000);
  }

  return Result;

}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::LoadMaskMetrologyFile(MString FileName)
{
  //Read the Mask Metrology File
  // Det ID, Side (l,h), Strip ID (0-63), x_mm, y_mm, z_mm, roll_deg, pitch_deg, yaw_deg
  MFile MetrologyFile;
  if (MetrologyFile.Open(FileName) == false) {
    cout << "ERROR in MModuleDepthCalibration2024::LoadMaskMetrologyFile: failed to open metrology file." << endl;
    return false;
  } 

  MString Line;
  while (MetrologyFile.ReadLine(Line)) {
    if (Line.BeginsWith('#') == true) continue;
    else {
      std::vector<MString> Tokens = Line.Tokenize(",");
      if (Tokens.size() == 9) {
        //Define the readout element to track det ID, strip ID, and lv/hv
        MReadOutElementDoubleStrip R;
        R.SetDetectorID(Tokens[0].ToInt());
        R.IsLowVoltageStrip((Tokens[1].ToString() == "p") || (Tokens[1].ToString() == "l"));
        R.SetStripID(Tokens[2].ToInt());
        double Strip_MetX = Tokens[3].ToDouble()/10; //convert to cm
        double Strip_MetY = Tokens[4].ToDouble()/10; //convert to cm
        double Strip_MetZ = Tokens[5].ToDouble()/10; //convert to cm
        double Strip_Roll = Tokens[6].ToDouble();
        double Strip_Pitch = Tokens[7].ToDouble();
        double Strip_Yaw = Tokens[8].ToDouble();
        vector<double> maskmet;
        maskmet.push_back(Strip_MetX); maskmet.push_back(Strip_MetY); maskmet.push_back(Strip_MetZ); 
        maskmet.push_back(Strip_Roll); maskmet.push_back(Strip_Pitch); maskmet.push_back(Strip_Yaw); 

        //make the map that defines the metrology info for each readout element
        m_MaskMetrology[R] = maskmet;
      } else {
        cout << "ERROR in MModuleDepthCalibration2024::LoadMaskMetrologyFile: incorrect number of tokens in the file." << endl;
        return false;
      }
    }
  }

  MetrologyFile.Close();

  return true;

}


/////////////////////////////////////////////////////////////////////////////////


vector<double> MModuleDepthCalibration2024::GetStripIntersection(MReadOutElementDoubleStrip R_LVStrip, MReadOutElementDoubleStrip R_HVStrip){
  // Function to find the intersection between two strips based on the Mask Metrology

  // Find the x position of two lines represented by the dominate strips:
  // LVstrip is centered at (x,y,z) = (lv_strip_met[0], lv_strip_met[1], lv_strip_met[2])
  // and is approximately parallel to the y axis, but rotated at angle lv_strip_met[5] 
  // around the z axis of the detector
  // HVstrip is centered at (x,y,z) = (hv_strip_met[0], hv_strip_met[1], hv_strip_met[2])
  // and is approximately parallel to the x axis, but rotated at angle (hv_strip_met[5] - pi/2) 
  // around the z axis of the detector
 
  vector<double> LVStripMet = m_MaskMetrology[R_LVStrip];
  vector<double> HVStripMet = m_MaskMetrology[R_HVStrip];
  int DetID = R_LVStrip.GetDetectorID();

  // Check for division by zero and return standard intersection without mask rotation in this case. 
  // These values will be the same for every detector since the rotation is the same for each strip within a detector.
  double denominator1 = tan(LVStripMet[5]*TMath::DegToRad());
  double denominator2 = tan((HVStripMet[5]-90)*TMath::DegToRad())-1/tan(LVStripMet[5]*TMath::DegToRad());
  if (denominator1 == 0.0 || denominator2 == 0.0) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Strip Intersection gives divide by zero - returning unrotated hit position"<<endl;
    double Xpos = m_YPitches[DetID]*((double)R_LVStrip.GetStripID() - ((m_NYStrips[DetID]-1)/2.0));
    double Ypos = m_XPitches[DetID]*((double)R_HVStrip.GetStripID() - ((m_NXStrips[DetID]-1)/2.0));
    return {Xpos, Ypos}; 
  }

  // Calculate XIntercept
  double XIntercept = (HVStripMet[0]*tan((HVStripMet[5]-90)*TMath::DegToRad()) - LVStripMet[0]/tan(LVStripMet[5]*TMath::DegToRad()) - LVStripMet[1] + HVStripMet[1])/(tan((HVStripMet[5]-90)*TMath::DegToRad())-1/tan(LVStripMet[5]*TMath::DegToRad()));
    
  // Solve for YIntercept
  double YIntercept = (XIntercept - HVStripMet[0])*tan((HVStripMet[5]-90)*TMath::DegToRad()) + HVStripMet[1];

  return {XIntercept, YIntercept};
}


/////////////////////////////////////////////////////////////////////////////////


int MModuleDepthCalibration2024::GetHitGrade(MHit* H){
  // Function for choosing which Depth-to-CTD relation to use for a given event.
  // At time of writing, intention is to choose a CTD based on sub-pixel region determined via charge sharing (Event "grade").
  // 5 possible grades, and one Error Grade, -1. GRADE 4 is as yet uncategorized complicated geometry. GRADE 5 means multiple, presumably separated strip hits.

  //organize x and y strips into vectors
  if (H == nullptr) {
    return -1;
  }
  if (H->GetNStripHits() == 0) {
    // Error if no strip hits listed. Bad grade is returned
    cout << "ERROR in MModuleDepthCalibration2024: HIT WITH NO STRIP HITS" << endl;
    return -1;
  }
   
  // Take a Hit and separate its activated p and n strips into separate vectors.
  std::vector<MStripHit*> LVStrips;
  std::vector<MStripHit*> HVStrips;
  vector<int> LVStripIDs;
  vector<int> HVStripIDs;
  for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
    MStripHit* SH = H->GetStripHit(j);
    if (SH == nullptr ) { cout << "ERROR in MModuleDepthCalibration2024: Depth Calibration: got NULL strip hit :( " << endl; return -1;}
    if (SH->GetEnergy() == 0 ) { cout << "ERROR in MModuleDepthCalibration2024: Depth Calibration: got strip without energy :( " << endl; return -1;}
    if (SH->IsLowVoltageStrip()) {
      LVStrips.push_back(SH); 
      LVStripIDs.push_back(SH->GetStripID());
    }
    else {
      HVStrips.push_back(SH);
      HVStripIDs.push_back(SH->GetStripID());
    }
  }

  // if the same strip has multiple hits, this is a bad grade.
  bool MultiHitX = H->GetStripHitMultipleTimesX();
  bool MultiHitY = H->GetStripHitMultipleTimesY();
  if (MultiHitX || MultiHitY) {
    return 5;  
  }

  if (LVStrips.size()>0 && HVStrips.size()>0) {
    int HVmin = * std::min_element(HVStripIDs.begin(), HVStripIDs.end());
    int HVmax = * std::max_element(HVStripIDs.begin(), HVStripIDs.end());

    int LVmin = * std::min_element(LVStripIDs.begin(), LVStripIDs.end());
    int LVmax = * std::max_element(LVStripIDs.begin(), LVStripIDs.end());

    // If the strip hits are not all adjacent, it's a bad grade.
    if ( ((HVmax - HVmin) >= (HVStrips.size())) || ((LVmax - LVmin) >= (LVStrips.size())) ) {
      return 6;
    }
  }
  else{
    return -1;
  }

  int return_value;
  // If 1 strip on each side, GRADE=0
  // This represents the center of the pixel
  if ( ((LVStrips.size() == 1) && (HVStrips.size() == 1)) || ((LVStrips.size() == 3) && (HVStrips.size() == 3)) ) {
    return_value = 0;
  } 
  // If 2 hits on N side and 1 on P, GRADE=1
  // This represents the middle of the edges of the pixel
  else if ( (LVStrips.size() == 1) && (HVStrips.size() == 2) ) {
    return_value = 1;
  } 

  // If 2 hits on P and 1 on N, GRADE=2
  // This represents the middle of the edges of the pixel
  else if ( (LVStrips.size() == 2) && (HVStrips.size() == 1) ) {
    return_value = 2;
  } 
  
  // If 2 strip hits on both sides, GRADE=3
  // This represents the corners the pixel
  else if ( (LVStrips.size() == 2) && (HVStrips.size() == 2) ) {
    return_value = 3;
  } 

  // If 3 hits on N side and 1 on P, GRADE=0
  // This represents the middle of the pixel, near the p (LV) side of the detector.
  else if ( (LVStrips.size() == 1) && (HVStrips.size() == 3) ) {
    return_value = 0;
  } 

  // If 3 hits on P and 1 on N, GRADE=0
  // This represents the middle of the pixel, near the n (HV) side of the detector.
  else if ( (LVStrips.size() == 3) && (HVStrips.size() == 1) ) {
    return_value = 0;
  } 

  // If 3 hits on N side and 2 on P, GRADE=0
  // This represents the middle of the edge of the pixel, near the p (LV) side of the detector.
  else if ( (LVStrips.size() == 2) && (HVStrips.size() == 3) ) {
    return_value = 2;
  } 

  // If 3 hits on P and 2 on N, GRADE=0
  // This represents the middle of the edge of the pixel, near the n (HV) side of the detector.
  else if ( (LVStrips.size() == 3) && (HVStrips.size() == 2) ) {
    return_value = 1;
  } 

  else {
    // If more complicated than the above cases, return 4 for now.
    // TODO: Handle more complicated charge distributions.
    return_value = 4;
  }

  return return_value;
}


/////////////////////////////////////////////////////////////////////////////////

bool MModuleDepthCalibration2024::AddDepthCTD(vector<double> Depth, vector<vector<double>> CTDArr, int DetID, unordered_map<int, vector<double>>& DepthGrid, unordered_map<int,vector<vector<double>>>& CTDMap, unordered_map<int,vector<TSpline3*>>& SplineMap, unsigned int NPoints)
{
  // Saves a CTD array, basically allowing for multiple CTDs as a function of depth 
  // Depth: list of simulated depth values
  // CTDArr: vector of vectors of simulated CTD values. Each vector of CTDs must be the same length as Depth
  // DetID: An integer which specifies which detector.
  // CTDMap: unordered map into which the array of CTDs should be placed
  // SplineMap: unordered map to store splines
  // NPoints: number of points in the depth/CTD grid to be produced

  // TODO: Possible energy dependence of CTD?

  // Check to make sure things look right.
  // First check that the CTDs all have the right length.

  for (unsigned int i = 0; i < CTDArr.size(); ++i) {
    if ((CTDArr[i].size() != Depth.size()) && (CTDArr[i].size() > 0)) {
      cout<<"ERROR in MModuleDepthCalibration2024::AddDepthCTD: The number of values in the CTD list is not equal to the number of depth values."<<endl;
      return false;
    }
  }

  // Check that the geometry file and the depth-CTD curve match within 0.1mm tolerance
  double MaxDepth = * std::max_element(Depth.begin(), Depth.end());
  double MinDepth = * std::min_element(Depth.begin(), Depth.end());
  if (fabs((MaxDepth-MinDepth) - m_Thicknesses[DetID]) > 0.01) {
    cout<<"ERROR in MModuleDepthCalibration2024::AddDepthCTD: The thickness of detector "<<DetID<<" listed in the geometry file does not match the depth-CTD file."<<endl;
    cout<<"Geometry file gives "<<m_Thicknesses[DetID]<<"cm, while the depth-CTD file gives "<<(MaxDepth-MinDepth)<<"cm."<<endl;
    return false;
  }

  // Check to make sure splines haven't already been loaded for this detector
  if (SplineMap.count(DetID)>0) {
    cout<<"MModuleDepthCalibration2024::AddDepthCTD: Splines already added for DetID "<<DetID<<"."<<endl;
    return false;
  } else {
    vector<TSpline3*> TempVec;
    SplineMap[DetID] = TempVec;
  }

  // Add a new point to the depth vector on the bottom and top of the detector to make sure we're sampling the full depth
  double dx_low = Depth[1] - Depth[0];
  double newx_low = Depth[0] - (dx_low/2.0);
  Depth.insert(Depth.begin(), newx_low);

  size_t N = Depth.size();
  double dx_high = Depth[N-1] - Depth[N-2];
  double newx_high = Depth[N-1] + (dx_high/2.0);
  Depth.push_back(newx_high);

  // If the depth vector is more dense than NPoints, use the length of the input vector
  if (NPoints < N+1) {
    NPoints = N+1;
  }

  // Reconstitute the depth vector to ensure that the depth is evenly sampled
  vector<double> NewDepth;
  for (unsigned int i=0; i<NPoints; ++i) {
    NewDepth.push_back(((m_Thicknesses[DetID]/(NPoints-1))*i) - (m_Thicknesses[DetID]/2));
  }
  DepthGrid[DetID] = NewDepth;

  // For each CTD vector, extrapolate to the top and bottom points we added above
  // Then make a depth-CTD spline and fill up a new CTD vector with values sampled from the spline
  vector<vector<double>> NewCTDArr;
  for (unsigned int i=0; i<CTDArr.size(); ++i) {
    vector<double> CTD = CTDArr[i];
    vector<double> NewCTD;
  
    //first extrapolate the lower side
    double dy, m, b, newy;
    dy = CTD[1] - CTD[0];
    m = dy / dx_low;
    b = CTD[0] - m*Depth[1];
    newy = m*newx_low + b;
    CTD.insert(CTD.begin(), newy);

    //next extrapolate the upper side
    dy = CTD[N-1] - CTD[N-2];
    m = dy / dx_high;
    b = CTD[N-1] - m*Depth[N-2];
    newy = m*newx_high + b;
    CTD.push_back(newy);

    TSpline3* Sp = new TSpline3("", &Depth[0], &CTD[0],Depth.size());
    SplineMap[DetID].push_back(Sp);

    for (unsigned int d=0; d<NewDepth.size(); ++d) {
      NewCTD.push_back(Sp->Eval(NewDepth[d]));
    }
    NewCTDArr.push_back(NewCTD);
  }

  CTDMap[DetID] = NewCTDArr;
  return true;
}


/////////////////////////////////////////////////////////////////////////////////


vector<double> MModuleDepthCalibration2024::GetCTD(int DetID, int Grade)
{
  // Retrieves the appropriate CTD vector given the Detector ID and Event Grade passed

  if (!m_SplinesFileIsLoaded) {
    cout << "MModuleDepthCalibration2024::GetCTD: cannot return Depth to CTD relation because the file was not loaded." << endl;
    return vector<double> ();
  }
  // If there is a CTD array for the given detector, return it.
  // If the Grade is larger than the number of CTD vectors stored, then just return Grade 0 vector.
  if (m_CTDMap.count(DetID) > 0) {
    if ( ((int)m_CTDMap[DetID].size()) > Grade) {
      return (m_CTDMap[DetID][Grade]);
    } else {
      return (m_CTDMap[DetID][0]);
    }
  } else {
    cout << "MModuleDepthCalibration2024::GetCTD: No CTD map is loaded for Det " << DetID << "." << endl;
    return vector<double> ();
  }
}


/////////////////////////////////////////////////////////////////////////////////


vector<double> MModuleDepthCalibration2024::GetDepth(int DetID)
{
  // Retrieves the appropriate CTD vector given the Detector ID and Event Grade passed

  if (!m_SplinesFileIsLoaded) {
    cout << "MModuleDepthCalibration2024::GetDepth: cannot return Depth grid because the file was not loaded." << endl;
    return vector<double> ();
  }

  // If there is a CTD array for the given detector, return it.
  // If the Grade is larger than the number of CTD vectors stored, then just return Grade 0 vector.
  if (m_DepthGrid.count(DetID) > 0){
    return m_DepthGrid[DetID];
    } else {
      cout << "MModuleDepthCalibration2024::GetDepth: No Depth grid is loaded for Det " << DetID << "." << endl;
      return vector<double> ();
  }
} 


/////////////////////////////////////////////////////////////////////////////////

TSpline3* MModuleDepthCalibration2024::GetSpline(int DetID, int Grade)
{
  // Retrieves the appropriate depth->CTD spline given the Detector ID and Event Grade passed

  if( !m_SplinesFileIsLoaded ){
    cout << "MModuleDepthCalibration2024::GetSpline: cannot return Depth to CTD spline because the file was not loaded." << endl;
    return nullptr;
  }

  // If there is a spline for the given detector, return it.
  // If the Grade is larger than the number of splines stored, then just return Grade 0 spline.
  if( m_SplineMap.count(DetID) > 0 ){
    if ( ((int)m_SplineMap[DetID].size()) > Grade) {
      return (m_SplineMap[DetID][Grade]);
    }
    else {
      return (m_SplineMap[DetID][0]);
    }
  } else {
    cout << "MModuleDepthCalibration2024::GetSpline: No spline is loaded for Det " << DetID << "." << endl;
    return nullptr;
  }
}


/////////////////////////////////////////////////////////////////////////////////


void MModuleDepthCalibration2024::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
  MGUIOptionsDepthCalibration2024* Options = new MGUIOptionsDepthCalibration2024(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleDepthCalibration2024::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* CoeffsFileNameNode = Node->GetNode("CoeffsFileName");
  if (CoeffsFileNameNode != nullptr) {
  m_CoeffsFile = CoeffsFileNameNode->GetValue();
  }

  MXmlNode* SplinesFileNameNode = Node->GetNode("SplinesFileName");
  if (SplinesFileNameNode != nullptr) {
  m_SplinesFile = SplinesFileNameNode->GetValue();
  }

  MXmlNode* MasKMetrologyNode = Node->GetNode("MaskMetrology");
  if (MasKMetrologyNode != nullptr) {
      m_MaskMetrologyEnabled = (bool) MasKMetrologyNode->GetValueAsBoolean();
  }

  MXmlNode* MaskMetrologyFileNameNode = Node->GetNode("MaskMetrologyFileName");
  if (MaskMetrologyFileNameNode != nullptr) {
    m_MaskMetrologyFile = MaskMetrologyFileNameNode->GetValue();
  }

  MXmlNode* UCSDOverrideNode = Node->GetNode("UCSDOverride");
  if (UCSDOverrideNode != nullptr) {
      m_UCSDOverride = (bool) UCSDOverrideNode->GetValueAsBoolean();
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
  new MXmlNode(Node, "MaskMetrology", (bool)m_MaskMetrologyEnabled);
  new MXmlNode(Node, "MaskMetrologyFileName", m_MaskMetrologyFile);
  new MXmlNode(Node, "UCSDOverride", (bool)m_UCSDOverride);  
  
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
  cout << "Number of hits with null strip hits: " << m_ErrorNullSH << endl;
  cout << "Number of hits 0 energy on a strip hit: " << m_ErrorNoE << endl;

  // Clean up maps and vectors
  m_Coeffs.clear();
  m_Thicknesses.clear();
  m_NXStrips.clear();
  m_NYStrips.clear();
  m_XPitches.clear();
  m_YPitches.clear();
  m_Detectors.clear();
  m_CTDMap.clear();
  m_DepthGrid.clear();
  m_SplineMap.clear();
  m_DetectorIDs.clear();

}



// MModuleDepthCalibration2024.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
