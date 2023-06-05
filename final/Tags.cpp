#include <iostream>
#include <list>
#include <string>
#include <sstream>

using namespace std;

class Variable {
private:
    string varname;
    string seq;
    string nl;
    bool addTextPos;
    bool addTextSegment;
    string doxygen;
    string stringContent;
    string convertContent;
    int lineNum;

public:

    static string replaceString(const string &input, const string &target, const string &replacement) {
        string result = input;
        size_t startPos = 0;
        while ((startPos = result.find(target, startPos)) != string::npos) {
            result.replace(startPos, target.length(), replacement);
            startPos += replacement.length();
        }
        return result;
    }

    static string encodeString(string input, const string &seq, const string &nl, int signperline) {
        ostringstream encoded;
        string encodedString;
        int lineLength = 0;

        if (seq == "ESC") {
            input = replaceString(input, "\"", "\\\"");
        }
        for (char c: input) {
            if (lineLength == 0) {
                encoded << "\"";
            }
            if (seq == "ESC") {
                if (c == '\n' || c == '\r') {
                    if (nl == "DOS")
                        encoded << "\\r\\n\" \\\n";
                    else if (nl == "MAC")
                        encoded << "\\r\" \\\n";
                    else // nl == "UNIX" (default)
                        encoded << "\\n\" \\\n";
                    lineLength = 0;
                } else if (c == '\\') {
                    encoded << c;
                } else {
                    encoded << c;
                    lineLength++;
                }
            } else if (seq == "OCT") {
                ostringstream octEncoded;
                octEncoded << "\\" << oct << static_cast<int>(c);
                int checkLength = lineLength + octEncoded.str().length();
                if (checkLength > signperline) {
                    encoded << "\" \\\n";
                    lineLength = 0;
                } else {
                    encoded << octEncoded.str();
                    lineLength += octEncoded.str().length();
                }
            } else if (seq == "HEX") {
                ostringstream hexEncoded;
                hexEncoded << "\\x";
                if (static_cast<int>(c) < 16) {
                    hexEncoded << "0";
                }
                hexEncoded << hex << static_cast<int>(c);

                int checkLength = lineLength + hexEncoded.str().length();
                if (checkLength > signperline) {
                    encoded << "\" \\\n";
                    lineLength = 0;
                } else {
                    encoded << hexEncoded.str();
                    lineLength += hexEncoded.str().length();
                }
            } else if (seq == "RAWHEX") {
                ostringstream rawHexEncoded;
                rawHexEncoded << "0x" << hex << static_cast<int>(c) << ", ";
                int checkLength = lineLength + rawHexEncoded.str().length();
                if (checkLength > signperline) {
                    encoded << "\" \\\n";
                    lineLength = 0;
                } else {
                    encoded << rawHexEncoded.str();
                    lineLength += rawHexEncoded.str().length();
                }
            }

            if (signperline > 0 && lineLength == signperline) {
                encoded << "\" \\\n";
                lineLength = 0;
            }
        }
        if (seq != "ESC")
            encoded << "\"";
        encodedString = encoded.str();
        return encodedString;
    }

    Variable(string varname = "variable", const string &seq = "ESC", const string &nl = "UNIX", bool addTextPos = false,
             bool addTextSegment = false, string doxygen = "",
             const string &stringContent = "", int signPerLine = 60, int lineNum = 0) {
        this->varname = varname;
        this->seq = seq;
        this->nl = nl;
        this->addTextPos = addTextPos;
        this->addTextSegment = addTextSegment;
        this->doxygen = doxygen;
        this->stringContent = stringContent;
        this->convertContent = encodeString(stringContent, seq, nl, signPerLine);
        this->lineNum = lineNum;
    }

    virtual ~Variable() = default;

    string getVarname() const {
        return varname;
    }

    string getStringContent() const {
        return stringContent;
    }

    string getConvertContent() const {
        return convertContent;
    }

    string getDoxygen() const {
        return doxygen;
    }

    bool getAddTextPos() const {
        return addTextPos;
    }

    bool getAddTextSegment() const {
        return addTextSegment;
    }

    int getLineNum() const {
        return lineNum;
    }

    void setVarname(string a) {
        this->varname = a;
    }

};

bool compareByName(const Variable &a, const Variable &b) {
    return a.getVarname() < b.getVarname();
}

class Global {
private:
    string outputFilename;
    string outputType;
    string headerDir;
    string sourceDir;
    string nameSpace;
    int signPerLine;
    bool sortByVarName;
    list<string> globalStatus;  // Tracks if options were pasted via commandline

public:

    Global(string filename = "generatedFile", string type = "C", string header = "./", string source = "./",
           string ns = "", int signLine = 60, bool sortByName = false, bool sort = false,
           list<string> status = {}, list<Variable> vars = {})
            : globalStatus(status) {

        // Check if the sentinel values are present and conditionally assign the default values
        if (filename != "")
            outputFilename = filename;

        if (type != "") {
            outputType = type;
        }

        if (header != "")
            headerDir = header;

        if (source != "")
            sourceDir = source;

        if (ns != "")
            nameSpace = ns;

        if (signLine != -1) {
            signPerLine = signLine;
        }

        if (sort) {
            sortByVarName = sortByName;
        }
    }

    // Getters
    string getOutputFilename() const {
        return outputFilename;
    }

    string getOutputType() const {
        return outputType;
    }

    string getHeaderDir() const {
        return headerDir;
    }

    string getSourceDir() const {
        return sourceDir;
    }

    string getNameSpace() const {
        return nameSpace;
    }

    int getSignPerLine() const {
        return signPerLine;
    }

    bool isSortByVarName() const {
        return sortByVarName;
    }

    list<string> getGlobalStatus() const {
        return globalStatus;
    }

    // Setters
    void setOutputFilename(const string &filename) {
        outputFilename = filename;
    }

    void setOutputType(const string &type) {
        outputType = type;
    }

    void setHeaderDir(const string &header) {
        headerDir = header;
    }

    void setSourceDir(const string &source) {
        sourceDir = source;
    }

    void setNameSpace(const string &ns) {
        nameSpace = ns;
    }

    void setSignPerLine(int signLine) {
        signPerLine = signLine;
    }

    void setSortByVarName(bool sortByName) {
        sortByVarName = sortByName;
    }

    void setGlobalStatus(const list<string> &status) {
        globalStatus = status;
    }

};

