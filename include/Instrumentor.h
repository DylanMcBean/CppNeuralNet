#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_map>

#define PROFILING 1
#if PROFILING
#ifdef _MSC_VER  // Miscrosoft Compiler
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#define COUNT_FUNCTION() InstrumentationCounter counter##__LINE__(__FUNCTION__)
#define TIME_FUNCTION() \
  InstrumentationTimerAccumulator timerAccumulator##__LINE__(__FUNCTION__)
#else  // Clang,  GCC
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#define COUNT_FUNCTION() \
  InstrumentationCounter counter##__LINE__(__PRETTY_FUNCTION__)
#define TIME_FUNCTION()                                       \
  InstrumentationTimerAccumulator timerAccumulator##__LINE__( \
      __PRETTY_FUNCTION__)
#endif
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#define COUNT_FUNCTION()
#define TIME_FUNCTION()
#endif

struct ProfileResult {
  std::string Name;
  long long Start, End;
  uint32_t ThreadID;
};

struct InstrumentCount {
  long long Start;
  long long Count;
};

struct InstrumentTime {
  long long Start;
  long long TotalTime;
};

struct InstrumentationSession {
  std::string Name;
};

class Instrumentor {
 private:
  std::unique_ptr<InstrumentationSession> m_CurrentSession;
  std::ofstream m_OutputStream;
  int m_ProfileCount;
  std::unordered_map<std::string, InstrumentCount> m_FunctionCounts;
  std::unordered_map<std::string, InstrumentTime> m_FunctionTimes;
  std::mutex m_Mutex;
  std::vector<ProfileResult> m_ProfileCache;

 public:
  Instrumentor();
  ~Instrumentor();

  void BeginSession(const std::string& name,
                    const std::string& filepath = "results.json");
  void EndSession();
  void WriteProfile(const ProfileResult& result, bool force_write = false);
  static Instrumentor& Get();
  void IncrementFunctionCount(const std::string& name);
  void AddFunctionTime(const std::string& name, long long time);
  void AddFunctionTime(const std::string& name);

 private:
  void WriteHeader();
  void WriteFooter();
  void WriteCacheToFile();
  void AddProfileToCache(const ProfileResult& result);
};

class InstrumentationTimer {
 public:
  InstrumentationTimer(const char* name);
  ~InstrumentationTimer();

  void Stop();

 private:
  const char* m_Name;
  std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
  bool m_Stopped;
};

class InstrumentationCounter {
 public:
  InstrumentationCounter(const char* name) : m_Name(name) {
    Instrumentor::Get().IncrementFunctionCount(m_Name);
  }
  ~InstrumentationCounter() {}

 private:
  const char* m_Name;
};

class InstrumentationTimerAccumulator {
 public:
  InstrumentationTimerAccumulator(const char* name);

 private:
  const char* m_Name;
};