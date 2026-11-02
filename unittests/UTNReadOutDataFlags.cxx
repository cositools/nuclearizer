/*
 * UTNReadOutDataFlags.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
using namespace std;

// MEGAlib:
#include "MUnitTest.h"
#include "MTokenizer.h"

// Nuclearizer:
#include "MReadOutDataFlags.h"
#include "MReadOutDataEnergy.h"
#include "MReadOutDataTAC.h"


//! Unit test class for MReadOutDataFlags
class UTNReadOutDataFlags : public MUnitTest
{
 public:
  //! Default constructor
  UTNReadOutDataFlags() : MUnitTest("UTNReadOutDataFlags") {}
  //! Default destructor
  virtual ~UTNReadOutDataFlags() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the default values and Clear()
  bool TestDefaultConstruction();
  //! Test SetFlags/GetFlags and the type identification
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


bool UTNReadOutDataFlags::Run()
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


bool UTNReadOutDataFlags::TestDefaultConstruction()
{
  bool Passed = true;

  MReadOutDataFlags F;
  Passed = Evaluate("MReadOutDataFlags()", "default", "The default flags are 0", F.GetFlags(), 0u) && Passed;
  Passed = Evaluate("MReadOutDataFlags()", "default", "The default number of parsable elements is 1", F.GetNumberOfParsableElements(), 1u) && Passed;
  Passed = Evaluate("MReadOutDataFlags()", "default", "The default combined type is 'flags'", F.GetCombinedType(), MString("flags")) && Passed;

  MReadOutDataFlags W(nullptr);
  Passed = Evaluate("MReadOutDataFlags(nullptr)", "no wrapped data", "The flags are 0 after construction with a null wrapped element", W.GetFlags(), 0u) && Passed;

  F.SetFlags(0xFFu);
  F.Clear();
  Passed = Evaluate("Clear()", "after SetFlags(0xFF)", "Clear() resets the flags to 0", F.GetFlags(), 0u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataFlags::TestGettersSetters()
{
  bool Passed = true;

  MReadOutDataFlags F;
  F.SetFlags(0x15u);
  Passed = Evaluate("SetFlags/GetFlags", "bit mask 0x15", "GetFlags returns the bit mask 0x15", F.GetFlags(), 0x15u) && Passed;
  F.SetFlags(0x80000001u);
  Passed = Evaluate("SetFlags/GetFlags", "highest and lowest bit", "GetFlags keeps the highest and the lowest bit", F.GetFlags(), 0x80000001u) && Passed;

  Passed = Evaluate("GetType()", "flags", "GetType returns 'flags'", F.GetType(), MString("flags")) && Passed;
  Passed = Evaluate("GetTypeID()", "static type ID", "GetTypeID returns the hash of the type name", F.GetTypeID(), MString("flags").GetHash()) && Passed;
  Passed = EvaluateTrue("IsOfType(MString)", "flags", "IsOfType('flags') is true", F.IsOfType(MString("flags"))) && Passed;
  Passed = EvaluateFalse("IsOfType(MString)", "tac", "IsOfType('tac') is false", F.IsOfType(MString("tac"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataFlags::TestWrapping()
{
  bool Passed = true;

  MReadOutDataTAC* TAC = new MReadOutDataTAC(new MReadOutDataEnergy());
  MReadOutDataFlags F(TAC);
  F.SetFlags(3);

  Passed = Evaluate("GetCombinedType()", "flags wrapping tac and energy", "The combined type is 'energy-tac-flags'", F.GetCombinedType(), MString("energy-tac-flags")) && Passed;
  Passed = Evaluate("GetNumberOfParsableElements()", "flags wrapping tac and energy", "The number of parsable elements is 3", F.GetNumberOfParsableElements(), 3u) && Passed;
  Passed = EvaluateTrue("Get()", "tac type ID", "Get(tac) returns the wrapped TAC", F.Get(MReadOutDataTAC::m_TypeID) == TAC) && Passed;
  Passed = EvaluateTrue("Get()", "energy type ID", "Get(energy) finds the innermost energy", F.Get(MReadOutDataEnergy::m_TypeID) != nullptr) && Passed;
  Passed = EvaluateTrue("Get()", "unknown type ID", "Get(12345) returns nullptr", F.Get(12345) == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataFlags::TestClone()
{
  bool Passed = true;

  MReadOutDataFlags F(new MReadOutDataTAC());
  F.SetFlags(0xA5u);
  MReadOutDataFlags* C = F.Clone();
  Passed = EvaluateTrue("Clone()", "with wrapped tac", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "with wrapped tac", "The clone has the same flags", C->GetFlags(), 0xA5u) && Passed;
    Passed = Evaluate("Clone()", "with wrapped tac", "The clone has the same combined type", C->GetCombinedType(), MString("tac-flags")) && Passed;
    Passed = EvaluateTrue("Clone()", "with wrapped tac", "The clone contains a TAC element", C->Get(MReadOutDataTAC::m_TypeID) != nullptr) && Passed;
    Passed = EvaluateTrue("Clone()", "with wrapped tac", "The wrapped TAC is deep copied", C->Get(MReadOutDataTAC::m_TypeID) != F.Get(MReadOutDataTAC::m_TypeID)) && Passed;
    C->SetFlags(0);
    Passed = Evaluate("Clone()", "independence", "Modifying the clone does not modify the original", F.GetFlags(), 0xA5u) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataFlags::TestStream()
{
  bool Passed = true;

  MReadOutDataFlags F;
  F.SetFlags(21);
  Passed = Evaluate("ToString()", "21", "ToString returns the decimal flags followed by a space", F.ToString(), MString("21 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "with descriptor", "ToParsableString(true) prefixes the type name", F.ToParsableString(true), MString("flags 21 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "without descriptor", "ToParsableString(false) equals ToString", F.ToParsableString(false), MString("21 ")) && Passed;

  ostringstream Out;
  Out<<F;
  Passed = Evaluate("operator<<", "21", "operator<< writes ToString", MString(Out.str()), MString("21 ")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataFlags::TestParse()
{
  bool Passed = true;

  MReadOutDataFlags Writer(new MReadOutDataTAC());
  Writer.SetFlags(0x80000001u);
  MReadOutDataTAC* WriterTAC = dynamic_cast<MReadOutDataTAC*>(Writer.Get(MReadOutDataTAC::m_TypeID));
  Passed = EvaluateTrue("Get()", "writer tac type ID", "The writer contains a TAC element", WriterTAC != nullptr) && Passed;
  if (WriterTAC != nullptr) WriterTAC->SetTAC(12);
  MTokenizer Tok;
  Tok.AnalyzeFast(Writer.ToParsableString(false));
  MReadOutDataFlags Reader(new MReadOutDataTAC());
  Passed = EvaluateTrue("Parse()", "round trip", "Parse() succeeds on the output of ToParsableString", Reader.Parse(Tok, 0)) && Passed;
  Passed = Evaluate("Parse()", "round trip", "The flags including the highest bit survive the round trip", Reader.GetFlags(), 0x80000001u) && Passed;
  MReadOutDataTAC* ReaderTAC = dynamic_cast<MReadOutDataTAC*>(Reader.Get(MReadOutDataTAC::m_TypeID));
  Passed = EvaluateTrue("Parse()", "round trip", "The parsed object contains a TAC element", ReaderTAC != nullptr) && Passed;
  if (ReaderTAC != nullptr) {
    Passed = Evaluate("Parse()", "round trip", "The wrapped TAC survives the round trip", ReaderTAC->GetTAC(), 12u) && Passed;
  }

  MTokenizer TokOffset;
  TokOffset.AnalyzeFast("x 7");
  MReadOutDataFlags F;
  Passed = EvaluateTrue("Parse()", "start element 1", "Parse() succeeds with a start offset", F.Parse(TokOffset, 1)) && Passed;
  Passed = Evaluate("Parse()", "start element 1", "Parse() reads the token at the start offset", F.GetFlags(), 7u) && Passed;

  MTokenizer TokEmpty;
  TokEmpty.AnalyzeFast("");
  DisableDefaultStreams();
  bool EmptyResult = F.Parse(TokEmpty, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Parse()", "no tokens", "Parse() returns false without tokens", EmptyResult) && Passed;
  Passed = Evaluate("Parse()", "no tokens", "A failed Parse() preserves the previous flags", F.GetFlags(), 7u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNReadOutDataFlags Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNReadOutDataFlags.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
