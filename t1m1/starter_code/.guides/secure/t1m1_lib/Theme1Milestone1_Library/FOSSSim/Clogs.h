#ifndef __CLOGS_H__
#define __CLOGS_H__

#include <iostream>
#include <fstream>
#include <string>
#include <map>

typedef std::map<std::string, std::ostream*>::iterator LogIterator;
namespace clogs {
  // an ostream that essentially ignores all input.
  class NullStream : public std::ostream
  {
    public:
      NullStream() : std::ostream( 0 ) {}
  };

  class LogManager {
    public:
      LogManager() : suppressOutput(false) {};
      virtual ~LogManager() {
        if (suppressOutput) return;
        for (LogIterator it = streams.begin();
            it != streams.end(); ++it) {
          std::cout << "Log saved to '" << it->first << "'" << std::endl;
        }
      };

      std::ostream& getLog(const std::string& key) {
        LogIterator it = streams.find(key);

        if (it == streams.end()) {
          return openLog(key);
        } else {
          return *it->second;
        }
      };

      std::ostream& openLog(const std::string& key) {
          std::ostream* p_stream;
          if (suppressOutput) {
            p_stream = new NullStream();
          } else {
            p_stream = new std::ofstream(key.c_str(), std::ofstream::app);
          }

          streams.insert(std::pair<std::string, std::ostream*>(key, p_stream));

          return *p_stream;
      };

      bool suppressOutput;
    private:
      std::map<std::string, std::ostream*> streams;
  };

  extern std::ostream& clog(const std::string& key);
}

#define CLOGS_OPEN \
static clogs::LogManager CLOG_MANAGER; \
std::ostream& clogs::clog(const std::string& key) { \
  return CLOG_MANAGER.getLog(key); \
}

#define SUPPRESS_ALL_CLOGS() CLOG_MANAGER.suppressOutput = true

#endif /* end of include guard: __CLOGS_H__ */
