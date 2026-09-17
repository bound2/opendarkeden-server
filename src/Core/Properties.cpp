//--------------------------------------------------------------------------------
//
// Filename    : Properties.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "Properties.h"

#include <stdlib.h> // atoi()

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
const char Properties::Comment = '#';
const char Properties::Separator = ':';
const char* Properties::WhiteSpaces = " \t";


//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
Properties::Properties(){__BEGIN_TRY __END_CATCH}

Properties::Properties(const string& filename)
    : m_Filename(filename){__BEGIN_TRY __END_CATCH}


      //--------------------------------------------------------------------------------
      // destructor
      //--------------------------------------------------------------------------------
      Properties::~Properties() noexcept {
    // Delete every pair.
    m_Properties.clear();
}


//--------------------------------------------------------------------------------
// load from file
//--------------------------------------------------------------------------------
void Properties::load() {
    __BEGIN_TRY

    if (m_Filename.empty())
        throw Error("filename not specified");

    ifstream ifile(m_Filename.c_str(), ios::in);

    if (!ifile)
        throw FileNotExistException(m_Filename.c_str());

    while (true) {
        string line;
        getline(ifile, line);

        if (ifile.eof())
            break;

        // It is a comment line or an empty line, so skip it.
        if (line.size() == 0 || line[0] == Comment)
            continue;

        // Find the start of the key (the first character that is not white space).
        size_t key_begin = line.find_first_not_of(WhiteSpaces);

        // If key_begin is npos, no such character was found.
        // That is, the line is nothing but white space, so skip it.
        if (key_begin == string::npos)
            continue;

        // Find the separator that divides the key and the value.
        // find_last_not_of() is used rather than searching for sep from key_end,
        // so that key_end, the character just before sep, is found. ^^;
        size_t sep = line.find(Separator, key_begin);

        // If no Separator is found, treat it as a parse error.
        if (sep == string::npos)
            throw IOException("missing separator");

        // Find key_end, the character just before sep.
        size_t key_end = line.find_last_not_of(WhiteSpaces, sep - 1);

        // Find value_begin after sep.
        size_t value_begin = line.find_first_not_of(WhiteSpaces, sep + 1);

        // The key has no value; it is an empty line.
        if (value_begin == string::npos)
            throw IOException("missing value");

        // Find value_end, the last character that is not white space.
        // ( If value_begin is empty, value_end is empty too.)
        size_t value_end = line.find_last_not_of(WhiteSpaces);

        // Using key_begin,key_end and value_begin,value_end,
        // take the key and the value out of the line as substrings.
        string key = line.substr(key_begin, key_end - key_begin + 1);
        string value = line.substr(value_begin, value_end - value_begin + 1);

        // Register the property.
        setProperty(key, value);
    }

    ifile.close();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// save to file
//--------------------------------------------------------------------------------
void Properties::save() {
    __BEGIN_TRY

    if (m_Filename.empty())
        throw Error("filename not specified");

    ofstream ofile(m_Filename.c_str(), ios::out | ios::trunc);

    for (map<string, string, StringCompare>::iterator itr = m_Properties.begin(); itr != m_Properties.end(); itr++)
        ofile << itr->first << ' ' << Separator << ' ' << itr->second << endl;

    ofile.close();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get property
//--------------------------------------------------------------------------------
string Properties::getProperty(string key) const {
    __BEGIN_TRY

    string value;

    map<string, string, StringCompare>::const_iterator itr = m_Properties.find(key);

    if (itr != m_Properties.end())
        value = itr->second;
    else
        throw NoSuchElementException(key);

    return value;

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get property as int
//--------------------------------------------------------------------------------
int Properties::getPropertyInt(string key) const {
    __BEGIN_TRY

    return atoi(getProperty(key).c_str());

    __END_CATCH
}


//--------------------------------------------------------------------------------
// set property
//--------------------------------------------------------------------------------
void Properties::setProperty(string key, string value) {
    __BEGIN_TRY

    // If the key exists already, the value is overwritten.
    m_Properties[key] = value;

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string Properties::toString() const {
    __BEGIN_TRY

    StringStream msg;

    for (map<string, string, StringCompare>::const_iterator itr = m_Properties.begin(); itr != m_Properties.end();
         itr++) {
        msg << itr->first << " : " << itr->second << "\n";
    }

    if (msg.isEmpty())
        msg << "empty properties";

    return msg.toString();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// global variable definition
//--------------------------------------------------------------------------------
Properties* g_pConfig = NULL;
