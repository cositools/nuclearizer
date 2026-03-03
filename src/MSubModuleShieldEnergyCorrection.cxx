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
  // Delete this instance of MSubModuleShieldEnergyCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::Initialize()
{
  // Initialize the module

  // load shield energy correction file
  if (ParseShieldEnergyCorrectionFile() == false)
    return false;

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldEnergyCorrection::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level

  if (Event == nullptr) {
      if (g_Verbosity >= c_Error) cout << "ERROR: AnalyzeEvent() received nullptr event" << endl;
      return false;
  }
    
  // DEE shield energy correction: for each voxel of the shield crystal, the deposited energy is corrected following a gaussian distribution. The energy centroid and the fwhm are computed using the formula in (Ciabattoni et al. 2025) using a set of 5 parameters, defined in the shield energy correction file. Each voxel has different parameters.
  
  // Energy correction
  list<MDEECrystalHit>& Hits = Event->GetDEECrystalHitListReference();
  for (MDEECrystalHit& CH : Hits) {

     const double true_energy = CH.m_SimulatedEnergy;
      
     if (true_energy < 0) {
        if (g_Verbosity >= c_Warning) cout << "WARNING: Negative simulated energy (" << true_energy
              << ")" << endl;
        CH.m_Energy = true_energy;
        continue;
     }
     
     MString det_id = CH.m_DetectorID;
     int crystal_id = CH.m_CrystalID;

     MVector voxel_id = CH.m_VoxelInDetector;

     const double shield_corrected_centroid = NoiseShieldEnergyCentroid(true_energy, det_id, crystal_id, voxel_id[0], voxel_id[1], voxel_id[2]);
     const double shield_FWHM_value = NoiseShieldEnergyFWHM(true_energy, det_id, crystal_id, voxel_id[0], voxel_id[1], voxel_id[2]);

     // If FWHM not available or invalid, do not smear
     if (shield_FWHM_value <= 0) {
        if (g_Verbosity >= c_Warning) cout << "WARNING: Non-positive FWHM (" << shield_FWHM_value << ") for DetectorID = " << det_id
              << " CrystalID = " << crystal_id << " Voxel_ID = " << voxel_id << " -> using centroid only" << endl;
        CH.m_Energy = shield_corrected_centroid;
        continue;
     }
      
     double shield_sigma = shield_FWHM_value / 2.35;
     double corrected_energy = m_Random.Gaus(shield_corrected_centroid, shield_sigma);

     if (corrected_energy < 0) {
         if (g_Verbosity >= c_Warning) cout  << "WARNING: Corrected energy negative (" << corrected_energy
              << ") from centroid = " << shield_corrected_centroid << " sigma=" << shield_sigma
              << " -> clamping to 0" << endl;
        CH.m_Energy = 0.0;
        continue; 
     }      
      
      if (g_Verbosity >= c_Info) { 
          cout << "DEE shield energy correction:" << endl;
          cout << "DetectorID: " << det_id << " CrystalID: " << crystal_id << " Voxel_ID: " << voxel_id << endl;   
          cout << "Simulated energy: " << true_energy << endl;
          cout << "Corrected energy: " << corrected_energy << endl;
      }
      
     CH.m_Energy = corrected_energy;
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
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()

  MSubModule::Finalize();
}

//! centroid and fwhm for the gaussian noise
double MSubModuleShieldEnergyCorrection::NoiseShieldEnergyCentroid(double energy, MString det_id, int crystal_id, int voxelx_id, int voxely_id, int voxelz_id)
{

  MReadOutElementVoxel3D hit_V;
  hit_V.SetDetectorID(det_id);
  hit_V.SetCrystalID(crystal_id);
  hit_V.SetVoxelXID(voxelx_id);
  hit_V.SetVoxelYID(voxely_id);
  hit_V.SetVoxelZID(voxelz_id);

  double corrected_centroid = 0.0;

  auto it = m_Centroid.find(hit_V);
  if (it != m_Centroid.end()) {
    TF1* gauss_centroid = it->second;
    if (gauss_centroid == nullptr) {
        if (g_Verbosity >= c_Error) cout << "ERROR: Null TF1 pointer for centroid map entry" << endl;
    }
    corrected_centroid = gauss_centroid->Eval(energy);
  } else {
    if (g_Verbosity >= c_Error) cout << "ERROR: Centroid correction not found for shield " << det_id << ", " << crystal_id << " and voxel (" << voxelx_id << "," << voxely_id << "," << voxelz_id << ")" << endl;
  }

  return corrected_centroid;
}

double MSubModuleShieldEnergyCorrection::NoiseShieldEnergyFWHM(double energy, MString det_id, int crystal_id, int voxelx_id, int voxely_id, int voxelz_id)
{

  MReadOutElementVoxel3D hit_V;
  hit_V.SetDetectorID(det_id);
  hit_V.SetCrystalID(crystal_id);
  hit_V.SetVoxelXID(voxelx_id);
  hit_V.SetVoxelYID(voxely_id);
  hit_V.SetVoxelZID(voxelz_id);

  double FWHM_value = 0.0;

  auto it_fwhm = m_FWHM.find(hit_V);

  if (it_fwhm != m_FWHM.end()) {
    TF1* gauss_fwhm = it_fwhm->second;
    if (gauss_fwhm == nullptr) {
          if (g_Verbosity >= c_Error) cout << "ERROR: Null TF1 pointer for fwhm map entry" << endl;
    }
    FWHM_value = gauss_fwhm->Eval(energy); // E_true in keV
  } else {
    if (g_Verbosity >= c_Error) cout << "ERROR: FWHM correction not found for shield " << det_id << ", " << crystal_id << " and voxel (" << voxelx_id << "," << voxely_id << "," << voxelz_id << ")" << endl;
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
        if (g_Verbosity >= c_Warning) cout << "WARNING: Line " << i << ": expected 12 tokens, got " << NTokens
            << " (skipping)" << endl;
        ++Skipped;
        continue;
    } //this shouldn't happen but just in case

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
      if (g_Verbosity >= c_Error) cout << "ERROR: Parsed 0 valid correction lines from "
           << m_ShieldEnergyCorrectionFileName << endl;
      return false;
  }

  if (Skipped > 0) {
      if (g_Verbosity >= c_Warning) cout << "WARNING: Skipped " << Skipped << " line(s) while parsing "
            << m_ShieldEnergyCorrectionFileName << endl;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* ShieldEnergyCorrectionFileName = Node->GetNode("ShieldEnergyCorrectionFileName");
  if (ShieldEnergyCorrectionFileName != 0) {
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
