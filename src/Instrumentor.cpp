#include "Instrumentor.h"

#include <iostream>
#include <mutex>

std::mutex mtx;

Instrumentor::Instrumentor() : m_CurrentSession(nullptr), m_ProfileCount(0){};

Instrumentor::~Instrumentor() { EndSession(); }

void Instrumentor::BeginSession(const std::string& name,
                                const std::string& filepath) {
  m_OutputStream.open(filepath);
  if (!m_OutputStream.is_open()) {
    std::cerr << "Failed to open the file: " << filepath << std::endl;
    return;
  }
  m_CurrentSession = std::make_unique<InstrumentationSession>();
  m_CurrentSession->Name = name;
  WriteHeader();
}

void Instrumentor::EndSession() {
  WriteCacheToFile();
  WriteFooter();
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_OutputStream.close();
  m_CurrentSession = nullptr;
  m_ProfileCount = 0;
  m_FunctionCounts.clear();
}

void Instrumentor::WriteProfile(const ProfileResult& result, bool force_write) {
  if (force_write) {
    AddProfileToCache(result);
    WriteCacheToFile();
  } else {
    AddProfileToCache(result);
  }
}

void Instrumentor::WriteHeader() {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_OutputStream << "{\"session\": \"" << m_CurrentSession->Name << "\",";
  m_OutputStream << "\"traceEvents\": [";
  m_OutputStream.flush();
}

void Instrumentor::WriteFooter() {
  std::lock_guard<std::mutex> lock(m_Mutex);

  for (auto it = m_FunctionCounts.begin(); it != m_FunctionCounts.end(); ++it) {
    if (m_ProfileCount++ > 0) m_OutputStream << ",";
    m_OutputStream << "{";
    m_OutputStream << "\"cat\": \"Function Call Count\",";
    m_OutputStream << "\"args\":{\"Count\":" << it->second.Count << "},";
    m_OutputStream << "\"name\":\"_c " << it->first << "\",";
    m_OutputStream << "\"ph\": \"C\",";
    m_OutputStream << "\"pid\": 65536,";
    m_OutputStream << "\"tid\": 0,";
    m_OutputStream << "\"ts\":" << it->second.Start;
    m_OutputStream << "}";
  }

  for (auto it = m_FunctionTimes.begin(); it != m_FunctionTimes.end(); ++it) {
    if (m_ProfileCount++ > 0) m_OutputStream << ",";
    m_OutputStream << "{";
    m_OutputStream << "\"cat\": \"Function Total Runtime\",";
    m_OutputStream << "\"dur\":" << it->second.TotalTime << ',';
    m_OutputStream << "\"name\":\"_tr " << it->first << "\",";
    m_OutputStream << "\"ph\": \"X\",";
    m_OutputStream << "\"pid\": 0,";
    m_OutputStream << "\"tid\": 1,";
    m_OutputStream << "\"ts\":" << it->second.Start;
    m_OutputStream << "}";
  }

  m_OutputStream << "]}";
  m_OutputStream.flush();
}

void Instrumentor::AddProfileToCache(const ProfileResult& result) {
  m_ProfileCache.push_back(result);
  if (m_ProfileCache.size() >= 10000) {
    WriteCacheToFile();
  }
}

void Instrumentor::WriteCacheToFile() {
  std::lock_guard<std::mutex> lock(m_Mutex);

  for (const auto& result : m_ProfileCache) {
    if (m_ProfileCount++ > 0) m_OutputStream << ",";

    std::string name = result.Name;
    std::replace(name.begin(), name.end(), '"', '\'');

    m_OutputStream << "{";
    m_OutputStream << "\"cat\": \"Function Runtime\",";
    m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
    m_OutputStream << "\"name\":\"" << name << "\",";
    m_OutputStream << "\"ph\": \"X\",";
    m_OutputStream << "\"pid\": 0,";
    m_OutputStream << "\"tid\":" << result.ThreadID << ",";
    m_OutputStream << "\"ts\":" << result.Start;
    m_OutputStream << "}";

    m_OutputStream.flush();
  }

  m_ProfileCache.clear();
}

Instrumentor& Instrumentor::Get() {
  static Instrumentor instance;
  return instance;
}

void Instrumentor::IncrementFunctionCount(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_FunctionCounts[name].Count++;

  if (m_FunctionCounts[name].Count == 1) {
    m_FunctionCounts[name].Start =
        std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now())
            .time_since_epoch()
            .count();
  }
}

void Instrumentor::AddFunctionTime(const std::string& name, long long time) {
  std::lock_guard<std::mutex> lock(m_Mutex);

  // if time is -1 and not in map, add it and set time to just now
  // else increment time only
  if (time == -1) {
    if (m_FunctionTimes.find(name) == m_FunctionTimes.end()) {
      m_FunctionTimes[name].TotalTime = 0;
      m_FunctionTimes[name].Start =
          std::chrono::time_point_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now())
              .time_since_epoch()
              .count();
    }
  } else {
    m_FunctionTimes[name].TotalTime += time;
  }
}

InstrumentationTimer::InstrumentationTimer(const char* name)
    : m_Name(name), m_Stopped(false) {
  m_StartTimepoint = std::chrono::steady_clock::now();
}

InstrumentationTimer::~InstrumentationTimer() {
  if (!m_Stopped) Stop();
}

void InstrumentationTimer::Stop() {
  auto endTimepoint = std::chrono::steady_clock::now();

  long long start =
      std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint)
          .time_since_epoch()
          .count();

  long long end =
      std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint)
          .time_since_epoch()
          .count();

  uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
  Instrumentor::Get().WriteProfile({m_Name, start, end, threadID});

  Instrumentor::Get().AddFunctionTime(m_Name, end - start);

  m_Stopped = true;
}

InstrumentationTimerAccumulator::InstrumentationTimerAccumulator(
    const char* name)
    : m_Name(name) {
  Instrumentor::Get().AddFunctionTime(m_Name, -1);
}