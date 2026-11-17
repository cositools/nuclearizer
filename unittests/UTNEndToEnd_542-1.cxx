/*
 * UTNEndToEnd_542-1.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFile.h"
#include "MString.h"
#include "MUnitTest.h"


//! End-to-end test: run nuclearizer on 542-1 HDF5 data and verify the .tra and .roa output, and read the .roa output back
class UTNEndToEnd_542_1 : public MUnitTest
{
 public:
  //! Default constructor
  UTNEndToEnd_542_1() : MUnitTest("UTNEndToEnd_542-1") {}
  //! Default destructor
  virtual ~UTNEndToEnd_542_1() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Run nuclearizer with the hdf5-to-tra, hdf5-to-roa, and roa-to-roa configurations and compare the output to the reference files
  bool TestConfigurations();

  //! Copy ConfigFile to TestConfigFile with OriginalOutputFile replaced by OutputFile; returns false if a file cannot be opened or nothing was replaced
  bool WriteTestConfiguration(const MString& ConfigFile, const MString& OriginalOutputFile, const MString& OutputFile, const MString& TestConfigFile);
  //! Run nuclearizer with arguments and capture stdout/stderr in a log file; returns the wait status
  int RunNuclearizer(const MString& Arguments, const MString& LogFile);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_542_1::Run()
{
  bool Passed = true;

  Passed = TestConfigurations() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_542_1::TestConfigurations()
{
  bool Passed = true;

  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  bool HasNuclearizer = (NuclearizerEnv != nullptr && NuclearizerEnv[0] != '\0');
  Passed = EvaluateTrue("End-to-end test 542-1", "environment variable", "$NUCLEARIZER environment variable must be set", HasNuclearizer) && Passed;
  if (HasNuclearizer == false) return Passed;

  MString DataDir = MString(NuclearizerEnv) + "/resource/unittestdata/542-1";

  // Configuration name, output suffix, reference file; roa-to-roa reads hdf5-to-roa.reference.roa, thus writing it again has to reproduce it
  vector<vector<MString>> Configurations = {
    { "hdf5-to-tra", "tra", "hdf5-to-tra.reference.tra" },
    { "hdf5-to-roa", "roa", "hdf5-to-roa.reference.roa" },
    { "roa-to-roa", "roa", "hdf5-to-roa.reference.roa" }
  };

  for (const vector<MString>& C : Configurations) {
    MString Name = C[0];
    MString ConfigFile = DataDir + "/" + Name + ".nuclearizer.cfg";
    MString ReferenceFile = DataDir + "/" + C[2];
    MString OriginalOutputFile = MString("$(NUCLEARIZER)/resource/unittestdata/542-1/") + Name + "." + C[1];
    MString TestConfigFile = GetTemporaryFileName(Name + ".nuclearizer.cfg");
    MString OutputFile = GetTemporaryFileName(Name + "." + C[1]);
    MString LogFile = GetTemporaryFileName(Name + ".log");

    bool ConfigExists = MFile::Exists(ConfigFile);
    bool ReferenceExists = MFile::Exists(ReferenceFile);
    Passed = EvaluateTrue("End-to-end test 542-1", Name + " config file", "The nuclearizer config file exists", ConfigExists) && Passed;
    Passed = EvaluateTrue("End-to-end test 542-1", Name + " reference file", "The reference file exists", ReferenceExists) && Passed;
    if (ConfigExists == false || ReferenceExists == false) continue;

    bool ConfigWritten = WriteTestConfiguration(ConfigFile, OriginalOutputFile, OutputFile, TestConfigFile);
    Passed = EvaluateTrue("End-to-end test 542-1", Name + " temporary config file", "The config file is copied into the temporary directory with the output file redirected", ConfigWritten) && Passed;
    if (ConfigWritten == false) continue;

    int Status = RunNuclearizer(MString("-c ") + TestConfigFile + " -a -n", LogFile);
    Passed = Evaluate("End-to-end test 542-1", Name + " exit status", "nuclearizer exits with status 0 (log: " + LogFile + ")", Status, 0) && Passed;

    bool OutputExists = MFile::Exists(OutputFile);
    Passed = EvaluateTrue("End-to-end test 542-1", Name + " output file", "The output file was created", OutputExists) && Passed;
    if (OutputExists == false) continue;

    Passed = EvaluateFilesNumericallyEquivalent("End-to-end test 542-1", Name + " output file", "The generated output is numerically equivalent to the reference", OutputFile, ReferenceFile) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_542_1::WriteTestConfiguration(const MString& ConfigFile, const MString& OriginalOutputFile, const MString& OutputFile, const MString& TestConfigFile)
{
  ifstream In(ConfigFile.Data());
  if (In.is_open() == false) return false;

  MString Content;
  bool Replaced = false;
  string Line;
  while (getline(In, Line).fail() == false) {
    MString ConfigLine(Line.c_str());
    if (ConfigLine.Contains(OriginalOutputFile) == true) {
      ConfigLine.ReplaceAllInPlace(OriginalOutputFile, OutputFile);
      Replaced = true;
    }
    Content += ConfigLine + "\n";
  }

  if (Replaced == false) return false;

  return WriteTextFile(TestConfigFile, Content);
}


////////////////////////////////////////////////////////////////////////////////


int UTNEndToEnd_542_1::RunNuclearizer(const MString& Arguments, const MString& LogFile)
{
  // Use the nuclearizer of the MEGAlib installation under test
  const char* MEGAlibEnv = getenv("MEGALIB");
  MString Program = (MEGAlibEnv != nullptr && MEGAlibEnv[0] != '\0') ? MString(MEGAlibEnv) + "/bin/nuclearizer" : MString("nuclearizer");
  MString Command = Program + " " + Arguments;

  // Access mode of the log file: read and write for everyone, reduced by the umask
  const mode_t LogFileMode = 0666;
  // Exit code if execl() fails, as the shell uses for a command not found
  const int ExecFailed = 127;

  pid_t Child = fork();
  if (Child == 0) {
    int Log = open(LogFile.Data(), O_WRONLY | O_CREAT | O_TRUNC, LogFileMode);
    if (Log >= 0) {
      dup2(Log, STDOUT_FILENO);
      dup2(Log, STDERR_FILENO);
      close(Log);
    }

    execl("/bin/sh", "sh", "-c", Command.Data(), static_cast<char*>(nullptr));
    _exit(ExecFailed);
  }

  if (Child < 0) return -1;

  int Status = 0;
  if (waitpid(Child, &Status, 0) < 0) return -1;

  return Status;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNEndToEnd_542_1 Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNEndToEnd_542-1.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
