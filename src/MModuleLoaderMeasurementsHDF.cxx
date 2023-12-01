/*
 * MModuleLoaderMeasurementsHDF.cxx
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
// MModuleLoaderMeasurementsHDF
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderMeasurementsHDF.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsLoaderMeasurementsHDF.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"
#include "MReadOutDataOrigins.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderMeasurementsHDF)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsHDF::MModuleLoaderMeasurementsHDF() : MModuleLoaderMeasurements()
{
  // Construct an instance of MModuleLoaderMeasurementsHDF
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Measurement loader for HDF files";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderHDF";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  m_FileNameStripMap = "";
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsHDF::~MModuleLoaderMeasurementsHDF()
{
  // Delete this instance of MModuleLoaderMeasurementsHDF
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::Initialize()
{
  // Initialize the module 
  
  // Clean:
  m_FileType = "Unknown";
  m_Detector = "Unknown";
  m_Version = -1;
  m_StartObservationTime = MTime(0);
  m_EndObservationTime = MTime(0);
  m_StartClock = numeric_limits<long>::max();
  m_EndClock = numeric_limits<long>::max();
  
  // Need to put this all in s try-catch block

  cout<<"FileName: "<<m_FileName<<endl;
  m_FileHDF5 = H5::H5File(m_FileName, H5F_ACC_RDONLY);

  // Step 3: Open the dataset
  H5::DataSet dataset = m_FileHDF5.openDataSet("Buffers");

  // Step 4: Determine the dataset dimensions
  H5::DataSpace dataspace = dataset.getSpace();

  // Step 5: Get the number of elements in the dataset
  int rank = dataspace.getSimpleExtentNdims();
  std::vector<hsize_t> dims(rank);
  dataspace.getSimpleExtentDims(dims.data(), nullptr);
  hsize_t num_elements = dims[0];

  // Step 6: Getthe data type and make sure it is H5T_VLEN
  H5::DataType datatype = dataset.getDataType();

  if (datatype.getClass() != H5T_VLEN) {
    cout<<"Data must be a variable length data type"<<endl;
    return false;
  }

  // Step 7: Prepare an intermediate data buffer Allocate memory to store the data
  m_RawBuffer.resize(num_elements);

  // Read the data from the dataset into the buffer
  cout<<"Reading the buffer from file"<<endl;
  dataset.read(m_RawBuffer.data(), datatype);

  // Store the position of the next to read element:
  m_NextRawBufferPosition = 0;

  // Prepare the intermediate buffer - how many elements we have
  m_IntermediateBufferSize = 0;

  // Read and convert some elements
  Convert();

  if (m_IntermediateBufferSize == 0) {
    cout<<"Unable to read any data"<<endl;
    return false;
  }

  m_LastOscillatorValue = 0;
  m_OscillatorFrequency = 200000;
  m_OscillatorOffset = 0;

  m_LastEventID = 0;

  /*
  unsigned int i_min = 0;
  for (unsigned int i = i_min; i < i_min + 8; i+=4) {
    cout<<endl;
    uint32_t headers_orig = data[i+0];
    uint32_t osc_chan = data[i+1];
    uint32_t index = data[i+2];
    uint32_t adc_orig = data[i+3];
    uint32_t dt = (headers_orig >> 10) & 0x1;
    uint32_t oscillator = ((osc_chan & 0xfff0) >> 4) + ((headers_orig & 0x00ff) << 12);
    uint32_t headers = (headers_orig >> 8);
    uint32_t asic = (osc_chan & 0x000f) >> 1;
    uint32_t channel = ((osc_chan & 0x0001) << 4) + ((index & 0xf000) >> 12);
    uint32_t tac = ((index & 0x0fff) << 2) + ((adc_orig & 0xc000) >> 14);
    uint32_t adc = (adc_orig & 0x3fff);

    cout<<"0x"<<hex<<data[i+0]<<", 0x"<<data[i+1]<<", 0x"<<data[i+2]<<", 0x"<<data[i+3]<<endl;
    cout<<dec<<data[i+0]<<", "<<data[i+1]<<", "<<data[i+2]<<", "<<data[i+3]<<endl;
    cout<<"dT: "<<dt<<endl;
    cout<<"OSCI: "<<oscillator<<endl;
    cout<<"HEADERS: "<<headers<<endl;
    cout<<"ASIC: "<<asic<<endl;
    cout<<"Channel: "<<channel<<endl;
    cout<<"TAC: "<<tac<<endl;
    cout<<"ADC: "<<adc<<endl;
  }
  */

  m_NEventsInFile = 0;
  m_NGoodEventsInFile = 0;
    
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.

  // Check if we need to read more data
  Convert();

  if (m_IntermediateBufferSize == 0) {
    cout<<"No more data"<<endl;
    return false;
  }

  // Check how many events to convert by looking at the time stamp
  if (m_IntermediateBuffer.front().m_OscillatorValue < m_LastOscillatorValue) {
    m_OscillatorOffset += 65536;
  }
  m_LastOscillatorValue = m_IntermediateBuffer.front().m_OscillatorValue;
  Event->SetTime(double(m_OscillatorOffset + m_IntermediateBuffer.front().m_OscillatorValue)/m_OscillatorFrequency);
  Event->SetID(++m_LastEventID);

  // Create a strip hit for all events with the same oscillator value
  while (m_IntermediateBuffer.empty() == false) {
    MReadOutHDF& RO = m_IntermediateBuffer.front();
    if (RO.m_OscillatorValue != m_LastOscillatorValue) break;

    MStripHit* H = new MStripHit();
    H->SetDetectorID(0);
    H->SetStripID(0);
    H->IsPositiveStrip(false);
    H->SetADCUnits(RO.m_ADCValue);
    H->SetTiming(RO.m_TACValue);

    Event->AddStripHit(H);

    m_IntermediateBuffer.pop_front();
  }

  
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsHDF::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();
  
  cout<<"MModuleLoaderMeasurementsHDF: "<<endl;
  cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;

  m_FileHDF5.close();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::Convert()
{
  if (m_NextRawBufferPosition >= m_RawBuffer.size()) {
    // No more data
    return false;
  }

  while (m_IntermediateBufferSize < 100) {
    // Iterate through the buffer and access the variable-length data

    if (m_NextRawBufferPosition < m_RawBuffer.size()) {
      vector<uint32_t> Level2Buffer;
      if (m_RawBuffer[m_NextRawBufferPosition].len > 0) {
        cout<<"Buffer length: "<<m_RawBuffer[m_NextRawBufferPosition].len<<endl;

        // Assuming the data is a byte array, you can access it like this:
        uint32_t* dataPtr = static_cast<uint32_t*>(m_RawBuffer[m_NextRawBufferPosition].p);
        for (size_t j = 0; j < m_RawBuffer[m_NextRawBufferPosition].len; ++j) {
          Level2Buffer.push_back(dataPtr[j]);
        }
      }
      m_NextRawBufferPosition++;

      if (Level2Buffer.size() % 2 != 0) {
        cout<<"ERROR: Dataset (HDF chunk #"<<m_NextRawBufferPosition<<") is not divisable by 2 and thus corrupted. Ignoring!"<<endl;
        continue;
      }

      // Now we need to split the data into uint16's and convert to little endian
      vector<uint16_t> Level3Buffer;
      for (const uint32_t& I: Level2Buffer) {
        uint16_t firstHalf = static_cast<uint16_t>(I & 0xFFFF);
        uint16_t secondHalf = static_cast<uint16_t>((I >> 16) & 0xFFFF);

        firstHalf = ((firstHalf & 0xFF) << 8) | ((firstHalf & 0xFF00) >> 8);
        secondHalf = ((secondHalf & 0xFF) << 8) | ((secondHalf & 0xFF00) >> 8);

        Level3Buffer.push_back(firstHalf);
        Level3Buffer.push_back(secondHalf);
      }

      // and convert to ASIC data sets
      for (unsigned int i = 0; i < Level3Buffer.size(); i+=4) {
        MReadOutHDF HDF;
        uint16_t headers_orig = Level3Buffer[i+0];
        uint16_t osc_chan = Level3Buffer[i+1];
        uint16_t index = Level3Buffer[i+2];
        uint16_t adc_orig = Level3Buffer[i+3];

        HDF.m_DT = (headers_orig >> 10) & 0x1;
        HDF.m_AsicID = (osc_chan & 0x000f) >> 1;
        HDF.m_ChannelID = ((osc_chan & 0x0001) << 4) + ((index & 0xf000) >> 12);
        HDF.m_ADCValue = (adc_orig & 0x3fff);
        HDF.m_TACValue = ((index & 0x0fff) << 2) + ((adc_orig & 0xc000) >> 14);
        HDF.m_OscillatorValue = ((osc_chan & 0xfff0) >> 4) + ((headers_orig & 0x00ff) << 12);
        HDF.m_Headers = (headers_orig >> 8);

        m_IntermediateBuffer.push_back(HDF);
        m_IntermediateBufferSize++;
      }
    } else {
      // No more data
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameHDF5Node = Node->GetNode("FileNameHDF5");
  if (FileNameHDF5Node != nullptr) {
    m_FileName = FileNameHDF5Node->GetValue();
  }

  MXmlNode* FileNameStripMapNode = Node->GetNode("FileNameStripMap");
  if (FileNameStripMapNode != nullptr) {
    m_FileNameStripMap = FileNameStripMapNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderMeasurementsHDF::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileNameHDF5", m_FileName);
  new MXmlNode(Node, "FileNameStripMap", m_FileNameStripMap);

  return Node;
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsHDF::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderMeasurementsHDF* Options = new MGUIOptionsLoaderMeasurementsHDF(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleLoaderMeasurementsHDF.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
