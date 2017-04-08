#include <iostream>
#include <fstream>
#include <map>
#include <set>
#ifdef WIN32
#include <experimental/filesystem>
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using namespace std;
namespace fs = std::experimental::filesystem;

const char CR = '\r';
const char LF = '\n';
const char SPACE = ' ';
const char TAB = '\t';

bool validate_args(int argc, char *argv[], string& inputDir)
{
  if (argc != 2)
  {
    cout << "Usage: crlfstat <folder>" << endl;
    return false;
  }
  inputDir = argv[1];
  struct stat st = {};
  if (fs::is_directory(inputDir))
  {
    return true;
  }
  cout << "Error! '" << inputDir.c_str() << "' is not a valid directory name." << endl;
  return false;
}

struct Statistics
{
  int totalfiles = 0;
  int binaryfiles = 0;
  set<string> binaryExtensions;
  int lfOnlyFiles = 0;
  int crlfOnlyFiles = 0;
  int mixedEOLs = 0;
  int tabOnlyFiles = 0;
  int spaceOnlyFiles = 0;
  int mixedIndents = 0;
  set<string> mixedEOLsExts;
  set<string> mixedIndentsExts;
};

void iterate_directory(const string& inputDir, Statistics& stat)
{
  for (auto& p: fs::directory_iterator(inputDir))
  {
    if (fs::is_directory(p))
    {
      iterate_directory(p.path().string(), stat);
    }
    else
    {
      ifstream is(p.path().string(), std::ios_base::in | std::ios_base::binary);
      if (!is.good())
      {
        cout << "Error! '" << p.path().string() << "' cannot be opened." << endl;
      }
      else
      {
        bool bin = false;
        const streamsize n = 24;
        char buff[n];
        bool hasLFs = false;
        bool hasSPACEs = false;
        bool hasTABs = false;
        bool hasCRLFs = false;
        char prevc = 0;
        bool isBOF = false;
        while (!is.eof())
        {
          is.read(buff, n);
          streamsize length = is.gcount();
          for (streamsize i = 0; i < length; i++)
          {
            if (i == 0 && prevc == 0 && buff[0] == '\xEF' && buff[1] == '\xBB' && buff[2] == '\xBF')
            {
              // UTF8 BOM found
              i += 2;
              continue;
            }
            if (isBOF)
            {
              if (buff[i] == SPACE)
                hasSPACEs = true;
              else if (buff[i] == TAB)
                hasTABs = true;
              else
                isBOF = false;
            }
            if (buff[i] < 9 || (buff[i] > 13 && buff[i] < 32) || buff[i] > 126)
            {
              bin = true;
              break;
            }
            if (buff[i] == LF)
            {
              if (prevc == CR)
                hasCRLFs = true;
              else
                hasLFs = true;
                isBOF = true;
            }
            prevc = buff[i];
          }
          if (bin)
            break;
        }
        if (bin)
        {
          stat.binaryfiles++;
          stat.binaryExtensions.insert(p.path().extension().string());
        }
        else
        {
          if (hasCRLFs && hasLFs)
          {
            stat.mixedEOLs++;
            stat.mixedEOLsExts.insert(p.path().extension().string());
          }
          else if (hasCRLFs)
          {
            stat.crlfOnlyFiles++;
          }
          else if (hasLFs)
          {
            stat.lfOnlyFiles++;
          }
          if (hasSPACEs && hasTABs)
          {
            stat.mixedIndents++;
            stat.mixedIndentsExts.insert(p.path().extension().string());
          }
          else if (hasSPACEs)
          {
            stat.spaceOnlyFiles++;
          }
          else if (hasTABs)
          {
            stat.tabOnlyFiles++;
          }
        }
        stat.totalfiles++;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  int err = 0;

  try
  {
    string inputDir;
    if (!validate_args(argc, argv, inputDir))
      return 1;
    Statistics stat;
    iterate_directory(inputDir, stat);
    cout << "Total files analyzed: " << stat.totalfiles << endl;
    cout << "Line endings CRLF/LF/MIX: " << stat.crlfOnlyFiles << "/" << stat.lfOnlyFiles << "/" << stat.mixedEOLs << endl;
    cout << "Indents SPACE/TAB/MIX: " << stat.spaceOnlyFiles << "/" << stat.tabOnlyFiles << "/" << stat.mixedIndents << endl;
    cout << "Binary files: " << stat.binaryfiles;
    if (stat.binaryfiles)
    {
      cout << " [ ";
      for (auto ext : stat.binaryExtensions)
        cout << ext << " ";
        cout << "]";
    }
    cout << endl;
  }
  catch (exception& e)
  {
    cout << "Error! " << e.what() << std::endl;
    err = 3;
  }

  return err;
}
