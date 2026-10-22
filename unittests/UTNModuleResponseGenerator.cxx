/*
 * UTNModuleResponseGenerator.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <functional>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleResponseGenerator.h"


//! Unit test class for MModuleResponseGenerator
class UTNModuleResponseGenerator : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleResponseGenerator() : MUnitTest("UTNModuleResponseGenerator") {}
  //! Default destructor
  virtual ~UTNModuleResponseGenerator() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the defaults
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the lifecycle error paths
  bool TestLifecycleErrors();


  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  MString RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestLifecycleErrors() && Passed;
  // The response generation itself is not tested: the module is not complete

  // Not covered: the content of a generated response, which needs many simulated events.
  // ShowOptionsGUI() opens a GUI and is not tested.

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleResponseGenerator::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return "fork failed";

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Time-out of the child process in seconds
    const unsigned int ChildTimeout = 60;
    alarm(ChildTimeout);
    g_Verbosity = c_Quiet;
    bool Result = Function();
    _exit(Result == true ? 0 : 1);
  }

  // The end of the child as text, so that a failed check shows the cause, e.g. "signal 11" for a crash or "signal 14" for the time-out
  int Status = 0;
  if (waitpid(Pid, &Status, 0) != Pid) return "wait failed";
  if (WIFSIGNALED(Status) != 0) return MString("signal ") + WTERMSIG(Status);
  if (WIFEXITED(Status) == 0) return "abnormal end";
  if (WEXITSTATUS(Status) == 0) return "true";
  if (WEXITSTATUS(Status) == 1) return "false";
  return MString("exit code ") + WEXITSTATUS(Status);
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleResponseGenerator M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Response creator'", M.GetName(), MString("Response creator")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagResponseGenerator'", M.GetXmlTag(), MString("XmlTagResponseGenerator")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_ResponseGeneration", M.ProvidesModuleType(MAssembly::c_ResponseGeneration)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The position determination is required", M.IsHardPreceedingModule(MAssembly::c_PositionDetermiation)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = Evaluate("GetMode()", "default", "The default mode is c_Spectrum", M.GetMode(), MModuleResponseGenerator::c_Spectrum) && Passed;
  Passed = Evaluate("GetResponseName()", "default", "The default response name is 'Response'", M.GetResponseName(), MString("Response")) && Passed;
  Passed = Evaluate("GetRevanConfigurationFileName()", "default", "The default revan file name is empty", M.GetRevanConfigurationFileName(), MString("")) && Passed;
  Passed = Evaluate("GetMimrecConfigurationFileName()", "default", "The default mimrec file name is empty", M.GetMimrecConfigurationFileName(), MString("")) && Passed;

  Passed = Evaluate("Mode constants", "c_Spectrum", "c_Spectrum is 0", MModuleResponseGenerator::c_Spectrum, 0u) && Passed;
  Passed = Evaluate("Mode constants", "c_Clustering", "c_Clustering is 1", MModuleResponseGenerator::c_Clustering, 1u) && Passed;
  Passed = Evaluate("Mode constants", "c_BayesianER", "c_BayesianER is 2", MModuleResponseGenerator::c_BayesianER, 2u) && Passed;
  Passed = Evaluate("Mode constants", "c_Efficiency", "c_Efficiency is 3", MModuleResponseGenerator::c_Efficiency, 3u) && Passed;
  Passed = Evaluate("Mode constants", "c_Imaging", "c_Imaging is 4", MModuleResponseGenerator::c_Imaging, 4u) && Passed;

  MModuleResponseGenerator* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::TestConfiguration()
{
  bool Passed = true;

  MModuleResponseGenerator Writer;
  Writer.SetMode(MModuleResponseGenerator::c_Imaging);
  Writer.SetResponseName("MyResponse");
  Writer.SetRevanConfigurationFileName("/data/revan.cfg");
  Writer.SetMimrecConfigurationFileName("/data/mimrec.cfg");
  Passed = Evaluate("SetMode/GetMode", "c_Imaging", "Returns c_Imaging", Writer.GetMode(), MModuleResponseGenerator::c_Imaging) && Passed;
  Passed = Evaluate("SetResponseName/GetResponseName", "MyResponse", "Returns the set name", Writer.GetResponseName(), MString("MyResponse")) && Passed;
  Passed = Evaluate("SetRevanConfigurationFileName/GetRevanConfigurationFileName", "/data/revan.cfg", "Returns the set name", Writer.GetRevanConfigurationFileName(), MString("/data/revan.cfg")) && Passed;
  Passed = Evaluate("SetMimrecConfigurationFileName/GetMimrecConfigurationFileName", "/data/mimrec.cfg", "Returns the set name", Writer.GetMimrecConfigurationFileName(), MString("/data/mimrec.cfg")) && Passed;

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleResponseGenerator Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "all values", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "Mode", "The mode survives", Reader.GetMode(), MModuleResponseGenerator::c_Imaging) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "Name", "The response name survives", Reader.GetResponseName(), MString("MyResponse")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "RevanConfigurationFileName", "The revan file name survives", Reader.GetRevanConfigurationFileName(), MString("/data/revan.cfg")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "MimrecConfigurationFileName", "The mimrec file name survives", Reader.GetMimrecConfigurationFileName(), MString("/data/mimrec.cfg")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::TestLifecycleErrors()
{
  bool Passed = true;

  MString InvalidMode = RunInChild([]() -> bool {
    MModuleResponseGenerator M;
    M.SetMode(7);
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(Evaluate("Initialize()", "invalid mode 7", "Initialize returns false for an invalid mode without crashing", InvalidMode, MString("true"))) && Passed;

  MString NoGeometry = RunInChild([]() -> bool {
    MModuleResponseGenerator M;
    M.SetMode(MModuleResponseGenerator::c_Spectrum);
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(Evaluate("Initialize()", "no geometry", "Initialize returns false without a geometry and without crashing", NoGeometry, MString("true"))) && Passed;

  MString FinalizeOnly = RunInChild([]() -> bool {
    MModuleResponseGenerator M;
    M.Finalize();
    return true;
  });
  Passed = ExpectedToFail(Evaluate("Finalize()", "without Initialize", "Finalize without Initialize does not crash", FinalizeOnly, MString("true"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleResponseGenerator", "Unit tests for MModuleResponseGenerator") == false) return 1;

  UTNModuleResponseGenerator Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleResponseGenerator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
