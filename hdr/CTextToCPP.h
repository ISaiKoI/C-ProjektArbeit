#ifndef CTEXTTOCPP_H
#define CTEXTTOCPP_H

#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <sys/stat.h> // For mkdir() on Unix-based systems

#ifdef _WIN32

#include <direct.h> // For _mkdir() on Windows

#define MKDIR(directoryPath) _mkdir(directoryPath)
#else
#define MKDIR(directoryPath) mkdir(directoryPath, 0777)
#endif

#include <sstream>
#include "Tags.h"

using namespace std;

class CTextToCPP {
protected:
    Variable variable;
    CTextToCPP *next = nullptr;
    CTextToCPP *head = this;

public:
    explicit CTextToCPP(const Variable &vr) : variable(vr) {}

    virtual ~CTextToCPP() = default;

    virtual void convert(int signPerLine) = 0;

    static bool createDirectory(const string &directoryPath) {
        int result = MKDIR(directoryPath.c_str());

        return result == 0 || errno == EEXIST;
    }

    static bool createDirectories(const string &path) {
        size_t pos;
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

        // Add trailing slash to the header directory path if necessary
        if (!global.getHeaderDir().empty() && global.getHeaderDir().back() != '\\') {
            global.setHeaderDir(global.getHeaderDir() + "\\");
        }

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
            sort(); // Sort variables
        }
        // Write to header file
        headerFile << "#ifndef _" << headerString << "_" << endl;
        headerFile << "#define _" << headerString << "_" << endl << endl;

        if (!global.getNameSpace().empty()) {
            headerFile << "namespace " << global.getNameSpace() << " {" << endl << endl;
        }

        CTextToCPP *temp = head;
        while (temp != nullptr) {
            headerFile << "/** " << temp->variable.getDoxygen() << " */" << endl;
            if (temp->variable.getSeq() == "RAWHEX")
                headerFile << "extern const char " << temp->variable.getVarname() << ";";
            else
                headerFile << "extern const char * const " << temp->variable.getVarname() << ";";
            if (temp->variable.getAddTextPos())
                headerFile << "\t\t//Variable declared in line " << temp->variable.getLineNum() << " of Input" << endl;
            else
                headerFile << endl;
            temp = temp->getNext();
        }

        if (!global.getNameSpace().empty()) {
            headerFile << "} // namespace " << global.getNameSpace() << endl;
        }

        headerFile << "#endif";
        headerFile.close();
        cout << "Files created successfully!" << endl;
    }

    void writeImplementation(const Global &global) {
        string type = global.getOutputType();
        transform(type.begin(), type.end(), type.begin(), ::tolower);
        string sourceFileName = global.getSourceDir() + global.getOutputFilename() + "." + type;

        // Create directories if they don't exist
        if (!createDirectories(global.getSourceDir())) {
            return;
        }

        ofstream sourceFile(sourceFileName);

        if (!sourceFile.is_open()) {
            cout << "Failed to create source file: " << sourceFileName << endl;
            return;
        }
        // Write to source file
        sourceFile << "#include \"" << global.getOutputFilename() << ".h\"" << endl << endl;
        if (!global.getNameSpace().empty()) {
            sourceFile << "namespace " << global.getNameSpace() << " {" << endl << endl;
        }

        CTextToCPP *temp = head;
        while (temp != nullptr) {
            if (temp->variable.getSeq() == "RAWHEX")
                sourceFile << "const char " << temp->variable.getVarname() << " = {" << endl;
            else
                sourceFile << "const char * const " << temp->variable.getVarname() << " = {" << endl;
            sourceFile << temp->variable.getConvertContent() << "};\n" << endl;
            if (temp->variable.getAddTextSegment()) {
                sourceFile << "/*\nOriginal text from variable section '" << temp->variable.getVarname() << "'" << endl
                           << endl;
                sourceFile << temp->variable.getStringContent() << endl << "*/" << endl << endl;
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
        vector<CTextToCPP *> objects(count);
        vector<std::string> varnames(count);

        // Traverse the linked list and store the objects and variable names in the arrays
        current = this;
        int i = 0;
        while (current != nullptr) {
            objects[i] = current;
            varnames[i] = current->variable.getVarname();
            current = current->getNext();
            i++;
        }

        // Sort the objects and variable names vectors based on the variable name
        std::sort(varnames.begin(), varnames.end());
        std::sort(objects.begin(), objects.end(), [](CTextToCPP *a, CTextToCPP *b) {
            return a->variable.getVarname() < b->variable.getVarname();
        });

        // Update the next pointers based on the sorted order
        for (int j = 0; j < count - 1; j++) {
            objects[j]->setNext(objects[j + 1]);
        }
        objects[count - 1]->setNext(nullptr);

        // Save the new head object
        CTextToCPP *newHead = objects[0];

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
            CTextToCPP *nxt = current->getNext();
            delete current;
            current = nxt;
        }

        // Reset head and next pointers
        head = nullptr;
        next = nullptr;
    }

    void createFiles(const Global &global) {
        this->writeDeclaration(global);
        this->writeImplementation(global);
        this->clear();
    }

    // Getter
    CTextToCPP *getNext() {
        return next;
    }

    // Setters
    void addElement(CTextToCPP *nxt) {
        this->next = nxt;
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
    explicit CTextToEscSeq(Variable &vr) : CTextToCPP(vr) {}

    static string replaceString(const string &input, const string &target, const string &replacement) {
        string result = input;
        size_t startPos = 0;
        while ((startPos = result.find(target, startPos)) != string::npos) {
            result.replace(startPos, target.length(), replacement);
            startPos += replacement.length();
        }
        return result;
    }

    void convert(int signPerLine) override {
        ostringstream encoded;
        string encodedString;
        int lineLength = 0;
        string nl = variable.getNl();
        string input = variable.getStringContent();
        input = replaceString(input, "\\", "\\\\"); // Replace chars which need to be escaped
        input = replaceString(input, "\"", "\\\"");
        for (size_t i = 0; i < input.size(); ++i) {
            char c = input[i];
            if (i == input.size() - 1) {
                continue;
            } else {
                if (lineLength == 0) {
                    encoded << "\"";
                }
                if (c == '\n' || c == '\r') {   // Replace newline depending on nl
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
                if (signPerLine > 0 && lineLength == signPerLine) {
                    encoded << "\" \\\n";
                    lineLength = 0;
                }
            }
        }
        encoded << "\"";
        encodedString = encoded.str();
        variable.setConvertedString(encodedString);
    }
};

class CTextToOctSeq : public CTextToCPP {
public:
    explicit CTextToOctSeq(Variable &vr) : CTextToCPP(vr) {}

    void convert(int signPerLine) override {
        ostringstream encoded;
        string encodedString;
        unsigned int lineLength = 0;
        string input = variable.getStringContent();
        for (char c: input) {
            if (lineLength == 0) {
                encoded << "\"";
            }
            ostringstream octEncoded;
            octEncoded << "\\" << oct << static_cast<int>(c);
            unsigned int checkLength = lineLength + octEncoded.str().length();
            if (checkLength > signPerLine) {
                encoded << "\" \\\n";
                lineLength = 0;
            } else {
                encoded << octEncoded.str();
                lineLength += octEncoded.str().length();
            }
            if (signPerLine > 0 && lineLength == signPerLine) {
                encoded << "\" \\\n";
                lineLength = 0;
            }
        }
        encoded << "\"";
        encodedString = encoded.str();
        variable.setConvertedString(encodedString);
    }
};

class CTextToHexSeq : public CTextToCPP {
public:
    explicit CTextToHexSeq(Variable &vr) : CTextToCPP(vr) {}

    void convert(int signPerLine) override {
        ostringstream encoded;
        string encodedString;
        unsigned int lineLength = 0;
        string input = variable.getStringContent();
        for (char c: input) {
            if (lineLength == 0) {
                encoded << "\"";
            }
            ostringstream hexEncoded;
            hexEncoded << "\\x";
            if (static_cast<int>(c) < 16) {
                hexEncoded << "0";
            }
            hexEncoded << hex << static_cast<int>(c);

            unsigned int checkLength = lineLength + hexEncoded.str().length();
            if (checkLength > signPerLine) {
                encoded << "\" \\\n";
                lineLength = 0;
            } else {
                encoded << hexEncoded.str();
                lineLength += hexEncoded.str().length();
            }
            if (signPerLine > 0 && lineLength == signPerLine) {
                encoded << "\" \\\n";
                lineLength = 0;
            }
        }
        encoded << "\"";
        encodedString = encoded.str();
        variable.setConvertedString(encodedString);
    }
};

class CTextToRawHexSeq : public CTextToCPP {
public:
    explicit CTextToRawHexSeq(Variable &vr) : CTextToCPP(vr) {}

    void convert(int signPerLine) override {
        variable.setVarname(variable.getVarname() + "[]");
        ostringstream encoded;
        string encodedString;
        unsigned int lineLength = 0;
        string input = variable.getStringContent();
        for (char c: input) {
            ostringstream rawHexEncoded;
            rawHexEncoded << "0x" << hex << static_cast<int>(c) << ", ";
            unsigned int checkLength = lineLength + rawHexEncoded.str().length();
            if (checkLength > signPerLine) {
                encoded << " \\\n";
                lineLength = 0;
            } else {
                encoded << rawHexEncoded.str();
                lineLength += rawHexEncoded.str().length();
            }
            if (signPerLine > 0 && lineLength == signPerLine) {
                encoded << " \\\n";
                lineLength = 0;
            }
        }
        encodedString = encoded.str();
        encodedString.erase(encodedString.length() - 2);
        this->variable.setConvertedString(encodedString);
    }
};

#endif
