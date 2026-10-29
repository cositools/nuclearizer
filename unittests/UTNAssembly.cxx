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
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT:
#include "TSystem.h"
#include "TRandom.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MFile.h"
#include "MSupervisor.h"
#include "MModule.h"
#include "MFretalonRegistry.h"

// Nuclearizer:
#include "MAssembly.h"


//! Unit test class for MAssembly
class UTNAssembly : public MUnitTest
{
 public:
  //! Default constructor
  UTNAssembly() : MUnitTest("UTNAssembly") {}
  //! Default destructor
  virtual ~UTNAssembly() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test that the module type constants are distinct single bits
  bool TestModuleTypeConstants();
  //! Test the constructor: registry, verbosity, and available modules
  bool TestConstruction(MAssembly& A);
  //! Test SetInterrupt()
  bool TestSetInterrupt(MAssembly& A);
  //! Test the help options and the usage text
  bool TestParseCommandLineHelp(MAssembly& A);
  //! Test ParseCommandLine() with options missing their argument
  bool TestParseCommandLineMissingArguments(MAssembly& A);
  //! Test ParseCommandLine() with invalid values for -v and -m
  bool TestParseCommandLineInvalidValues(MAssembly& A);
  //! Test the supervisor state set by accepted options
  bool TestParseCommandLineOptions(MAssembly& A);
  //! Test that a failed -c or -C stops the parser
  bool TestParseCommandLineConfigurationErrors(MAssembly& A);
  //! Test that a failed analysis (-a) or test run (-t) is reported as error
  bool TestParseCommandLineAnalysisErrors(MAssembly& A);

  //! Call ParseCommandLine() with the given arguments (without the program name), the output is discarded
  int CallParseCommandLine(MAssembly& A, const vector<MString>& Arguments);
  //! Call ParseCommandLine() with the given arguments (without the program name), the output is returned in Output
  int CallParseCommandLine(MAssembly& A, const vector<MString>& Arguments, MString& Output);
  //! Call ParseCommandLine() in a child process and return its return value, or c_ChildFailed if the child did not exit normally
  int CallParseCommandLineInChild(MAssembly& A, const vector<MString>& Arguments);

  //! Return value of CallParseCommandLineInChild() if the child crashed, timed out, or could not be started
  static const int c_ChildFailed = -2;
  //! Write a configuration file with the given module sequence and geometry file name, return its name or "" on failure
  MString WriteConfiguration(const MString& Name, const vector<MString>& Sequence, const MString& GeometryFileName);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::Run()
{
  bool Passed = true;

  Passed = TestModuleTypeConstants() && Passed;

  // MAssembly reads and writes ~/.nuclearizer.cfg: MUnitTest points HOME to its private temporary directory
  int OldVerbosity = g_Verbosity;
  MAssembly A;
  int AssemblyVerbosity = g_Verbosity;
  g_Verbosity = OldVerbosity;

  // The constructor seeds gRandom with a fixed value to reproduce DEE results
  const unsigned int NumberOfDraws = 5;
  vector<double> AfterConstruction;
  for (unsigned int i = 0; i < NumberOfDraws; ++i) AfterConstruction.push_back(gRandom->Rndm());
  gRandom->SetSeed(20170912);
  for (unsigned int i = 0; i < NumberOfDraws; ++i) {
    Passed = EvaluateNear("MAssembly()", MString("random draw ") + i, "The constructor seeds gRandom with 20170912", AfterConstruction[i], gRandom->Rndm(), 1e-12) && Passed;
  }

  Passed = Evaluate("MAssembly()", "verbosity", "The constructor sets the verbosity to c_Error", AssemblyVerbosity, (int) c_Error) && Passed;
  Passed = TestConstruction(A) && Passed;
  Passed = TestSetInterrupt(A) && Passed;
  Passed = TestParseCommandLineHelp(A) && Passed;
  Passed = TestParseCommandLineMissingArguments(A) && Passed;
  Passed = TestParseCommandLineInvalidValues(A) && Passed;
  Passed = TestParseCommandLineOptions(A) && Passed;
  Passed = TestParseCommandLineConfigurationErrors(A) && Passed;
  Passed = TestParseCommandLineAnalysisErrors(A) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTNAssembly::CallParseCommandLine(MAssembly& A, const vector<MString>& Arguments)
{
  MString Output;
  return CallParseCommandLine(A, Arguments, Output);
}


////////////////////////////////////////////////////////////////////////////////


int UTNAssembly::CallParseCommandLine(MAssembly& A, const vector<MString>& Arguments, MString& Output)
{
  vector<MString> All;
  All.push_back("nuclearizer");
  All.insert(All.end(), Arguments.begin(), Arguments.end());

  vector<char*> Argv;
  for (MString& S : All) {
    Argv.push_back(const_cast<char*>(S.Data()));
  }
  Argv.push_back(nullptr);

  ostringstream Capture;
  streambuf* Old = cout.rdbuf(Capture.rdbuf());
  int Result = A.ParseCommandLine((int) All.size(), Argv.data());
  cout.rdbuf(Old);
  Output = Capture.str().c_str();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


int UTNAssembly::CallParseCommandLineInChild(MAssembly& A, const vector<MString>& Arguments)
{
  cout.flush();
  mout.flush();

  // Exit code of the child for the return value -1, which is no valid exit code
  const int UILaunched = 2;
  // Exit code if stdout cannot be redirected
  const int RedirectFailed = 3;

  pid_t Pid = fork();
  if (Pid < 0) return c_ChildFailed;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Child: never return into the test
    // Time-out of the child process in seconds
    const unsigned int ChildTimeout = 30;
    alarm(ChildTimeout);
    if (freopen("/dev/null", "w", stdout) == nullptr) _exit(RedirectFailed);
    int Result = CallParseCommandLine(A, Arguments);
    _exit(Result == -1 ? UILaunched : Result);
  }

  int Status = 0;
  if (waitpid(Pid, &Status, 0) != Pid) return c_ChildFailed;
  if (WIFEXITED(Status) == false) return c_ChildFailed;
  if (WEXITSTATUS(Status) == RedirectFailed) return c_ChildFailed;
  return WEXITSTATUS(Status) == UILaunched ? -1 : WEXITSTATUS(Status);
}


////////////////////////////////////////////////////////////////////////////////


MString UTNAssembly::WriteConfiguration(const MString& Name, const vector<MString>& Sequence, const MString& GeometryFileName)
{
  MString C;
  C += "<NuclearizerData>\n";
  C += "  <Version>1</Version>\n";
  C += "  <ModuleSequence>\n";
  for (const MString& Tag : Sequence) {
    C += MString("    <ModuleSequenceItem>") + Tag + "</ModuleSequenceItem>\n";
  }
  C += "  </ModuleSequence>\n";
  C += MString("  <GeometryFileName>") + GeometryFileName + "</GeometryFileName>\n";
  C += "</NuclearizerData>\n";

  MString FileName = GetTemporaryFileName(Name);
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestModuleTypeConstants()
{
  bool Passed = true;

  vector<uint64_t> Types = { MAssembly::c_EventLoader, MAssembly::c_EventLoaderSimulation, MAssembly::c_EventLoaderMeasurement, MAssembly::c_EventOrdering, MAssembly::c_Coincidence, MAssembly::c_TACCalibration, MAssembly::c_NearestNeighbor, MAssembly::c_DetectorEffectsEngine, MAssembly::c_EventFilter, MAssembly::c_EnergyCalibration, MAssembly::c_ChargeSharingCorrection, MAssembly::c_DepthCorrection, MAssembly::c_StripPairing, MAssembly::c_Aspect, MAssembly::c_CrosstalkCorrection, MAssembly::c_EventReconstruction, MAssembly::c_Else, MAssembly::c_NoRestriction, MAssembly::c_EventSaver, MAssembly::c_EventTransmitter, MAssembly::c_PositionDetermiation, MAssembly::c_Statistics, MAssembly::c_FlagHits, MAssembly::c_Diagnostics, MAssembly::c_ResponseGeneration, MAssembly::c_TrappingCorrection };

  uint64_t Combined = 0;
  for (unsigned int i = 0; i < Types.size(); ++i) {
    MString Input = MString("constant ") + i;
    Passed = EvaluateTrue("Module type constants", Input, "The module type is not zero", Types[i] != 0) && Passed;
    Passed = EvaluateTrue("Module type constants", Input, "The module type has at most one bit set", (Types[i] & (Types[i] - 1)) == 0) && Passed;
    Passed = EvaluateTrue("Module type constants", Input, "The module type is bit i", Types[i] == (uint64_t(1) << i)) && Passed;
    Passed = EvaluateTrue("Module type constants", Input, "The module type does not overlap with any previous type", (Combined & Types[i]) == 0) && Passed;
    Combined |= Types[i];
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestConstruction(MAssembly& A)
{
  bool Passed = true;

  MFretalonRegistry& R = MFretalonRegistry::Instance();
  Passed = EvaluateTrue("MAssembly()", "registry", "The read-out data 'tac' is registered", R.IsReadOutDataRegistered("tac")) && Passed;
  Passed = EvaluateTrue("MAssembly()", "registry", "The read-out data 'energy' is registered", R.IsReadOutDataRegistered("energy")) && Passed;
  Passed = EvaluateTrue("MAssembly()", "registry", "The read-out data 'flags' is registered", R.IsReadOutDataRegistered("flags")) && Passed;
  Passed = EvaluateTrue("MAssembly()", "registry", "The read-out element 'voxel3d' is registered", R.IsReadOutElementRegistered("voxel3d")) && Passed;

  // All modules the constructor should register -- add new modules here
  // Deliberately hard-coded: a list generated from the registry would not catch a module missing in the constructor
  vector<MString> XmlTags = { "XmlTagLoaderSimulations", "XmlTagMeasurementLoaderROA", "XmlTagMeasurementLoaderTRA", "XmlTagMeasurementLoaderHDF",
                              "XmlTagMeasurementLoaderFITS", "XmlTagMeasurementLoaderL0", "XmlTagDEESMEX", "XmlTagEventFilter",
                              "XmlTagEnergyCalibration", "XmlTagStripPairingMultiRoundChiSquare", "XmlTagStripPairingChiSquare",
                              "XmlTagDepthCalibration", "XmlTagTrappingCorrection", "XmlTagEventSaver", "XmlTagSaverMeasurementsL0",
                              "XmlTagSaverMeasurementsFITSL1a", "XmlTagSaverMeasurementsFITSL1b", "XmlTagSaverMeasurementsFITSL2",
                              "XmlTagTransmitterRealta", "XmlTagResponseGenerator", "XmlTagRevan", "XmlTagTACCalibration",
                              "XmlTagDiagnostics", "XmlTagDiagnosticsEnergyPerStrip" };

  MSupervisor* S = MSupervisor::GetSupervisor();
  Passed = EvaluateSize("MAssembly()", "available modules", "The constructor registers exactly the listed modules", S->GetNAvailableModules(), XmlTags.size()) && Passed;
  Passed = EvaluateTrue("MAssembly()", "default", "Multithreading is enabled by default", S->IsUsingMultiThreading()) && Passed;

  set<MString> Tags;
  set<MString> Names;
  bool AllValid = true;
  for (unsigned int m = 0; m < S->GetNAvailableModules(); ++m) {
    MModule* M = S->GetAvailableModule(m);
    if (M == nullptr) {
      AllValid = false;
      continue;
    }
    MString Input = MString("module ") + M->GetXmlTag();
    Passed = EvaluateTrue("MAssembly()", Input, "The XML tag of the module is unique", Tags.insert(M->GetXmlTag()).second) && Passed;
    Passed = EvaluateTrue("MAssembly()", Input, "The name of the module is unique", Names.insert(M->GetName()).second) && Passed;
    Passed = EvaluateFalse("MAssembly()", Input, "The XML tag is not empty", M->GetXmlTag().IsEmpty()) && Passed;
    Passed = EvaluateTrue("MAssembly()", Input, "The module provides at least one module type", M->GetNModuleTypes() > 0) && Passed;
  }
  Passed = EvaluateTrue("MAssembly()", "available modules", "All available modules are non-null", AllValid) && Passed;

  for (MString& Tag : XmlTags) {
    Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", Tag, "The module with this XML tag is available", S->GetAvailableModuleByXmlTag(Tag) != nullptr) && Passed;
  }

  // The nearest neighbor module is deliberately not registered in the constructor
  Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", "XmlTagNearestNeighbor", "The nearest neighbor module is not available", S->GetAvailableModuleByXmlTag("XmlTagNearestNeighbor") == nullptr) && Passed;
  Passed = EvaluateTrue("GetAvailableModuleByXmlTag()", "XmlTagDoesNotExist", "An unknown XML tag is not found", S->GetAvailableModuleByXmlTag("XmlTagDoesNotExist") == nullptr) && Passed;
  Passed = EvaluateFalse("MAssembly()", "registry", "An unknown read-out data type is not registered", R.IsReadOutDataRegistered("doesnotexist")) && Passed;

  // Modules can also be found by their name, which is how old configuration files are read
  MModule* EventFilter = S->GetAvailableModuleByName("Event Filter");
  Passed = EvaluateTrue("GetAvailableModuleByName()", "Event Filter", "A module can be found by its name", EventFilter != nullptr) && Passed;
  if (EventFilter != nullptr) {
    Passed = Evaluate("GetAvailableModuleByName()", "Event Filter", "The module found by name has the matching XML tag", EventFilter->GetXmlTag(), MString("XmlTagEventFilter")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestSetInterrupt(MAssembly& A)
{
  bool Passed = true;

  MSupervisor* S = MSupervisor::GetSupervisor();
  A.SetInterrupt(true);
  Passed = EvaluateTrue("SetInterrupt()", "true", "SetInterrupt(true) sets the hard interrupt of the supervisor", S->GetHardInterrupt()) && Passed;
  A.SetInterrupt(false);
  Passed = EvaluateFalse("SetInterrupt()", "false", "SetInterrupt(false) clears the hard interrupt of the supervisor", S->GetHardInterrupt()) && Passed;
  A.SetInterrupt();
  Passed = EvaluateTrue("SetInterrupt()", "default argument", "SetInterrupt() defaults to true", S->GetHardInterrupt()) && Passed;
  A.SetInterrupt(false);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineHelp(MAssembly& A)
{
  bool Passed = true;

  MString Output;
  Passed = Evaluate("ParseCommandLine()", "-h", "ParseCommandLine returns 0 for -h: printing the help is no error", CallParseCommandLine(A, { "-h" }, Output), 0) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-h", "The help output contains the usage header", Output.Contains("Usage: Nuclearizer")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-h", "The usage names ~/.nuclearizer.cfg as default configuration file", Output.Contains("If no configuration file is given ~/.nuclearizer.cfg is used")) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-h", "The usage does not name the old ~/.nuclearizer.xml.cfg", Output.Contains(".nuclearizer.xml.cfg")) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-h", "The usage names multithreading as default, as set in the constructor", Output.Contains("0: false, else: true (default)")) && Passed;

  Passed = Evaluate("ParseCommandLine()", "--help", "ParseCommandLine returns 0 for --help", CallParseCommandLine(A, { "--help" }), 0) && Passed;
  Passed = Evaluate("ParseCommandLine()", "?", "ParseCommandLine returns 0 for ?", CallParseCommandLine(A, { "?" }), 0) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-?", "ParseCommandLine returns 0 for -?", CallParseCommandLine(A, { "-?" }), 0) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-a -h", "Help takes precedence over -a", CallParseCommandLine(A, { "-a", "-h" }), 0) && Passed;

  // Help is handled first, so no other option on the same command line reaches the supervisor
  MSupervisor* S = MSupervisor::GetSupervisor();
  MString OldGeometryFileName = S->GetGeometryFileName();
  MString GeometryFileName = GetTemporaryFileName("help.geo.setup");
  Passed = Evaluate("ParseCommandLine()", "-g <file> --help", "ParseCommandLine returns 0", CallParseCommandLine(A, { "-g", GeometryFileName, "--help" }), 0) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-g <file> --help", "Help returns before -g changes the supervisor", S->GetGeometryFileName(), OldGeometryFileName) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineMissingArguments(MAssembly& A)
{
  bool Passed = true;

  // A missing argument used to read argv[argc], i.e. a null pointer: run in a child process to survive a crash
  vector<MString> Options = { "-c", "--configuration", "-C", "--change-configuration", "-g", "--geometry", "-m", "--multithreading", "-v", "--verbosity" };
  for (const MString& O : Options) {
    Passed = Evaluate("ParseCommandLine()", O, "ParseCommandLine returns 1 for the option without argument and does not crash", CallParseCommandLineInChild(A, { O }), 1) && Passed;
  }

  // An argument must not start with "-"
  Passed = Evaluate("ParseCommandLine()", "-c -a", "ParseCommandLine returns 1 for another option as argument of -c", CallParseCommandLineInChild(A, { "-c", "-a" }), 1) && Passed;

  MString Output;
  Passed = Evaluate("ParseCommandLine()", "--geometry", "ParseCommandLine returns 1 if --geometry has no argument", CallParseCommandLine(A, { "--geometry" }, Output), 1) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "--geometry", "The error message names the whole option", Output.Contains("Option --geometry needs a second argument")) && Passed;

  // A negative value starts with "-" and is therefore a missing argument
  Passed = Evaluate("ParseCommandLine()", "-v -1", "ParseCommandLine returns 1 for a negative verbosity", CallParseCommandLine(A, { "-v", "-1" }, Output), 1) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-v -1", "A negative value is reported as missing argument", Output.Contains("Option -v needs a second argument")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineInvalidValues(MAssembly& A)
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;

  // Verbosity: whole numbers between c_Quiet and c_Info
  vector<MString> InvalidVerbosities = { "abc", "4", "2.5", "2e0" };
  for (const MString& V : InvalidVerbosities) {
    MString Input = MString("-v ") + V;
    Passed = Evaluate("ParseCommandLine()", Input, "ParseCommandLine returns 1 for an invalid verbosity", CallParseCommandLine(A, { "-v", V }), 1) && Passed;
    Passed = Evaluate("ParseCommandLine()", Input, "An invalid verbosity does not change the verbosity", g_Verbosity, OldVerbosity) && Passed;
  }

  MString Output;
  CallParseCommandLine(A, { "-v", "2.5" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "-v 2.5", "A decimal verbosity is reported as non-integer argument", Output.Contains("Option -v needs an integer argument, not \"2.5\"")) && Passed;
  CallParseCommandLine(A, { "--verbosity", "4" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "--verbosity 4", "An out-of-range verbosity is reported with the valid range", Output.Contains("Option --verbosity needs an argument between 0 and 3")) && Passed;

  // Multithreading: whole numbers
  vector<MString> InvalidMultiThreading = { "abc", "0.4" };
  for (const MString& V : InvalidMultiThreading) {
    MString Input = MString("-m ") + V;
    Passed = Evaluate("ParseCommandLine()", Input, "ParseCommandLine returns 1 for an invalid multithreading value", CallParseCommandLine(A, { "-m", V }), 1) && Passed;
  }

  g_Verbosity = OldVerbosity;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineOptions(MAssembly& A)
{
  bool Passed = true;

  MSupervisor* S = MSupervisor::GetSupervisor();
  int OldVerbosity = g_Verbosity;
  MString OldGeometryFileName = S->GetGeometryFileName();

  // An accepted command line without -a or -t launches the UI. Thus each command line ends with -t on a
  // module sequence which cannot run: the analysis fails before the geometry is loaded, the parser returns
  // without UI, and only the supervisor state is checked here. The geometry files are never opened
  const MString GeometryFileName = GetTemporaryFileName("options.geo.setup");
  const MString LongGeometryFileName = GetTemporaryFileName("options.long.geo.setup");
  const MString ConfiguredGeometryFileName = GetTemporaryFileName("options.configured.geo.setup");
  const MString ChangedGeometryFileName = GetTemporaryFileName("options.changed.geo.setup");

  const MString Empty = WriteConfiguration("options.empty.cfg", { }, "");
  Passed = EvaluateFalse("WriteConfiguration()", "options.empty.cfg", "The fixture file is written", Empty.IsEmpty()) && Passed;
  const MString Configuration = WriteConfiguration("options.cfg", { "XmlTagLoaderSimulations", "XmlTagEventFilter" }, ConfiguredGeometryFileName);
  Passed = EvaluateFalse("WriteConfiguration()", "options.cfg", "The fixture file is written", Configuration.IsEmpty()) && Passed;
  if (Empty.IsEmpty() == true || Configuration.IsEmpty() == true) return false;

  CallParseCommandLine(A, { "-c", Empty, "-g", GeometryFileName, "-t" });
  Passed = Evaluate("ParseCommandLine()", "-g <file>", "-g stores the geometry file name in the supervisor", S->GetGeometryFileName(), GeometryFileName) && Passed;
  CallParseCommandLine(A, { "-c", Empty, "--geometry", LongGeometryFileName, "-t" });
  Passed = Evaluate("ParseCommandLine()", "--geometry <file>", "--geometry stores the geometry file name in the supervisor", S->GetGeometryFileName(), LongGeometryFileName) && Passed;

  // -c loads the module sequence and the geometry file name
  MString Output;
  CallParseCommandLine(A, { "-c", Configuration, "-t" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "-c <file>", "-c reports the configuration file it uses", Output.Contains("Command-line parser: Use configuration file")) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-c <file>", "-c loads the geometry file name", S->GetGeometryFileName(), ConfiguredGeometryFileName) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-c <file>", "-c loads the module sequence", S->GetNModules(), 2u) && Passed;
  if (S->GetNModules() == 2) {
    Passed = Evaluate("ParseCommandLine()", "-c <file>", "The first module is the simulation loader", S->GetModule(0)->GetXmlTag(), MString("XmlTagLoaderSimulations")) && Passed;
    Passed = Evaluate("ParseCommandLine()", "-c <file>", "The second module is the event filter", S->GetModule(1)->GetXmlTag(), MString("XmlTagEventFilter")) && Passed;
  }

  // -c is always applied first and -C afterwards, independent of the order on the command line, thus the value of -C wins
  CallParseCommandLine(A, { "-C", MString("GeometryFileName=") + ChangedGeometryFileName, "-c", Empty, "-t" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "-C <pattern>", "-C reports the configuration value it changes", Output.Contains("Command-line parser: Changing this configuration value")) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-C <pattern> before -c <file>", "-c is applied before -C", S->GetGeometryFileName(), ChangedGeometryFileName) && Passed;

  // -C can be given multiple times, the last one wins
  CallParseCommandLine(A, { "-c", Empty, "-C", MString("GeometryFileName=") + ConfiguredGeometryFileName, "-C", MString("GeometryFileName=") + ChangedGeometryFileName, "-t" });
  Passed = Evaluate("ParseCommandLine()", "two -C patterns", "The last -C wins", S->GetGeometryFileName(), ChangedGeometryFileName) && Passed;

  // -g is applied after -C
  CallParseCommandLine(A, { "-c", Empty, "-g", GeometryFileName, "-C", MString("GeometryFileName=") + ChangedGeometryFileName, "-t" });
  Passed = Evaluate("ParseCommandLine()", "-C <pattern> and -g <file>", "-g overrides a geometry file name set with -C", S->GetGeometryFileName(), GeometryFileName) && Passed;

  CallParseCommandLine(A, { "-c", Empty, "-m", "1", "-t" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "-m 1", "-m 1 enables multithreading", Output.Contains("Command-line parser: Using multithreading: yes")) && Passed;
  CallParseCommandLine(A, { "-c", Empty, "-m", "2", "-t" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "-m 2", "Any non-zero value enables multithreading", Output.Contains("Command-line parser: Using multithreading: yes")) && Passed;
  CallParseCommandLine(A, { "-c", Empty, "--multithreading", "0", "-t" }, Output);
  Passed = EvaluateTrue("ParseCommandLine()", "--multithreading 0", "--multithreading 0 disables multithreading", Output.Contains("Command-line parser: Using multithreading: no")) && Passed;

  CallParseCommandLine(A, { "-c", Empty, "-v", "0", "-t" });
  int Verbosity = g_Verbosity;
  g_Verbosity = OldVerbosity;
  Passed = Evaluate("ParseCommandLine()", "-v 0", "-v 0 sets the verbosity to c_Quiet", Verbosity, (int) c_Quiet) && Passed;
  CallParseCommandLine(A, { "-c", Empty, "--verbosity", "3", "-t" });
  Verbosity = g_Verbosity;
  g_Verbosity = OldVerbosity;
  Passed = Evaluate("ParseCommandLine()", "--verbosity 3", "--verbosity 3 sets the verbosity to c_Info", Verbosity, (int) c_Info) && Passed;

  // An unknown option gives a warning, but the options after it are still applied
  CallParseCommandLine(A, { "-c", Empty, "--unknown-option", "-v", "3", "-t" }, Output);
  Verbosity = g_Verbosity;
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("ParseCommandLine()", "--unknown-option", "An unknown option gives a warning", Output.Contains("WARNING: Command-line parser: Unknown option: --unknown-option")) && Passed;
  Passed = Evaluate("ParseCommandLine()", "--unknown-option -v 3", "Parsing continues after the warning and applies -v", Verbosity, (int) c_Info) && Passed;

  // An argument which is not an option is ignored
  CallParseCommandLine(A, { "-c", Empty, "SomeFile.roa", "-t" }, Output);
  Passed = EvaluateFalse("ParseCommandLine()", "SomeFile.roa", "An argument which is not an option gives no unknown-option warning", Output.Contains("Unknown option")) && Passed;

  g_Verbosity = OldVerbosity;
  S->SetGeometryFileName(OldGeometryFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineConfigurationErrors(MAssembly& A)
{
  bool Passed = true;

  const MString Missing = GetTemporaryFileName("does-not-exist.cfg");

  // A configuration file which cannot be loaded stops the parser, i.e. no analysis with another configuration
  MString Output;
  Passed = Evaluate("ParseCommandLine()", "-c <missing file>", "ParseCommandLine returns 1: a configuration file which cannot be loaded is an error", CallParseCommandLine(A, { "-c", Missing }, Output), 1) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-c <missing file>", "The error is reported", Output.Contains("ERROR: Command-line parser: Unable to load configuration file")) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-c <missing file>", "The file is not reported as used", Output.Contains("Command-line parser: Use configuration file")) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-c <missing file> -a", "ParseCommandLine returns 1 without starting the analysis", CallParseCommandLineInChild(A, { "-c", Missing, "-a" }), 1) && Passed;
  CallParseCommandLine(A, { "-c", Missing, "-a" }, Output);
  Passed = EvaluateFalse("ParseCommandLine()", "-c <missing file> -a", "The analysis is not attempted", Output.Contains("The analysis failed")) && Passed;

  // A field which cannot be changed stops the parser too
  Passed = Evaluate("ParseCommandLine()", "-C <unknown field>", "ParseCommandLine returns 1: a configuration value which cannot be changed is an error", CallParseCommandLine(A, { "-C", "DoesNotExist=1" }, Output), 1) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-C <unknown field>", "The error is reported", Output.Contains("Unable to change this configuration value: DoesNotExist=1")) && Passed;
  CallParseCommandLine(A, { "-C", "DoesNotExist=1", "-a" }, Output);
  Passed = EvaluateFalse("ParseCommandLine()", "-C <unknown field> -a", "The analysis is not attempted", Output.Contains("The analysis failed")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineAnalysisErrors(MAssembly& A)
{
  bool Passed = true;

  // A successful -a or -t ends the program in MSupervisor::Exit(), thus only failures can be tested.
  // Without the error handling Exit() would be called, which crashes without a TApplication: run in a child process
  const MString Empty = WriteConfiguration("analysis.empty.cfg", { }, "");
  Passed = EvaluateFalse("WriteConfiguration()", "analysis.empty.cfg", "The fixture file is written", Empty.IsEmpty()) && Passed;
  const MString NoGeometry = WriteConfiguration("analysis.nogeometry.cfg", { "XmlTagLoaderSimulations" }, GetTemporaryFileName("does-not-exist.geo.setup"));
  Passed = EvaluateFalse("WriteConfiguration()", "analysis.nogeometry.cfg", "The fixture file is written", NoGeometry.IsEmpty()) && Passed;
  if (Empty.IsEmpty() == true || NoGeometry.IsEmpty() == true) return false;

  Passed = Evaluate("ParseCommandLine()", "-a, no modules", "ParseCommandLine returns 1: a failed analysis is an error", CallParseCommandLineInChild(A, { "-c", Empty, "-a" }), 1) && Passed;
  Passed = Evaluate("ParseCommandLine()", "--auto, missing geometry", "ParseCommandLine returns 1: a failed analysis is an error", CallParseCommandLineInChild(A, { "-c", NoGeometry, "--auto" }), 1) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-t, no modules", "ParseCommandLine returns 1: a failed test run is an error", CallParseCommandLineInChild(A, { "-c", Empty, "-t" }), 1) && Passed;
  Passed = Evaluate("ParseCommandLine()", "--test, missing geometry", "ParseCommandLine returns 1: a failed test run is an error", CallParseCommandLineInChild(A, { "-c", NoGeometry, "--test" }), 1) && Passed;

  MString Output;
  Passed = Evaluate("ParseCommandLine()", "-a, no modules, in process", "ParseCommandLine returns 1 without calling Exit()", CallParseCommandLine(A, { "-c", Empty, "-a" }, Output), 1) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-a, no modules", "The failed analysis is reported", Output.Contains("ERROR: Command-line parser: The analysis failed")) && Passed;
  Passed = Evaluate("ParseCommandLine()", "-t, no modules, in process", "ParseCommandLine returns 1 without calling Exit()", CallParseCommandLine(A, { "-c", Empty, "-t" }, Output), 1) && Passed;
  Passed = EvaluateTrue("ParseCommandLine()", "-t, no modules", "The failed test run is reported", Output.Contains("ERROR: Command-line parser: The test run failed")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNAssembly Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
