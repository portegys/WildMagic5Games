// Geometric Tools, LLC
// Copyright (c) 1998-2010
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.1.0 (2010/03/30)

// Modified by Jay Soyer.
// Copyright 2011 All rights reserved.
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
//
// File Version: 1.2 (2/19/11)


#include "ObjLoader.h"
#include <cassert>
#include <fstream>

// Disable Microsoft warning about unsafe functions (security).
#pragma warning(disable:4996)

//----------------------------------------------------------------------------
ObjLoader::ObjLoader (const string& path, const string& filename)
    :
    mCode(EC_SUCCESSFUL),
    mCurrentGroup(-1),
    mCurrentPos(-1),
    mCurrentTcd(-1),
    mCurrentNor(-1),
    mCurrentMtl(-1),
    mCurrentMesh(-1),
	mDefGroup(false),
	mHasNormals(false),
	mHasTextures(false),
	mMatRead(false),
	mRegGroup(false)
{
	
	
    mLogFile = fopen("ObjLogFile.txt", "wt");
    if (!mLogFile)
    {
        assert(false);
        mCode = EC_LOGFILE_OPEN_FAILED;
        return;
    }

    string filePath = path + filename;
    ifstream inFile(filePath.c_str());
    if (!inFile)
    {
        mCode = EC_FILE_OPEN_FAILED;
        fprintf(mLogFile, "%s: %s\n", msCodeString[mCode], filePath.c_str());
        fclose(mLogFile);
        assert(false);
        return;
    }

    string line;
    vector<string> tokens;

    while (!inFile.eof())
    {
        getline(inFile, line);

        // Lines too skip
        if (line == "") { continue; }
        if (line[0] == '#') { continue; }

        //Tokenize
		GetTokens(line, tokens);
        if (tokens.size() == 0) { mCode = EC_NO_TOKENS; break; }

        // mtllib
        if (GetMaterialLibrary(path, tokens)) { continue; }

        // g default
        if (GetDefaultGroup(tokens)) { continue; }

        // v x y z
        if (GetPosition(tokens)) { continue; }

        // vt x y
        if (GetTCoord(tokens)) { continue; }

        // vn x y z
        if (GetNormal(tokens)) { continue; }

        // g groupname
        if (GetGroup(tokens)) { continue; }

        // usemtl mtlname
        if (GetMaterialAndMesh(tokens)) { continue; }

        // f vertexList
        if (GetFace(tokens)) { continue; }

        //Ignore unknown symbols
		fprintf(mLogFile, "%s: %s\n", "Unrecognized Token: ", tokens[0].c_str());
		
		//If there was an error to halt parser
		if (mCode == EC_TOO_FEW_VERTEX_TOKENS || mCode == EC_TOO_FEW_TEXTURE_TOKENS ||
			mCode == EC_TOO_FEW_NORMAL_TOKENS)
		{
			fprintf(mLogFile, "%s: %s\n", msCodeString[mCode], line.c_str());
			fclose(mLogFile);
			inFile.close();
			assert(false);
		}	
    }
	
	//If no textures were supplied, make note of it
	if (!mHasTextures)
	{
		fprintf(mLogFile, "%s: %s\n", msCodeString[EC_NO_TEXTURES_SUPPLIED], line.c_str());
	}
	
	//if no normals were supplied, make note of it
	if (!mHasNormals)
	{
		fprintf(mLogFile, "%s: %s\n", msCodeString[EC_NO_NORMALS_SUPPLIED], line.c_str());
	}
	
	//Write out final result
	fprintf(mLogFile, "%s\n", msCodeString[EC_SUCCESSFUL]);
	fclose(mLogFile);
    inFile.close();
}
//----------------------------------------------------------------------------
ObjLoader::~ObjLoader ()
{
}
//----------------------------------------------------------------------------
void ObjLoader::GetTokens (const string & line, vector<string>& tokens)
{
    tokens.clear();

    string::size_type begin, end = 0;
    string token;

    while ((begin = line.find_first_not_of(" \t", end)) != string::npos)
    {
        end = line.find_first_of(" \t", begin);
        token = line.substr(begin, end-begin);
        tokens.push_back(token);
    }
}
//----------------------------------------------------------------------------
unsigned char ObjLoader::GetFaceIndices (const string &line, vector<string>& tokens)
{
	// 111 = v vt vn
	// 110 = v vt
	// 101 = v    vn
	// 100 = v
	// 000 = ERROR
	unsigned char bitflag = 0;	//Signals what indices were read
	
    tokens.clear();
    string::size_type begin, end = 0;
    string token;
	
	//Look for first vertex index
	begin = line.find_first_of("0123456789", end);
	if (begin == string::npos) { return bitflag; }  //No number found
	
	end = line.find_first_not_of("0123456789", begin);
	tokens.push_back(line.substr(begin, end-begin));
	bitflag = 100;
	
	//Look for second vertex index
    begin = line.find_first_of("0123456789", end);
	if (begin - end == 1)	//Vertex texture found
	{
		end = line.find_first_not_of("0123456789", begin);
		tokens.push_back(line.substr(begin, end-begin));
		bitflag += 10;
		
		//Look for third vertex index
		begin = line.find_first_of("0123456789", end);
		if (begin == string::npos) { return bitflag; }		//No vertex normal index
	}
	else if (begin == string::npos) { return bitflag; } //No vertex texture or normal index
	
	//Handle vertex normal index
	end = line.find_first_not_of("0123456789", begin);
	tokens.push_back(line.substr(begin, end-begin));
	bitflag += 1;
	
	return bitflag;
}

//----------------------------------------------------------------------------
bool ObjLoader::GetMaterialLibrary (const string& path,
    const vector<string>& tokens)
{
    if (tokens[0] == "mtllib")
    {
        if (tokens.size() == 1)
        {
            mCode = EC_MTLLIB_MISSING_NAME;
            return false;
        }
        if (tokens.size() > 2)
        {
            mCode = EC_MTLLIB_UNKNOWN_TOKEN;
            return false;
        }

        MtlLoader loader(path, tokens[1]);
        if (loader.GetCode() != MtlLoader::EC_SUCCESSFUL)
        {	//Had probelm with Material file, ignore and continue parsing
			mMatRead = false;
            mCode = EC_SUCCESSFUL;
        }
		else
		{
			mMatRead = true;
			mMaterials = loader.GetMaterials();
		}
		
		return true;
		
    }
	
    return false;
}
//----------------------------------------------------------------------------
bool ObjLoader::GetDefaultGroup (const vector<string>& tokens)
{
    if (tokens[0] == "g" && tokens[1] == "default")
    {	
		InitDefaultGroup();
            
		return true;
	}
	
	return false;
}
//----------------------------------------------------------------------------
void ObjLoader::InitDefaultGroup ()
{
	mDefGroup = true;
	mCurrentPos = (int)mPositions.size();
	mCurrentTcd = (int)mTCoords.size();
	mCurrentNor = (int)mNormals.size();	
}

//----------------------------------------------------------------------------
bool ObjLoader::GetPosition (const vector<string>& tokens)
{
    if (tokens[0] == "v")
    {
		//If no default group was detected by now, add one
		if (!mDefGroup) { InitDefaultGroup(); }
		
        if (tokens.size() < 4)
        {
            mCode = EC_TOO_FEW_VERTEX_TOKENS;
            return false;
        }
        if (tokens.size() > 4)
        {
            mCode = EC_TOO_MANY_VERTEX_TOKENS;
            return false;
        }

        Float3 pos;
        pos.x = (float)atof(tokens[1].c_str());
        pos.y = (float)atof(tokens[2].c_str());
        pos.z = (float)atof(tokens[3].c_str());
        mPositions.push_back(pos);
        return true;
    }
	
    return false;
}
//----------------------------------------------------------------------------
bool ObjLoader::GetTCoord (const vector<string>& tokens)
{
    if (tokens[0] == "vt")
    {
		//If no default group was detected by now, add one
		if (!mDefGroup) { InitDefaultGroup(); }
		
        if (tokens.size() < 3)
        {
            mCode = EC_TOO_FEW_TEXTURE_TOKENS;
            return false;
        }
        if (tokens.size() > 3)
        {
            //TODO.  Need to handle 3D texture coordinates.
            mCode = EC_TOO_MANY_TEXTURE_TOKENS;
        }

		mHasTextures = true;
		
        Float2 tcd;
        tcd.x = (float)atof(tokens[1].c_str());
        tcd.y = (float)atof(tokens[2].c_str());
        mTCoords.push_back(tcd);
        return true;
    }
	
    return false;
}
//----------------------------------------------------------------------------
bool ObjLoader::GetNormal (const vector<string>& tokens)
{
    if (tokens[0] == "vn")
    {
		//If no default group was detected by now, add one
		if (!mDefGroup) { InitDefaultGroup(); }
		
        if (tokens.size() < 4)
        {
            mCode = EC_TOO_FEW_NORMAL_TOKENS;
            return false;
        }
        if (tokens.size() > 4)
        {
            mCode = EC_TOO_MANY_NORMAL_TOKENS;
            return false;
        }

		mHasNormals = true;
		
        Float3 nor;
        nor.x = (float)atof(tokens[1].c_str());
        nor.y = (float)atof(tokens[2].c_str());
        nor.z = (float)atof(tokens[3].c_str());
        mNormals.push_back(nor);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
bool ObjLoader::GetGroup (const vector<string>& tokens)
{
    if (tokens[0] == "g")
    {
        mCurrentGroup = (int)mGroups.size();
        mGroups.push_back(Group());
        Group& group = mGroups[mCurrentGroup];
		
		if (tokens.size() == 1)
		{
			group.Name = "Forgot_a_Group_Name";
		} 
		else
		{	
			group.Name = tokens[1];
			for (int i = 2; i < (int)tokens.size(); ++i)
			{
				group.Name += tokens[i];
			}
		}
        
        group.PosStart = mCurrentPos;
        group.TcdStart = mCurrentTcd;
        group.NorStart = mCurrentNor;
        mCurrentPos = (int)mPositions.size();
        mCurrentTcd = (int)mTCoords.size();
        mCurrentNor = (int)mNormals.size();
        mRegGroup = true;
		
        return true;
    }
	
    return false;
}
//----------------------------------------------------------------------------
void ObjLoader::InitGroup ()
{
	vector<string> gTokens;
	gTokens.push_back("g");
	gTokens.push_back("No_Group_Found");
	
	GetGroup(gTokens);
}
//----------------------------------------------------------------------------
bool ObjLoader::GetMaterialAndMesh (const vector<string>& tokens)
{
	static unsigned int count = 0;
	
    if (tokens[0] == "usemtl")
    {
		if (!mRegGroup) { InitGroup(); }
        if (tokens.size() == 1)
        {
            mCode = EC_TOO_FEW_MTL_TOKENS;
            return false;
        }
        if (tokens.size() > 2)
        {
            mCode = EC_TOO_MANY_MTL_TOKENS;
            return false;
        }
		
		int i;
		
		if (mMatRead)	//If material file has been read
		{
			for (i = 0; i < (int)mMaterials.size(); ++i)
			{
				if (tokens[1] == mMaterials[i].Name)
				{
					break;
				}
			}
			if (i == (int)mMaterials.size())
			{
				mCode = EC_FAILED_TO_FIND_MATERIAL;
				return false;
			}
			mCurrentMtl = i;	
		}
        else	//Pretend we did read a material file
		{
			mCurrentMtl = count++;
			tokens[1] == "default_material";
		}

        Group& group = mGroups[mCurrentGroup];
        for (i = 0; i < (int)group.Meshes.size(); ++i)
        {
            if (group.Meshes[i].MtlIndex == mCurrentMtl)
            {
                break;
            }
        }
        if (i == (int)group.Meshes.size())
        {
            // Mesh with this material does not yet exist.
            group.Meshes.push_back(Mesh());
            group.Meshes.back().MtlIndex = mCurrentMtl;
        }

        mCurrentMesh = i;
		
        return true;
    }
	
    return false;
}
//----------------------------------------------------------------------------
bool ObjLoader::GetFace (const vector<string>& tokens)
{
    if (tokens[0] == "f")
    {
		if (!mRegGroup) { InitGroup(); }
        if (tokens.size() < 4)
        {
            // A face must have at least three vertices.
            mCode = EC_TOO_FEW_FACE_TOKENS;
            return false;
        }

        Group& group = mGroups[mCurrentGroup];
        Mesh& mesh = group.Meshes[mCurrentMesh];
        mesh.Faces.push_back(Face());
        Face& face = mesh.Faces.back();

        /* A vertex is one of the following:
        v/vt/vn/
		 v/vt/vn
        v/vt/
		 v/vt
		 v//vn
        v//
		 v/
		 v
		 where "/" is any none space, none numeric character 
		*/
		
        const int numVertices = (int)tokens.size() - 1;
        face.Vertices.resize(numVertices);
        for (int i = 0; i < numVertices; ++i)
        {            
			vector<string> nTokens;			
			switch (GetFaceIndices(tokens[i + 1], nTokens)) 
			{
				case 100:	//v
					face.Vertices[i].PosIndex = atoi(nTokens[0].c_str()) - 1;
					break;
					
				case 110:	//v vt
					face.Vertices[i].PosIndex = atoi(nTokens[0].c_str()) - 1;
					face.Vertices[i].TcdIndex = atoi(nTokens[1].c_str()) - 1;
					break;
					
				case 111:	//v vt vn
					face.Vertices[i].PosIndex = atoi(nTokens[0].c_str()) - 1;
					face.Vertices[i].TcdIndex = atoi(nTokens[1].c_str()) - 1;
					face.Vertices[i].NorIndex = atoi(nTokens[2].c_str()) - 1;
					break;
					
				case 101:	//v    vn
					face.Vertices[i].PosIndex = atoi(nTokens[0].c_str()) - 1;
					face.Vertices[i].NorIndex = atoi(nTokens[1].c_str()) - 1;
					break;
					
				default:	//Error
					mCode = EC_INVALID_FACE_VERTEX;
					return false;
			}
        }
		
        return true;
    }
	
    return false;
}
//----------------------------------------------------------------------------
