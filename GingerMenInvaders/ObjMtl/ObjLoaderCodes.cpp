// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.1.0 (2010/03/30)

// Modified by Jay Soyer
// Copyright 2011 All rights reserved.
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
//
// File Version: 1.0 (2/19/11).

#include "ObjLoader.h"

const char* ObjLoader::msCodeString[EC_MAX_ERROR_CODES] =
{
    "Loaded successfully",				// EC_SUCCESSFUL
    "Logfile open failed",				// EC_LOGFILE_OPEN_FAILED
    "File open failed",					// EC_FILE_OPEN_FAILED
    "No tokens",						// EC_NO_TOKENS
    "Too few vertex tokens",			// EC_TOO_FEW_VERTEX_TOKENS
    "Too many vertex tokens",			// EC_TOO_MANY_VERTEX_TOKENS
	"Too few texture tokens",			// EC_TOO_FEW_TEXTURE_TOKENS
	"Too many texture tokens",			// EC_TOO_MANY_TEXTURE_TOKENS
	"No texture coordinates supplied",	//	EC_NO_TEXTURES_SUPPLIED
	"Too few normal tokens",			// EC_TOO_FEW_NORMAL_TOKENS
	"Too many normal tokens",			// EC_TOO_MANY_NORMAL_TOKENS
	"No normal coordinates supplied",	// EC_NO_NORMALS_SUPPLIED
    "Failed to find material",			// EC_FAILED_TO_FIND_MATERIAL
	"Mtllib missing name label",		// EC_MTLLIB_MISSING_NAME
	"Mtllib contained unknown token",	// EC_MTLLIB_UNKNOWN_TOKEN
	"Usmtl has too few tokens",			// EC_TOO_FEW_MTL_TOKENS
	"Usmtl has extra tokens",			// EC_TOO_MANY_MTL_TOKENS
    "Invalid face vertex",				// EC_INVALID_FACE_VERTEX
	"Not enough tokens for face",		// EC_TOO_FEW_FACE_TOKENS
};
