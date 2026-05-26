# Nuclearizer Unit Test Conventions

## 0. Preamble

### Information for biological life forms:

This document is written by and for AIs.
When you want to add, extend, or review a unit test — or have an AI review
production code — have the AI read this document together with the style guide
`CodingConventions.md`.

Begin every such prompt with:

```
Please read the nuclearizer style guide: CodingConventions.md
Please read the nuclearizer unit test conventions: unittests/TestingConventions.md

If you find a bug in production code (src/*, include/*, apps/*), do not modify
those files automatically — show a diff and ask before applying, and fix only
one bug at a time, pausing for approval after each.
```

Then state the task.  Example prompts:

Add a unit test for a class:

```
Add a unit test for the class in src/MFoo.cxx to the unittests directory,
following the same rules and style as the existing unit tests. Cover the whole
public API and work through the checklist in section "Test Completeness".
```

Review or extend an existing unit test:

```
Review unittests/UTNFoo.cxx against MFoo for completeness and correctness using
the checklist in section "Test Completeness". List the coverage gaps and weak
assertions, then add the missing coverage.
```

Review production code for bugs:

```
Do a thorough review of src/MFoo.cxx and include/MFoo.h for bugs. Report each
finding with a severity, propose fixes as diffs, and apply them one at a time
only after I approve.
```

Add an end-to-end test for a dataset:

```
Add an end-to-end test for the data in resource/unittestdata/<dataset>/, modeled
on UTNEndToEnd_542-1.cxx (see section "End-to-End Tests"). It must be fully
automatic and safe to run in parallel with the other end-to-end tests.
```


### Information for AIs

This document captures the rules and patterns that govern every unit test in
`nuclearizer/unittests/`.

Read it before writing a new test; keep it up to date when a new pattern is established.

When you find a bug in production code (`src/*`, `include/*`, `apps/*`), do not
fix it silently while writing tests.  Fix one bug at a time, show the diff and
wait for approval, make the minimum change that fixes it, and do not refactor or
restyle unrelated code unless explicitly asked.

---

## 1. File and Class Structure

There are two kinds of test files:

- **Class unit tests** (`UTN<ClassName>.cxx`) — test a single class `MClassName` in isolation.
- **End-to-end tests** (`UTNEndToEnd_<dataset>.cxx`) — run a full pipeline and compare output to a reference file.

Each class unit test file tests exactly one class `MFoo`.

### Header block

```cpp
// Standard libs:
#include <sstream>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"

// Nuclearizer:
#include "MFoo.h"
```

Include `<sstream>` whenever any stream test is needed.  Group includes in
three blocks — standard, MEGAlib, Nuclearizer — in that order.

Add other standard headers explicitly when they are used.  Common examples:
`<cstdlib>` for `getenv`, `<fstream>` for fixture files, and `<unistd.h>` for
`getpid()` when building process-ID-suffixed file names.

Tests emit any diagnostic output — a skip notice, a progress line — through the
MEGAlib streams `mout`/`merr`, never `cout`/`cerr`.

### Test class

```cpp
class UTFoo : public MUnitTest
{
public:
  UTFoo() : MUnitTest("UTFoo") {}
  virtual ~UTFoo() {}

  virtual bool Run();

private:
  bool TestDefaultConstruction();
  bool TestGettersSetters();
  // ... one method per logical concern
};
```

The constructor passes the class name as a string to `MUnitTest`.

### `Run()`

```cpp
bool UTFoo::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters()      && Passed;
  // ...

  Summarize();
  return Passed;
}
```

Every sub-test is chained with `&& Passed` so all sub-tests run even when one
fails, but the overall result is false if any failed.

### `main()`

```cpp
int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTFoo", "Unit tests for MFoo") == false) return 1;

  UTFoo Test;
  return Test.Run() == true ? 0 : 1;
}
```

If the class under test does not need `MGlobal::Initialize` (e.g., pure data
classes like `MStripHit` or `MHit`), call only `Test.Run()`:

```cpp
int main()
{
  UTFoo Test;
  return Test.Run() == true ? 0 : 1;
}
```

---

## 2. Evaluate Calls

All assertions use the four `MUnitTest` helpers.

All four helpers share the same argument prefix: **Function**, **Input**, **Description**, then the value(s).

### `Evaluate` — equality

```cpp
Passed = Evaluate("SetFoo/GetFoo", "representative value 42",
                  "GetFoo returns the representative value 42",
                  H.GetFoo(), 42) && Passed;
```

Arguments: `Function`, `Input`, `Description`, `Output`, `Truth`.

Use for exact equality of `int`, `unsigned int`, `MString`, `bool`-as-int, etc.

`Output` and `Truth` share a single template parameter, so they must be the
**exact same type** or the call fails to compile (`deduced conflicting types
for parameter 'T2'`).  Match the truth literal to the getter's return type:

- `size_t` counts (`.size()`) — cast both sides to `(unsigned int)`.
- `GetID()` returns `unsigned long` — write the literal as `(unsigned long) 1`.
- otherwise cast the literal to whatever the getter returns.

### `EvaluateNear` — floating-point

```cpp
Passed = EvaluateNear("SetEnergy/GetEnergy", "representative value",
                      "GetEnergy returns the representative value 511.0 keV",
                      H.GetEnergy(), 511.0, 1e-9) && Passed;
```

Arguments: `Function`, `Input`, `Description`, `Output`, `Truth`, `Tolerance`.

Always supply an explicit tolerance.  Use `1e-9` for a double-precision
round-trip, `1e-4` for values limited by a stream's default `ostream` output
precision (6 significant digits), and `0.5` for integer-valued fields read back
via `sscanf` with `%d`.  Never compare floating-point values with `==` — always
use `EvaluateNear` with an explicit tolerance.

### `EvaluateTrue` / `EvaluateFalse` — boolean

```cpp
Passed = EvaluateTrue("IsLowVoltageStrip()", "alias true",
                      "IsLowVoltageStrip() returns true after IsXStrip(true)",
                      H.IsLowVoltageStrip() == true) && Passed;

Passed = EvaluateFalse("HasTriggered()", "default",
                       "Default HasTriggered is false",
                       H.HasTriggered()) && Passed;
```

Arguments: `Function`, `Input`, `Description`, `bool`.  All four arguments are
required — omitting `Description` causes a compile error.

Prefer `EvaluateTrue`/`EvaluateFalse` over `Evaluate(..., true)` /
`Evaluate(..., false)` for boolean results.

### `Summarize()`

```cpp
Summarize();   // no arguments, returns void
return Passed;
```

`Summarize()` takes no arguments and returns `void`.  Call it at the end of
`Run()` and return `Passed` separately.

---

## 3. Standard Sub-Test Set

Every class test should cover these concerns in this order when applicable.

| Sub-test | Covers |
|---|---|
| `TestDefaultConstruction` | Default values after `T obj;` and after explicit `obj.Clear()` — do **not** add a trivially-true `Evaluate(..., true, true)` "construction" check; if construction crashes the test crashes, which is sufficient signal |
| `TestGettersSetters` | Every public setter/getter pair, once with a representative value |
| `TestXxx` (domain logic) | Bit layouts, deduplication, sorting, accumulation, etc. |
| `TestStreamDatParse` | `StreamDat()` round-trip through `Parse()` for each version |
| `TestStreamRoa` / `TestStreamEvta` | Output format, conditional fields, edge cases |

Not every class needs all of them; omit what does not apply.

---

## 4. Test Completeness

Use this checklist before calling a class unit test complete.

### Public API coverage

- Every public function declared or overridden by the class under test gets a
  direct functional test.  Indirect coverage through another API does not count.
- Every public setter/getter pair is tested directly.
- Inherited framework APIs do not need retesting unless the class changes their
  behaviour or relies on them in a nontrivial way.
- Public functions that are intentionally unsupported, deprecated, or impossible
  to exercise in a unit test must be documented in the test with a short comment.
- Non-public functions are tested only when needed to verify behaviour that is not
  reachable through the public API.

### Real usage coverage

- Audit real in-tree usage of the class before deciding the test is complete.
- Add tests for the usage patterns found there, not only for abstract API
  behaviour.
- If production code relies on a non-obvious convention, such as a filename
  timestamp, ownership transfer, event ordering, or a sentinel value, the unit
  test must cover that convention directly.

### Inputs and edge cases

- Test representative typical inputs and meaningful edge cases.
- Edge cases include, when applicable: empty input, single-element input, zero,
  negative values, maximum size, and boundary values (`n`, `n-1`, `n+1`).
- Do not only test trivial, symmetric, or axis-point inputs.  Add nontrivial
  interior-domain cases when the function has a meaningful domain.
- Every documented error path is tested.  Verify the documented behaviour:
  `false`, `nullptr`, sentinel value, thrown exception, or preserved old state.

### State and lifecycle

- Stateful classes must be tested with repeated operations on the same object:
  repeated parses, reads, loads, clears, resets, or reinitializations.
- For I/O classes, failed operations on reused objects must not leave stale state
  that affects the next successful operation.
- Owning classes need lifecycle tests for `Clear()`, removal, destruction
  semantics, and copy/assignment when those operations are public and supported.
- Tests must verify that copied state is independent when copy or assignment is
  part of the supported API.

### Exactness

- When a concrete expected value can be derived, assert the exact value.
- Range, finiteness, and non-empty checks are fallback assertions only when no
  exact expected value is reasonably derivable.
- For math, geometry, calibration, and time-conversion helpers, prefer direct
  value assertions or inverse/round-trip checks with explicit tolerances.
- For stochastic code, fix the random seed explicitly before asserting results.

### Cross-boundary changes

- If a change crosses class boundaries, add or update the relevant integration or
  end-to-end test in addition to the class unit test.
- Reassess completeness only after public API coverage, real usage coverage,
  edge/error cases, and cross-boundary behaviour have all been considered.

---

## 5. TestDefaultConstruction Conventions

- Construct the object and assert every field that has a reliable default.
- For numeric "not defined" sentinels use `== g_DoubleNotDefined` or
  `== g_UnsignedIntNotDefined` (exact equality is correct for sentinel values).
- For "not defined" vector sentinels use `== g_VectorNotDefined`.
- For fields that are **not** reset by `Clear()` (e.g., `MHit`'s bool flags)
  or that are intentionally set to a non-obvious default (e.g.,
  `MReadOutAssembly::m_Trigger = true` after `Clear()`), add a comment
  explaining the behaviour rather than omitting the test silently.
- After asserting the freshly-constructed state, mutate every covered field,
  call `Clear()`, then re-assert all the same properties to confirm that
  `Clear()` actually resets them.

---

## 6. TestGettersSetters Conventions

- One representative value per setter/getter pair — not a sweep of edge values.
- For `bool` properties, test both `true` and `false`.
- For aliases (e.g., `IsXStrip` ↔ `IsLowVoltageStrip`), assert that setting
  via the alias is visible through both names.
- Use physically meaningful values when they exist (e.g., `511.0` for an
  annihilation energy, `10452` for a TAC count, `25.3` for a temperature in °C).

---

## 7. Stream / Parse Tests

### Output format checks

Use `ostringstream`, capture to `MString`, then search with `Index()`:

```cpp
ostringstream Out;
H.StreamRoa(Out, /*flags*/);
MString S(Out.str().c_str());

// Presence: token exists somewhere in output
Passed = EvaluateTrue("StreamRoa()", "contains ADC 4053",
                      "Output contains ADC value 4053",
                      S.Contains("4053")) && Passed;

// Absence: token must NOT appear
Passed = EvaluateTrue("StreamRoa()", "no TAC",
                      "Output does not contain TAC value 10452",
                      S.Contains("10452") == false) && Passed;

// Starts-with: token is at position 0
Passed = EvaluateTrue("StreamRoa()", "starts with UH",
                      "Output starts with 'UH'",
                      S.BeginsWith("UH")) && Passed;
```

Use `S.Index(...)` only when positional information or occurrence counting is
needed.

### Round-trip checks

For `StreamDat`/`Parse` pairs, write with one instance and read into another:

```cpp
MFoo Writer;
// ... populate Writer ...
ostringstream Out;
Writer.StreamDat(Out);

MFoo Reader;
Reader.Parse(MString(Out.str().c_str()));
// ... assert Reader fields match Writer fields ...
```

Then also assert that `Parse()` returns `false` for a line that does not start
with the expected keyword:

```cpp
MString BadLine("UH 0 41 l 4053 10452 4");
MFoo Dummy;
Passed = EvaluateFalse("Parse()", "non-HT line",
                       "Parse() returns false for a line that does not start with 'HT'",
                       Dummy.Parse(BadLine)) && Passed;
```

### Tolerances for Parse()

Fields written as integers (ADC, TAC, strip ID, detector ID) and read back
with `%d` use a tolerance of `0.5`.  Fields limited by the default `ostream`
output precision (6 significant digits) use `1e-4`.  Fields that survive a full
`double` round-trip use `1e-9`.

---

## 8. Ownership and Pointer Tests

### Classes that OWN their children (e.g., `MReadOutAssembly` owning `MStripHit*`)

Always allocate children with `new`; the owner deletes them.  Never pass a
stack-allocated object to an owning container.

```cpp
MStripHit* SH = new MStripHit();
SH->SetDetectorID(3);
R.AddStripHit(SH);      // R now owns SH; do NOT delete SH manually
```

After `Clear()` or at destruction, the children are gone — verify with count:

```cpp
R.Clear();
Passed = EvaluateTrue("TestClearOwnership", "NStripHits=0 after Clear",
                      R.GetNStripHits() == 0) && Passed;
```

If `RemoveXxx(i)` erases from the container **without** deleting the pointer,
delete the pointer manually in the test to avoid a leak.

### Classes that do NOT own their children (e.g., `MHit` over `MStripHit*`)

Pass stack-allocated or caller-managed pointers; the parent does not delete them:

```cpp
MStripHit SH0, SH1;   // stack-allocated; caller's lifetime
H.AddStripHit(&SH0);
```

After `RemoveStripHit`, verify the remaining element:

```cpp
H.RemoveStripHit((unsigned int) 1);     // removes index 1, shifts down
Passed = EvaluateTrue(..., H.GetStripHit(1) == &SH2) && Passed;
```

### Out-of-bounds access

Wrap any call that is expected to produce a `merr` (error message) with
`DisableDefaultStreams()` / `EnableDefaultStreams()` and assert the return value:

```cpp
DisableDefaultStreams();
Passed = EvaluateTrue("GetStripHit()", "out of bounds returns null",
                      "GetStripHit(99) returns nullptr",
                      H.GetStripHit(99) == nullptr) && Passed;
EnableDefaultStreams();
```

---

## 9. Error-State and Error-Suppression Tests

When testing that a function correctly rejects invalid input (e.g., a missing
file, a bad configuration, a malformed line), suppress the expected diagnostic
output.  Which mechanism to use depends on **how the code under test prints**.

### MEGAlib streams — `DisableDefaultStreams()` / `EnableDefaultStreams()`

When the code under test reports through the MEGAlib streams (`mout`, `merr`,
`mlog`, `mgui`) — for example `MHit::GetStripHit` does `merr<<"..."<<show` —
wrap the call:

```cpp
DisableDefaultStreams();
bool Result = Loader.Initialize();
EnableDefaultStreams();

Passed = EvaluateFalse("Initialize()", "missing HDF5 file",
                       "Initialize() returns false when the HDF5 file does not exist",
                       Result) && Passed;
```

Always re-enable streams immediately after the call under test.  Never leave
streams disabled across a sub-test boundary — the sub-test that called
`DisableDefaultStreams()` must call `EnableDefaultStreams()` before it returns,
on every path.

### Verbosity-guarded `cout` — lower `g_Verbosity`

Much of the nuclearizer source prints diagnostics with the construct
`if (g_Verbosity >= c_Error) cout<<...<<endl;` — for example `MStripHit::Parse`.
This is raw `cout`, so `DisableDefaultStreams()` does **not** silence it.  Do not
convert these prints to `merr` — the verbosity-guarded `cout` is an established
nuclearizer convention.  Instead, lower `g_Verbosity` to `c_Quiet` (which is
below `c_Error`, so the guard evaluates false) for the duration of the noisy
calls, and restore the previous value afterwards:

```cpp
int OldVerbosity = g_Verbosity;
g_Verbosity = c_Quiet;

Passed = EvaluateFalse("Parse()", "non-SH line",
                       "Parse() returns false for a line that does not start with 'SH'",
                       Dummy.Parse(NonSHLine)) && Passed;
// ... further malformed-input cases ...

g_Verbosity = OldVerbosity;
```

`g_Verbosity` is process-wide global state: always save the old value and
restore it before the sub-test returns, on every path (see section "Test
Isolation").

---

## 10. Tests That Need External Data Files

Use the `NUCLEARIZER` environment variable to locate test data:

```cpp
const char* NuclearizerEnv = getenv("NUCLEARIZER");
if (NuclearizerEnv == nullptr || NuclearizerEnv[0] == '\0') {
  mout << "TestFoo: NUCLEARIZER not set - skipping test" << endl;
  return Passed;   // passes vacuously
}
MString TestDataDir = MString(NuclearizerEnv) + "/resource/unittestdata/<dataset>";
```

Never hard-code absolute paths.  If the variable is unset in a class or module
unit test, skip the committed-data sub-test with a printed notice and return
`Passed` (true), so local builds do not fail when the data is unavailable.
Dedicated end-to-end tests are required-data tests: CI must run them with
`NUCLEARIZER` set, and if `NUCLEARIZER` is set but a required file is missing,
the test must fail.

### Test-generated fixture files

When a test builds a small input file itself (rather than using committed
data), write it under `/tmp/`, assert the stream opened, and remove it
afterwards:

```cpp
MString DatFile("/tmp/UTNFoo_valid.dat");
ofstream Out(DatFile.Data());
Passed = EvaluateTrue("Foo()", "write fixture", "The fixture file can be created",
                      Out.is_open()) && Passed;
Out << "..." << endl;
Out.close();

// ... use the file ...

MFile::Remove(DatFile);
```

Always assert `is_open()` so a write failure is reported directly instead of
surfacing later as a confusing read error.  Give each fixture a name unique to
the test (process-ID-suffixed if it may run concurrently with itself — see
section "End-to-End Tests").

---

## 11. Build System

The nuclearizer `Makefile` discovers all unit test sources via wildcard:

```makefile
CXX_UT := $(wildcard $(NUCLEARIZER_DIR)/unittests/*.cxx)
EXE_UT := $(patsubst %.cxx,%,$(CXX_UT))
EXE_UT := $(patsubst $(NUCLEARIZER_DIR)/unittests/%,$(BN)/%,$(EXE_UT))
```

A new file `unittests/UTFoo.cxx` is automatically compiled and linked as
`$(MEGALIB)/bin/UTFoo` the next time `make unittests` is run.  No Makefile
edit is needed.

---

## 12. Naming and Comment Style

- Sub-test method names: `TestCamelCase`, one logical concern per method.
- Every sub-test method has a `//! ...` doxygen comment on the declaration.
- The first argument to every `Evaluate*` call is the function signature being
  tested: `"SetFoo/GetFoo"`, `"StreamDat() V2"`, `"Parse()"`, etc.
  **Exception:** in end-to-end tests there is no single function under test; use
  the test scenario name instead: `"End-to-end test 542-1"` (see section
  "End-to-End Tests").
- The second argument is a short description of the input scenario:
  `"representative value 42"`, `"after Clear"`, `"missing HDF5 file"`.
- The third argument is a complete English sentence describing the expected
  outcome: `"GetFoo returns the representative value 42"`.
- Comments inside sub-tests explain non-obvious behaviour (e.g., which fields
  `Clear()` does *not* reset, or why a particular sentinel value is expected).

---

## 13. End-to-End Tests

End-to-end tests run a full nuclearizer pipeline and compare the output file to
a checked-in reference.

### File and class naming

```
UTNEndToEnd_<dataset>.cxx   →  class UTNEndToEnd_<dataset_underscored>
```

The dash in the file name is replaced with an underscore in the C++ class name
because dashes are not valid in identifiers:

```
UTNEndToEnd_542-1.cxx   →   class UTNEndToEnd_542_1
```

The constructor string keeps the original dash so the runner output is readable:

```cpp
UTNEndToEnd_542_1() : MUnitTest("UTNEndToEnd_542-1") {}
```

### `Evaluate*` first argument

Because there is no single function under test, the first argument is the
human-readable test scenario name, not a function signature:

```cpp
Passed = EvaluateTrue("End-to-end test 542-1", "exit status",
                      "nuclearizer exits with status 0",
                      Status == 0) && Passed;
```

Use the same string consistently for every `Evaluate*` call within the test.

### Running the pipeline with `RunChildProcess`

Build the full argument string before calling `RunChildProcess`:

```cpp
MString Arguments = MString("-c ") + ConfigFile + " -a -n";
int Status = MSystem::RunChildProcess("nuclearizer", Arguments, LogFile);
```

`MSystem::RunChildProcess` invokes `/bin/sh -c "nuclearizer <arguments>"`, so the shell
splits the string into separate argv entries.  Stdout and stderr are captured to
`LogFile` (in `/tmp/`) for post-failure inspection.

### Parallel safety and reproducibility

End-to-end tests are separate executables and may run concurrently, so every
path a test *writes* must be unique to it:

- **Log file** — name it `/tmp/UTNEndToEnd_<dataset>_<pid>.log`, building the
  process-ID suffix with `getpid()` (`#include <unistd.h>`).  A fixed log name
  would be clobbered if the same test is run twice at once:

  ```cpp
  MString LogFile = MString("/tmp/UTNEndToEnd_542-1_") + (unsigned int) getpid() + ".log";
  ```

- **Output file** — generated output must live under `/tmp/`, not in the source
  tree.  Give it a dataset-and-process-ID-specific name.  If the committed
  nuclearizer configuration hard-codes the output file path, copy the
  configuration to `/tmp/` for the test run and rewrite only the output path in
  that temporary copy.  Delete any pre-existing output *before* running
  nuclearizer, so the "output file was created" check and the comparison reflect
  strictly the current run, not a stale artifact:

  ```cpp
  MString OutputFile = MString("/tmp/UTNEndToEnd_542-1_") + (unsigned int) getpid() + ".tra";
  if (MFile::Exists(OutputFile) == true) {
    MFile::Remove(OutputFile);
  }
  ```

The test is fully automatic: it takes no command-line arguments and needs no
interactive input — the data is located via `$NUCLEARIZER` and nuclearizer is
run headless (`-a` runs the analysis, `-n` suppresses the GUI).

### Comparing output to a reference file

Use `EvaluateFilesIdentical`, which streams both files line by line without
loading them into memory and stops at the first mismatch:

```cpp
Passed = EvaluateFilesIdentical("End-to-end test 542-1", "output file",
                                "The output .tra file is identical to the reference",
                                OutputFile, ReferenceFile) && Passed;
```

The reference file lives alongside the config and input data under
`$NUCLEARIZER/resource/unittestdata/<dataset>/` and is named
`<output-stem>.reference.<ext>`.

If a production bug fix changes the end-to-end output, do **not** blindly
regenerate the reference.  First inspect the old and new outputs and document why
the old reference was wrong or why the intended behaviour changed.  For example,
the HDF v2 loader once merged consecutive `/Events` rows into one
`MReadOutAssembly`; the stale reference could be identified because the new
single-event energies summed exactly to the old merged-event energy.

When practical, pair an end-to-end reference comparison with a focused module
unit test for the semantic behaviour being protected.  A full `.tra` comparison
can detect that output changed, but a module test should identify the actual
contract, e.g. "one HDF `/Events` row produces one `MReadOutAssembly` with this
ID and this strip-hit count".

Reference files should have documented provenance either in nearby comments, in
the test source, or in a small companion note: input file, configuration file,
expected event count, and whether the reference was generated before or after a
known behaviour change.

### Locating files

Use `$NUCLEARIZER` (see section "Tests That Need External Data Files") and
`MFile::Exists` (not `ifstream`) to check that prerequisites exist before
launching the pipeline:

```cpp
Passed = EvaluateTrue("End-to-end test 542-1", "config file",
                      "The nuclearizer config file exists",
                      MFile::Exists(ConfigFile)) && Passed;
```

If production code derives semantics from a fixture file name, the test must
document that filename contract and use a representative real filename pattern.
For HDF v2.2+ input, for example, the loader needs an acquisition start timestamp
to convert SpaceWire time codes into UTC, so the fixture name must contain the
timestamp form accepted by production code.

---

## 14. Module Unit Tests

A test for an `MModule` subclass (e.g. `MModuleLoaderMeasurementsHDF`) is still
a class unit test, but covers a few module-specific concerns beyond the standard
getter/setter set.

### `Initialize()` error paths

Verify `Initialize()` returns `false` for each missing prerequisite — a missing
input file, a missing calibration, a bad configuration.  Suppress the expected
diagnostics (see section "Error-State and Error-Suppression Tests"):

```cpp
DisableDefaultStreams();
bool Result = Loader.Initialize();
EnableDefaultStreams();
Passed = EvaluateFalse("Initialize()", "missing HDF5 file",
                       "Initialize() returns false when the HDF5 file does not exist",
                       Result) && Passed;
```

### XML configuration round-trip

Serialise with `CreateXmlConfiguration()`, read the node back into a fresh
instance with `ReadXmlConfiguration()`, and assert every configured value
survives.  Delete the node when done:

```cpp
MModuleFoo Writer;
Writer.SetSomething(Value);
MXmlNode* Node = Writer.CreateXmlConfiguration();

MModuleFoo Reader;
Reader.ReadXmlConfiguration(Node);
Passed = Evaluate("ReadXmlConfiguration()", "Something",
                  "ReadXmlConfiguration() restores the configured value",
                  Reader.GetSomething(), Value) && Passed;
delete Node;
```

### `AnalyzeEvent()` against committed data

For a module that produces events, run `Initialize()` and then a series of
`AnalyzeEvent()` calls on a committed `$NUCLEARIZER` dataset (see section
"Tests That Need External Data Files"), and assert the reconstructed
`MReadOutAssembly` matches documented reference values
— event ID, strip-hit count, and per-hit detector / strip / ADC / TAC.  Guard
per-hit access with the count so a wrong count cannot crash the test:

```cpp
if (Event->GetNStripHits() == 4) {
  MStripHit* H0 = Event->GetStripHit(0);
  Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0 strip",
                    "Event 1 hit 0 has the representative strip ID 41",
                    H0->GetStripID(), 41u) && Passed;
  // ...
}
```

`AnalyzeEvent()` events are heap-allocated `MReadOutAssembly*`; `delete` each one
after its checks (the assembly owns and frees its strip hits).

For loaders that read grouped event formats, include checks that protect the
event-boundary contract directly.  Do not only assert that `AnalyzeEvent()`
returns true: assert the expected sequence of event IDs and per-event hit counts
so accidental merging, splitting, or skipping of file events is visible in the
module unit test.

---

## 15. Test Isolation

Each test executable must pass on its own and in any order relative to the
others.

- A test must not depend on another test's side effects, files, or global state.
- Restore any global, `static`, singleton, or environment-variable state the
  test mutates.  In particular, do not assert the *absolute* value of a
  process-wide counter (e.g. `MReadOutAssembly`'s static assembly-ID counter) —
  assert only relative properties such as uniqueness and strict increase.
- Do all filesystem work under `/tmp/` (see section "Tests That Need External
  Data Files"), never in the source tree or the real user filesystem, and
  remove what you create.
- No network access in a unit test.
- A flaky test is a bug — fix the cause, never paper over it with retries.
- Use real objects and small real inputs; do not mock the class under test, and
  do not write tests for ROOT, Geant4, or other third-party behaviour.
