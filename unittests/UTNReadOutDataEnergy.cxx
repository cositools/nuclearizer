/*
 * UTNReadOutDataEnergy.cxx
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
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MTokenizer.h"

// Nuclearizer:
#include "MReadOutDataEnergy.h"
#include "MReadOutDataTAC.h"
#include "MReadOutDataFlags.h"


//! Unit test class for MReadOutDataEnergy
class UTNReadOutDataEnergy : public MUnitTest
{
 public:
  //! Default constructor
  UTNReadOutDataEnergy() : MUnitTest("UTNReadOutDataEnergy") {}
  //! Default destructor
  virtual ~UTNReadOutDataEnergy() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the default values and Clear()
  bool TestDefaultConstruction();
  //! Test SetEnergy/GetEnergy and the type identification
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


bool UTNReadOutDataEnergy::Run()
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


bool UTNReadOutDataEnergy::TestDefaultConstruction()
{
  bool Passed = true;

  MReadOutDataEnergy E;
  Passed = EvaluateNear("MReadOutDataEnergy()", "default", "The default energy is 0", E.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MReadOutDataEnergy()", "default", "The default number of parsable elements is 1", E.GetNumberOfParsableElements(), 1u) && Passed;
  Passed = Evaluate("MReadOutDataEnergy()", "default", "The default combined type is 'energy'", E.GetCombinedType(), MString("energy")) && Passed;

  MReadOutDataEnergy W(nullptr);
  Passed = EvaluateNear("MReadOutDataEnergy(nullptr)", "no wrapped data", "The energy is 0 after construction with a null wrapped element", W.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MReadOutDataEnergy(nullptr)", "no wrapped data", "The number of parsable elements is 1 without wrapped data", W.GetNumberOfParsableElements(), 1u) && Passed;

  E.SetEnergy(662.0);
  E.Clear();
  Passed = EvaluateNear("Clear()", "after SetEnergy(662)", "Clear() resets the energy to 0", E.GetEnergy(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataEnergy::TestGettersSetters()
{
  bool Passed = true;

  MReadOutDataEnergy E;
  E.SetEnergy(511.0);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "511 keV", "GetEnergy returns the representative value 511 keV", E.GetEnergy(), 511.0, 1e-12) && Passed;
  E.SetEnergy(-3.5);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "negative energy", "GetEnergy returns a negative value unchanged", E.GetEnergy(), -3.5, 1e-12) && Passed;

  Passed = Evaluate("GetType()", "energy", "GetType returns 'energy'", E.GetType(), MString("energy")) && Passed;
  Passed = Evaluate("GetType()", "static type", "GetType matches the static type name", E.GetType(), MReadOutDataEnergy::m_Type) && Passed;
  Passed = Evaluate("GetTypeID()", "static type ID", "GetTypeID returns the hash of the type name", E.GetTypeID(), MString("energy").GetHash()) && Passed;
  Passed = EvaluateTrue("GetTypeID()", "uniqueness", "The energy type ID differs from the TAC type ID", MReadOutDataEnergy::m_TypeID != MReadOutDataTAC::m_TypeID) && Passed;
  Passed = EvaluateTrue("GetTypeID()", "uniqueness", "The energy type ID differs from the flags type ID", MReadOutDataEnergy::m_TypeID != MReadOutDataFlags::m_TypeID) && Passed;
  Passed = EvaluateTrue("IsOfType(MString)", "energy", "IsOfType('energy') is true", E.IsOfType(MString("energy"))) && Passed;
  Passed = EvaluateFalse("IsOfType(MString)", "tac", "IsOfType('tac') is false", E.IsOfType(MString("tac"))) && Passed;
  Passed = EvaluateTrue("IsOfType(long)", "energy type ID", "IsOfType(m_TypeID) is true", E.IsOfType(MReadOutDataEnergy::m_TypeID)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataEnergy::TestWrapping()
{
  bool Passed = true;

  MReadOutDataTAC* TAC = new MReadOutDataTAC();
  TAC->SetTAC(10452);
  MReadOutDataEnergy E(TAC); // E owns TAC
  E.SetEnergy(511.0);

  Passed = Evaluate("GetCombinedType()", "energy wrapping tac", "The combined type is 'tac-energy'", E.GetCombinedType(), MString("tac-energy")) && Passed;
  Passed = Evaluate("GetNumberOfParsableElements()", "energy wrapping tac", "The number of parsable elements is 2", E.GetNumberOfParsableElements(), 2u) && Passed;
  Passed = EvaluateTrue("Get()", "energy type ID", "Get(energy) returns the object itself", E.Get(MReadOutDataEnergy::m_TypeID) == &E) && Passed;
  Passed = EvaluateTrue("Get()", "tac type ID", "Get(tac) returns the wrapped TAC object", E.Get(MReadOutDataTAC::m_TypeID) == TAC) && Passed;
  Passed = EvaluateTrue("Get()", "flags type ID", "Get(flags) returns nullptr when no flags data is wrapped", E.Get(MReadOutDataFlags::m_TypeID) == nullptr) && Passed;

  E.Clear();
  Passed = EvaluateNear("Clear()", "energy wrapping tac", "Clear() resets the energy", E.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("Clear()", "energy wrapping tac", "Clear() also resets the wrapped TAC", TAC->GetTAC(), 0u) && Passed;
  Passed = Evaluate("Clear()", "energy wrapping tac", "Clear() keeps the wrapped element", E.GetCombinedType(), MString("tac-energy")) && Passed;

  // Three levels: flags <- tac <- energy
  MReadOutDataFlags* Flags = new MReadOutDataFlags();
  MReadOutDataTAC* TAC2 = new MReadOutDataTAC(Flags);
  MReadOutDataEnergy E3(TAC2);
  Passed = Evaluate("GetCombinedType()", "three levels", "The combined type is 'flags-tac-energy'", E3.GetCombinedType(), MString("flags-tac-energy")) && Passed;
  Passed = Evaluate("GetNumberOfParsableElements()", "three levels", "The number of parsable elements is 3", E3.GetNumberOfParsableElements(), 3u) && Passed;
  Passed = EvaluateTrue("Get()", "three levels", "Get(flags) finds the innermost flags element", E3.Get(MReadOutDataFlags::m_TypeID) == Flags) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataEnergy::TestClone()
{
  bool Passed = true;

  MReadOutDataEnergy Plain;
  Plain.SetEnergy(1173.2);
  MReadOutDataEnergy* PlainClone = Plain.Clone();
  Passed = EvaluateTrue("Clone()", "no wrapped data", "Clone() returns a non-null object", PlainClone != nullptr) && Passed;
  if (PlainClone != nullptr) {
    Passed = EvaluateTrue("Clone()", "no wrapped data", "Clone() returns a distinct object", PlainClone != &Plain) && Passed;
    Passed = EvaluateNear("Clone()", "no wrapped data", "The clone has the same energy", PlainClone->GetEnergy(), 1173.2, 1e-12) && Passed;
    Passed = Evaluate("Clone()", "no wrapped data", "The clone has the same combined type", PlainClone->GetCombinedType(), MString("energy")) && Passed;
    PlainClone->SetEnergy(1.0);
    Passed = EvaluateNear("Clone()", "independence", "Modifying the clone does not modify the original", Plain.GetEnergy(), 1173.2, 1e-12) && Passed;
  }
  delete PlainClone;

  MReadOutDataTAC* TAC = new MReadOutDataTAC();
  TAC->SetTAC(321);
  MReadOutDataEnergy Wrapped(TAC);
  Wrapped.SetEnergy(59.5);
  MReadOutDataEnergy* WrappedClone = Wrapped.Clone();
  Passed = EvaluateTrue("Clone()", "with wrapped tac", "Clone() returns a non-null object", WrappedClone != nullptr) && Passed;
  if (WrappedClone != nullptr) {
    Passed = Evaluate("Clone()", "with wrapped tac", "The clone keeps the combined type 'tac-energy'", WrappedClone->GetCombinedType(), MString("tac-energy")) && Passed;
    MReadOutDataTAC* ClonedTAC = dynamic_cast<MReadOutDataTAC*>(WrappedClone->Get(MReadOutDataTAC::m_TypeID));
    Passed = EvaluateTrue("Clone()", "with wrapped tac", "The clone contains a TAC element", ClonedTAC != nullptr) && Passed;
    if (ClonedTAC != nullptr) {
      Passed = EvaluateTrue("Clone()", "with wrapped tac", "The wrapped TAC is deep copied", ClonedTAC != TAC) && Passed;
      Passed = Evaluate("Clone()", "with wrapped tac", "The wrapped TAC value is copied", ClonedTAC->GetTAC(), 321u) && Passed;
      ClonedTAC->SetTAC(7);
      Passed = Evaluate("Clone()", "independence", "Modifying the cloned TAC does not modify the original", TAC->GetTAC(), 321u) && Passed;
    }
  }
  delete WrappedClone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataEnergy::TestStream()
{
  bool Passed = true;

  MReadOutDataEnergy E;
  E.SetEnergy(511.5);
  Passed = Evaluate("ToString()", "511.5", "ToString returns the energy followed by a space", E.ToString(), MString("511.5 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "without descriptor", "ToParsableString(false) equals ToString", E.ToParsableString(false), MString("511.5 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "with descriptor", "ToParsableString(true) prefixes the type name", E.ToParsableString(true), MString("energy 511.5 ")) && Passed;

  ostringstream Out;
  Out<<E;
  Passed = Evaluate("operator<<", "511.5", "operator<< writes ToString", MString(Out.str()), MString("511.5 ")) && Passed;

  MReadOutDataTAC* TAC = new MReadOutDataTAC();
  TAC->SetTAC(10452);
  MReadOutDataEnergy W(TAC);
  W.SetEnergy(662);
  Passed = Evaluate("ToString()", "wrapping tac", "ToString writes the wrapped TAC first, then the energy", W.ToString(), MString("10452 662 ")) && Passed;
  Passed = Evaluate("ToParsableString()", "wrapping tac", "ToParsableString(true) prefixes the combined type", W.ToParsableString(true), MString("tac-energy 10452 662 ")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutDataEnergy::TestParse()
{
  bool Passed = true;

  MTokenizer T;
  T.AnalyzeFast("511.25");
  MReadOutDataEnergy E;
  Passed = EvaluateTrue("Parse()", "single token", "Parse() succeeds for one energy token", E.Parse(T, 0)) && Passed;
  Passed = EvaluateNear("Parse()", "single token", "Parse() reads the energy 511.25", E.GetEnergy(), 511.25, 1e-9) && Passed;

  MTokenizer TOffset;
  TOffset.AnalyzeFast("XX 5 1332.5");
  MReadOutDataEnergy EOffset;
  Passed = EvaluateTrue("Parse()", "start element 2", "Parse() succeeds with a start offset", EOffset.Parse(TOffset, 2)) && Passed;
  Passed = EvaluateNear("Parse()", "start element 2", "Parse() reads the token at the start offset", EOffset.GetEnergy(), 1332.5, 1e-9) && Passed;

  // Round trip through ToParsableString
  MReadOutDataTAC* TAC = new MReadOutDataTAC();
  TAC->SetTAC(4711);
  MReadOutDataEnergy Writer(TAC);
  Writer.SetEnergy(356.25);
  MTokenizer TRound;
  TRound.AnalyzeFast(Writer.ToParsableString(false));
  MReadOutDataEnergy Reader(new MReadOutDataTAC());
  Passed = EvaluateTrue("Parse()", "round trip", "Parse() succeeds on the output of ToParsableString", Reader.Parse(TRound, 0)) && Passed;
  Passed = EvaluateNear("Parse()", "round trip", "The energy survives the round trip", Reader.GetEnergy(), 356.25, 1e-9) && Passed;
  MReadOutDataTAC* ReaderTAC = dynamic_cast<MReadOutDataTAC*>(Reader.Get(MReadOutDataTAC::m_TypeID));
  Passed = EvaluateTrue("Parse()", "round trip", "The parsed object contains a TAC element", ReaderTAC != nullptr) && Passed;
  if (ReaderTAC != nullptr) {
    Passed = Evaluate("Parse()", "round trip", "The wrapped TAC survives the round trip", ReaderTAC->GetTAC(), 4711u) && Passed;
  }

  // Repeated parse on the same object
  MTokenizer TSecond;
  TSecond.AnalyzeFast("1 2");
  Passed = EvaluateTrue("Parse()", "repeated parse", "A second Parse() succeeds", Reader.Parse(TSecond, 0)) && Passed;
  Passed = EvaluateNear("Parse()", "repeated parse", "The second Parse() overwrites the energy", Reader.GetEnergy(), 2.0, 1e-9) && Passed;

  // Error paths
  MTokenizer TEmpty;
  TEmpty.AnalyzeFast("");
  MReadOutDataEnergy EEmpty;
  EEmpty.SetEnergy(42.0);
  DisableDefaultStreams();
  bool EmptyResult = EEmpty.Parse(TEmpty, 0);
  bool OffsetResult = EOffset.Parse(TOffset, 3);
  MTokenizer TShort;
  TShort.AnalyzeFast("17");
  MReadOutDataEnergy EShort(new MReadOutDataTAC());
  bool ShortResult = EShort.Parse(TShort, 0);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Parse()", "no tokens", "Parse() returns false without tokens", EmptyResult) && Passed;
  Passed = EvaluateNear("Parse()", "no tokens", "A failed Parse() preserves the previous energy", EEmpty.GetEnergy(), 42.0, 1e-12) && Passed;
  Passed = EvaluateFalse("Parse()", "start element past end", "Parse() returns false when the start element is past the last token", OffsetResult) && Passed;
  Passed = EvaluateFalse("Parse()", "wrapped, one token", "Parse() returns false if the tokens do not cover the wrapped data", ShortResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNReadOutDataEnergy Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNReadOutDataEnergy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
