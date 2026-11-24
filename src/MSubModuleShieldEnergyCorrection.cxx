/*
 * MSubModuleShieldEnergyCorrection.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer, Valentina Fioretti.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MSubModuleShieldEnergyCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleShieldEnergyCorrection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleShieldEnergyCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldEnergyCorrection::MSubModuleShieldEnergyCorrection()
    : MSubModule(),
      m_Random(0)
{
  // Construct an instance of MSubModuleShieldEnergyCorrection

  m_Name = "DEE shield energy correction module";
  m_ShieldEnergyCorrectionFileName = "";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldEnergyCorrection::~MSubModuleShieldEnergyCorrection()
{
  //! Delete this instance of MSubModuleShieldEnergyCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::Initialize()
{
  // Initialize the module
  m_Centroid.clear();
  m_FWHM.clear();

  //! load shield energy correction file
  if (ParseShieldEnergyCorrectionFile() == false) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR: Failed to parse shield energy correction file " << m_ShieldEnergyCorrectionFileName << endl;
    }
    return false;
  }

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldEnergyCorrection::Clear()
{
  //! Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::AnalyzeEvent(MReadOutAssembly* Event)
{
  //! Main data analysis routine, which updates the event to a new level

  if (Event == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR: AnalyzeEvent() received nullptr event" << endl;
    }    
    return false;
  }

  // DEE shield energy correction: for each voxel of the shield crystal, the deposited energy is corrected following a gaussian distribution. The energy centroid and the fwhm are computed using the formula in (Ciabattoni et al. 2025) using a set of 5 parameters, defined in the shield energy correction file. Each voxel has different parameters.

  // Energy correction
  list<MDEECrystalHit>& Hits = Event->GetDEECrystalHitListReference();
  for (MDEECrystalHit& CH : Hits) {

    const double SimulatedEnergy = CH.m_SimulatedEnergy;

    if (SimulatedEnergy < 0) {
      if (g_Verbosity >= c_Error) cout << "<ERROR>: Negative simulated energy (" << SimulatedEnergy << ")" << endl;
      return false;
    }

    MString DetectorID = CH.m_DetectorID;
    int CrystalID = CH.m_CrystalID;

    MVector VoxelInDetector = CH.m_VoxelInDetector;

    const double ShieldCorrectedCentroid = NoiseShieldEnergyCentroid(SimulatedEnergy, DetectorID, CrystalID, VoxelInDetector[0], VoxelInDetector[1], VoxelInDetector[2]);
    const double ShieldFWHMValue = NoiseShieldEnergyFWHM(SimulatedEnergy, DetectorID, CrystalID, VoxelInDetector[0], VoxelInDetector[1], VoxelInDetector[2]);

    // If FWHM not available or invalid, apply default value
    // default value, in keV, for the sigma of the gaussian noise if FWHM is not available or invalid
    const double SigmaNoiseDefault = 10;
    double CorrectedEnergyDefault = 0;
    if (ShieldFWHMValue <= 0) {
      if (g_Verbosity >= c_Warning) {
        cout << "WARNING: Non-positive FWHM (" << ShieldFWHMValue << ") for DetectorID = " << DetectorID << " CrystalID = " << CrystalID << " Voxel_ID = " << VoxelInDetector[0] << "," << VoxelInDetector[1] << "," << VoxelInDetector[2] << " -> applying default" << endl;
      }
      CorrectedEnergyDefault = m_Random.Gaus(SimulatedEnergy, SigmaNoiseDefault);
      CH.m_Energy = CorrectedEnergyDefault;
      continue;
    }

    double ShieldSigma = ShieldFWHMValue / FWHM_TO_SIGMA;
    double CorrectedEnergy = m_Random.Gaus(ShieldCorrectedCentroid, ShieldSigma);

    if (CorrectedEnergy < 0) {
      if (g_Verbosity >= c_Warning) {
        cout << "WARNING: Corrected energy negative (" << CorrectedEnergy << ") from centroid = " << ShieldCorrectedCentroid << " sigma=" << ShieldSigma << " -> setting to 0 keV" << endl;
      }
      CH.m_Energy = 0.0;
      continue;
    }

    if (g_Verbosity >= c_Info) {
      cout << "DEE shield energy correction:" << endl;
      cout << "DetectorID: " << DetectorID << " CrystalID: " << CrystalID << " Voxel_ID: " << VoxelInDetector << endl;
      cout << "Simulated energy: " << SimulatedEnergy << endl;
      cout << "Corrected energy: " << CorrectedEnergy << endl;
    }

    CH.m_Energy = CorrectedEnergy;
  }

  // Merge hits:
  for (auto IterLV1 = Hits.begin(); IterLV1 != Hits.end(); ++IterLV1) {
    auto IterLV2 = std::next(IterLV1);
    while (IterLV2 != Hits.end()) {
      if (IterLV1->m_ROE == IterLV2->m_ROE) {
        IterLV1->m_Energy += IterLV2->m_Energy;
        IterLV2 = Hits.erase(IterLV2);
      } else {
        ++IterLV2;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldEnergyCorrection::Finalize()
{
  //! Finalize the analysis - do all cleanup, i.e., undo Initialize()
  m_Centroid.clear();
  m_FWHM.clear();

  MSubModule::Finalize();
}

//! centroid and fwhm for the gaussian noise
double MSubModuleShieldEnergyCorrection::NoiseShieldEnergyCentroid(double Energy, MString DetectorID, int CrystalID, int VoxelXID, int VoxelYID, int VoxelZID)
{

  MReadOutElementVoxel3D hit_V;
  hit_V.SetDetectorID(DetectorID);
  hit_V.SetCrystalID(CrystalID);
  hit_V.SetVoxelXID(VoxelXID);
  hit_V.SetVoxelYID(VoxelYID);
  hit_V.SetVoxelZID(VoxelZID);

  double CorrectedCentroid = 0.0;

  auto it = m_Centroid.find(hit_V);
  if (it != m_Centroid.end()) {
    TF1* GaussCentroid = it->second.get();
    if (GaussCentroid == nullptr) {
        if (g_Verbosity >= c_Error) {
            cout << "ERROR: Null TF1 pointer for centroid map entry" << endl;
        }
        return Energy;
    } else {
        CorrectedCentroid = GaussCentroid->Eval(Energy);
    }
  } else {
      if (g_Verbosity >= c_Error) {
          cout << "ERROR: Centroid correction not found for shield " << DetectorID << ", " << CrystalID << " and voxel (" << VoxelXID << "," << VoxelYID << "," << VoxelZID << ")" << endl;
      }
      return Energy;
  }

  return CorrectedCentroid;
}

double MSubModuleShieldEnergyCorrection::NoiseShieldEnergyFWHM(double Energy, MString DetectorID, int CrystalID, int VoxelXID, int VoxelYID, int VoxelZID)
{

  MReadOutElementVoxel3D hit_V;
  hit_V.SetDetectorID(DetectorID);
  hit_V.SetCrystalID(CrystalID);
  hit_V.SetVoxelXID(VoxelXID);
  hit_V.SetVoxelYID(VoxelYID);
  hit_V.SetVoxelZID(VoxelZID);

  double FWHM_value = 0.0;

  auto it_fwhm = m_FWHM.find(hit_V);
  // default value, in keV, for the sigma of the gaussian noise if FWHM is not available or invalid
  const double SigmaNoiseDefault = 10;
  // converting from Sigma to FWHM
  const double FWHMNoiseDefault = SigmaNoiseDefault * FWHM_TO_SIGMA;
    
  if (it_fwhm != m_FWHM.end()) {
    TF1* GaussFWHM = it_fwhm->second.get();
    if (GaussFWHM == nullptr) {
        if (g_Verbosity >= c_Error) {
            cout << "ERROR: Null TF1 pointer for fwhm map entry" << endl;
        }
        return FWHMNoiseDefault;
    }
    FWHM_value = GaussFWHM->Eval(Energy); 
  } else {
      if (g_Verbosity >= c_Error) {
          cout << "ERROR: FWHM correction not found for shield " << DetectorID << ", " << CrystalID << " and voxel (" << VoxelXID << "," << VoxelYID << "," << VoxelZID << ")" << endl;
      }
      return FWHMNoiseDefault;
  }


  return FWHM_value;
}

bool MSubModuleShieldEnergyCorrection::ParseShieldEnergyCorrectionFile()
{

  if (m_ShieldEnergyCorrectionFileName == "") {
    if (g_Verbosity >= c_Error) cout << "ERROR: Shield energy correction filename is empty." << endl;
    return false;
  }

  MParser Parser;
  if (Parser.Open(m_ShieldEnergyCorrectionFileName, MFile::c_Read) == false) {
    if (g_Verbosity >= c_Error) cout << "Unable to open shield energy correction file " << m_ShieldEnergyCorrectionFileName << endl;
    return false;
  }

  unsigned int Parsed = 0;
  unsigned int Skipped = 0;

  for (unsigned int i = 0; i < Parser.GetNLines(); i++) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens == 0)
      continue; // skip empty lines

    // Skip comment lines
    if (Parser.GetTokenizerAt(i)->GetTokenAtAsString(0).BeginsWith("#"))
      continue;

    if (NTokens != 12) {
      if (g_Verbosity >= c_Warning) {
        cout << "WARNING: Line " << i << ": expected 12 tokens, got " << NTokens << " (skipping)" << endl;
      }
      ++Skipped;
      continue;
    } // this shouldn't happen but just in case

    // For each voxel of the shield crystal, the deposited energy is corrected generating a random energy correction following a gaussian distribution. The energy centroid and the fwhm can be computed from the parameters below (Ciabattoni et al. 2025)

    // Detector ID
    MString DetectorID = Parser.GetTokenizerAt(i)->GetTokenAtAsString(0);
    // Crystal ID
    int CrystalID = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
    // MEGAlib voxel X, Y, Z ID
    int VoxelXID = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2);
    int VoxelYID = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3);
    int VoxelZID = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(4);
    // model parameters
    // centroid: E_measured = m*E_true + q
    double Par_m = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(7);
    double Par_q = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(8);
    // FWHM = sqrt(a^2 + b^2*E_true + c^2*E_true^2)
    double Par_a = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(9);
    double Par_b = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(10);
    double Par_c = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(11);

    MReadOutElementVoxel3D V;

    V.SetDetectorID(DetectorID);
    V.SetCrystalID(CrystalID);
    V.SetVoxelXID(VoxelXID);
    V.SetVoxelYID(VoxelYID);
    V.SetVoxelZID(VoxelZID);

    //TF1* GaussCentroid = new TF1("centroid_" + DetectorID + "_" + MString(CrystalID) + "_" + MString(VoxelXID) + "_" +MString(VoxelYID)  + "_" + MString(VoxelZID), "[0]*x + [1]");
      
    auto GaussCentroid = make_unique<TF1>("centroid_" + DetectorID + "_" + MString(CrystalID) + "_" + MString(VoxelXID) + "_" + MString(VoxelYID) + "_" + MString(VoxelZID), "[0]*x + [1]");  
    
    GaussCentroid->SetParameter(0, Par_m);
    GaussCentroid->SetParameter(1, Par_q);

    auto GaussFWHM = make_unique<TF1>("fwhm_" + DetectorID + "_" + MString(CrystalID) + "_" + MString(VoxelXID)  + "_" + MString(VoxelYID)  + "_" + MString(VoxelZID), "sqrt([0]**2 + ([1]**2)*x + ([2]**2)*(x**2))");
    GaussFWHM->SetParameter(0, Par_a);
    GaussFWHM->SetParameter(1, Par_b);
    GaussFWHM->SetParameter(2, Par_c);

    m_Centroid[V] = move(GaussCentroid);
    m_FWHM[V] = move(GaussFWHM);

    ++Parsed;
  }

  if (Parsed == 0) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Parsed 0 valid correction lines from " << m_ShieldEnergyCorrectionFileName << endl;
    return false;
  }

  if (Skipped > 0) {
    if (g_Verbosity >= c_Warning) cout << "WARNING: Skipped " << Skipped << " line(s) while parsing " << m_ShieldEnergyCorrectionFileName << endl;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* ShieldEnergyCorrectionFileName = Node->GetNode("ShieldEnergyCorrectionFileName");
  if (ShieldEnergyCorrectionFileName != nullptr) {
    m_ShieldEnergyCorrectionFileName = ShieldEnergyCorrectionFileName->GetValue();
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleShieldEnergyCorrection::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  new MXmlNode(Node, "ShieldEnergyCorrectionFileName", m_ShieldEnergyCorrectionFileName);


  return Node;
}


// MSubModuleShieldEnergyCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
