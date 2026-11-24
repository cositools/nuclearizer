/*
 * UTNRunner.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <dirent.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <sys/wait.h>
#include <thread>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
using namespace std;

// MEGAlib:
#include "MFile.h"
#include "MStreams.h"
#include "MString.h"


//! Parse the output of one unit test executable: the summary of the last Summarize() and the reports of MUnitTest
class UTNRunnerTestParser
{
 public:
  //! Default constructor
  UTNRunnerTestParser() : m_HasSummary(false), m_NumberOfPassedChecks(0), m_NumberOfFailedChecks(0), m_NumberOfKnownFailures(0), m_NumberOfUnexpectedPasses(0), m_IsAborted(false) {}
  //! Default destructor
  virtual ~UTNRunnerTestParser() {}

  //! Parse the output of the test
  void Parse(const MString& Output);

  //! Return true if the "Passed tests:" and "Failed tests:" summary lines were found
  bool HasSummary() const { return m_HasSummary; }
  //! Return the number of passed checks
  long GetNumberOfPassedChecks() const { return m_NumberOfPassedChecks; }
  //! Return the number of failed checks, including unexpected passes
  long GetNumberOfFailedChecks() const { return m_NumberOfFailedChecks; }
  //! Return the number of failed checks marked with ExpectedToFail()
  long GetNumberOfKnownFailures() const { return m_NumberOfKnownFailures; }
  //! Return the number of checks marked with ExpectedToFail() which passed
  long GetNumberOfUnexpectedPasses() const { return m_NumberOfUnexpectedPasses; }
  //! Return true if MUnitTest aborted before any check since the test could not be isolated from the user environment
  bool IsAborted() const { return m_IsAborted; }
  //! Return the FAILED, UNEXPECTED PASS, and ABORTED reports of MUnitTest, each with its indented detail lines
  const vector<MString>& GetFailureReports() const { return m_FailureReports; }
  //! Return the last lines of the output, e.g. to show the cause of a crash
  const vector<MString>& GetLastLines() const { return m_LastLines; }

 private:
  //! Return the number after the first line starting with Prefix at or after Start; -1 if not found
  long ParseSummaryValue(const vector<MString>& Lines, size_t Start, const MString& Prefix) const;
  //! Count the lines starting with Prefix
  long CountLinesStartingWith(const vector<MString>& Lines, const MString& Prefix) const;

  //! True if the "Passed tests:" and "Failed tests:" summary lines were found
  bool m_HasSummary;
  //! Passed checks
  long m_NumberOfPassedChecks;
  //! Failed checks, including unexpected passes
  long m_NumberOfFailedChecks;
  //! Failed checks marked with ExpectedToFail()
  long m_NumberOfKnownFailures;
  //! Checks marked with ExpectedToFail() which passed
  long m_NumberOfUnexpectedPasses;
  //! True if MUnitTest aborted before any check
  bool m_IsAborted;
  //! The FAILED, UNEXPECTED PASS, and ABORTED reports with their detail lines
  vector<MString> m_FailureReports;
  //! The last lines of the output
  vector<MString> m_LastLines;

  //! Number of output lines kept in m_LastLines
  static constexpr size_t c_NumberOfLastLines = 20;
};


////////////////////////////////////////////////////////////////////////////////


//! The run of a single unit test executable: launch, capture of its output, and classification of the result
class UTNRunnerSingleRun
{
 public:
  //! Run lifecycle states
  enum EStatus {
    c_StatusPending = 0,  //!< not yet launched
    c_StatusRunning,      //!< child process is running
    c_StatusFailed,       //!< failed checks or unexpected passes, an exit code inconsistent with the summary, or aborted before any check
    c_StatusCrashed,      //!< killed by a signal, failed to launch, or no summary
    c_StatusKnown,        //!< exited with status 0, but with known failures
    c_StatusPassed        //!< exited with status 0 without any failure
  };

  //! Default constructor
  UTNRunnerSingleRun(const MString& Name, const MString& Executable) : m_Name(Name), m_Executable(Executable), m_Status(c_StatusPending), m_Pid(-1), m_Runtime(0.0) {}
  //! Default destructor
  virtual ~UTNRunnerSingleRun() {}

  //! Start the test in a child process with stdout and stderr captured in a temporary file; on failure the run is marked as crashed
  bool Launch();
  //! Evaluate the finished child process from its wait status
  void Finish(int WaitStatus);
  //! Mark the run as crashed, e.g. if it never finished
  void MarkCrashed(const MString& Detail);

  //! Return the name of the test
  const MString& GetName() const { return m_Name; }
  //! Return the status of the run
  EStatus GetStatus() const { return m_Status; }
  //! Return the process ID of the running test
  pid_t GetPid() const { return m_Pid; }
  //! Return the runtime in seconds of a finished run
  double GetRuntime() const { return m_Runtime; }
  //! Return the parsed output of a finished run
  const UTNRunnerTestParser& GetParser() const { return m_Parser; }
  //! Return true if the run is neither pending nor running
  bool IsDone() const { return m_Status != c_StatusPending && m_Status != c_StatusRunning; }
  //! Return the fixed-width label of the status: "PASS ", "KNOWN", "FAIL ", "CRASH", or blanks if not done
  MString GetStatusLabel() const;
  //! Return the name with counts and details, e.g. "UTNFoo (146 passed, 2 known failures)"
  MString GetDescription() const;

 private:
  //! Set the status and the detail from the wait status and the parsed output
  void Classify(int WaitStatus);
  //! Format the counts as e.g. "146 passed, 1 failed, 2 known failures"
  MString FormatCounts() const;
  //! Read the captured output
  MString ReadOutput() const;

  //! Exit code of the child process if the test executable cannot be started, as in the shell
  static constexpr int c_ExitCodeExecFailed = 127;

  //! Name of the test
  MString m_Name;
  //! Full path of the test executable
  MString m_Executable;
  //! Status of the run
  EStatus m_Status;
  //! Process ID of the running test
  pid_t m_Pid;
  //! Temporary file capturing stdout and stderr
  MString m_OutputFile;
  //! Description of crashes and inconsistencies
  MString m_Detail;
  //! Start time of the run
  chrono::steady_clock::time_point m_StartTime;
  //! Runtime in seconds
  double m_Runtime;
  //! Parsed output
  UTNRunnerTestParser m_Parser;
};


////////////////////////////////////////////////////////////////////////////////


//! One line of a dashboard pane, either a section heading (drawn in bold) or a normal line
class UTNRunnerDashboardLine
{
 public:
  //! Default constructor
  UTNRunnerDashboardLine(const MString& Text, bool IsHeading = false) : m_Text(Text), m_IsHeading(IsHeading) {}
  //! Default destructor
  virtual ~UTNRunnerDashboardLine() {}

  //! Return the text
  const MString& GetText() const { return m_Text; }
  //! Return true if the line is a section heading
  bool IsHeading() const { return m_IsHeading; }

 private:
  //! The text
  MString m_Text;
  //! True if the line is a section heading
  bool m_IsHeading;
};


////////////////////////////////////////////////////////////////////////////////


//! The live terminal dashboard of UTNRunner: running tests, failed runs, runs with known failures, and the progress
class UTNRunnerDashboard
{
 public:
  //! Enable the dashboard only if stdout is a terminal; the cursor is hidden while it is enabled
  UTNRunnerDashboard(unsigned int MaxParallel);
  //! Show the cursor again
  virtual ~UTNRunnerDashboard();

  //! Return true if stdout is a terminal and the dashboard is drawn
  bool IsEnabled() const { return m_IsEnabled; }
  //! Advance the spinner shown next to the running tests
  void AdvanceSpinner() { ++m_SpinnerIndex; }
  //! Redraw the dashboard; does nothing if it is not enabled
  void Render(const vector<UTNRunnerSingleRun>& Runs) const;

 private:
  //! Cut Text to Width characters, ending in "..." if it is cut
  MString Clip(const MString& Text, size_t Width) const;
  //! Center Text in Width characters
  MString Center(const MString& Text, size_t Width) const;
  //! Pad Text with blanks to Width characters, in bold if requested; the escape codes are added after the padding
  MString Pad(const MString& Text, size_t Width, bool Bold) const;
  //! Format a duration as "N.Ns"
  MString FormatRuntime(double Seconds) const;

  //! Terminal width if it cannot be determined
  static constexpr size_t c_DefaultTerminalWidth = 100;
  //! Minimum terminal width the dashboard is drawn for
  static constexpr size_t c_MinimumTerminalWidth = 80;
  //! Minimum width of each of the two panes
  static constexpr size_t c_MinimumPaneWidth = 24;
  //! Characters of the outer border of a row: "|" left and right
  static constexpr size_t c_OuterBorderWidth = 2;
  //! Characters between the outer borders of a row besides the two panes: " ", " | ", " "
  static constexpr size_t c_InnerFrameWidth = 5;
  //! Width of the progress bar between "[" and "]"
  static constexpr size_t c_ProgressBarWidth = 32;

  //! True if stdout is a terminal
  bool m_IsEnabled;
  //! Width of the terminal, at least c_MinimumTerminalWidth
  size_t m_TerminalWidth;
  //! Maximum number of tests running in parallel, which sets the height of the running-tests pane
  unsigned int m_MaxParallel;
  //! Position of the spinner
  unsigned int m_SpinnerIndex;
  //! Start of the test suite
  chrono::steady_clock::time_point m_StartTime;
};


////////////////////////////////////////////////////////////////////////////////


//! Discover, schedule, and run UTN* unit test executables in parallel,
//! report live progress on a terminal dashboard, and persist per-test
//! runtimes so the slowest tests are always started first.
class UTNRunner
{
 public:
  //! Default constructor
  UTNRunner() : m_Strict(false) {}
  //! Default destructor
  virtual ~UTNRunner() {}

  //! Run the requested tests; returns 1 if any test fails or crashes, if no tests are found, or with --strict if any test has known failures; 0 otherwise
  int Execute(int argc, char** argv);

 private:
  //! Return the directory that contains argv[0], or working-directory/bin as fallback
  MString GetBinDirectory(const char* Argv0) const;
  //! Return the path to the timing cache file in the platform config directory
  MString GetTimingFile() const;
  //! Resolve a command-line test request (e.g. "Hit", "MHit", "unittests/UTNHit.cxx") to a discovered test name
  MString ResolveRequest(const MString& Input) const;
  //! Scan the bin directory for UTN* executables and populate m_AllTests
  bool DiscoverTests();
  //! Load prior runtimes from the timing cache; an absent or unreadable cache leaves Timings empty
  void LoadTimings(map<MString, double>& Timings) const;
  //! Save the runtimes to the timing cache; failures are ignored since the cache is optional
  void SaveTimings(const map<MString, double>& Timings) const;
  //! Separate the options (--strict) from the test requests; returns false for an unknown option
  bool ParseArguments(int argc, char** argv, vector<MString>& Requests);
  //! Build the ordered list of tests to run from the test requests, or all tests if none given
  bool BuildRequestedTests(const vector<MString>& Requests, vector<MString>& RequestedTests) const;
  //! Reorder tests so the slowest prior runs come first, maximising CPU utilisation
  void SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const;

  //! Time between two checks for finished tests in microseconds
  static constexpr unsigned int c_PollingInterval = 200000;

  //! The bin directory containing the test executables
  MString m_BinDirectory;
  //! All discovered unit tests keyed by name, valued by full path
  map<MString, MString> m_AllTests;
  //! Full path to the timing cache file
  MString m_TimingFile;
  //! If true (--strict), known failures make the run fail
  bool m_Strict;
};


////////////////////////////////////////////////////////////////////////////////


MString UTNRunner::GetBinDirectory(const char* Argv0) const
{
  // Primary: if argv[0] contains a path separator, its directory is the bin directory
  MString Executable = Argv0 != nullptr ? Argv0 : "";
  if (Executable.Contains("/") == true || Executable.Contains("\\") == true) {
    MString Dir = MFile::GetDirectoryName(Executable);
    if (Dir.IsEmpty() == false) {
      return Dir;
    }
  }

  // Secondary: $MEGALIB/bin is always correct in a configured MEGAlib environment
  const char* MEGAlibEnv = getenv("MEGALIB");
  if (MEGAlibEnv != nullptr && MEGAlibEnv[0] != '\0') {
    return MString(MEGAlibEnv) + "/bin";
  }

  // Fallback: assume the bin directory is relative to the working directory
  return MFile::GetWorkingDirectory() + "/bin";
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunner::GetTimingFile() const
{
  const char* Home = getenv("HOME");

  MString BaseDir;
#ifdef __APPLE__
  if (Home != nullptr && Home[0] != '\0') {
    BaseDir = MString(Home) + "/Library/Application Support";
  } else {
    BaseDir = "/tmp";
  }
#else
  const char* XdgConfigHome = getenv("XDG_CONFIG_HOME");
  if (XdgConfigHome != nullptr && XdgConfigHome[0] != '\0') {
    BaseDir = XdgConfigHome;
  } else if (Home != nullptr && Home[0] != '\0') {
    BaseDir = MString(Home) + "/.config";
  } else {
    BaseDir = "/tmp";
  }
#endif

  MString ConfigDir = BaseDir + "/MEGAlib";
  if (MFile::CreateDirectory(ConfigDir) == false) {
    return "";
  }

  return ConfigDir + "/UTNRunner.timings";
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunner::ResolveRequest(const MString& Input) const
{
  // Accept paths and source file names, e.g. "unittests/UTNHit.cxx"
  const MString Extension = ".cxx";
  MString Name = MFile::GetBaseName(Input);
  if (Name.EndsWith(Extension) == true) {
    Name.RemoveLast(Extension.Length());
  }

  vector<MString> Candidates;

  if (Name.BeginsWith("UTN") == true) {
    Candidates.push_back(Name);
  } else {
    Candidates.push_back("UTN" + Name);
    if (Name.BeginsWith("M") == true && Name.Length() > 1) {
      Candidates.push_back("UTN" + Name.GetSubString(1));
    }
  }

  for (const MString& Candidate : Candidates) {
    if (m_AllTests.find(Candidate) != m_AllTests.end()) {
      return Candidate;
    }
  }

  if (Candidates.empty() == false) {
    return Candidates.front();
  }

  return Name;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNRunner::DiscoverTests()
{
  m_AllTests.clear();

  DIR* Directory = opendir(m_BinDirectory.Data());
  if (Directory == nullptr) {
    return false;
  }

  dirent* Entry = nullptr;
  while ((Entry = readdir(Directory)) != nullptr) {
    MString Name = Entry->d_name;
    if (Name.BeginsWith("UTN") == false) {
      continue;
    }
    if (Name == "UTNRunner") {
      continue;
    }

    MString Path = m_BinDirectory + "/" + Name;
    if (MFile::IsExecutable(Path) == false) {
      continue;
    }

    m_AllTests[Name] = Path;
  }

  closedir(Directory);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunner::LoadTimings(map<MString, double>& Timings) const
{
  Timings.clear();

  if (m_TimingFile == "") {
    return;
  }

  ifstream Input(m_TimingFile.Data());
  if (Input.is_open() == false) {
    return;
  }

  MString Line;
  while (Line.ReadLine(Input).fail() == false) {
    if (Line.IsEmpty() == true || Line.BeginsWith("#") == true) {
      continue;
    }
    vector<MString> Tokens = Line.Tokenize(" ");
    if (Tokens.size() < 2 || Tokens[1].IsNumber() == false) {
      continue;
    }
    double Time = Tokens[1].ToDouble();
    if (Time < 0.0) {
      continue;
    }
    Timings[Tokens[0]] = Time;
  }
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunner::SaveTimings(const map<MString, double>& Timings) const
{
  if (m_TimingFile == "") {
    return;
  }

  ofstream Output(m_TimingFile.Data(), ios::trunc);
  if (Output.is_open() == false) {
    return;
  }

  Output << "# UTNRunner timing cache" << '\n';
  Output << "# test_name seconds" << '\n';
  for (const auto& Entry : Timings) {
    Output << Entry.first.Data() << ' ' << Entry.second << '\n';
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTNRunner::ParseArguments(int argc, char** argv, vector<MString>& Requests)
{
  Requests.clear();
  m_Strict = false;

  for (int i = 1; i < argc; ++i) {
    MString Argument = argv[i];
    if (Argument == "--strict") {
      m_Strict = true;
    } else if (Argument.BeginsWith("--") == true) {
      merr<<"Unknown option: "<<Argument<<" (known options: --strict)"<<show;
      return false;
    } else {
      Requests.push_back(Argument);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNRunner::BuildRequestedTests(const vector<MString>& Requests, vector<MString>& RequestedTests) const
{
  RequestedTests.clear();

  if (Requests.empty() == true) {
    for (const auto& Entry : m_AllTests) {
      RequestedTests.push_back(Entry.first);
    }
    return true;
  }

  set<MString> Seen;
  for (const MString& Request : Requests) {
    MString Requested = ResolveRequest(Request);
    if (Seen.insert(Requested).second == false) {
      continue;
    }

    if (m_AllTests.find(Requested) == m_AllTests.end()) {
      merr<<"Unknown unit test request: "<<Request<<" -> "<<Requested<<show;
      return false;
    }

    RequestedTests.push_back(Requested);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunner::SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const
{
  stable_sort(RequestedTests.begin(), RequestedTests.end(), [&](const MString& A, const MString& B) {
    map<MString, double>::const_iterator Ta = Timings.find(A);
    map<MString, double>::const_iterator Tb = Timings.find(B);
    double TimeA = Ta != Timings.end() ? Ta->second : -1.0;
    double TimeB = Tb != Timings.end() ? Tb->second : -1.0;
    if (TimeA != TimeB) {
      return TimeA > TimeB;
    }
    return A < B;
  });
}


////////////////////////////////////////////////////////////////////////////////


bool UTNRunnerSingleRun::Launch()
{
  // mkstemp() replaces the X in a writable buffer
  const char* TmpDir = getenv("TMPDIR");
  const MString TemplateName = MString((TmpDir != nullptr && TmpDir[0] != '\0') ? TmpDir : "/tmp") + "/UTNRunner_XXXXXX";
  vector<char> Template(TemplateName.Data(), TemplateName.Data() + TemplateName.Length() + 1);
  int FD = mkstemp(Template.data());
  if (FD < 0) {
    MarkCrashed("failed to launch test process");
    return false;
  }
  close(FD);
  m_OutputFile = Template.data();

  pid_t Pid = fork();
  if (Pid < 0) {
    unlink(m_OutputFile.Data());
    m_OutputFile = "";
    MarkCrashed("failed to launch test process");
    return false;
  }

  if (Pid == 0) {
    int Out = open(m_OutputFile.Data(), O_WRONLY | O_TRUNC);
    if (Out < 0) {
      _exit(c_ExitCodeExecFailed);
    }
    dup2(Out, STDOUT_FILENO);
    dup2(Out, STDERR_FILENO);
    close(Out);

    execl(m_Executable.Data(), m_Executable.Data(), (char*) nullptr);
    _exit(c_ExitCodeExecFailed);
  }

  m_Pid = Pid;
  m_Status = c_StatusRunning;
  m_StartTime = chrono::steady_clock::now();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunnerSingleRun::Finish(int WaitStatus)
{
  m_Runtime = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - m_StartTime).count();

  m_Parser.Parse(ReadOutput());
  unlink(m_OutputFile.Data());
  m_OutputFile = "";

  Classify(WaitStatus);
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunnerSingleRun::MarkCrashed(const MString& Detail)
{
  m_Status = c_StatusCrashed;
  m_Detail = Detail;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerSingleRun::ReadOutput() const
{
  ifstream Input(m_OutputFile.Data());
  if (Input.is_open() == false) {
    return "";
  }

  MString Output;
  Output.Read(Input);
  return Output;
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunnerTestParser::Parse(const MString& Output)
{
  m_HasSummary = false;
  m_NumberOfPassedChecks = 0;
  m_NumberOfFailedChecks = 0;
  m_NumberOfKnownFailures = 0;
  m_NumberOfUnexpectedPasses = 0;
  m_IsAborted = false;
  m_FailureReports.clear();
  m_LastLines.clear();

  vector<MString> Lines = Output.Tokenize("\n", false);

  // A report is its first line plus the following detail lines, which MUnitTest indents by three blanks
  for (size_t i = 0; i < Lines.size(); ++i) {
    if (Lines[i].BeginsWith("FAILED:") == true || Lines[i].BeginsWith("UNEXPECTED PASS:") == true || Lines[i].BeginsWith("ABORTED:") == true) {
      MString Report = Lines[i];
      while (i + 1 < Lines.size() && Lines[i + 1].BeginsWith("   ") == true) {
        Report += MString("\n") + Lines[++i];
      }
      m_FailureReports.push_back(Report);
    }
  }

  size_t First = Lines.size() > c_NumberOfLastLines ? Lines.size() - c_NumberOfLastLines : 0;
  m_LastLines.assign(Lines.begin() + First, Lines.end());

  // Only the block printed by the last Summarize() counts, i.e. from the last line starting with "Unit test: "
  size_t Block = Lines.size();
  for (size_t i = 0; i < Lines.size(); ++i) {
    if (Lines[i].BeginsWith("Unit test: ") == true) {
      Block = i;
    }
  }

  long Passed = -1;
  long Failed = -1;
  long Known = -1;
  if (Block < Lines.size()) {
    Passed = ParseSummaryValue(Lines, Block, "Passed tests:");
    Failed = ParseSummaryValue(Lines, Block, "Failed tests:");
    // Summarize() prints the known failures only if there are any
    Known = ParseSummaryValue(Lines, Block, "Known failures:");
  }
  if (Passed >= 0 && Failed >= 0) {
    m_HasSummary = true;
    m_NumberOfPassedChecks = Passed;
    m_NumberOfFailedChecks = Failed;
  }
  m_NumberOfKnownFailures = Known > 0 ? Known : 0;

  m_NumberOfUnexpectedPasses = CountLinesStartingWith(Lines, "UNEXPECTED PASS:");

  // MUnitTest aborts before any check, thus a run with a summary was not aborted
  m_IsAborted = (m_HasSummary == false && CountLinesStartingWith(Lines, "ABORTED: HOME and the working directory") > 0);
}


////////////////////////////////////////////////////////////////////////////////


long UTNRunnerTestParser::ParseSummaryValue(const vector<MString>& Lines, size_t Start, const MString& Prefix) const
{
  for (size_t i = Start; i < Lines.size(); ++i) {
    if (Lines[i].BeginsWith(Prefix) == true) {
      MString Value = Lines[i].GetSubString(Prefix.Length());
      return Value.IsNumber() == true ? Value.ToLong() : -1;
    }
  }
  return -1;
}


////////////////////////////////////////////////////////////////////////////////


long UTNRunnerTestParser::CountLinesStartingWith(const vector<MString>& Lines, const MString& Prefix) const
{
  long Count = 0;
  for (const MString& Line : Lines) {
    if (Line.BeginsWith(Prefix) == true) {
      ++Count;
    }
  }
  return Count;
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunnerSingleRun::Classify(int WaitStatus)
{
  m_Detail = "";

  if (WIFSIGNALED(WaitStatus) != 0) {
    m_Status = c_StatusCrashed;
    m_Detail = MString("signal ") + WTERMSIG(WaitStatus);
    if (m_Parser.HasSummary() == false) m_Detail += ", no summary";
    return;
  }
  if (WIFEXITED(WaitStatus) == 0) {
    m_Status = c_StatusCrashed;
    m_Detail = "abnormal termination";
    return;
  }

  int ExitCode = WEXITSTATUS(WaitStatus);
  if (m_Parser.IsAborted() == true) {
    m_Status = c_StatusFailed;
    m_Detail = "aborted: HOME and the working directory could not be isolated, no check was run";
    return;
  }
  if (m_Parser.HasSummary() == false) {
    m_Status = c_StatusCrashed;
    m_Detail = MString("exit code ") + ExitCode + ", no summary";
    return;
  }

  if (ExitCode == 0) {
    if (m_Parser.GetNumberOfFailedChecks() > 0) {
      // Usually a missing "&& Passed" in the test
      m_Status = c_StatusFailed;
      m_Detail = "exit code 0 although checks failed";
      return;
    }
    m_Status = m_Parser.GetNumberOfKnownFailures() > 0 ? c_StatusKnown : c_StatusPassed;
    return;
  }

  m_Status = c_StatusFailed;
  if (m_Parser.GetNumberOfFailedChecks() == 0) {
    m_Detail = MString("exit code ") + ExitCode + " although no check failed";
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerSingleRun::FormatCounts() const
{
  if (m_Parser.HasSummary() == false) {
    return "";
  }

  long Unexpected = m_Parser.GetNumberOfUnexpectedPasses();
  long Known = m_Parser.GetNumberOfKnownFailures();

  MString Text = MString(m_Parser.GetNumberOfPassedChecks()) + " passed";
  if (m_Parser.GetNumberOfFailedChecks() > 0) {
    Text += MString(", ") + m_Parser.GetNumberOfFailedChecks() + " failed";
    if (Unexpected > 0) {
      Text += MString(": ") + Unexpected + (Unexpected == 1 ? " unexpected pass" : " unexpected passes") + " - remove ExpectedToFail()";
    }
  }
  if (Known > 0) {
    Text += MString(", ") + Known + (Known == 1 ? " known failure" : " known failures");
  }

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerSingleRun::GetStatusLabel() const
{
  if (m_Status == c_StatusPassed) return "PASS ";
  if (m_Status == c_StatusKnown) return "KNOWN";
  if (m_Status == c_StatusFailed) return "FAIL ";
  if (m_Status == c_StatusCrashed) return "CRASH";
  return "     ";
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerSingleRun::GetDescription() const
{
  MString Text = FormatCounts();
  if (m_Detail.Length() > 0) {
    Text += (Text.Length() > 0 ? MString("; ") : MString("")) + m_Detail;
  }
  return Text.Length() > 0 ? m_Name + " (" + Text + ")" : m_Name;
}


////////////////////////////////////////////////////////////////////////////////


UTNRunnerDashboard::UTNRunnerDashboard(unsigned int MaxParallel)
{
  m_IsEnabled = (isatty(STDOUT_FILENO) != 0);
  m_MaxParallel = MaxParallel;
  m_SpinnerIndex = 0;
  m_StartTime = chrono::steady_clock::now();

  m_TerminalWidth = c_DefaultTerminalWidth;
  if (m_IsEnabled == true) {
    winsize WinSize = {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &WinSize) == 0 && WinSize.ws_col > 0) {
      m_TerminalWidth = WinSize.ws_col;
    }
    cout << "\x1b[?25l" << flush;
  }
  if (m_TerminalWidth < c_MinimumTerminalWidth) {
    m_TerminalWidth = c_MinimumTerminalWidth;
  }
}


////////////////////////////////////////////////////////////////////////////////


UTNRunnerDashboard::~UTNRunnerDashboard()
{
  if (m_IsEnabled == true) {
    cout << "\x1b[?25h" << flush;
  }
}


////////////////////////////////////////////////////////////////////////////////


void UTNRunnerDashboard::Render(const vector<UTNRunnerSingleRun>& Runs) const
{
  if (m_IsEnabled == false) {
    return;
  }

  size_t NumberOfTests = Runs.size();
  size_t NumberOfDoneTests = 0;
  size_t NumberOfPassedTests = 0;
  vector<MString> RunningTests;
  vector<MString> FailedRuns;
  vector<MString> KnownRuns;
  for (const UTNRunnerSingleRun& Run : Runs) {
    if (Run.IsDone() == true) {
      ++NumberOfDoneTests;
    }
    if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusPassed) {
      ++NumberOfPassedTests;
    } else if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusRunning) {
      RunningTests.push_back(Run.GetName());
    } else if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusFailed || Run.GetStatus() == UTNRunnerSingleRun::c_StatusCrashed) {
      FailedRuns.push_back(Run.GetStatusLabel() + " " + Run.GetDescription());
    } else if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusKnown) {
      KnownRuns.push_back(Run.GetDescription());
    }
  }
  double ElapsedSeconds = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - m_StartTime).count();

  const MString SpinnerChars = "|/-\\";

  vector<UTNRunnerDashboardLine> LeftLines;
  LeftLines.emplace_back("Status:", true);
  LeftLines.push_back(MString("Done: ") + NumberOfDoneTests + "/" + NumberOfTests);
  LeftLines.push_back(MString("Running: ") + RunningTests.size());
  LeftLines.push_back(MString("Passed: ") + NumberOfPassedTests);
  LeftLines.push_back(MString("Failed: ") + FailedRuns.size());
  LeftLines.push_back(MString("Only known failures: ") + KnownRuns.size());
  LeftLines.emplace_back("");
  LeftLines.emplace_back("Runs with failures:", true);
  if (FailedRuns.empty() == true) {
    LeftLines.emplace_back("none");
  }
  LeftLines.insert(LeftLines.end(), FailedRuns.begin(), FailedRuns.end());
  LeftLines.emplace_back("");
  LeftLines.emplace_back("Runs with known failures only:", true);
  if (KnownRuns.empty() == true) {
    LeftLines.emplace_back("none");
  }
  LeftLines.insert(LeftLines.end(), KnownRuns.begin(), KnownRuns.end());

  vector<UTNRunnerDashboardLine> RightLines;
  RightLines.emplace_back("Running tests:", true);
  for (const MString& Test : RunningTests) {
    RightLines.push_back(SpinnerChars.GetSubString(m_SpinnerIndex % SpinnerChars.Length(), 1) + " " + Test);
  }
  if (RunningTests.empty() == true) {
    RightLines.emplace_back("none");
  }
  while (RightLines.size() < m_MaxParallel + 1) {
    RightLines.emplace_back("");
  }

  size_t Filled = NumberOfTests == 0 ? c_ProgressBarWidth : (NumberOfDoneTests * c_ProgressBarWidth) / NumberOfTests;
  MString Bar = "[";
  for (size_t i = 0; i < c_ProgressBarWidth; ++i) {
    Bar += (i < Filled ? "#" : ".");
  }
  Bar += "]";
  MString Progress = MString("Progress: ") + Bar + MString("  ") + NumberOfDoneTests + "/" + NumberOfTests + MString("  ") + FormatRuntime(ElapsedSeconds);

  // Both panes have the same width: wide enough for the longest line, but not wider than the terminal
  size_t PaneWidth = c_MinimumPaneWidth;
  for (const UTNRunnerDashboardLine& Line : LeftLines) {
    PaneWidth = max(PaneWidth, (size_t) Line.GetText().Length());
  }
  for (const UTNRunnerDashboardLine& Line : RightLines) {
    PaneWidth = max(PaneWidth, (size_t) Line.GetText().Length());
  }
  size_t MaxInnerWidth = m_TerminalWidth - c_OuterBorderWidth;
  if (PaneWidth * 2 + c_InnerFrameWidth > MaxInnerWidth) {
    PaneWidth = (MaxInnerWidth - c_InnerFrameWidth) / 2;
  }
  if (Progress.Length() / 2 > PaneWidth) {
    PaneWidth = Progress.Length() / 2;
  }
  size_t InnerWidth = PaneWidth * 2 + c_InnerFrameWidth;

  MString Border = MString("+") + MString(string(InnerWidth, '-')) + "+";
  size_t MaxLines = max(LeftLines.size(), RightLines.size());

  cout << "\x1b[2J\x1b[H";
  cout << Border << '\n';
  cout << "|" << "\x1b[1m" << Center("Nuclearizer unit & end-to-end testing dashboard", InnerWidth) << "\x1b[0m" << "|" << '\n';
  cout << Border << '\n';
  for (size_t i = 0; i < MaxLines; ++i) {
    UTNRunnerDashboardLine Left = i < LeftLines.size() ? LeftLines[i] : UTNRunnerDashboardLine("");
    UTNRunnerDashboardLine Right = i < RightLines.size() ? RightLines[i] : UTNRunnerDashboardLine("");
    cout << "| " << Pad(Clip(Left.GetText(), PaneWidth), PaneWidth, Left.IsHeading()) << " | " << Pad(Clip(Right.GetText(), PaneWidth), PaneWidth, Right.IsHeading()) << " |" << '\n';
  }
  cout << Border << '\n';
  cout << "|" << Center(Progress, InnerWidth) << "|" << '\n';
  cout << Border << '\n';
  cout.flush();
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerDashboard::Clip(const MString& Text, size_t Width) const
{
  if (Text.Length() <= Width) {
    return Text;
  }
  const MString Ellipsis = "...";
  if (Width <= Ellipsis.Length()) {
    return Text.GetSubString(0, Width);
  }
  return Text.GetSubString(0, Width - Ellipsis.Length()) + Ellipsis;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerDashboard::Center(const MString& Text, size_t Width) const
{
  MString Clipped = Clip(Text, Width);
  if (Clipped.Length() >= Width) {
    return Clipped;
  }
  size_t PadLeft = (Width - Clipped.Length()) / 2;
  size_t PadRight = Width - Clipped.Length() - PadLeft;
  return MString(string(PadLeft, ' ')) + Clipped + MString(string(PadRight, ' '));
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerDashboard::Pad(const MString& Text, size_t Width, bool Bold) const
{
  MString Padded = Text;
  if (Text.Length() < Width) {
    Padded += MString(string(Width - Text.Length(), ' '));
  }
  return Bold == true ? MString("\x1b[1m") + Padded + "\x1b[0m" : Padded;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNRunnerDashboard::FormatRuntime(double Seconds) const
{
  ostringstream Out;
  Out.setf(ios::fixed);
  Out.precision(1);
  Out << Seconds << "s";
  return Out.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


int UTNRunner::Execute(int argc, char** argv)
{
  vector<MString> Requests;
  if (ParseArguments(argc, argv, Requests) == false) {
    return 1;
  }

  m_BinDirectory = GetBinDirectory(argc > 0 ? argv[0] : nullptr);
  m_TimingFile = GetTimingFile();
  if (DiscoverTests() == false || m_AllTests.empty() == true) {
    merr<<"No unit test executables found in "<<m_BinDirectory<<show;
    return 1;
  }

  map<MString, double> Timings;
  LoadTimings(Timings);

  vector<MString> RequestedTests;
  if (BuildRequestedTests(Requests, RequestedTests) == false) {
    return 1;
  }
  SortRequestedTests(RequestedTests, Timings);

  // One run per requested test
  vector<UTNRunnerSingleRun> Runs;
  for (const MString& Test : RequestedTests) {
    Runs.push_back(UTNRunnerSingleRun(Test, m_AllTests[Test]));
  }

  // Detect the CPU count before the dashboard is created, so its panel height is correct from the start
  unsigned int MaxParallel = std::thread::hardware_concurrency();
  if (MaxParallel == 0) MaxParallel = 1;

  UTNRunnerDashboard Dashboard(MaxParallel);
  Dashboard.Render(Runs);

  size_t Next = 0;
  size_t Running = 0;
  while (Next < Runs.size() || Running > 0) {
    while (Next < Runs.size() && Running < MaxParallel) {
      if (Runs[Next].Launch() == true) {
        ++Running;
      }
      ++Next;
      Dashboard.Render(Runs);
    }

    int WaitStatus = 0;
    pid_t Child = waitpid(-1, &WaitStatus, WNOHANG);
    if (Child < 0) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
    if (Child == 0) {
      Dashboard.AdvanceSpinner();
      Dashboard.Render(Runs);
      usleep(c_PollingInterval);
      continue;
    }

    for (UTNRunnerSingleRun& Run : Runs) {
      if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusRunning && Run.GetPid() == Child) {
        Run.Finish(WaitStatus);
        Timings[Run.GetName()] = Run.GetRuntime();
        --Running;
        Dashboard.Render(Runs);
        break;
      }
    }
  }

  // A test which never finished (waitpid error) counts as crashed
  for (UTNRunnerSingleRun& Run : Runs) {
    if (Run.IsDone() == false) {
      Run.MarkCrashed("did not finish");
    }
  }

  unsigned int NPassed = 0;
  unsigned int NKnown = 0;
  unsigned int NFailed = 0;
  unsigned int NCrashed = 0;
  long ChecksPassed = 0;
  long ChecksKnown = 0;
  long ChecksFailed = 0;
  long ChecksUnexpected = 0;
  for (const UTNRunnerSingleRun& Run : Runs) {
    if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusPassed) ++NPassed;
    else if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusKnown) ++NKnown;
    else if (Run.GetStatus() == UTNRunnerSingleRun::c_StatusFailed) ++NFailed;
    else ++NCrashed;
    ChecksPassed += Run.GetParser().GetNumberOfPassedChecks();
    ChecksKnown += Run.GetParser().GetNumberOfKnownFailures();
    ChecksFailed += Run.GetParser().GetNumberOfFailedChecks();
    ChecksUnexpected += Run.GetParser().GetNumberOfUnexpectedPasses();
  }

  if (Dashboard.IsEnabled() == false) {
    for (const UTNRunnerSingleRun& Run : Runs) {
      cout << Run.GetStatusLabel() << " " << Run.GetDescription() << '\n';
    }
    cout << '\n';
  } else {
    Dashboard.Render(Runs);
    cout << '\n';
  }

  // Show why runs failed, e.g. in a CI log: the reports of failed checks, or the last lines of the output if there are none
  for (const UTNRunnerSingleRun& Run : Runs) {
    if (Run.GetStatus() != UTNRunnerSingleRun::c_StatusFailed && Run.GetStatus() != UTNRunnerSingleRun::c_StatusCrashed) {
      continue;
    }
    cout << "===== " << Run.GetStatusLabel() << " " << Run.GetDescription() << '\n';
    const vector<MString>& Reports = Run.GetParser().GetFailureReports();
    const vector<MString>& Details = Reports.empty() == false ? Reports : Run.GetParser().GetLastLines();
    for (const MString& Detail : Details) {
      cout << Detail << '\n';
    }
    cout << '\n';
  }

  cout << "Executables: " << NPassed << " passed, " << NKnown << " with known failures, " << NFailed << " failed, " << NCrashed << " crashed" << '\n';
  cout << "Checks: " << ChecksPassed << " passed, " << ChecksKnown << " known failures, " << ChecksFailed << " failed";
  if (ChecksUnexpected > 0) {
    cout << " (" << ChecksUnexpected << (ChecksUnexpected == 1 ? " unexpected pass" : " unexpected passes") << ")";
  }
  cout << '\n';
  if (m_Strict == true && NKnown > 0) {
    cout << "Strict mode: known failures fail the run" << '\n';
  }

  SaveTimings(Timings);

  bool Success = (NFailed == 0 && NCrashed == 0) && (m_Strict == false || NKnown == 0);
  return Success == true ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  UTNRunner Execute;
  return Execute.Execute(argc, argv);
}


// UTNRunner.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
