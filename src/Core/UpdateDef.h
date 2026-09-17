//--------------------------------------------------------------------------------
//
// Filename   : UpdateDef.h
// Written By : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __UPDATE_DEF_H__
#define __UPDATE_DEF_H__

#include "Types.h"

//--------------------------------------------------------------------------------
//
// The data types, their sizes and the limits used by the Smart Update classes
//
//--------------------------------------------------------------------------------

typedef DWORD FileSize_t;
const uint szFileSize = sizeof(FileSize_t);

typedef BYTE FilenameLen_t;
const uint szFilenameLen = sizeof(FilenameLen_t);
const uint maxFilename = 256;

// version
typedef WORD Version_t;
const uint szVersion = sizeof(Version_t);

// Parameter length type
typedef WORD ParameterLen_t;
const uint szParameterLen = sizeof(ParameterLen_t);

// Maximum number of parameters
const uint maxParams = 6;

// max parameter length
// The maximum length is the length of the filename (256) + the file size (a 15-digit integer).
const uint maxParameterLen = 256 + 15;

#endif
