#include <string.h> // memset
#include <unistd.h> // environ

#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>

#include "serverd/Process.h"

Process::Process(const std::string &name, char **argv) : 
  m_name(name),
  m_argv(argv),
  m_logFile(nullptr){}

Process::~Process(){}

void Process::set_title(){
  // 将环境变量复制到新创建的空间
  char *oldEnv = environ[0];
  size_t len;
  for(int i = 0; environ[i]; ++ i){
    m_env.push_back(std::string(environ[i]));

    len = strlen(environ[i]) + 1;
    memset(environ[i], 0, len);
    environ[i] = const_cast<char*>(m_env.back().c_str());
  }

  // 清除命令行参数
  len = 0;
  for(int i = 0; m_argv[i]; ++ i){
      len += strlen(m_argv[i]) + 1;
  }

  memset(m_argv[0], 0, len);
  m_argv[1] = nullptr;

  // 拷贝标题
  strncpy(m_argv[0], m_name.c_str(), m_name.size()); 
}

void Process::set_mask(int signal){
  m_mask.add(signal);
  m_mask.update();
}

void Process::set_mask(const std::initializer_list<int> &signals){
  for(const int sig : signals){
    m_mask.add(sig);
  }

  m_mask.update();
}

void Process::clear_mask(){
  m_mask.clear();
  m_mask.update();
}

void Process::set_log(){
  ConfigFile &cf = Singleton<ConfigFile>::instance();
  std::string logFilePath = cf.get<std::string>("LogFilePath");
  std::string logLevel = cf.get<std::string>("LogLevel");
  int logFileSize = cf.get<int>("LogFileSize");
  m_logFile.reset(new LogFile(logFilePath.append(get_name()), logFileSize)); // 默认的flushInterval = 3s, checkEveryN = 1024
  Log::set_output(std::bind(&LogFile::append, m_logFile.get(), std::placeholders::_1, std::placeholders::_2));
  Log::set_flush(std::bind(&LogFile::flush, m_logFile.get()));
  Log::set_level(logLevel);
}