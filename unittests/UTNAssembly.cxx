/*
 * UTNAssembly.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>
using namespace std;

// ROOT libs:
#include "TRandom.h"
#include "TROOT.h"

// MEGAlib:
#include "MFretalonRegistry.h"
#include "MGlobal.h"
#include "MModule.h"
#include "MSupervisor.h"
#include "MUnitTest.h"

// Nuclearizer:
#include "MAssembly.h"


//! Unit test class for MAssembly
class UTNAssembly : public MUnitTest
{
public:
  UTNAssembly() : MUnitTest("UTNAssembly") {}
  virtual ~UTNAssembly() {}

  virtual bool Run();

private:
  //! Test construction and interrupt forwarding
  bool TestDefaultConstructionAndInterrupt();
  //! Test public module type constants
  bool TestModuleTypeConstants();
  //! Test that construction registers the nuclearizer read-out data types
  bool TestReadOutDataRegistration();
  //! Test that construction makes all nuclearizer modules available in the supervisor
  bool TestAvailableModules();
  //! Test that construction seeds the random number generator reproducibly
  bool TestFixedRandomSeed();
  //! Test command-line parser branches that do not launch analysis or the UI
  bool TestParseCommandLine();
  //! Test the supervisor state and messages produced by accepted command-line options
  bool TestParseCommandLineSideEffects();

  //! Run ParseCommandLine and capture its direct cout output
  bool ParseAndCapture(MAssembly& Assembly, vector<char*>& Args, MString& Output);
  //! Run ParseCommandLine while suppressing its direct cout output
  bool ParseSilently(MAssembly& Assembly, vector<char*>& Args);
  //! Copy a file name into a mutable buffer usable as an element of argv
  void ToArgument(const MString& Value, char* Buffer, unsigned int Size);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstructionAndInterrupt() && Passed;
  Passed = TestModuleTypeConstants() && Passed;
  Passed = TestReadOutDataRegistration() && Passed;
  Passed = TestAvailableModules() && Passed;
  Passed = TestFixedRandomSeed() && Passed;
  Passed = TestParseCommandLine() && Passed;
  Passed = TestParseCommandLineSideEffects() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::ParseAndCapture(MAssembly& Assembly, vector<char*>& Args, MString& Output)
{
  // ParseCommandLine() writes its own diagnostics with plain cout, so capture that stream.
  // The supervisor calls it triggers - loading a configuration file, launching the UI -
  // report through the MEGAlib streams instead, so silence those for the duration of the call.
  streambuf* OldBuffer = cout.rdbuf();
  ostringstream Sink;
  cout.rdbuf(Sink.rdbuf());
  DisableDefaultStreams();
  bool Result = Assembly.ParseCommandLine((int) Args.size(), Args.data());
  EnableDefaultStreams();
  cout.rdbuf(OldBuffer);
  Output = Sink.str().c_str();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::ParseSilently(MAssembly& Assembly, vector<char*>& Args)
{
  MString Output;
  return ParseAndCapture(Assembly, Args, Output);
}


////////////////////////////////////////////////////////////////////////////////


void UTNAssembly::ToArgument(const MString& Value, char* Buffer, unsigned int Size)
{
  strncpy(Buffer, Value.Data(), Size - 1);
  Buffer[Size - 1] = '\0';
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestDefaultConstructionAndInterrupt()
{
  bool Passed = true;

  MAssembly Assembly;

  Passed = Evaluate("MAssembly()", "default verbosity", "Construction sets g_Verbosity to c_Error",
                    g_Verbosity, c_Error) && Passed;

  MSupervisor* Supervisor = MSupervisor::GetSupervisor();
  Assembly.SetInterrupt(true);
  Passed = EvaluateTrue("SetInterrupt()", "true", "SetInterrupt(true) forwards a hard interrupt to the supervisor",
                        Supervisor->GetHardInterrupt() == true) && Passed;

  Assembly.SetInterrupt(false);
  Passed = EvaluateFalse("SetInterrupt()", "false", "SetInterrupt(false) clears the supervisor hard interrupt",
                         Supervisor->GetHardInterrupt()) && Passed;

  // SetInterrupt() defaults to true
  Assembly.SetInterrupt();
  Passed = EvaluateTrue("SetInterrupt()", "no argument", "SetInterrupt() without an argument sets a hard interrupt",
                        Supervisor->GetHardInterrupt() == true) && Passed;
  Assembly.SetInterrupt(false);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestModuleTypeConstants()
{
  bool Passed = true;

  Passed = Evaluate("c_EventLoader", "bit value", "c_EventLoader is bit 0",
                    MAssembly::c_EventLoader, (uint64_t) 1 << 0) && Passed;
  Passed = Evaluate("c_EventLoaderSimulation", "bit value", "c_EventLoaderSimulation is bit 1",
                    MAssembly::c_EventLoaderSimulation, (uint64_t) 1 << 1) && Passed;
  Passed = Evaluate("c_EventLoaderMeasurement", "bit value", "c_EventLoaderMeasurement is bit 2",
                    MAssembly::c_EventLoaderMeasurement, (uint64_t) 1 << 2) && Passed;
  Passed = Evaluate("c_EventOrdering", "bit value", "c_EventOrdering is bit 3",
                    MAssembly::c_EventOrdering, (uint64_t) 1 << 3) && Passed;
  Passed = Evaluate("c_Coincidence", "bit value", "c_Coincidence is bit 4",
                    MAssembly::c_Coincidence, (uint64_t) 1 << 4) && Passed;
  Passed = Evaluate("c_TACcut", "bit value", "c_TACcut is bit 5",
                    MAssembly::c_TACcut, (uint64_t) 1 << 5) && Passed;
  Passed = Evaluate("c_NearestNeighbor", "bit value", "c_NearestNeighbor is bit 6",
                    MAssembly::c_NearestNeighbor, (uint64_t) 1 << 6) && Passed;
  Passed = Evaluate("c_DetectorEffectsEngine", "bit value", "c_DetectorEffectsEngine is bit 7",
                    MAssembly::c_DetectorEffectsEngine, (uint64_t) 1 << 7) && Passed;
  Passed = Evaluate("c_EventFilter", "bit value", "c_EventFilter is bit 8",
                    MAssembly::c_EventFilter, (uint64_t) 1 << 8) && Passed;
  Passed = Evaluate("c_EnergyCalibration", "bit value", "c_EnergyCalibration is bit 9",
                    MAssembly::c_EnergyCalibration, (uint64_t) 1 << 9) && Passed;
  Passed = Evaluate("c_ChargeSharingCorrection", "bit value", "c_ChargeSharingCorrection is bit 10",
                    MAssembly::c_ChargeSharingCorrection, (uint64_t) 1 << 10) && Passed;
  Passed = Evaluate("c_DepthCorrection", "bit value", "c_DepthCorrection is bit 11",
                    MAssembly::c_DepthCorrection, (uint64_t) 1 << 11) && Passed;
  Passed = Evaluate("c_StripPairing", "bit value", "c_StripPairing is bit 12",
                    MAssembly::c_StripPairing, (uint64_t) 1 << 12) && Passed;
  Passed = Evaluate("c_Aspect", "bit value", "c_Aspect is bit 13",
                    MAssembly::c_Aspect, (uint64_t) 1 << 13) && Passed;
  Passed = Evaluate("c_CrosstalkCorrection", "bit value", "c_CrosstalkCorrection is bit 14",
                    MAssembly::c_CrosstalkCorrection, (uint64_t) 1 << 14) && Passed;
  Passed = Evaluate("c_EventReconstruction", "bit value", "c_EventReconstruction is bit 15",
                    MAssembly::c_EventReconstruction, (uint64_t) 1 << 15) && Passed;
  Passed = Evaluate("c_Else", "bit value", "c_Else is bit 16",
                    MAssembly::c_Else, (uint64_t) 1 << 16) && Passed;
  Passed = Evaluate("c_NoRestriction", "bit value", "c_NoRestriction is bit 17",
                    MAssembly::c_NoRestriction, (uint64_t) 1 << 17) && Passed;
  Passed = Evaluate("c_EventSaver", "bit value", "c_EventSaver is bit 18",
                    MAssembly::c_EventSaver, (uint64_t) 1 << 18) && Passed;
  Passed = Evaluate("c_EventTransmitter", "bit value", "c_EventTransmitter is bit 19",
                    MAssembly::c_EventTransmitter, (uint64_t) 1 << 19) && Passed;
  Passed = Evaluate("c_PositionDetermiation", "bit value", "c_PositionDetermiation is bit 20",
                    MAssembly::c_PositionDetermiation, (uint64_t) 1 << 20) && Passed;
  Passed = Evaluate("c_Statistics", "bit value", "c_Statistics is bit 21",
                    MAssembly::c_Statistics, (uint64_t) 1 << 21) && Passed;
  Passed = Evaluate("c_FlagHits", "bit value", "c_FlagHits is bit 22",
                    MAssembly::c_FlagHits, (uint64_t) 1 << 22) && Passed;
  Passed = Evaluate("c_Diagnostics", "bit value", "c_Diagnostics is bit 23",
                    MAssembly::c_Diagnostics, (uint64_t) 1 << 23) && Passed;
  Passed = Evaluate("c_ResponseGeneration", "bit value", "c_ResponseGeneration is bit 24",
                    MAssembly::c_ResponseGeneration, (uint64_t) 1 << 24) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestReadOutDataRegistration()
{
  bool Passed = true;

  // The read-out data types are registered by the constructor and are looked up by
  // their type string, not by their class name
  MAssembly Assembly;

  MFretalonRegistry& Registry = MFretalonRegistry::Instance();

  Passed = EvaluateTrue("MAssembly()", "read-out data \"tac\"", "Construction registers the TAC read-out data type", Registry.IsReadOutDataRegistered("tac")) && Passed;
  Passed = EvaluateTrue("MAssembly()", "read-out data \"energy\"", "Construction registers the energy read-out data type", Registry.IsReadOutDataRegistered("energy")) && Passed;
  Passed = EvaluateFalse("MAssembly()", "read-out data \"doesnotexist\"", "A read-out data type which was never registered is not reported as registered", Registry.IsReadOutDataRegistered("doesnotexist")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestAvailableModules()
{
  bool Passed = true;

  MAssembly Assembly;
  MSupervisor* Supervisor = MSupervisor::GetSupervisor();

  // The supervisor is a process-wide singleton whose available-module list only ever grows,
  // so every check here is a lookup by XML tag - never a total count and never an assertion
  // that a module occurs only once
  vector<MString> XmlTags = { "XmlTagLoaderSimulations", "XmlTagMeasurementLoaderROA", "XmlTagMeasurementLoaderHDF",
                              "XmlTagMeasurementLoaderFITS", "XmlTagMeasurementLoaderL0", "XmlTagDEESMEX",
                              "XmlTagEventFilter", "EnergyCalibration", "XmlTagStripPairingMultiRoundChiSquare",
                              "XmlTagStripPairingChiSquare", "DepthCalibration", "XmlTagEventSaver",
                              "XmlTagSaverMeasurementsL0", "XmlTagSaverMeasurementsFITS", "XmlTagTransmitterRealta",
                              "XmlTagResponseGenerator", "XmlTagRevan", "XmlTagTACcut", "XmlTagDiagnostics",
                              "XmlTagDiagnosticsEnergyPerStrip" };

  for (MString XmlTag: XmlTags) {
    Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", XmlTag, "Construction makes the module with this XML tag available in the supervisor", Supervisor->GetAvailableModuleByXmlTag(XmlTag) != nullptr) && Passed;
  }

  // The nearest neighbor module is deliberately not registered - its AddAvailableModule() call
  // in the MAssembly constructor is commented out
  Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", "XmlTagNearestNeighbor", "The nearest neighbor module is deliberately not available", Supervisor->GetAvailableModuleByXmlTag("XmlTagNearestNeighbor") == nullptr) && Passed;
  Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", "XmlTagDoesNotExist", "An XML tag which belongs to no module is not found", Supervisor->GetAvailableModuleByXmlTag("XmlTagDoesNotExist") == nullptr) && Passed;

  // Modules can also be found by their user visible name, which is how old configuration files are read
  MModule* EventFilter = Supervisor->GetAvailableModuleByName("Event Filter");
  Passed = EvaluateTrue("GetAvailableModuleByName()", "Event Filter", "A module can also be found by its name", EventFilter != nullptr) && Passed;
  if (EventFilter != nullptr) {
    Passed = Evaluate("GetAvailableModuleByName()", "Event Filter", "The module found by name carries the matching XML tag", EventFilter->GetXmlTag(), MString("XmlTagEventFilter")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestFixedRandomSeed()
{
  bool Passed = true;

  // The constructor seeds gRandom with a fixed value so that detector effects engine
  // results are reproducible - two assemblies must therefore produce the same sequence
  const unsigned int NumberOfDraws = 5;

  double FirstSequence[NumberOfDraws];
  {
    MAssembly First;
    for (unsigned int i = 0; i < NumberOfDraws; ++i) {
      FirstSequence[i] = gRandom->Rndm();
    }
  }

  double SecondSequence[NumberOfDraws];
  {
    MAssembly Second;
    for (unsigned int i = 0; i < NumberOfDraws; ++i) {
      SecondSequence[i] = gRandom->Rndm();
    }
  }

  for (unsigned int i = 0; i < NumberOfDraws; ++i) {
    Passed = EvaluateNear("MAssembly()", "fixed random seed", "Two assemblies produce the same random number sequence", SecondSequence[i], FirstSequence[i], 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLine()
{
  bool Passed = true;

  MAssembly Assembly;

  // -a/--auto and -t/--test are not tested here: both run the full analysis in the supervisor
  // and call Exit(), which cannot be exercised from a unit test

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--help";
    vector<char*> Args = { Arg0, Arg1 };
    MString Output;
    Passed = EvaluateFalse("ParseCommandLine()", "--help", "ParseCommandLine() returns false after printing help",
                           ParseAndCapture(Assembly, Args, Output)) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "--help output", "The help output contains the usage header",
                          Output.Contains("Usage: Nuclearizer")) && Passed;
    Passed = EvaluateFalse("HasCommandLineError()", "--help", "Printing the help is not a command line error",
                           Assembly.HasCommandLineError()) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-h";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "-h", "ParseCommandLine() returns false for the short help option",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-?";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "-?", "ParseCommandLine() returns false for the -? help option",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "?";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "?", "ParseCommandLine() returns false for the ? help option",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-c";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "missing -c argument", "ParseCommandLine() returns false when -c has no filename",
                           ParseSilently(Assembly, Args)) && Passed;
    Passed = EvaluateTrue("HasCommandLineError()", "missing -c argument", "A missing argument is flagged as a command line error",
                          Assembly.HasCommandLineError()) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--geometry";
    vector<char*> Args = { Arg0, Arg1 };
    MString Output;
    Passed = EvaluateFalse("ParseCommandLine()", "missing --geometry argument", "ParseCommandLine() returns false when --geometry has no filename",
                           ParseAndCapture(Assembly, Args, Output)) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "missing --geometry message", "Missing-argument diagnostics name the full long option",
                          Output.Contains("Option --geometry needs a second argument")) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-m";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "missing -m argument", "ParseCommandLine() returns false when -m has no value",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-v";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "missing -v argument", "ParseCommandLine() returns false when -v has no value",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-C";
    vector<char*> Args = { Arg0, Arg1 };
    Passed = EvaluateFalse("ParseCommandLine()", "missing -C argument", "ParseCommandLine() returns false when -C has no pattern",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  // An argument may not start with a "-", so a negative value is reported as a missing
  // argument and never reaches the range check below
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-v";
    char Arg2[] = "-1";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    Passed = EvaluateFalse("ParseCommandLine()", "negative -v argument", "ParseCommandLine() returns false for a negative verbosity",
                           ParseAndCapture(Assembly, Args, Output)) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "negative -v message", "A negative value is reported as a missing argument",
                          Output.Contains("Option -v needs a second argument")) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--unknown-option";
    char Arg2[] = "-c";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    Passed = EvaluateFalse("ParseCommandLine()", "unknown option warning", "ParseCommandLine() continues after warning about an unknown option",
                           ParseAndCapture(Assembly, Args, Output)) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "unknown option warning text", "Unknown options emit a warning",
                          Output.Contains("WARNING: Command-line parser: Unknown option: --unknown-option")) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-v";
    char Arg2[] = "abc";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    Passed = EvaluateFalse("ParseCommandLine()", "invalid -v argument", "ParseCommandLine() returns false when -v is not an integer",
                           ParseSilently(Assembly, Args)) && Passed;
    Passed = EvaluateTrue("HasCommandLineError()", "invalid -v argument", "An invalid value is flagged as a command line error",
                          Assembly.HasCommandLineError()) && Passed;
  }

  // The error flag belongs to the last call only, so a good command line clears it again
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--help";
    vector<char*> Args = { Arg0, Arg1 };
    ParseSilently(Assembly, Args);
    Passed = EvaluateFalse("HasCommandLineError()", "help after an invalid command line", "The error flag is reset at the beginning of each call",
                           Assembly.HasCommandLineError()) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--verbosity";
    char Arg2[] = "4";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    Passed = EvaluateFalse("ParseCommandLine()", "out-of-range --verbosity argument", "ParseCommandLine() returns false when verbosity is outside 0..3",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  // A value must be a whole number: a decimal is rejected instead of being truncated silently
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-v";
    char Arg2[] = "2.5";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    Passed = EvaluateFalse("ParseCommandLine()", "decimal -v argument", "ParseCommandLine() returns false when -v is not a whole number",
                           ParseAndCapture(Assembly, Args, Output)) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "decimal -v message", "A decimal verbosity is reported as a non-integer argument",
                          Output.Contains("Option -v needs an integer argument, not \"2.5\"")) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-v";
    char Arg2[] = "2e0";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    Passed = EvaluateFalse("ParseCommandLine()", "exponential -v argument", "ParseCommandLine() returns false when -v uses an exponent",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-m";
    char Arg2[] = "abc";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    Passed = EvaluateFalse("ParseCommandLine()", "invalid -m argument", "ParseCommandLine() returns false when -m is not an integer",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-m";
    char Arg2[] = "0.4";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    Passed = EvaluateFalse("ParseCommandLine()", "decimal -m argument", "ParseCommandLine() returns false when -m is not a whole number",
                           ParseSilently(Assembly, Args)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineSideEffects()
{
  bool Passed = true;

  MAssembly Assembly;
  MSupervisor* Supervisor = MSupervisor::GetSupervisor();

  // The supervisor and the verbosity are process-wide state, so remember and restore them
  int OldVerbosity = g_Verbosity;
  MString OldGeometryFileName = Supervisor->GetGeometryFileName();

  // An accepted command line without -a or -t ends in MSupervisor::LaunchUI(), whose result
  // depends on whether a display is available. The return value of ParseCommandLine() therefore
  // carries no information in these cases and only the supervisor state is checked.

  // The geometry file names below are never opened, they are only stored in the supervisor
  const MString GeometryFileName = "/tmp/UTNAssembly_Geometry.geo.setup";
  const MString LongGeometryFileName = "/tmp/UTNAssembly_LongGeometry.geo.setup";
  const MString ConfiguredGeometryFileName = "/tmp/UTNAssembly_ConfiguredGeometry.geo.setup";
  const MString ChangedGeometryFileName = "/tmp/UTNAssembly_ChangedGeometry.geo.setup";

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-g";
    char Arg2[1024];
    ToArgument(GeometryFileName, Arg2, sizeof(Arg2));
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    ParseSilently(Assembly, Args);
    Passed = Evaluate("ParseCommandLine()", "-g <filename>", "-g stores the geometry file name in the supervisor", Supervisor->GetGeometryFileName(), GeometryFileName) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--geometry";
    char Arg2[1024];
    ToArgument(LongGeometryFileName, Arg2, sizeof(Arg2));
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    ParseSilently(Assembly, Args);
    Passed = Evaluate("ParseCommandLine()", "--geometry <filename>", "--geometry stores the geometry file name in the supervisor", Supervisor->GetGeometryFileName(), LongGeometryFileName) && Passed;
  }

  // Help is handled in the very first loop, so it wins wherever it appears and no other
  // option on the same command line may reach the supervisor
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-g";
    char Arg2[1024];
    ToArgument(GeometryFileName, Arg2, sizeof(Arg2));
    char Arg3[] = "--help";
    vector<char*> Args = { Arg0, Arg1, Arg2, Arg3 };
    MString Output;
    Passed = EvaluateFalse("ParseCommandLine()", "-g <filename> --help", "A command line containing --help returns false", ParseAndCapture(Assembly, Args, Output)) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "-g <filename> --help", "A command line containing --help prints the usage", Output.Contains("Usage: Nuclearizer")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-g <filename> --help", "--help short-circuits before any other option changes the supervisor", Supervisor->GetGeometryFileName(), LongGeometryFileName) && Passed;
  }

  // -c loads a real configuration file, so build a minimal one and verify that its content arrives
  {
    MString ConfigurationFileName = GetTemporaryFileName("UTNAssembly.cfg");
    ofstream Out(ConfigurationFileName.Data());
    Passed = EvaluateTrue("ParseCommandLine()", "write configuration fixture", "The configuration fixture file can be created", Out.is_open()) && Passed;
    Out<<"<NuclearizerData>"<<endl;
    Out<<"  <Version>1</Version>"<<endl;
    Out<<"  <ModuleSequence>"<<endl;
    Out<<"    <ModuleSequenceItem>XmlTagLoaderSimulations</ModuleSequenceItem>"<<endl;
    Out<<"    <ModuleSequenceItem>XmlTagEventFilter</ModuleSequenceItem>"<<endl;
    Out<<"  </ModuleSequence>"<<endl;
    Out<<"  <GeometryFileName>"<<ConfiguredGeometryFileName<<"</GeometryFileName>"<<endl;
    Out<<"</NuclearizerData>"<<endl;
    Out.close();

    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-c";
    char Arg2[1024];
    ToArgument(ConfigurationFileName, Arg2, sizeof(Arg2));
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);

    Passed = EvaluateTrue("ParseCommandLine()", "-c <filename>", "-c reports the configuration file it uses", Output.Contains("Command-line parser: Use configuration file")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-c <filename>", "-c loads the geometry file name from the configuration file", Supervisor->GetGeometryFileName(), ConfiguredGeometryFileName) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-c <filename>", "-c loads the module sequence from the configuration file", (unsigned int) Supervisor->GetNModules(), (unsigned int) 2) && Passed;
    if (Supervisor->GetNModules() == 2) {
      Passed = Evaluate("ParseCommandLine()", "-c <filename>", "The first module of the loaded sequence is the simulation loader", Supervisor->GetModule(0)->GetXmlTag(), MString("XmlTagLoaderSimulations")) && Passed;
      Passed = Evaluate("ParseCommandLine()", "-c <filename>", "The second module of the loaded sequence is the event filter", Supervisor->GetModule(1)->GetXmlTag(), MString("XmlTagEventFilter")) && Passed;
    }

    // The options are applied in the order of the parser loops, not in the order they were
    // given: -c is always loaded first, so -C changes a value of the freshly loaded file
    // even when -C appears before -c on the command line
    MString Pattern = MString("GeometryFileName=") + ChangedGeometryFileName;
    char OrderArg0[] = "UTNAssembly";
    char OrderArg1[] = "-C";
    char OrderArg2[1024];
    ToArgument(Pattern, OrderArg2, sizeof(OrderArg2));
    char OrderArg3[] = "-c";
    char OrderArg4[1024];
    ToArgument(ConfigurationFileName, OrderArg4, sizeof(OrderArg4));
    vector<char*> OrderArgs = { OrderArg0, OrderArg1, OrderArg2, OrderArg3, OrderArg4 };
    ParseSilently(Assembly, OrderArgs);
    Passed = Evaluate("ParseCommandLine()", "-C before -c", "-c is applied before -C regardless of the order on the command line", Supervisor->GetGeometryFileName(), ChangedGeometryFileName) && Passed;

    RemoveTemporaryFile(ConfigurationFileName);
  }

  // A configuration file which does not exist is not an error: MSupervisor::Load() clears the
  // old configuration, reports through the MEGAlib streams, and parsing continues
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-c";
    char Arg2[] = "/tmp/UTNAssembly_DoesNotExist.cfg";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);

    Passed = EvaluateTrue("ParseCommandLine()", "-c <missing file>", "-c warns when the configuration file cannot be loaded", Output.Contains("WARNING: Command-line parser: Unable to load configuration file")) && Passed;
    Passed = EvaluateFalse("ParseCommandLine()", "-c <missing file>", "-c does not claim to use a configuration file it could not load", Output.Contains("Command-line parser: Use configuration file")) && Passed;
    Passed = EvaluateFalse("HasCommandLineError()", "-c <missing file>", "A configuration file which cannot be loaded is not a command line error", Assembly.HasCommandLineError()) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-c <missing file>", "A missing configuration file leaves the supervisor with a cleared geometry file name", Supervisor->GetGeometryFileName(), MString("")) && Passed;
  }

  // -C changes a single field of the configuration
  {
    MString Pattern = MString("GeometryFileName=") + ChangedGeometryFileName;
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-C";
    char Arg2[1024];
    ToArgument(Pattern, Arg2, sizeof(Arg2));
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);

    Passed = EvaluateTrue("ParseCommandLine()", "-C <pattern>", "-C reports the configuration value it changes", Output.Contains("Command-line parser: Changing this configuration value")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-C <pattern>", "-C changes the geometry file name in the supervisor", Supervisor->GetGeometryFileName(), ChangedGeometryFileName) && Passed;
  }

  // The usage text promises that -C can be given more than once, so the last one must win
  {
    MString FirstPattern = MString("GeometryFileName=") + ConfiguredGeometryFileName;
    MString SecondPattern = MString("GeometryFileName=") + ChangedGeometryFileName;
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-C";
    char Arg2[1024];
    ToArgument(FirstPattern, Arg2, sizeof(Arg2));
    char Arg3[] = "-C";
    char Arg4[1024];
    ToArgument(SecondPattern, Arg4, sizeof(Arg4));
    vector<char*> Args = { Arg0, Arg1, Arg2, Arg3, Arg4 };
    ParseSilently(Assembly, Args);
    Passed = Evaluate("ParseCommandLine()", "two -C patterns", "-C can be given multiple times and the last one wins", Supervisor->GetGeometryFileName(), ChangedGeometryFileName) && Passed;
  }

  // A pattern which names no existing configuration node is reported but does not stop the parser
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-C";
    char Arg2[] = "DoesNotExist=1";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);

    Passed = EvaluateTrue("ParseCommandLine()", "-C <unknown pattern>", "An unknown configuration field is reported as an error", Output.Contains("Unable to change this configuration value: DoesNotExist=1")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-C <unknown pattern>", "An unknown configuration field leaves the geometry file name untouched", Supervisor->GetGeometryFileName(), ChangedGeometryFileName) && Passed;
  }

  // -g is parsed after -C, so it wins if both name the geometry file
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-C";
    char Arg2[1024];
    MString Pattern = MString("GeometryFileName=") + ChangedGeometryFileName;
    ToArgument(Pattern, Arg2, sizeof(Arg2));
    char Arg3[] = "-g";
    char Arg4[1024];
    ToArgument(GeometryFileName, Arg4, sizeof(Arg4));
    vector<char*> Args = { Arg0, Arg1, Arg2, Arg3, Arg4 };
    ParseSilently(Assembly, Args);
    Passed = Evaluate("ParseCommandLine()", "-C and -g", "-g overrides a geometry file name set with -C", Supervisor->GetGeometryFileName(), GeometryFileName) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-m";
    char Arg2[] = "1";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);
    Passed = EvaluateTrue("ParseCommandLine()", "-m 1", "-m 1 enables multithreading", Output.Contains("Command-line parser: Using multithreading: yes")) && Passed;
  }

  // The usage text documents the value as "0: false (default), else: true"
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-m";
    char Arg2[] = "2";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);
    Passed = EvaluateTrue("ParseCommandLine()", "-m 2", "Any non-zero value for -m enables multithreading", Output.Contains("Command-line parser: Using multithreading: yes")) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--multithreading";
    char Arg2[] = "0";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);
    Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 0", "--multithreading 0 disables multithreading", Output.Contains("Command-line parser: Using multithreading: no")) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "-v";
    char Arg2[] = "0";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    ParseSilently(Assembly, Args);
    Passed = Evaluate("ParseCommandLine()", "-v 0", "-v 0 sets the verbosity to c_Quiet", g_Verbosity, c_Quiet) && Passed;
  }

  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--verbosity";
    char Arg2[] = "3";
    vector<char*> Args = { Arg0, Arg1, Arg2 };
    ParseSilently(Assembly, Args);
    Passed = Evaluate("ParseCommandLine()", "--verbosity 3", "--verbosity 3 sets the verbosity to c_Info", g_Verbosity, c_Info) && Passed;
  }

  // A warning about an unknown option must not stop the options after it from being parsed
  {
    g_Verbosity = c_Error;
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "--unknown-option";
    char Arg2[] = "-v";
    char Arg3[] = "3";
    vector<char*> Args = { Arg0, Arg1, Arg2, Arg3 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);
    Passed = EvaluateTrue("ParseCommandLine()", "unknown option before a valid one", "An unknown option emits a warning", Output.Contains("WARNING: Command-line parser: Unknown option: --unknown-option")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "unknown option before a valid one", "Parsing continues after the warning and still applies -v", g_Verbosity, c_Info) && Passed;
  }

  // A stray argument which is not an option is silently ignored
  {
    char Arg0[] = "UTNAssembly";
    char Arg1[] = "SomeFile.roa";
    vector<char*> Args = { Arg0, Arg1 };
    MString Output;
    ParseAndCapture(Assembly, Args, Output);
    Passed = EvaluateFalse("ParseCommandLine()", "stray argument", "An argument which is not an option does not trigger an unknown-option warning", Output.Contains("Unknown option")) && Passed;
  }

  g_Verbosity = OldVerbosity;
  Supervisor->SetGeometryFileName(OldGeometryFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNAssembly", "Unit tests for MAssembly") == false) return 1;

  // An accepted command line ends in MSupervisor::LaunchUI(): make sure no test can ever
  // open a window, no matter which options a future test case passes
  gROOT->SetBatch(true);

  UTNAssembly Test;
  return Test.Run() == true ? 0 : 1;
}
