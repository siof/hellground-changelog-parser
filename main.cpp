#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <stdarg.h>

using namespace std;

#define SEPARATOR           "::**::"
#define TMP_FILE_NAME       "hghist.log"
#define COMMAND_FMT         "hg history --template '{author}\n{rev}\n{desc}\n%s\n' %s > %s"
#define DEFAULT_OUT_FORMAT  "<strong><span style=\"color: #5d7870;\">[<strong>%u</strong> %s]</span></strong> %s"

std::string GetFormattedString(const char * format, ...)
{
    char buffer[strlen(format) + 10000];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    return std::string(buffer);
}

enum Mode
{
    MODE_NONE       = 0,
    MODE_ALL        = 1,
    MODE_REV_NUM    = 2,
    MODE_REV_COUNT  = 3
};

enum Part
{
    PART_AUTHOR = 0,
    PART_REV    = 1,
    PART_DESC   = 2,
    PART_SEPARATOR
};

struct RevInfo
{
    RevInfo() : author(""), rev(0), description("")
    {

    }

    std::string author;
    unsigned int rev;
    std::string description;

    void Clear()
    {
        author = "";
        rev = 0;
        description = "";
    }
};

int main()
{
    cout << " *********************************************** " << endl;
    cout << "*                                               *" << endl;
    cout << "*              HG Changelog parser              *" << endl;
    cout << "*                                               *" << endl;
    cout << " *********************************************** " << endl;

    Mode chosenMode = MODE_NONE;

    do
    {
        int method;

        cout << endl << "Choose method:" << endl;
        cout << "1 - None, just exit" << endl;
        cout << "2 - All revisions" << endl;
        cout << "3 - From revision number" << endl;
        cout << "4 - Revisions count" << endl << endl;

        cout << "0 - exit" << endl;
        cin >> method;

        if (method == 0 || method == 1)
            return 0;

        switch (method)
        {
            case 2:
                chosenMode = MODE_ALL;
                break;
            case 3:
                chosenMode = MODE_REV_NUM;
                break;
            case 4:
                chosenMode = MODE_REV_COUNT;
                break;
            default:
                break;
        }
    }
    while (chosenMode == MODE_NONE);

    cout << endl << "You choose: ";

    std::string options = "";

    switch (chosenMode)
    {
        case MODE_REV_COUNT:
        {
            cout << "Rev count mode" << endl;
            unsigned int revCount;
            cout << "Rev count to parse: ";
            cin >> revCount;
            options = GetFormattedString("-l %u", revCount);
            break;
        }
        case MODE_REV_NUM:
        {
            cout << "Rev number mode" << endl;
            unsigned int revNum;
            cout << "Start from rev: ";
            cin >> revNum;
            options = GetFormattedString("-r tip:%u", revNum);
            break;
        }
        case MODE_ALL:
            cout << "All revs mode" << endl;
            break;
        default:
            break;
    }

    cout << "Creating temporary " << TMP_FILE_NAME << " file." << endl;
    system(GetFormattedString(COMMAND_FMT, SEPARATOR, options.c_str(), TMP_FILE_NAME).c_str());

    vector<RevInfo> revs;
    Part actualPart = PART_AUTHOR;
    ifstream file;

    cout << endl << "Opening " << TMP_FILE_NAME << " file." << endl;
    file.open(TMP_FILE_NAME);

    RevInfo tmpInfo;

    int actual = 0;

    cout << "Begin file processing ..." << endl;
    while (!file.eof())
    {
        char tmpStr[2000];
        cout << "\rProcessing " << actual++ << " line.                 " << flush;
        file.getline(tmpStr, 2000);
//        cout << "tmpStr: " << tmpStr << endl;
        switch (actualPart)
        {
            case PART_AUTHOR:
                tmpInfo.author = tmpStr;
                actualPart = PART_REV;
                break;
            case PART_REV:
                tmpInfo.rev = atoi(tmpStr);
                actualPart = PART_DESC;
                break;
            case PART_DESC:
            {
                if (!strcmp(tmpStr, SEPARATOR))
                {
                    revs.push_back(tmpInfo);
                    tmpInfo.Clear();
                    actualPart = PART_AUTHOR;
                }
                else
                    tmpInfo.description = tmpInfo.description + (tmpInfo.description.empty() ? "" : "\n") + tmpStr;

                break;
            }
            default:
                actualPart = PART_AUTHOR;
                break;
        }
    }

    file.close();

    cout << "\rFile processing ended.                                     " << endl << endl;

    std::string format, outFileName;
    cout << "Output format (choose 'x' for default format): ";
    cin >> format;

    if (format.empty() || format == "" || format == "x")
        format = DEFAULT_OUT_FORMAT;

    cout << "Output file: ";
    cin >> outFileName;

    ofstream oFile;
    oFile.open(outFileName.c_str());

    int count = revs.size();
    actual = 1;

    cout << "Begin creating output file." << endl;
    cout << "Used format: " << format << endl;
    cout << "Output file name: " << outFileName << endl;
    cout << "Rev count: " << count << endl << endl;

    RevInfo tmpRevInfo;
    for (vector<RevInfo>::const_iterator itr = revs.begin(); itr != revs.end(); ++itr)
    {
        tmpRevInfo = *itr;
        cout << "\rProcessing " << actual << " rev (from " << count << ")                         " << flush;
        oFile << GetFormattedString(format.c_str(), tmpRevInfo.rev, tmpRevInfo.author.c_str(), tmpRevInfo.description.c_str()) << endl;
    }

    cout << "\rChangelog output file creating ended.                                " << endl;

    oFile.close();

    system(GetFormattedString("rm %s", TMP_FILE_NAME).c_str());

    return 0;
}
