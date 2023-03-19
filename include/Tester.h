#pragma once

#include <VEmu.h>

struct TestCase
{
    std::string bin_file_name;
    bool skip = false;

    TestCase(std::string fname)
      : bin_file_name(std::move(fname)){};

    TestCase(std::string fname, bool to_skip)
      : bin_file_name(std::move(fname))
      , skip(to_skip)
    {
    }
};

class Tester
{
  public:
    static void run();

  private:
    const static std::vector<TestCase> test_cases;
};
