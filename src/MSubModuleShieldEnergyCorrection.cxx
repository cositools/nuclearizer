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

  //! load shield energy correction file
  if (!ParseShieldEnergyCorrectionFile()) {
    if (g_Verbosity >= c_Error)
      cout << "ERROR: Failed to parse shield energy correction file " << m_ShieldEnergyCorrectionFileName << endl;
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
    if (g_Verbosity >= c_Error)
      cout << "ERROR: AnalyzeEvent() received nullptr event" << endl;
    return false;
  }

  // DEE shield energy correction: for each voxel of the shield crystal, the deposited energy is corrected following a gaussian distribution. The energy centroid and the fwhm are computed using the formula in (Ciabattoni et al. 2025) using a set of 5 parameters, defined in the shield energy correction file. Each voxel has different parameters.

  // Energy correction
  list<MDEECrystalHit>& Hits = Event->GetDEECrystalHitListReference();
  for (MDEECrystalHit& CH : Hits) {

    const double SimulatedEnergy = CH.m_SimulatedEnergy;

    if (SimulatedEnergy < 0) {
      if (g_Verbosity >= c_Error)
        cout << "<ERROR>: Negative simulated energy (" << SimulatedEnergy
             << ")" << endl;
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
      if (g_Verbosity >= c_Warning)
        cout << "WARNING: Non-positive FWHM (" << ShieldFWHMValue << ") for DetectorID = " << DetectorID
             << " CrystalID = " << CrystalID << " Voxel_ID = " << VoxelInDetector[0] << "," << VoxelInDetector[1] << "," << VoxelInDetector[2] << " -> applying default" << endl;
      CorrectedEnergyDefault = m_Random.Gaus(SimulatedEnergy, SigmaNoiseDefault);
      CH.m_Energy = CorrectedEnergyDefault;
      continue;
    }

    double ShieldSigma = ShieldFWHMValue / 2.35;
    double CorrectedEnergy = m_Random.Gaus(ShieldCorrectedCentroid, ShieldSigma);

    if (CorrectedEnergy < 0) {
      if (g_Verbosity >= c_Warning)
        cout << "WARNING: Corrected energy negative (" << CorrectedEnergy
             << ") from centroid = " << ShieldCorrectedCentroid << " sigma=" << ShieldSigma
             << " -> setting to 0 keV" << endl;
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
    TF1* gauss_centroid = it->second;
    if (gauss_centroid == nullptr) {
      if (g_Verbosity >= c_Error)
        cout << "ERROR: Null TF1 pointer for centroid map entry" << endl;
    }
    CorrectedCentroid = gauss_centroid->Eval(Energy);
  } else {
    if (g_Verbosity >= c_Error)
      cout << "ERROR: Centroid correction not found for shield " << DetectorID << ", " << CrystalID << " and voxel (" << VoxelXID << "," << VoxelYID << "," << VoxelZID << ")" << endl;
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

  if (it_fwhm != m_FWHM.end()) {
    TF1* gauss_fwhm = it_fwhm->second;
    if (gauss_fwhm == nullptr) {
      if (g_Verbosity >= c_Error)
        cout << "ERROR: Null TF1 pointer for fwhm map entry" << endl;
    }
    FWHM_value = gauss_fwhm->Eval(Energy); // E_true in keV
  } else {
    if (g_Verbosity >= c_Error)
      cout << "ERROR: FWHM correction not found for shield " << DetectorID << ", " << CrystalID << " and voxel (" << VoxelXID << "," << VoxelYID << "," << VoxelZID << ")" << endl;
  }


  return FWHM_value;
}

bool MSubModuleShieldEnergyCorrection::ParseShieldEnergyCorrectionFile()
{

  if (m_ShieldEnergyCorrectionFileName == "") {
    if (g_Verbosity >= c_Error)
      cout << "ERROR: Shield energy correction filename is empty." << endl;
    return false;
  }

  MParser Parser;
  if (Parser.Open(m_ShieldEnergyCorrectionFileName, MFile::c_Read) == false) {
    if (g_Verbosity >= c_Error)
      cout << "Unable to open shield energy correction file " << m_ShieldEnergyCorrectionFileName << endl;
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
      if (g_Verbosity >= c_Warning)
        cout << "WARNING: Line " << i << ": expected 12 tokens, got " << NTokens
             << " (skipping)" << endl;
      ++Skipped;
      continue;
    } // this shouldn't happen but just in case

    // For each voxel of the shield crystal, the deposited energy is corrected generating a random energy correction following a gaussian distribution. The energy centroid and the fwhm can be computed from the parameters below (Ciabattoni et al. 2025)

    // Detector ID
    MString det_id = Parser.GetTokenizerAt(i)->GetTokenAtAsString(0);
    // Crystal ID
    int crystal_id = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
    // MEGAlib voxel X, Y, Z ID
    int voxel_X = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(2);
    int voxel_Y = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(3);
    int voxel_Z = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(4);
    // model parameters
    // centroid: E_measured = m*E_true + q
    double m_par = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(7);
    double q_par = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(8);
    // FWHM = sqrt(a^2 + b^2*E_true + c^2*E_true^2)
    double a_par = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(9);
    double b_par = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(10);
    double c_par = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(11);

    MReadOutElementVoxel3D V;

    V.SetDetectorID(det_id);
    V.SetCrystalID(crystal_id);
    V.SetVoxelXID(voxel_X);
    V.SetVoxelYID(voxel_Y);
    V.SetVoxelZID(voxel_Z);

    TF1* gauss_centroid = new TF1("centroid_" + det_id + "_" + MString(crystal_id) + "_" + MString(voxel_X) + MString(voxel_Y) + MString(voxel_Z), "[0]*x + [1]");
    gauss_centroid->SetParameter(0, m_par);
    gauss_centroid->SetParameter(1, q_par);

    TF1* gauss_fwhm = new TF1("fwhm_" + det_id + "_" + MString(crystal_id) + "_" + MString(voxel_X) + MString(voxel_Y) + MString(voxel_Z), "sqrt([0]**2 + ([1]**2)*x + ([2]**2)*(x**2))");
    gauss_fwhm->SetParameter(0, a_par);
    gauss_fwhm->SetParameter(1, b_par);
    gauss_fwhm->SetParameter(2, c_par);

    m_Centroid[V] = gauss_centroid;
    m_FWHM[V] = gauss_fwhm;

    ++Parsed;
  }

  if (Parsed == 0) {
    if (g_Verbosity >= c_Error)
      cout << "ERROR: Parsed 0 valid correction lines from "
           << m_ShieldEnergyCorrectionFileName << endl;
    return false;
  }

  if (Skipped > 0) {
    if (g_Verbosity >= c_Warning)
      cout << "WARNING: Skipped " << Skipped << " line(s) while parsing "
           << m_ShieldEnergyCorrectionFileName << endl;
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
