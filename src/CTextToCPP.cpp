#include <iostream>
#include <getopt.h>
#include <string>
#include <fstream>
#include <algorithm>
//#include <sstream>
#include <unordered_map>
//#include <cctype>
#include <list>
#include <regex>
#include "CTextToEscSeq.cpp"

using namespace std;

string outputFilename = "generatedFile";
string outputType = "C";
string headerDir = "./";
string sourceDir = "./";
string nameSpace;
int signPerLine = 60;
bool sortByVarName = false;
list<Variable> varList;

void printHelp() {
    cout << "Usage: gentxtsrccode [options] INPUTFILE.TXT\n"
         << "Options:\n"
         << "  --help             Display this help message\n"
         << "  --input <filename>     Specify input file\n"
         << "  --output <filename>    Specify output file\n"
         << "  --outputtype <type>    Specify output file type (C or CPP)\n"
         << "  --headerDir <Dir>      Specify header file Directory\n"
         << "  --sourceDir <Dir>      Specify source file Directory\n"
         << "  --namespace <name>     Specify namespace\n"
         << "  --signperline <num>    Specify number of characters per line\n"
         << "  --sortbyvarname <bool> Sort variables alphabetically\n"
         << "\nCreated by ..................................................\n";
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

//    for (const auto& kvp : variables) {
//        cout << "Key: " << kvp.first << ", Value: " << kvp.second << endl;
//    }

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

//    for (const auto& kvp : variables) {
//        cout << "Key: " << kvp.first << ", Value: " << kvp.second << endl;
//    }

    return variables;
}

void parseGlobal(const string &line) {

    unordered_map<string, string> variables = parseToMapGlobal(line);

    // Assign values to corresponding variables
    nameSpace = variables["namespace"];
    outputFilename = variables["outputfilename"];
    outputType = variables["outputtype"];
    headerDir = variables["headerdir"];
    sourceDir = variables["sourcedir"];
    istringstream(variables["signperline"]) >> signPerLine;
    istringstream(variables["sortbyvarname"]) >> boolalpha >> sortByVarName;

//    // Print the values for debugging
//    cout << "Global variables: " << endl;
//    cout << "nameSpace: " << nameSpace << endl;
//    cout << "outputFilename: " << outputFilename << endl;
//    cout << "outputType: " << outputType << endl;
//    cout << "headerDir: " << headerDir << endl;
//    cout << "sourceDir: " << sourceDir << endl;
//    cout << "signPerLine: " << signPerLine << endl;
//    cout << "sortByVarName: " << sortByVarName << endl;
}

void parseVariable(const string &line) {
//    cout << line << "\nnewline" << endl;
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

//    // Print the values for debugging
//    cout << "Variable: " << endl;
//    cout << "varname: " << varname << endl;
//    cout << "seq: " << seq << endl;
//    cout << "nl: " << nl << endl;
//    cout << "addTextPos: " << addTextPos << endl;
//    cout << "addTextSegment: " << addTextSegment << endl;
//    cout << "doxygen: " << doxygen << endl;
//    cout << "stringContent: " << stringContent << endl;
//    cout << "\nnewVar" << endl;

    Variable var = Variable(varname, seq, nl, addTextPos, addTextSegment, doxygen, stringContent);
    varList.push_back(var);
}

void createFile() {
    transform(outputType.begin(), outputType.end(), outputType.begin(), ::tolower);
    string headerFileName = headerDir + outputFilename + ".h";
    string sourceFileName = sourceDir + outputFilename + "." + outputType;

    ofstream headerFile(headerFileName);
    ofstream sourceFile(sourceFileName);

    if (!headerFile.is_open()) {
        cout << "Failed to create header file: " << headerFileName << endl;
        return;
    }

    if (!sourceFile.is_open()) {
        cout << "Failed to create source file: " << sourceFileName << endl;
        return;
    }

    headerFile << "#ifndef " << outputFilename << "_" << endl << endl;
    if (!nameSpace.empty()) {
        headerFile << "namespace " << nameSpace << " {" << endl << endl;
    }
    for (const auto &variable: varList) {
        headerFile << "/** " << variable.getDoxygen() << " */" << endl;
        headerFile << "extern const char * const " << variable.getVarname() << ";" << endl;
    }
    if (!nameSpace.empty()) {
        headerFile << "} // namespace " << nameSpace << endl;
    }
    headerFile << "#endif";

    sourceFile << "#include <" << outputFilename << ".h>" << endl << endl;
    if (!nameSpace.empty()) {
        sourceFile << "namespace " << nameSpace << " {" << endl << endl;
    }
    for (const auto &variable: varList) {
        sourceFile << "const char * const " << variable.getVarname() << " = {\n" << endl;
        sourceFile << variable.getConvertContent() << "};" << endl;
        if (variable.getAddTextSegment()) {
            sourceFile << "/*\nOriginaltext aus der Variablensektion '" << variable.getVarname() << "'" << endl << endl;
            sourceFile << variable.getStringContent() << endl << "*/" << endl << endl;
        }
    }
    if (!nameSpace.empty()) {
        sourceFile << "} // namespace " << nameSpace << endl;
    }

    headerFile.close();
    sourceFile.close();

    cout << "Files created successfully!" << endl;
}

void processFile(const string &filename) {
    bool process = false;
    ifstream file(filename); //open file

    if (!file) {
        cout << "Failed to open file: " << filename << '\n';
        return;
    }

    string line;
    string variable;
    bool processVar = false;
    char c;
    while (file.get(c)) {   //read lines
        line += c;
        if (c == '\n') {
            if (line.find("@start") != string::npos) {
                process = true;
                line.clear();
            } else if (line == "@end") {
                process = false;
                line.clear();
            }
            if (process) {
                if (line.find("@global") != string::npos) {
//                    cout << 1 << line << endl << endl;
                    parseGlobal(line);
                    line.clear();
                } else if (line.find("@variable") != string::npos) {
//                    cout << 2 << line << endl << endl;
                    variable += line;
                    processVar = true;
                    line.clear();
                } else if (line.find("@endvariable") != string::npos) {
//                    cout << 3 << line << endl << endl;
                    processVar = false;
                    parseVariable(variable);
                    variable.clear();
//                    cout << "cleaned: " << variable << endl;
                    line.clear();
                } else if (processVar && line != "@endvariable") {
//                    cout << 4 << line << endl << endl;
                    variable += line;
                    line.clear();
                } else {
                    line.clear();
                }
            }
        }
    }
    file.close();
    createFile();
}


int main(int argc, char *argv[]) {
    const char *const shortOpts = "h";
    const option longOpts[] = {
            {"help",           no_argument,       nullptr, 'h'},
            {"inputfilename",  required_argument, nullptr, 0},
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

    while ((opt = getopt_long(argc, argv, shortOpts, longOpts, &longIndex)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return 0;
            case 0:
                // Long option
                if (string(longOpts[longIndex].name) == "inputfilename") {
                    // Handle --input option
                    cout << "Input file: " << optarg << '\n';
                } else if (string(longOpts[longIndex].name) == "outputfilename") {
                    outputFilename = optarg; //set generated file name
                } else if (string(longOpts[longIndex].name) == "outputtype") {
//                    if (optarg != "C" && optarg != "CPP") {
//                        cerr << "Not supported outputtype\n";
//                        return 1;
//                    } else {
                    outputType = optarg;    //set outputtype
//                    }
                } else if (string(longOpts[longIndex].name) == "headerdir") {
                    headerDir = optarg; //set header path
                } else if (string(longOpts[longIndex].name) == "sourcedir") {
                    sourceDir = optarg; //set source path
                } else if (string(longOpts[longIndex].name) == "namespace") {
                    nameSpace = optarg; //set namespace
                } else if (string(longOpts[longIndex].name) == "signperline") {
                    istringstream(optarg) >> signPerLine; // set sing per line
                } else if (string(longOpts[longIndex].name) == "sortbyvarname") {
                    sortByVarName = optarg;
                }
                break;
            default:
                cerr << "Invalid input\n";
                return 1;
        }
    }
//    createFile();

    // Process remaining arguments (files)
    for (int i = optind; i < argc; i++) {
        processFile(argv[i]);
    }

    return 0;
}
