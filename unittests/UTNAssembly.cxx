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
#include "TGClient.h"

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
  //! Test the ParseCommandLine() paths which return before any analysis or GUI is started
  bool TestParseCommandLineEarlyExits(MAssembly& A);
  //! Test ParseCommandLine() with options missing their argument in a child process
  bool TestParseCommandLineMissingArguments(MAssembly& A);

  //! Call ParseCommandLine() with the given arguments (without the program name), the usage output is discarded
  bool CallParseCommandLine(MAssembly& A, const vector<MString>& Arguments);
  //! Call ParseCommandLine() in a child process; returns true if the child exited normally and ParseCommandLine() returned false
  bool CallParseCommandLineInChild(MAssembly& A, const vector<MString>& Arguments);
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

  Passed = Evaluate("MAssembly()", "verbosity", "The constructor sets the verbosity to c_Error", AssemblyVerbosity, (int) c_Error) && Passed;
  Passed = TestConstruction(A) && Passed;
  Passed = TestSetInterrupt(A) && Passed;
  Passed = TestParseCommandLineEarlyExits(A) && Passed;
  Passed = TestParseCommandLineMissingArguments(A) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::CallParseCommandLine(MAssembly& A, const vector<MString>& Arguments)
{
  vector<MString> All;
  All.push_back("nuclearizer");
  All.insert(All.end(), Arguments.begin(), Arguments.end());

  vector<char*> Argv;
  for (MString& S : All) {
    Argv.push_back(const_cast<char*>(S.Data()));
  }
  Argv.push_back(nullptr);

  ostringstream Discard;
  streambuf* Old = cout.rdbuf(Discard.rdbuf());
  bool Result = A.ParseCommandLine((int) All.size(), Argv.data());
  cout.rdbuf(Old);

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::CallParseCommandLineInChild(MAssembly& A, const vector<MString>& Arguments)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Child: never return into the test
    // Time-out of the child process in seconds
    const unsigned int ChildTimeout = 30;
    // Exit code if stdout cannot be redirected
    const int RedirectFailed = 3;
    alarm(ChildTimeout);
    if (freopen("/dev/null", "w", stdout) == nullptr) _exit(RedirectFailed);
    bool Result = CallParseCommandLine(A, Arguments);
    _exit(Result == false ? 0 : 1);
  }

  int Status = 0;
  if (waitpid(Pid, &Status, 0) != Pid) return false;
  if (WIFEXITED(Status) == false) return false;
  return WEXITSTATUS(Status) == 0;
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

  MSupervisor* S = MSupervisor::GetSupervisor();
  Passed = Evaluate("MAssembly()", "available modules", "The constructor registers 24 available modules", S->GetNAvailableModules(), 24u) && Passed;

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

  vector<MString> Expected = { "XmlTagSaverMeasurementsFITSL1a", "XmlTagSaverMeasurementsFITSL1b", "XmlTagSaverMeasurementsFITSL2" };
  for (MString& Tag : Expected) {
    Passed = EvaluateTrue("MAssembly()", Tag, "The FITS saver for this level is available", S->GetAvailableModuleByXmlTag(Tag) != nullptr) && Passed;
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


bool UTNAssembly::TestParseCommandLineEarlyExits(MAssembly& A)
{
  bool Passed = true;

  Passed = EvaluateFalse("ParseCommandLine()", "-h", "ParseCommandLine returns false for -h", CallParseCommandLine(A, { "-h" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "--help", "ParseCommandLine returns false for --help", CallParseCommandLine(A, { "--help" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "?", "ParseCommandLine returns false for ?", CallParseCommandLine(A, { "?" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-?", "ParseCommandLine returns false for -?", CallParseCommandLine(A, { "-?" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-a -h", "Help takes precedence over -a", CallParseCommandLine(A, { "-a", "-h" })) && Passed;

  Passed = EvaluateFalse("ParseCommandLine()", "-c", "ParseCommandLine returns false if -c has no argument", CallParseCommandLine(A, { "-c" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "--configuration", "ParseCommandLine returns false if --configuration has no argument", CallParseCommandLine(A, { "--configuration" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-c -a", "ParseCommandLine returns false if the argument of -c is another option", CallParseCommandLine(A, { "-c", "-a" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-g", "ParseCommandLine returns false if -g has no argument", CallParseCommandLine(A, { "-g" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "--geometry", "ParseCommandLine returns false if --geometry has no argument", CallParseCommandLine(A, { "--geometry" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "-m", "ParseCommandLine returns false if -m has no argument", CallParseCommandLine(A, { "-m" })) && Passed;
  Passed = EvaluateFalse("ParseCommandLine()", "--multithreading", "ParseCommandLine returns false if --multithreading has no argument", CallParseCommandLine(A, { "--multithreading" })) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNAssembly::TestParseCommandLineMissingArguments(MAssembly& A)
{
  bool Passed = true;

  // These options take an argument too, but are not part of the argument check
  Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", "-v", "ParseCommandLine rejects -v without argument without crashing", CallParseCommandLineInChild(A, { "-v" }))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", "--verbosity", "ParseCommandLine rejects --verbosity without argument without crashing", CallParseCommandLineInChild(A, { "--verbosity" }))) && Passed;
#ifdef _LIBCPP_VERSION
  // The parser does not check for the missing argument and streams the null argv[argc] into cout, which crashes with libc++
  Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", "-C", "ParseCommandLine rejects -C without argument without crashing", CallParseCommandLineInChild(A, { "-C" }))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", "--change-configuration", "ParseCommandLine rejects --change-configuration without argument without crashing", CallParseCommandLineInChild(A, { "--change-configuration" }))) && Passed;
#else
  // Other standard libraries (e.g. libstdc++) ignore a null char pointer in a stream, which hides the missing argument check:
  // the parser continues and returns false only if LaunchUI() fails.
  // Without a display ROOT starts in batch mode with a dummy gClient, which lets LaunchUI() succeed
  if (gClient != nullptr) {
    Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", "-C", "ParseCommandLine rejects -C without argument without crashing", CallParseCommandLineInChild(A, { "-C" }))) && Passed;
    Passed = ExpectedToFail(EvaluateTrue("ParseCommandLine()", "--change-configuration", "ParseCommandLine rejects --change-configuration without argument without crashing", CallParseCommandLineInChild(A, { "--change-configuration" }))) && Passed;
  } else {
    Passed = EvaluateTrue("ParseCommandLine()", "-C", "ParseCommandLine rejects -C without argument without crashing", CallParseCommandLineInChild(A, { "-C" })) && Passed;
    Passed = EvaluateTrue("ParseCommandLine()", "--change-configuration", "ParseCommandLine rejects --change-configuration without argument without crashing", CallParseCommandLineInChild(A, { "--change-configuration" })) && Passed;
  }
#endif

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
