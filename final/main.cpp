#include <iostream>
#include <getopt.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <list>
#include <regex>
#include <dirent.h>
#include "CTextToCPP.cpp"

using namespace std;

void printHelp() {
    cout << "Usage: gentxtsrccode [options] INPUTFILE.TXT\n"
         << "Options:\n"
         << "  --help                           Display this help message\n"
         << "  --outputfilename <filename>      Specify output filename\n"
         << "  --outputtype <type>              Specify output file type (C or CPP)\n"
         << "  --headerdir <dir>                Specify header file Directory\n"
         << "  --sourcedir <dir>                Specify source file Directory\n"
         << "  --namespace <name>               Specify namespace\n"
         << "  --signperline <num>              Specify number of characters per line\n"
         << "  --sortbyvarname <bool>           Sort variables alphabetically\n"
         << "\nCreated by: John Doe, Jane Smith\n"
         << "Contact: example@example.com\n";
}

unordered_map<string, string> parseToMapVariable(const string &input) {
    unordered_map<string, string> variables;

    size_t closingBraceIndex = input.find('}');

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
            variables[key] = value;
        }
    }

    variables["content"] = part2;

    return variables;
}


unordered_map<string, string> parseToMapGlobal(const string &line) {
    unordered_map<string, string> variables;

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

    unordered_map<string, string> variables = parseToMapGlobal(line);
    list<string> globalStatus = global.getGlobalStatus();

    // Assign values to corresponding variables, if not inside the globalStatus list
    if (find(globalStatus.begin(), globalStatus.end(), "nameSpace") == globalStatus.end())
        global.setNameSpace(variables["namespace"]);
    if (find(globalStatus.begin(), globalStatus.end(), "outputFilename") == globalStatus.end())
        global.setOutputFilename(variables["outputfilename"]);
    if (find(globalStatus.begin(), globalStatus.end(), "outputType") == globalStatus.end())
        global.setOutputType(variables["outputtype"]);
    if (find(globalStatus.begin(), globalStatus.end(), "headerDir") == globalStatus.end())
        global.setHeaderDir(variables["headerdir"]);
    if (find(globalStatus.begin(), globalStatus.end(), "sourceDir") == globalStatus.end())
        global.setSourceDir(variables["sourcedir"]);
    if (find(globalStatus.begin(), globalStatus.end(), "signPerLine") == globalStatus.end()) {
        int temp;
        istringstream(variables["signperline"]) >> temp;
        global.setSignPerLine(temp);
    }
    if (find(globalStatus.begin(), globalStatus.end(), "sortByVarName") == globalStatus.end()) {
        bool temp;
        istringstream(variables["sortbyvarname"]) >> boolalpha >> temp;
        global.setSortByVarName(temp);
    }
    return global;
}

CTextToCPP *parseVariable(const string &line, CTextToCPP *obj, int signPerLine, int lineNum) {
    unordered_map<string, string> variables = parseToMapVariable(line);

    string varname = variables["varname"];
    string seq = variables["seq"];
    string nl = variables["nl"];
    bool addTextPos;
    istringstream(variables["addtextpos"]) >> boolalpha >> addTextPos;
    bool addTextSegment;
    istringstream(variables["addtextsegment"]) >> boolalpha >> addTextSegment;
    string doxygen = variables["doxygen"];
    string stringContent = variables["content"];

    Variable var = Variable(varname, seq, nl, addTextPos, addTextSegment, doxygen, stringContent, signPerLine, lineNum);
    CTextToCPP *a;
    if (seq == "ESC") {
        a = new CTextToEscSeq(var);
    } else if (seq == "HEX") {
        a = new CTextToHexSeq(var);
    } else if (seq == "RAWHEX") {
        a = new CTextToRawHexSeq(var);
    } else if (seq == "OCT") {
        a = new CTextToOctSeq(var);
    }
    if (obj == nullptr) {
        obj = a;
    } else {
        CTextToCPP *temp = obj;
        while (temp->getNext() != nullptr) {
            temp = temp->getNext();
        }
        temp->addElement(a);
    }
    return obj;
}

void processFile(const string &filename, Global global) {
    bool process = false;
    ifstream file(filename); //open file

    if (!file) {
        cout << "Failed to open file: " << filename << '\n';
        return;
    }

    string line;
    string variable;
    bool processVar = false; // Track if a variable has been processed
    char c;
    bool tagsFound = false; // Track if any tags were found
    CTextToCPP *obj = nullptr;
    int lineNum = 0; // Track line number
    while (file.get(c)) {   // Read lines
        line += c;
        if (c == '\n') {
            if (line.find("@start") != string::npos) {
                process = true;
                line.clear();
                tagsFound = true;
            } else if (line == "@end") {
                process = false;
                line.clear();
            }
            if (process) {
                if (line.find("@global") != string::npos) {
                    global = parseGlobal(line, global);
                    line.clear();
                } else if (line.find("@variable") != string::npos) {
                    variable += line;
                    processVar = true;
                    line.clear();
                } else if (line.find("@endvariable") != string::npos) {
                    processVar = false;
                    obj = parseVariable(variable, obj, global.getSignPerLine(), lineNum);
                    variable.clear();
                    line.clear();
                } else if (processVar && line != "@endvariable") {
                    variable += line;
                    line.clear();
                } else {
                    line.clear();
                }
            }
            lineNum++;
        }
    }

    stringstream content;
    content << file.rdbuf();
    file.close();
    if (!tagsFound) {
        // If no tags were found, store the entire text as a single string
        string varname;
        if (filename.find('.') != string::npos) {
            varname = filename.substr(0, filename.find_last_of('.'));
        } else {
            varname = filename;
        }
        string outputFilename = varname;

        // Write the content to a new file
        ofstream outputFile(outputFilename);
        if (!outputFile) {
            cout << "Failed to create file: " << outputFilename << '\n';
            return;
        }
        outputFile << content.rdbuf();
        outputFile.close();

        cout << "File created successfully: " << outputFilename << '\n';
    } else
        obj->createFiles(global);
}

vector<string> getFilesInDirectory(const string &directoryPath) {
    cout << "getDir" << endl;
    vector<string> files;
    DIR *dir = opendir(directoryPath.c_str());
    if (dir) {
        dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            string fileName = entry->d_name;
            if (fileName != "." && fileName != "..") {
                string filePath = directoryPath + "/" + fileName;
                files.push_back(filePath);
            }
        }
        closedir(dir);
    }
    return files;
}

int main(int argc, char *argv[]) {

    string outputFilename = "";
    string outputType = "";
    string headerDir = "";
    string sourceDir = "";
    string nameSpace = "";
    int signPerLine = -1;
    bool sortByVarName = false;
    bool sort = false;
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

    while ((opt = getopt_long(argc, argv, shortOpts, longOpts, &longIndex)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 0:
                // Long option
                if (string(longOpts[longIndex].name) == "outputfilename") {
                    outputFilename = optarg; //set generated file name
                    globalStatus.emplace_back("outputFilename");
                } else if (string(longOpts[longIndex].name) == "outputtype") {
                    outputType = optarg;    //set outputtype
                    // Convert the outputType to lowercase for case-insensitive comparison
                    transform(outputType.begin(), outputType.end(), outputType.begin(), ::tolower);
                    // Check if the outputType is either "c" or "cpp"
                    if (outputType == "c" || outputType == "cpp") {
                        globalStatus.emplace_back("outputType");
                    } else {
                        cout << "Invalid output type specified: " << outputType << endl;
                        return 1;
                    }
                } else if (string(longOpts[longIndex].name) == "headerdir") {
                    headerDir = optarg; //set header path
                    globalStatus.emplace_back("headerDir");
                } else if (string(longOpts[longIndex].name) == "sourcedir") {
                    sourceDir = optarg; //set source path
                    globalStatus.emplace_back("sourceDir");
                } else if (string(longOpts[longIndex].name) == "namespace") {
                    nameSpace = optarg; //set namespace
                    globalStatus.emplace_back("nameSpace");
                } else if (string(longOpts[longIndex].name) == "signperline") {
                    // Input validation: signPerLine should not be negative
                    if (signPerLine < 0) {
                        cout << "Invalid value for signPerLine. It should not be negative." << endl;
                        return 1;
                    }
                    istringstream(optarg) >> signPerLine; // set sing per line
                    globalStatus.emplace_back("signPerLine");
                } else if (string(longOpts[longIndex].name) == "sortbyvarname") {
                    // Input validation: sortByVarName should be "true" or "false"
                    if (optarg != "true" && optarg != "false") {
                        cout << "Invalid value for sortByVarName. It should be either 'true' or 'false'." << endl;
                        return 1;
                    }
                    sortByVarName = optarg;
                    globalStatus.emplace_back("sortByVarName");
                    sort = true;
                }
                break;
            default:
                cerr << "Invalid input\n";
                return 1;
        }
    }

    Global global(outputFilename, outputType, headerDir, sourceDir, nameSpace, signPerLine,
                  sortByVarName, sort, globalStatus);

    vector<string> files;

    // Process remaining arguments (files or directories)
    for (int i = 1; i < argc; i++) {
        string path = argv[i];

        // Check if the argument is a valid file
        FILE *file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            files.push_back(path);
        }
            // Check if the argument is a directory
        else {
            vector<string> dirFiles = getFilesInDirectory(path);
            files.insert(files.end(), dirFiles.begin(), dirFiles.end());
        }
    }

    // Process each file
    for (const auto &file: files) {
        processFile(file, global);
    }

    return 0;
}
