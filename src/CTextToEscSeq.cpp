#include <iostream>

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

public:
    static string convertString(const string& content, const string& nl, const string& seq) {
        return content;
    }

    Variable(string varname, const string& seq, const string& nl, bool addTextPos, bool addTextSegment, string doxygen,
             const string& stringContent) {
        this->varname = varname;
        this->seq = seq;
        this->nl = nl;
        this->addTextPos = addTextPos;
        this->addTextSegment = addTextSegment;
        this->doxygen = doxygen;
        this->stringContent = stringContent;
        this->convertContent = convertString(stringContent, nl, seq);
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
};
