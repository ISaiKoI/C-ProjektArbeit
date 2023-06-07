#ifndef TAGS_H
#define TAGS_H

#include <list>
#include <string>
#include <utility>

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
    int signPerLine;

public:

    Variable(string var = "variable", string sq = "ESC", string n = "UNIX",
             bool addTxtPos = false,
             bool addTxtSegment = false, string dxygen = "",
             const string &strngContent = "", int signPerL = 60, int lNum = 0) :
            varname(std::move(var)), seq(std::move(sq)), nl(std::move(n)), addTextPos(addTxtPos),
            addTextSegment(addTxtSegment),
            doxygen(std::move(dxygen)), stringContent(strngContent), convertContent(strngContent),
            signPerLine(signPerL),
            lineNum(lNum) {}

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

    int getSignPerLine() const {
        return signPerLine;
    }

    int getLineNum() const {
        return lineNum;
    }

    string getNl() const {
        return nl;
    }

    string getSeq() const {
        return seq;
    }

    void setVarname(string var) {
        this->varname = std::move(var);
    }

    void setString(string s) {
        this->stringContent = std::move(s);
    }

    void setConvertedString(string converted) {
        this->convertContent = std::move(converted);
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

    Global(const string& filename = "generatedFile", const string& type = "C", const string& header = "./", const string& source = "./",
           const string& ns = "", int signLine = 60, bool sortByName = false, bool sort = false,
           list<string> status = {})
            : globalStatus(std::move(status)) {

        // Check if the sentinel values are present and conditionally assign the default values
        if (!filename.empty())
            outputFilename = filename;

        if (!type.empty()) {
            outputType = type;
        }

        if (!header.empty())
            headerDir = header;

        if (!source.empty())
            sourceDir = source;

        if (!ns.empty())
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

};

#endif