#include <iostream>
#include <getopt.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <list>
#include <regex>
#include <dirent.h>
#include "CTextToCPP.h"

using namespace std;

int Variable::counter = 0;

void printHelp() {
    cout << "Usage: gentxtsrccode [options] INPUTFILE.TXT (or Path e.g. C:\\User\\src\\INPUTFILE.TXT\\)\n"
         << "Options:\n"
         << "  --help                           Display this help message\n"
         << "  --outputfilename <filename>      Specify output filename\n"
         << "  --outputtype <type>              Specify output file type (C or CPP)\n"
         << "  --headerdir <dir>                Specify header file Directory(e.g. C:\\User\\src\\file\\)\n"
         << "  --sourcedir <dir>                Specify source file Directory(e.g. C:\\User\\src\\file\\)\n"
         << "  --namespace <name>               Specify namespace\n"
         << "  --signperline <num>              Specify number of characters per line(e.g. 50)\n"
         << "  --sortbyvarname <bool>           Sort variables alphabetically(true or false)\n"
         << "\nCreated by: Marcus Unglert, Tobias Schilling, Daniel Balla, Max Domitrovic\n"      //TODO
         << "Contact: balla.daniel-it22@it.dhbw-ravensburg.de\n";
}

unordered_map<string, string> parseToMapVariable(const string &input) {
    unordered_map<string, string> variables;

    size_t closingBraceIndex = input.find('}');
    size_t openingBraceIndex = input.find('{');

    // Check for opening/closing brackets
    if (openingBraceIndex == string::npos || closingBraceIndex == string::npos) {
        cout << "Error: Invalid input format. Missing opening/closing bracket." << endl;
        exit(EXIT_FAILURE);
        ;
    }

    string insideBraces = input.substr(openingBraceIndex + 1, closingBraceIndex - openingBraceIndex - 1);

    // Check if the inside of the brackets is not empty
    if (insideBraces.empty()) {
        cerr << "Error: Invalid input format. Missing content inside the brackets." << endl;
        exit(EXIT_FAILURE);
    }

    string part1 = input.substr(0, closingBraceIndex);
    string part2 = input.substr(closingBraceIndex + 2, input.length());

    // Remove leading/trailing whitespaces
    string cleanedInput = regex_replace(part1, regex("^\\s+"), "");
    cleanedInput = regex_replace(cleanedInput, regex("\\s+$"), "");

    // Remove '@variable {' prefix if present
    if (cleanedInput.substr(0, 11) == "@variable {") {
        cleanedInput = cleanedInput.substr(11);
    }

    // Remove trailing '}' character if present
    if (cleanedInput.back() == '}') {
        cleanedInput = cleanedInput.substr(0, cleanedInput.size() - 1);
    }

    // Remove additional characters
    cleanedInput.erase(remove(cleanedInput.begin(), cleanedInput.end(), '\"'), cleanedInput.end());

    // Split the input into key-value pairs
    istringstream iss(cleanedInput);
    string pair;
    while (getline(iss, pair, ',')) {
        size_t separatorIndex = pair.find(':');
        if (separatorIndex != string::npos) {
            string key = pair.substr(1, separatorIndex - 1);
            string value = pair.substr(separatorIndex + 2);
            // Check if key is not empty
            if (key.empty()) {
                cerr << "Error: Invalid input format. Incorrect key-value pair." << endl;
                exit(EXIT_FAILURE);
            }
            variables[key] = value;
        }
    }

    variables["content"] = part2;

    return variables;
}


unordered_map<string, string> parseToMapGlobal(const string &line) {
    unordered_map<string, string> variables;

    size_t closingBraceIndex = line.find('}');
    size_t openingBraceIndex = line.find('{');

    // Check for opening/closing brackets
    if (openingBraceIndex == string::npos || closingBraceIndex == string::npos) {
        cout << "Error: Invalid input format. Missing opening/closing bracket." << endl;
        exit(EXIT_FAILURE);
        ;
    }

    string insideBraces = line.substr(openingBraceIndex + 1, closingBraceIndex - openingBraceIndex - 1);

    // Check if the inside of the brackets is not empty
    if (insideBraces.empty()) {
        cerr << "Error: Invalid input format. Missing content inside the brackets." << endl;
        exit(EXIT_FAILURE);
    }

    // Remove leading/trailing whitespaces
    string cleanedInput = line;
    cleanedInput = regex_replace(cleanedInput, regex("^\\s+"), "");
    cleanedInput = regex_replace(cleanedInput, regex("\\s+$"), "");

    // Remove '@global {' or '@variable {' prefix if present
    if (cleanedInput.substr(0, 9) == "@global {") {
        cleanedInput = cleanedInput.substr(cleanedInput.find('{') + 1);
    }

    // Remove trailing '}' character if present
    if (cleanedInput.back() == '}') {
        cleanedInput = cleanedInput.substr(0, cleanedInput.size() - 1);
    }

    // Remove additional characters
    cleanedInput.erase(remove(cleanedInput.begin(), cleanedInput.end(), '\"'), cleanedInput.end());

    // Split the input into key-value pairs
    istringstream iss(cleanedInput);
    string pair;
    while (getline(iss, pair, ',')) {
        size_t separatorIndex = pair.find(':');
        if (separatorIndex != string::npos) {
            string key = pair.substr(1, separatorIndex - 1);
            string value = pair.substr(separatorIndex + 2, pair.length());
            variables[key] = value;
        }
    }

    return variables;
}

Global parseGlobal(const string &line, Global global) {

    unordered_map<string, string> variables = parseToMapGlobal(line);   // Map global tag content
    list<string> globalStatus = global.getGlobalStatus();

    // Assign values to corresponding variables, if not inside the globalStatus list
    if (find(globalStatus.begin(), globalStatus.end(), "nameSpace") == globalStatus.end() &&
        variables.count("namespace") > 0) {
        global.setNameSpace(variables["namespace"]);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "outputFilename") == globalStatus.end() &&
        variables.count("outputfilename") > 0) {
        global.setOutputFilename(variables["outputfilename"]);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "outputType") == globalStatus.end() &&
        variables.count("outputtype") > 0) {
        global.setOutputType(variables["outputtype"]);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "headerDir") == globalStatus.end() &&
        variables.count("headerdir") > 0) {
        global.setHeaderDir(variables["headerdir"]);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "sourceDir") == globalStatus.end() &&
        variables.count("sourcedir") > 0) {
        global.setSourceDir(variables["sourcedir"]);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "signPerLine") == globalStatus.end() &&
        variables.count("signperline") > 0) {
        int temp;
        istringstream(variables["signperline"]) >> temp;
        global.setSignPerLine(temp);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "sortByVarName") == globalStatus.end() &&
        variables.count("sortbyvarname") > 0) {
        bool temp;
        istringstream(variables["sortbyvarname"]) >> boolalpha >> temp;
        global.setSortByVarName(temp);
    }
    return global;
}

CTextToCPP *parseVariable(const string &line, CTextToCPP *obj, int signPerLine, int lineNum) {
    unordered_map<string, string> variables = parseToMapVariable(line); // Map variable contents

    string varname = variables["varname"];
    string seq = variables["seq"];
    string nl = variables["nl"];
    bool addTextPos;
    istringstream(variables["addtextpos"]) >> boolalpha >> addTextPos;
    bool addTextSegment;
    istringstream(variables["addtextsegment"]) >> boolalpha >> addTextSegment;
    string doxygen = variables["doxygen"];
    string stringContent = variables["content"];
    // Create variable
    Variable var = Variable(varname, seq, nl, addTextPos, addTextSegment, doxygen, stringContent, lineNum);
    CTextToCPP *cText = nullptr;
    // Create obj depending on seq
    if (seq == "ESC") {
        cText = new CTextToEscSeq(var);
    } else if (seq == "HEX") {
        cText = new CTextToHexSeq(var);
    } else if (seq == "RAWHEX") {
        cText = new CTextToRawHexSeq(var);
    } else if (seq == "OCT") {
        cText = new CTextToOctSeq(var);
    }
    if (cText != nullptr)
        cText->convert(signPerLine);    // Encode content string
    if (obj == nullptr) {
        obj = cText;
    } else {
        CTextToCPP *temp = obj;
        while (temp->getNext() != nullptr) {
            temp = temp->getNext();
        }
        temp->addElement(cText);    // Set next
    }
    return obj;
}

void processFile(const string &filename, Global global) {
    bool process = false;
    ifstream file(filename); //open file

    if (!file) {
        cout << "1Failed to open file: " << filename << '\n';
        return;
    }


    string line;
    string variable;
    bool processVar = false; // Track if a variable has been processed
    char c;
    bool tagsFound = false; // Track if any tags were found
    CTextToCPP *obj = nullptr;
    int lineNum = 0; // Track line number
    int varLine = 0;
    while (file.get(c)) {   // Read lines
        line += c;
        if (c == '\n') {
            if (line.find("@start") != string::npos) {  // @start
                process = true;
                line.clear();
                tagsFound = true;
            } else if (line == "@end") {    // @end
                process = false;
                line.clear();
            }
            if (process) {
                if (line.find("@global") != string::npos) { // Parse global tag
                    global = parseGlobal(line, global);
                    line.clear();
                } else if (line.find("@variable") != string::npos) {    // Parse variable tag
                    variable += line;
                    processVar = true;
                    varLine = lineNum;
                    line.clear();
                } else if (line.find("@endvariable") != string::npos) { // @endvariable
                    processVar = false;
                    obj = parseVariable(variable, obj, global.getSignPerLine(), varLine);
                    variable.clear();
                    line.clear();
                } else if (processVar && line != "@endvariable") {  // !@endvariable
                    variable += line;
                    line.clear();
                } else {
                    line.clear();
                }
            }
            lineNum++;
        }
    }


    file.close();
    if (!tagsFound) {
        // If no tags were found, store the entire text as a single string
        ifstream fileS(filename); //open file

        if (!fileS) {
            cout << "1Failed to open file: " << filename << '\n';
            return;
        }
        stringstream content;
        content << fileS.rdbuf();
        string strContent = content.str();
        fileS.close();
        string varname;
        if (filename.find('.') != string::npos) {
            varname = filename.substr(0, filename.find_last_of('.'));
        } else {
            varname = filename;
        }
        Variable strVar = Variable(varname, "ESC", "UNIX", false, false, "", strContent, 0);
        obj = new CTextToEscSeq(strVar);
        obj->convert(global.getSignPerLine());
    }
    if (obj != nullptr)
        obj->createFiles(global);
}


int main(int argc, char *argv[]) {

    map<string, vector<string>> options;
    // Creare copy of argv[]
    char **argvCopy = new char *[argc];

    for (int i = 0; i < argc; ++i) {
        size_t argLen = strlen(argv[i]) + 1;
        argvCopy[i] = new char[argLen];
        strcpy(argvCopy[i], argv[i]);
    }

    string outputFilename;
    string outputType;
    string headerDir;
    string sourceDir;
    string nameSpace;
    int signPerLine;
    bool sortByVarName;
    list<string> globalStatus;

    const char *const shortOpts = "h";
    const option longOpts[] = {
            {"help",           no_argument,       nullptr, 'h'},
            {"outputfilename", required_argument, nullptr, 0},
            {"outputtype",     required_argument, nullptr, 0},
            {"headerdir",      required_argument, nullptr, 0},
            {"sourcedir",      required_argument, nullptr, 0},
            {"namespace",      required_argument, nullptr, 0},
            {"signperline",    required_argument, nullptr, 0},
            {"sortbyvarname",  required_argument, nullptr, 0},
            {nullptr,          no_argument,       nullptr, 0}
    };

    int opt;
    int longIndex = 0;
    optind = 1;
    vector<pair<string, string>> opts;

    while ((opt = getopt_long(argc, argv, shortOpts, longOpts, &longIndex)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 0:
                opts.emplace_back(longOpts[longIndex].name, optarg);
                break;
            default:
                cerr << "Invalid input\n";
                return 1;
        }
    }

    int count = 0;

    for (int ind = 1; ind < argc; ind++) {
        string argument = argvCopy[ind];

        if (argument.substr(0, 2) == "--") {
            ind++;
            ++count;
        } else {
            for (int i = 0; i < count && i < opts.size(); i++) {
                const auto &pair = opts[i];
                // Long option
                if (pair.first == "outputfilename") {
                    outputFilename = pair.second; // Set generated file name
                    globalStatus.emplace_back("outputFilename");
                } else if (pair.first == "outputtype") {
                    outputType = pair.second;    // Set outputtype
                    // Convert the outputType to lowercase for case-insensitive comparison
                    transform(outputType.begin(), outputType.end(), outputType.begin(), ::tolower);
                    // Check if the outputType is either "c" or "cpp"
                    if (outputType == "c" || outputType == "cpp") {
                        globalStatus.emplace_back("outputType");
                    } else {
                        cout << "Invalid output type specified: " << outputType << endl;
                        return 1;
                    }
                } else if (pair.first == "headerdir") {
                    headerDir = pair.second; // Set header path
                    globalStatus.emplace_back("headerDir");
                } else if (pair.first == "sourcedir") {
                    sourceDir = pair.second; // Set source path
                    globalStatus.emplace_back("sourceDir");
                } else if (pair.first == "namespace") {
                    nameSpace = pair.second; // Set namespace
                    globalStatus.emplace_back("nameSpace");
                } else if (pair.first == "signperline") {
                    istringstream(pair.second) >> signPerLine; // Set sing per line
                    // Input validation: signPerLine should not be negative
                    if (signPerLine < 0) {
                        cout << "Invalid value for signPerLine. It should not be negative." << endl;
                        return 1;
                    }
                    globalStatus.emplace_back("signPerLine");
                } else if (pair.first == "sortbyvarname") {
                    // Input validation: sortByVarName should be "true" or "false"
                    if (pair.second != "true" && pair.second != "false") {
                        cout << "Invalid value for sortByVarName. It should be either 'true' or 'false'." << endl;
                        return 1;
                    }
                    istringstream iss(pair.second);
                    iss >> boolalpha >> sortByVarName;
                    globalStatus.emplace_back("sortByVarName");
                }
            }
            // Erase used opt
            opts.erase(opts.begin(), opts.begin() + count);
            // Check if the argument is a valid file
            FILE *file = fopen(argument.c_str(), "r");
            if (file)
                fclose(file);
            else
                cerr << "Error: The argument is not a valid file." << endl;
            Global global(outputFilename, outputType, headerDir, sourceDir, nameSpace, signPerLine,
                          sortByVarName, globalStatus);
            processFile(argument, global); // Process file
            // Reset all possible arguments
            outputFilename = "generatedFile";
            outputType = "C";
            headerDir = "./";
            sourceDir = "./";
            nameSpace.clear();
            signPerLine = 60;
            sortByVarName = false;
            globalStatus.clear();
            count = 0;
        }
    }

    return 0;
}
