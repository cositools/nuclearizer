/*
 * UTNReadOutDataTAC.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <limits>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MUnitTest.h"
#include "MTokenizer.h"

// Nuclearizer:
#include "MReadOutDataTAC.h"
#include "MReadOutDataEnergy.h"
#include "MReadOutDataFlags.h"


//! Unit test class for MReadOutDataTAC
class UTNReadOutDataTAC : public MUnitTest
{
 public:
  //! Default constructor
  UTNReadOutDataTAC() : MUnitTest("UTNReadOutDataTAC") {}
  //! Default destructor
  virtual ~UTNReadOutDataTAC() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the default values and Clear()
  bool TestDefaultConstruction();
  //! Test SetTAC/GetTAC and the type identification
  bool TestGettersSetters();
  //! Test the decoration with wrapped read-out data
  bool TestWrapping();
  //! Test Clone() for deep, independent copies
  bool TestClone();
  //! Test ToString(), ToParsableString() and operator<<
  bool TestStream();
  //! Test Parse() including round trips and error paths
  bool TestParse();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestWrapping() && Passed;
  Passed = TestClone() && Passed;
  Passed = TestStream() && Passed;
  Passed = TestParse() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::TestDefaultConstruction()
{
  bool Passed = true;

  MReadOutDataTAC T;
  Passed = Evaluate("MReadOutDataTAC()", "default", "The default TAC is 0", T.GetTAC(), 0u) && Passed;
  Passed = Evaluate("MReadOutDataTAC()", "default", "The default number of parsable elements is 1", T.GetNumberOfParsableElements(), 1u) && Passed;
  Passed = Evaluate("MReadOutDataTAC()", "default", "The default combined type is 'tac'", T.GetCombinedType(), MString("tac")) && Passed;

  MReadOutDataTAC W(nullptr);
  Passed = Evaluate("MReadOutDataTAC(nullptr)", "no wrapped data", "The TAC is 0 after construction with a null wrapped element", W.GetTAC(), 0u) && Passed;

  T.SetTAC(10452);
  T.Clear();
  Passed = Evaluate("Clear()", "after SetTAC(10452)", "Clear() resets the TAC to 0", T.GetTAC(), 0u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::TestGettersSetters()
{
  bool Passed = true;

  MReadOutDataTAC T;
  T.SetTAC(10452);
  Passed = Evaluate("SetTAC/GetTAC", "10452", "GetTAC returns the representative value 10452", T.GetTAC(), 10452u) && Passed;
  T.SetTAC(numeric_limits<unsigned int>::max());
  Passed = Evaluate("SetTAC/GetTAC", "maximum unsigned int", "GetTAC returns the maximum unsigned int unchanged", T.GetTAC(), numeric_limits<unsigned int>::max()) && Passed;

  Passed = Evaluate("GetType()", "tac", "GetType returns 'tac'", T.GetType(), MString("tac")) && Passed;
  Passed = Evaluate("GetTypeID()", "static type ID", "GetTypeID returns the hash of the type name", T.GetTypeID(), MString("tac").GetHash()) && Passed;
  Passed = EvaluateTrue("GetTypeID()", "uniqueness", "The TAC type ID differs from the flags type ID", MReadOutDataTAC::m_TypeID != MReadOutDataFlags::m_TypeID) && Passed;
  Passed = EvaluateTrue("IsOfType(MString)", "tac", "IsOfType('tac') is true", T.IsOfType(MString("tac"))) && Passed;
  Passed = EvaluateFalse("IsOfType(MString)", "energy", "IsOfType('energy') is false", T.IsOfType(MString("energy"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::TestWrapping()
{
  bool Passed = true;

  MReadOutDataEnergy* Energy = new MReadOutDataEnergy();
  Energy->SetEnergy(511.0);
  MReadOutDataTAC T(Energy); // T owns Energy
  T.SetTAC(77);

  Passed = Evaluate("GetCombinedType()", "tac wrapping energy", "The combined type is 'energy-tac'", T.GetCombinedType(), MString("energy-tac")) && Passed;
  Passed = Evaluate("GetNumberOfParsableElements()", "tac wrapping energy", "The number of parsable elements is 2", T.GetNumberOfParsableElements(), 2u) && Passed;
  Passed = EvaluateTrue("Get()", "energy type ID", "Get(energy) returns the wrapped energy object", T.Get(MReadOutDataEnergy::m_TypeID) == Energy) && Passed;
  Passed = EvaluateTrue("Get()", "tac type ID", "Get(tac) returns the object itself", T.Get(MReadOutDataTAC::m_TypeID) == &T) && Passed;

  T.Clear();
  Passed = Evaluate("Clear()", "tac wrapping energy", "Clear() resets the TAC", T.GetTAC(), 0u) && Passed;
  Passed = EvaluateNear("Clear()", "tac wrapping energy", "Clear() also resets the wrapped energy", Energy->GetEnergy(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::TestClone()
{
  bool Passed = true;

  MReadOutDataFlags* Flags = new MReadOutDataFlags();
  Flags->SetFlags(5);
  MReadOutDataTAC T(Flags);
  T.SetTAC(9000);

  MReadOutDataTAC* C = T.Clone();
  Passed = EvaluateTrue("Clone()", "with wrapped flags", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "with wrapped flags", "The clone has the same TAC", C->GetTAC(), 9000u) && Passed;
    Passed = Evaluate("Clone()", "with wrapped flags", "The clone has the same combined type", C->GetCombinedType(), MString("flags-tac")) && Passed;
    MReadOutDataFlags* CF = dynamic_cast<MReadOutDataFlags*>(C->Get(MReadOutDataFlags::m_TypeID));
    Passed = EvaluateTrue("Clone()", "with wrapped flags", "The clone contains a flags element", CF != nullptr) && Passed;
    if (CF != nullptr) {
      Passed = EvaluateTrue("Clone()", "with wrapped flags", "The wrapped flags are deep copied", CF != Flags) && Passed;
      Passed = Evaluate("Clone()", "with wrapped flags", "The wrapped flags value is copied", CF->GetFlags(), 5u) && Passed;
    }
    C->SetTAC(1);
    Passed = Evaluate("Clone()", "independence", "Modifying the clone does not modify the original", T.GetTAC(), 9000u) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::TestStream()
{
  bool Passed = true;

  MReadOutDataTAC T;
  T.SetTAC(10452);
  Passed = Evaluate("ToString()", "10452", "ToString returns the TAC followed by a space", T.ToString(), MString("10452 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "without descriptor", "ToParsableString(false) equals ToString", T.ToParsableString(false), MString("10452 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "with descriptor", "ToParsableString(true) prefixes the type name", T.ToParsableString(true), MString("tac 10452 ")) && Passed;

  ostringstream Out;
  Out<<T;
  Passed = Evaluate("operator<<", "10452", "operator<< writes ToString", MString(Out.str()), MString("10452 ")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataTAC::TestParse()
{
  bool Passed = true;

  MTokenizer Tok;
  Tok.AnalyzeFast("10452");
  MReadOutDataTAC T;
  Passed = EvaluateTrue("Parse()", "single token", "Parse() succeeds for one TAC token", T.Parse(Tok, 0)) && Passed;
  Passed = Evaluate("Parse()", "single token", "Parse() reads the TAC 10452", T.GetTAC(), 10452u) && Passed;

  MTokenizer TokOffset;
  TokOffset.AnalyzeFast("a b 333");
  Passed = EvaluateTrue("Parse()", "start element 2", "Parse() succeeds with a start offset", T.Parse(TokOffset, 2)) && Passed;
  Passed = Evaluate("Parse()", "start element 2", "Parse() reads the token at the start offset", T.GetTAC(), 333u) && Passed;

  MReadOutDataTAC Writer(new MReadOutDataEnergy());
  Writer.SetTAC(8191);
  MReadOutDataEnergy* WriterEnergy = dynamic_cast<MReadOutDataEnergy*>(Writer.Get(MReadOutDataEnergy::m_TypeID));
  Passed = EvaluateTrue("Get()", "round trip writer", "The writer contains an energy element", WriterEnergy != nullptr) && Passed;
  if (WriterEnergy != nullptr) {
    WriterEnergy->SetEnergy(122.0);
  }
  MTokenizer TokRound;
  TokRound.AnalyzeFast(Writer.ToParsableString(false));
  MReadOutDataTAC Reader(new MReadOutDataEnergy());
  Passed = EvaluateTrue("Parse()", "round trip", "Parse() succeeds on the output of ToParsableString", Reader.Parse(TokRound, 0)) && Passed;
  Passed = Evaluate("Parse()", "round trip", "The TAC survives the round trip", Reader.GetTAC(), 8191u) && Passed;
  MReadOutDataEnergy* ReaderEnergy = dynamic_cast<MReadOutDataEnergy*>(Reader.Get(MReadOutDataEnergy::m_TypeID));
  Passed = EvaluateTrue("Parse()", "round trip", "The parsed object contains an energy element", ReaderEnergy != nullptr) && Passed;
  if (ReaderEnergy != nullptr) {
    Passed = EvaluateNear("Parse()", "round trip", "The wrapped energy survives the round trip", ReaderEnergy->GetEnergy(), 122.0, 1e-9) && Passed;
  }

  MTokenizer TokEmpty;
  TokEmpty.AnalyzeFast("");
  MReadOutDataTAC TEmpty;
  TEmpty.SetTAC(99);
  DisableDefaultStreams();
  bool EmptyResult = TEmpty.Parse(TokEmpty, 0);
  bool OffsetResult = T.Parse(TokOffset, 3);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Parse()", "no tokens", "Parse() returns false without tokens", EmptyResult) && Passed;
  Passed = Evaluate("Parse()", "no tokens", "A failed Parse() preserves the previous TAC", TEmpty.GetTAC(), 99u) && Passed;
  Passed = EvaluateFalse("Parse()", "start element past end", "Parse() returns false when the start element is past the last token", OffsetResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNReadOutDataTAC Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNReadOutDataTAC.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
