#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <direct.h>
#include "Tags.cpp"

using namespace std;

class CTextToCPP {
private:
    Variable variable;
    CTextToCPP *next = nullptr;
    CTextToCPP *head = this;

public:
    CTextToCPP(Variable vr = {}) : variable(vr) {}

    virtual ~CTextToCPP() {}

    virtual string convert() const = 0;

    bool createDirectory(const string &directoryPath) {
#ifdef _WIN32
        int result = _mkdir(directoryPath.c_str());
#else
        int result = mkdir(directoryPath.c_str(), 0777);
#endif

        return result == 0 || errno == EEXIST;
    }

    bool createDirectories(const string &path) {
        size_t pos = 0;
        string delimiter = "/";
        string directory;
        string remainingPath = path;

        while ((pos = remainingPath.find(delimiter)) != string::npos) {
            directory = remainingPath.substr(0, pos);
            if (!directory.empty() && !createDirectory(directory)) {
                cout << "Failed to create directory: " << directory << endl;
                return false;
            }
            remainingPath = remainingPath.substr(pos + delimiter.length());
        }

        // Create the last directory in the path
        if (!remainingPath.empty() && !createDirectory(remainingPath)) {
            cout << "Failed to create directory: " << remainingPath << endl;
            return false;
        }
        return true;
    }

    void writeDeclaration(Global global) {
        string headerString = global.getOutputFilename();
        transform(headerString.begin(), headerString.end(), headerString.begin(), ::toupper);
        string headerFileName = global.getHeaderDir() + global.getOutputFilename() + ".h";

        // Create directories if they don't exist
        if (!createDirectories(global.getHeaderDir())) {
            return;
        }

        ofstream headerFile(headerFileName);

        if (!headerFile.is_open()) {
            cout << "Failed to create header file: " << headerFileName << endl;
            return;
        }

        if (global.isSortByVarName()) {
            sort();
        }

        headerFile << "#ifndef _" << headerString << "_" << endl;
        headerFile << "#define _" << headerString << "_" << endl << endl;

        if (!global.getNameSpace().empty()) {
            headerFile << "namespace " << global.getNameSpace() << " {" << endl << endl;
        }

        CTextToCPP *temp = head;
        while (temp != nullptr) {
            cout << temp->getVariable().getVarname() << endl;
            headerFile << "/** " << temp->getVariable().getDoxygen() << " */" << endl;
            headerFile << "extern const char * const " << temp->getVariable().getVarname() << ";" << endl;
            if (temp->getVariable().getAddTextPos())
                headerFile << "//Variable declared in line " << temp->getVariable().getLineNum() << " of Input" << endl;
            temp = temp->getNext();
        }

        if (!global.getNameSpace().empty()) {
            headerFile << "} // namespace " << global.getNameSpace() << endl;
        }

        headerFile << "#endif";
        headerFile.close();
        cout << "Files created successfully!" << endl;
    }

    void writeImplementation(Global global) {
        string sourceFileName = global.getSourceDir() + global.getOutputFilename() + "." + global.getOutputType();

        // Create directories if they don't exist
        if (!createDirectories(global.getSourceDir())) {
            return;
        }

        ofstream sourceFile(sourceFileName);

        if (!sourceFile.is_open()) {
            cout << "Failed to create source file: " << sourceFileName << endl;
            return;
        }

        sourceFile << "#include <" << global.getOutputFilename() << ".h>" << endl << endl;
        if (!global.getNameSpace().empty()) {
            sourceFile << "namespace " << global.getNameSpace() << " {" << endl << endl;
        }

        CTextToCPP *temp = head;
        while (temp != nullptr) {
            sourceFile << "const char * const " << temp->getVariable().getVarname() << " = {" << endl;
            if (temp->getVariable().getAddTextPos())
                sourceFile << "//Variable declared in line " << temp->getVariable().getLineNum() << " of Input" << endl;
            sourceFile << temp->getVariable().getConvertContent() << "};\n" << endl;
            if (temp->getVariable().getAddTextSegment()) {
                sourceFile << "/*\nOriginal text from variable section '" << temp->variable.getVarname() << "'" << endl
                           << endl;
                sourceFile << temp->getVariable().getStringContent() << endl << "*/" << endl << endl;
            }
            temp = temp->getNext(); // Advance the temp pointer
        }

        if (!global.getNameSpace().empty()) {
            sourceFile << "} // namespace " << global.getNameSpace() << endl;
        }

        sourceFile.close();
        cout << "Files created successfully!" << endl;
    }

    void sort() {
        // Count the number of objects
        int count = 0;
        CTextToCPP *current = this;
        while (current != nullptr) {
            count++;
            current = current->getNext();
        }

        // Create arrays to store the objects and their variable names
        CTextToCPP **objects = new CTextToCPP *[count];
        string *varnames = new string[count];

        // Traverse the linked list and store the objects and variable names in the arrays
        current = this;
        int i = 0;
        while (current != nullptr) {
            objects[i] = current;
            varnames[i] = current->getVariable().getVarname();
            current = current->getNext();
            i++;
        }

        // Sort the objects and variable names arrays based on the variable name
        ::sort(varnames, varnames + count);
        ::sort(objects, objects + count, [](const CTextToCPP *a, const CTextToCPP *b) {
            return a->getVariable().getVarname() < b->getVariable().getVarname();
        });

        // Update the next pointers based on the sorted order
        for (int j = 0; j < count - 1; j++) {
            objects[j]->setNext(objects[j + 1]);
        }
        objects[count - 1]->setNext(nullptr);

        // Save the new head object
        CTextToCPP *newHead = objects[0];

        // Clean up the memory
        delete[] objects;
        delete[] varnames;

        // Update head for all objects
        current = newHead;
        while (current != nullptr) {
            current->setHead(newHead);
            current = current->getNext();
        }
    }

    void clear() {
        // Traverse the linked list and delete each object
        CTextToCPP *current = head;
        while (current != nullptr) {
            CTextToCPP *next = current->getNext();
            delete current;
            current = next;
        }

        // Reset head and next pointers
        head = nullptr;
        next = nullptr;
    }

    void createFiles(Global global) {
        this->writeDeclaration(global);
        this->writeImplementation(global);
        this->clear();
    }

    void addElement(CTextToCPP *next) {
        this->next = next;
    }

    CTextToCPP *getNext() {
        return next;
    }

    Variable getVariable() const {
        return variable;
    }

    void setNext(CTextToCPP *nx) {
        next = nx;
    }

    void setHead(CTextToCPP *h) {
        head = h;
    }

};

class CTextToEscSeq : public CTextToCPP {
public:
    CTextToEscSeq(Variable vr = {}) : CTextToCPP(vr) {}

    static string replaceString(const string &input, const string &target, const string &replacement) {
        string result = input;
        size_t startPos = 0;
        while ((startPos = result.find(target, startPos)) != string::npos) {
            result.replace(startPos, target.length(), replacement);
            startPos += replacement.length();
        }
        return result;
    }

    string convert() const override {
        string result;
        // Implementiere hier die Ersetzung der nicht darstellbaren Zeichen mit Escape-Sequenzen

        return result;
    }
};

class CTextToOctSeq : public CTextToCPP {
public:
    CTextToOctSeq(Variable vr = {}) : CTextToCPP(vr) {}

    string convert() const override {
        string result;
        // Implementiere hier die Ersetzung aller Zeichen mit oktalen Escape-Sequenzen

        return result;
    }
};

class CTextToHexSeq : public CTextToCPP {
public:
    CTextToHexSeq(Variable vr = {}) : CTextToCPP(vr) {}

    string convert() const override {
        string result;
        // Implementiere hier die Ersetzung aller Zeichen mit hexadezimalen Escape-Sequenzen

        return result;
    }
};

class CTextToRawHexSeq : public CTextToCPP {
public:
    CTextToRawHexSeq(Variable vr = {}) : CTextToCPP(vr) {}

    string convert() const override {
        string result;
        // Implementiere hier die Ersetzung aller Zeichen mit hexadezimalen Zeichen

        return result;
    }
};

