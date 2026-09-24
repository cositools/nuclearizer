/*
 * UTNEndToEnd_RoaRoundTrip.cxx
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
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFile.h"
#include "MString.h"
#include "MUnitTest.h"


//! End-to-end test: read a roa file with strip and crystal hits with nuclearizer and write it again
class UTNEndToEnd_RoaRoundTrip : public MUnitTest
{
public:
  UTNEndToEnd_RoaRoundTrip() : MUnitTest("UTNEndToEnd_RoaRoundTrip") {}
  virtual ~UTNEndToEnd_RoaRoundTrip() {}

  virtual bool Run();

private:
  //! Read and write a roa file with strip and crystal hits and all read-out data
  bool TestStripAndCrystalHits();
  //! Return a nuclearizer config reading RoaFileName and writing it to OutputFileName with the given roa switches
  MString CreateRoaToRoaConfig(const MString& RoaFileName, const MString& OutputFileName, bool WithEnergiesTimingsOrigins);
  //! Run nuclearizer with arguments and capture stdout/stderr in a log file
  int RunNuclearizer(const MString& Arguments, const MString& LogFile);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::Run()
{
  bool Passed = true;

  Passed = TestStripAndCrystalHits() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNEndToEnd_RoaRoundTrip::CreateRoaToRoaConfig(const MString& RoaFileName, const MString& OutputFileName, bool WithEnergiesTimingsOrigins)
{
  const MString Switch = (WithEnergiesTimingsOrigins == true) ? "true" : "false";

  ostringstream Out;
  Out<<"<NuclearizerData>"<<endl
     <<"  <Version>1</Version>"<<endl
     <<"  <ModuleSequence>"<<endl
     <<"    <ModuleSequenceItem>XmlTagMeasurementLoaderROA</ModuleSequenceItem>"<<endl
     <<"    <ModuleSequenceItem>XmlTagEventSaver</ModuleSequenceItem>"<<endl
     <<"  </ModuleSequence>"<<endl
     <<"  <GeometryFileName>$(NUCLEARIZER)/resource/unittestdata/542-1/hp52542-1.simplified.geo.setup</GeometryFileName>"<<endl
     <<"  <ModuleOptions>"<<endl
     <<"    <XmlTagMeasurementLoaderROA>"<<endl
     <<"      <FileName>"<<RoaFileName<<"</FileName>"<<endl
     <<"    </XmlTagMeasurementLoaderROA>"<<endl
     <<"    <XmlTagEventSaver>"<<endl
     <<"      <FileName>"<<OutputFileName<<"</FileName>"<<endl
     <<"      <Mode>0</Mode>"<<endl
     <<"      <SaveBadEvents>true</SaveBadEvents>"<<endl
     <<"      <SaveVetoEvents>true</SaveVetoEvents>"<<endl
     <<"      <AddTimeTag>false</AddTimeTag>"<<endl
     <<"      <SplitFile>false</SplitFile>"<<endl
     <<"      <SplitFileTime>600</SplitFileTime>"<<endl
     <<"      <RoaWithADCs>true</RoaWithADCs>"<<endl
     <<"      <RoaWithTACs>true</RoaWithTACs>"<<endl
     <<"      <RoaWithEnergies>"<<Switch<<"</RoaWithEnergies>"<<endl
     <<"      <RoaWithTimings>"<<Switch<<"</RoaWithTimings>"<<endl
     <<"      <RoaWithFlags>true</RoaWithFlags>"<<endl
     <<"      <RoaWithOrigins>"<<Switch<<"</RoaWithOrigins>"<<endl
     <<"      <RoaWithNearestNeighbors>true</RoaWithNearestNeighbors>"<<endl
     <<"    </XmlTagEventSaver>"<<endl
     <<"  </ModuleOptions>"<<endl
     <<"</NuclearizerData>"<<endl;

  return Out.str();
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::TestStripAndCrystalHits()
{
  bool Passed = true;

  const MString Header =
    "TYPE ROA\n"
    "UF UH doublesidedstrip adc-tac-energy-timing-flags-origins\n"
    "UF UC voxel3d adc-energy-flags-origins\n"
    "\n";
  // Strip hits with and without origins, a crystal hit in the same event, and an event with only a crystal hit
  const MString Input = Header +
    "SE\nID 1\nTI 1.500000000\n"
    "UH 0 41 l 4053 10452 59.5 12.25 4 1;2\n"
    "UH 0 49 h 1780 10251 33.25 11.5 0 -\n"
    "UC BGO1 2 0 1 3 812 511.5 0 3\n"
    "SE\nID 2\nTI 2.250000000\n"
    "UC BGO2 0 1 1 1 900 600.25 0 -\n"
    "EN\n";
  // The writer adds a blank line before the header, a PQ line to each event, and a blank line at the end
  const MString Expected = "\n" + Header +
    "SE\nID 1\nTI 1.500000000\n"
    "UH 0 41 l 4053 10452 59.5 12.25 4 1;2\n"
    "UH 0 49 h 1780 10251 33.25 11.5 0 -\n"
    "UC BGO1 2 0 1 3 812 511.5 0 3\n"
    "PQ\n"
    "SE\nID 2\nTI 2.250000000\n"
    "UC BGO2 0 1 1 1 900 600.25 0 -\n"
    "PQ\n"
    "EN\n\n";

  const MString InputRoa = GetTemporaryFileName("strips-and-crystals.roa");
  const MString OutputRoa = GetTemporaryFileName("strips-and-crystals.out.roa");
  const MString ExpectedRoa = GetTemporaryFileName("strips-and-crystals.expected.roa");
  const MString Config = GetTemporaryFileName("strips-and-crystals.cfg");
  const MString Log = GetTemporaryFileName("strips-and-crystals.log");
  WriteTextFile(InputRoa, Input);
  WriteTextFile(ExpectedRoa, Expected);
  WriteTextFile(Config, CreateRoaToRoaConfig(InputRoa, OutputRoa, true));

  int Status = RunNuclearizer(MString("-c ") + Config + " -a -n", Log);
  Passed = EvaluateTrue("End-to-end roa round trip", "strip and crystal hits", "nuclearizer reads and writes the roa file (log: " + Log + ")", Status == 0 && MFile::Exists(OutputRoa)) && Passed;
  if (Passed == false) return Passed;

  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa round trip", "strip and crystal hits",
                                              "Strip and crystal hits with all read-out data come back unchanged",
                                              OutputRoa, ExpectedRoa) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTNEndToEnd_RoaRoundTrip::RunNuclearizer(const MString& Arguments, const MString& LogFile)
{
  pid_t Child = fork();
  if (Child == 0) {
    int Log = open(LogFile.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (Log >= 0) {
      dup2(Log, STDOUT_FILENO);
      dup2(Log, STDERR_FILENO);
      close(Log);
    }

    MString Command = MString("nuclearizer ") + Arguments;
    execl("/bin/sh", "sh", "-c", Command.Data(), static_cast<char*>(0));
    _exit(127);
  }

  if (Child < 0) return -1;

  int Status = 0;
  if (waitpid(Child, &Status, 0) < 0) return -1;

  return Status;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNEndToEnd_RoaRoundTrip Test;
  return Test.Run() == true ? 0 : 1;
}
