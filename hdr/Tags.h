#ifndef TAGS_H
#define TAGS_H

#include <list>
#include <string>
#include <utility>

using namespace std;

class Variable {        // Contains variable tag content
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
    static int counter;

public:

    Variable(const string &var, const string &sq, const string &n,
             bool addTxtPos,
             bool addTxtSegment, string dxygen,
             const string &strngContent, int lNum)
            : varname(!var.empty() ? var : "VARIABLE" + to_string(++counter)),
              seq(!sq.empty() ? sq : "ESC"),
              nl(!n.empty() ? n : "UNIX"),
              addTextPos(addTxtPos),
              addTextSegment(addTxtSegment),
              doxygen(std::move(dxygen)),
              stringContent(!strngContent.empty() ? strngContent : ""),
              convertContent(!strngContent.empty() ? strngContent : ""),
              lineNum(lNum) {}

    virtual ~Variable() = default;

    // Getters
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

    string getNl() const {
        return nl;
    }

    string getSeq() const {
        return seq;
    }

    // Setters
    void setVarname(string var) {
        this->varname = std::move(var);
    }

    void setConvertedString(string converted) {
        this->convertContent = std::move(converted);
    }

};

class Global {      // Contains global tag content
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

    Global(const string &filename, const string &type, const string &header,
           const string &source,
           const string &ns, int signLine, bool sortByName,
           const list<string> &status)
            : outputFilename(!filename.empty() ? filename : "generatedFile"),
              outputType(!type.empty() ? type : "C"),
              headerDir(!header.empty() ? header : "./"),
              sourceDir(!source.empty() ? source : "./"),
              nameSpace(!ns.empty() ? ns : ""),
              signPerLine(signLine > 0 ? signLine : 60),
              sortByVarName(sortByName),
              globalStatus(status) {}

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