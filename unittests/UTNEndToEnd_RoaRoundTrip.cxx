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
  //! Split a roa file into one file per detector and side
  bool TestSplitByDetectorSide();
  //! Split a roa file by detector and side without nearest-neighbor hits
  bool TestSplitByDetectorSideWithoutNearestNeighbors();
  //! Split a roa file by detector and side with TACs only (no read-out data a crystal hit could have) into gzip'ed files
  bool TestSplitByDetectorSideTACOnlyGzip();
  //! Return the event saver's roa switches as XML
  MString CreateRoaOptions(bool WithADCs, bool WithEnergiesTimingsOrigins, bool WithFlags, bool WithNearestNeighbors, bool SplitByDetectorSide) const;
  //! Return a nuclearizer config reading RoaFileName and writing it to OutputFileName with the given roa switches (see CreateRoaOptions)
  MString CreateRoaToRoaConfig(const MString& RoaFileName, const MString& OutputFileName, const MString& RoaOptions) const;
  //! Return "true" or "false"
  MString XmlBoolean(bool Value) const { return (Value == true) ? "true" : "false"; }
  //! Decompress a gzip'ed text file into a plain one
  bool UnzipTextFile(const MString& ZippedFileName, const MString& FileName) const;
  //! Run nuclearizer with arguments and capture stdout/stderr in a log file
  int RunNuclearizer(const MString& Arguments, const MString& LogFile);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::Run()
{
  bool Passed = true;

  Passed = TestStripAndCrystalHits() && Passed;
  Passed = TestSplitByDetectorSide() && Passed;
  Passed = TestSplitByDetectorSideWithoutNearestNeighbors() && Passed;
  Passed = TestSplitByDetectorSideTACOnlyGzip() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNEndToEnd_RoaRoundTrip::CreateRoaOptions(bool WithADCs, bool WithEnergiesTimingsOrigins, bool WithFlags, bool WithNearestNeighbors, bool SplitByDetectorSide) const
{
  ostringstream Out;
  Out<<"      <RoaWithADCs>"<<XmlBoolean(WithADCs)<<"</RoaWithADCs>"<<endl
     <<"      <RoaWithTACs>true</RoaWithTACs>"<<endl
     <<"      <RoaWithEnergies>"<<XmlBoolean(WithEnergiesTimingsOrigins)<<"</RoaWithEnergies>"<<endl
     <<"      <RoaWithTimings>"<<XmlBoolean(WithEnergiesTimingsOrigins)<<"</RoaWithTimings>"<<endl
     <<"      <RoaWithFlags>"<<XmlBoolean(WithFlags)<<"</RoaWithFlags>"<<endl
     <<"      <RoaWithOrigins>"<<XmlBoolean(WithEnergiesTimingsOrigins)<<"</RoaWithOrigins>"<<endl
     <<"      <RoaWithNearestNeighbors>"<<XmlBoolean(WithNearestNeighbors)<<"</RoaWithNearestNeighbors>"<<endl
     <<"      <SplitByDetectorSide>"<<XmlBoolean(SplitByDetectorSide)<<"</SplitByDetectorSide>"<<endl;

  return Out.str();
}


////////////////////////////////////////////////////////////////////////////////


MString UTNEndToEnd_RoaRoundTrip::CreateRoaToRoaConfig(const MString& RoaFileName, const MString& OutputFileName, const MString& RoaOptions) const
{
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
     <<RoaOptions
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
  // Strip hits with and without origins, a crystal hit in the same event, an event with only a crystal hit, and an event without hits
  const MString Input = Header +
    "SE\nID 1\nTI 1.500000000\n"
    "UH 0 41 l 4053 10452 59.5 12.25 4 1;2\n"
    "UH 0 49 h 1780 10251 33.25 11.5 0 -\n"
    "UC BGO1 2 0 1 3 812 511.5 0 3\n"
    "SE\nID 2\nTI 2.250000000\n"
    "UC BGO2 0 1 1 1 900 600.25 0 -\n"
    "SE\nID 3\nTI 4.000000000\n"
    "BD No hits\n"
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
    "SE\nID 3\nTI 4.000000000\n"
    "BD No hits\n"
    "PQ\n"
    "EN\n\n";

  const MString InputRoa = GetTemporaryFileName("strips-and-crystals.roa");
  const MString OutputRoa = GetTemporaryFileName("strips-and-crystals.out.roa");
  const MString ExpectedRoa = GetTemporaryFileName("strips-and-crystals.expected.roa");
  const MString Config = GetTemporaryFileName("strips-and-crystals.cfg");
  const MString Log = GetTemporaryFileName("strips-and-crystals.log");
  WriteTextFile(InputRoa, Input);
  WriteTextFile(ExpectedRoa, Expected);
  WriteTextFile(Config, CreateRoaToRoaConfig(InputRoa, OutputRoa, CreateRoaOptions(true, true, true, true, false)));

  int Status = RunNuclearizer(MString("-c ") + Config + " -a -n", Log);
  Passed = EvaluateTrue("End-to-end roa round trip", "strip and crystal hits", "nuclearizer reads and writes the roa file (log: " + Log + ")", Status == 0 && MFile::Exists(OutputRoa)) && Passed;
  if (Passed == false) return Passed;

  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa round trip", "strip and crystal hits",
                                              "Strip and crystal hits with all read-out data come back unchanged",
                                              OutputRoa, ExpectedRoa) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::TestSplitByDetectorSide()
{
  bool Passed = true;

  const MString InputHeader =
    "TYPE ROA\n"
    "UF UH doublesidedstrip adc-tac-flags\n"
    "UF UC voxel3d adc-flags\n"
    "\n";
  // Detector 0 has hits on both sides, detector 1 only on the low-voltage side, plus a crystal-only and an empty event
  const MString Input = InputHeader +
    "SE\nID 1\nTI 1.5\n"
    "UH 0 41 l 4053 10452 0\n"
    "UH 0 49 h 1780 10251 0\n"
    "UH 1 12 l 2000 9000 0\n"
    "SE\nID 2\nTI 2.25\n"
    "UH 0 42 l 3000 10000 0\n"
    "UC BGO2 0 1 1 900 0\n"
    "SE\nID 3\nTI 3\n"
    "UC BGO1 2 0 1 812 0\n"
    "SE\nID 4\nTI 4\n"
    "BD No hits\n"
    "EN\n";

  // The detector side files only contain strip hits, thus only the strip read-out unit is declared
  const MString OutputHeader = "\nTYPE ROA\n"
    "UF UH doublesidedstrip adc-tac-flags\n"
    "\n";
  const MString ExpectedDet0LV = OutputHeader +
    "SE\nID 1\nTI 1.500000000\nUH 0 41 l 4053 10452 0\nPQ\n"
    "SE\nID 2\nTI 2.250000000\nUH 0 42 l 3000 10000 0\nPQ\n"
    "EN\n\n";
  const MString ExpectedDet0HV = OutputHeader +
    "SE\nID 1\nTI 1.500000000\nUH 0 49 h 1780 10251 0\nPQ\n"
    "EN\n\n";
  const MString ExpectedDet1LV = OutputHeader +
    "SE\nID 1\nTI 1.500000000\nUH 1 12 l 2000 9000 0\nPQ\n"
    "EN\n\n";

  const MString InputRoa = GetTemporaryFileName("split.roa");
  const MString OutputRoa = GetTemporaryFileName("split.out.roa");
  const MString OutputBase = GetTemporaryFileName("split.out");
  const MString Config = GetTemporaryFileName("split.cfg");
  const MString Log = GetTemporaryFileName("split.log");
  const MString ExpectedDet0LVFile = GetTemporaryFileName("split.expected.det0.lv.roa");
  const MString ExpectedDet0HVFile = GetTemporaryFileName("split.expected.det0.hv.roa");
  const MString ExpectedDet1LVFile = GetTemporaryFileName("split.expected.det1.lv.roa");
  WriteTextFile(InputRoa, Input);
  WriteTextFile(ExpectedDet0LVFile, ExpectedDet0LV);
  WriteTextFile(ExpectedDet0HVFile, ExpectedDet0HV);
  WriteTextFile(ExpectedDet1LVFile, ExpectedDet1LV);
  WriteTextFile(Config, CreateRoaToRoaConfig(InputRoa, OutputRoa, CreateRoaOptions(true, false, true, true, true)));

  int Status = RunNuclearizer(MString("-c ") + Config + " -a -n", Log);
  Passed = EvaluateTrue("End-to-end roa split by detector side", "3 detector sides with hits", "nuclearizer runs successfully (log: " + Log + ")", Status == 0) && Passed;
  if (Passed == false) return Passed;

  Passed = EvaluateFalse("End-to-end roa split by detector side", "3 detector sides with hits", "No main file is written", MFile::Exists(OutputRoa)) && Passed;
  Passed = EvaluateFalse("End-to-end roa split by detector side", "3 detector sides with hits", "No file for the detector side without hits", MFile::Exists(OutputBase + ".det1.hv.roa")) && Passed;

  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa split by detector side", "detector 0, low-voltage side",
                                              "Contains only the low-voltage strip hits of detector 0 and no crystal hits",
                                              OutputBase + ".det0.lv.roa", ExpectedDet0LVFile) && Passed;
  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa split by detector side", "detector 0, high-voltage side",
                                              "Contains only the high-voltage strip hits of detector 0",
                                              OutputBase + ".det0.hv.roa", ExpectedDet0HVFile) && Passed;
  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa split by detector side", "detector 1, low-voltage side",
                                              "Contains only the low-voltage strip hits of detector 1",
                                              OutputBase + ".det1.lv.roa", ExpectedDet1LVFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::TestSplitByDetectorSideWithoutNearestNeighbors()
{
  bool Passed = true;

  // Flag 2 marks a nearest-neighbor strip hit: detector 0 has a regular low-voltage hit and a high-voltage nearest neighbor
  const MString Input =
    "TYPE ROA\n"
    "UF UH doublesidedstrip adc-tac-flags\n"
    "\n"
    "SE\nID 1\nTI 1.5\n"
    "UH 0 41 l 4053 10452 0\n"
    "UH 0 42 l 150 10400 2\n"
    "UH 0 49 h 120 10251 2\n"
    "EN\n";

  const MString ExpectedDet0LV =
    "\nTYPE ROA\n"
    "UF UH doublesidedstrip adc-tac-flags\n"
    "\n"
    "SE\nID 1\nTI 1.500000000\nUH 0 41 l 4053 10452 0\nPQ\n"
    "EN\n\n";

  const MString InputRoa = GetTemporaryFileName("split-nn.roa");
  const MString OutputBase = GetTemporaryFileName("split-nn.out");
  const MString Config = GetTemporaryFileName("split-nn.cfg");
  const MString Log = GetTemporaryFileName("split-nn.log");
  const MString ExpectedDet0LVFile = GetTemporaryFileName("split-nn.expected.det0.lv.roa");
  WriteTextFile(InputRoa, Input);
  WriteTextFile(ExpectedDet0LVFile, ExpectedDet0LV);
  WriteTextFile(Config, CreateRoaToRoaConfig(InputRoa, OutputBase + ".roa", CreateRoaOptions(true, false, true, false, true)));

  int Status = RunNuclearizer(MString("-c ") + Config + " -a -n", Log);
  Passed = EvaluateTrue("End-to-end roa split by detector side", "nearest neighbors excluded", "nuclearizer runs successfully (log: " + Log + ")", Status == 0) && Passed;
  if (Passed == false) return Passed;

  Passed = EvaluateFalse("End-to-end roa split by detector side", "nearest neighbors excluded",
                         "No file for a detector side with only nearest-neighbor hits", MFile::Exists(OutputBase + ".det0.hv.roa")) && Passed;
  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa split by detector side", "nearest neighbors excluded",
                                              "The low-voltage file contains only the regular strip hit",
                                              OutputBase + ".det0.lv.roa", ExpectedDet0LVFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::TestSplitByDetectorSideTACOnlyGzip()
{
  bool Passed = true;

  // Includes a crystal hit, which a non-split TAC-only output would reject
  const MString Input =
    "TYPE ROA\n"
    "UF UH doublesidedstrip adc-tac\n"
    "UF UC voxel3d adc\n"
    "\n"
    "SE\nID 1\nTI 1.5\n"
    "UH 0 41 l 4053 10452\n"
    "UH 0 49 h 1780 10251\n"
    "UC BGO1 2 0 1 812\n"
    "EN\n";

  const MString OutputHeader =
    "\nTYPE ROA\n"
    "UF UH doublesidedstrip tac\n"
    "\n";
  const MString ExpectedDet0LV = OutputHeader +
    "SE\nID 1\nTI 1.500000000\nUH 0 41 l 10452\nPQ\n"
    "EN\n\n";
  const MString ExpectedDet0HV = OutputHeader +
    "SE\nID 1\nTI 1.500000000\nUH 0 49 h 10251\nPQ\n"
    "EN\n\n";

  const MString InputRoa = GetTemporaryFileName("split-tac.roa");
  const MString OutputBase = GetTemporaryFileName("split-tac.out");
  const MString Config = GetTemporaryFileName("split-tac.cfg");
  const MString Log = GetTemporaryFileName("split-tac.log");
  const MString UnzippedDet0LVFile = GetTemporaryFileName("split-tac.out.det0.lv.unzipped.roa");
  const MString UnzippedDet0HVFile = GetTemporaryFileName("split-tac.out.det0.hv.unzipped.roa");
  const MString ExpectedDet0LVFile = GetTemporaryFileName("split-tac.expected.det0.lv.roa");
  const MString ExpectedDet0HVFile = GetTemporaryFileName("split-tac.expected.det0.hv.roa");
  WriteTextFile(InputRoa, Input);
  WriteTextFile(ExpectedDet0LVFile, ExpectedDet0LV);
  WriteTextFile(ExpectedDet0HVFile, ExpectedDet0HV);
  WriteTextFile(Config, CreateRoaToRoaConfig(InputRoa, OutputBase + ".roa.gz", CreateRoaOptions(false, false, false, true, true)));

  int Status = RunNuclearizer(MString("-c ") + Config + " -a -n", Log);
  Passed = EvaluateTrue("End-to-end roa split by detector side", "TAC only, gzip", "nuclearizer accepts the strip-only configuration and runs successfully (log: " + Log + ")", Status == 0) && Passed;
  if (Passed == false) return Passed;

  Passed = EvaluateTrue("End-to-end roa split by detector side", "TAC only, gzip", "The low-voltage file is gzip'ed and can be decompressed",
                        UnzipTextFile(OutputBase + ".det0.lv.roa.gz", UnzippedDet0LVFile)) && Passed;
  Passed = EvaluateTrue("End-to-end roa split by detector side", "TAC only, gzip", "The high-voltage file is gzip'ed and can be decompressed",
                        UnzipTextFile(OutputBase + ".det0.hv.roa.gz", UnzippedDet0HVFile)) && Passed;
  if (Passed == false) return Passed;

  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa split by detector side", "TAC only, gzip, low-voltage side",
                                              "Contains only the TAC of the low-voltage strip hit",
                                              UnzippedDet0LVFile, ExpectedDet0LVFile) && Passed;
  Passed = EvaluateFilesNumericallyEquivalent("End-to-end roa split by detector side", "TAC only, gzip, high-voltage side",
                                              "Contains only the TAC of the high-voltage strip hit",
                                              UnzippedDet0HVFile, ExpectedDet0HVFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_RoaRoundTrip::UnzipTextFile(const MString& ZippedFileName, const MString& FileName) const
{
  if (ZippedFileName.EndsWith(".gz") == false || MFile::Exists(ZippedFileName) == false) return false;

  MFile In;
  if (In.Open(ZippedFileName, MFile::c_Read) == false) return false;

  MString Content;
  MString Line;
  while (In.ReadLine(Line) == true) {
    Content += Line;
    Content += "\n";
  }
  In.Close();

  return WriteTextFile(FileName, Content);
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
