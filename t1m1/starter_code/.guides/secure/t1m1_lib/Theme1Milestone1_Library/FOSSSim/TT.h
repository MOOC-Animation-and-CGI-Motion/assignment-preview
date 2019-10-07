#ifndef __TT_H__
#define __TT_H__

#include <iostream>
#include <string>
#include <sstream>

#define BOLDRED(stuff) BOLD(RED(stuff))
#define BOLD(stuff) "\e[1m" << stuff << "\e[0m"
#define UNDERLINE(stuff) "\e[4m" << stuff << "\e[0m"

#ifdef NOCOLOR
  #define RED(stuff) stuff
  #define BLUE(stuff) stuff
  #define GREEN(stuff) stuff
#else
  #define RED(stuff) "\e[31m" << stuff << "\e[0m"
  #define BLUE(stuff) "\e[34m" << stuff << "\e[0m"
  #define GREEN(stuff) "\e[32m" << stuff << "\e[0m"
#endif

#define TEST_NAME_FORMAT(stuff) BOLD(BLUE(stuff))
#define SUCCESS_FORMAT(stuff) BOLD(GREEN(stuff))
#define FAILURE_FORMAT(stuff) BOLD(RED(stuff))
using namespace std;

typedef void (*TestFunction)(bool& result, stringstream& out);

namespace tt {
  class Test;
}

#define MAX_TEST_COUNT 200

// tried a vector, had problems with it. simpler this way, anyways.
extern tt::Test* TESTS[MAX_TEST_COUNT];
extern int TEST_COUNT;

namespace tt {
  class Test {
    public:
      Test(const char* _name, TestFunction _test) : 
        name(_name), test(_test) 
        {};

      static int main() {
        cout << BOLD("tt") << ": Running " << TEST_COUNT << " tests..." << endl;
        cout << endl;

        int failures = 0;
        for(int i = 0; i < TEST_COUNT; i++)
        {
          if (!TESTS[i]->Run()) {
            failures++;
          }
        }

        cout << endl;
        cout << "Finished running all tests: " << BOLD(GREEN((TEST_COUNT - failures) << " / " << TEST_COUNT)) << " succeeded." << endl;
        return failures;
      };

      bool Run() {
#ifdef SHOW_FIRST_LINE
        cout << "       Running " << TEST_NAME_FORMAT(name) << "..." << endl;
#endif
        stringstream output;
        bool success = true;
        test(success, output);
        if (success) {
          cout << SUCCESS_FORMAT(" [ OK ] ") 
               << TEST_NAME_FORMAT(name) << " passed." 
               << endl;
        } else {
          cout << FAILURE_FORMAT(" [FAIL] ") 
               << TEST_NAME_FORMAT(name) << " failed: " 
               << output.str() << endl;
        }
        return success;
      };
    private:
      string name;
      TestFunction test;
  };
}

#define REGISTER_TEST(name) \
  int register_##name() {                   \
    TESTS[TEST_COUNT] = new tt::Test(#name, &name);   \
    TEST_COUNT++; \
    return 0; \
  }                                         \
  int magic_##name = register_##name();

#define TEST(name) \
  void name(bool& result, stringstream& output); \
  REGISTER_TEST(name); \
  void name(bool& result, stringstream& output)

#define ASSERT(condition, msg) \
  do { \
  if (!(condition)) { \
    output << msg; \
    result = false; \
  } \
  } while(0)

#define ASSERT_UNEQUAL(l, r, msg) \
  ASSERT((l) != (r), (l) << " == " << (r) << ". " << msg)

#define ASSERT_EQUAL(l, r, msg) \
  ASSERT((l) == (r), (l) << " != " << (r) << ". " << msg)

#define DEFINE_TESTS tt::Test* TESTS[MAX_TEST_COUNT]; \
  int TEST_COUNT = 0

#endif /* end of include guard: __TT_H__ */
