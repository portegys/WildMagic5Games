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
// File Version: 1.2 (2/19/11)


#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "MtlLoader.h"

class ObjLoader
{
public:
    ObjLoader (const string& path, const string& filename);
    ~ObjLoader ();

    enum ErrorCode
    {
        EC_SUCCESSFUL,
        EC_LOGFILE_OPEN_FAILED,
        EC_FILE_OPEN_FAILED,
        EC_NO_TOKENS,
        EC_TOO_FEW_VERTEX_TOKENS,
        EC_TOO_MANY_VERTEX_TOKENS,
		EC_TOO_FEW_TEXTURE_TOKENS,
		EC_TOO_MANY_TEXTURE_TOKENS,
		EC_NO_TEXTURES_SUPPLIED,
		EC_TOO_FEW_NORMAL_TOKENS,
		EC_TOO_MANY_NORMAL_TOKENS,
		EC_NO_NORMALS_SUPPLIED,
        EC_FAILED_TO_FIND_MATERIAL,
		EC_MTLLIB_MISSING_NAME,
		EC_MTLLIB_UNKNOWN_TOKEN,
		EC_TOO_FEW_MTL_TOKENS,
		EC_TOO_MANY_MTL_TOKENS,
        EC_INVALID_FACE_VERTEX,
		EC_TOO_FEW_FACE_TOKENS,
        EC_MAX_ERROR_CODES
    };

    class Float2
    {
    public:
		Float2 ()
			:
			x(0.0f),
			y(0.0f)
		{
		}
		
		Float2& operator+= (const Float2& coordinate)
		{
			x += coordinate.x;
			y += coordinate.y;
			
			return *this;
		}
		Float2& operator/= (const float number)
		{
			x /= number;
			y /= number;
			
			return *this;
		}
		
        float x, y;
    };

    class Float3
    {
    public:
		Float3 ()
			:
			x(0.0f),
			y(0.0f),
			z(0.0f)
		{
		}
		
		Float3& operator+= (const Float3& coordinate)
		{
			x += coordinate.x;
			y += coordinate.y;
			z += coordinate.z;
			
			return *this;
		}
		Float3& operator/= (const float number)
		{
			x /= number;
			y /= number;
			z /= number;
			
			return *this;
		}
		
        float x, y, z;
    };

    class Vertex
    {
    public:
        Vertex ()
            :
            PosIndex(-1),
            TcdIndex(-1),
            NorIndex(-1)
        {
        }

        bool operator< (const Vertex& vertex) const
        {
            if (PosIndex < vertex.PosIndex)
            {
                return true;
            }
            if (PosIndex > vertex.PosIndex)
            {
                return false;
            }
            if (TcdIndex < vertex.TcdIndex)
            {
                return true;
            }
            if (TcdIndex > vertex.TcdIndex)
            {
                return false;
            }
            if (NorIndex < vertex.NorIndex)
            {
                return true;
            }
            return false;
        }

        int PosIndex, TcdIndex, NorIndex;
    };

    class Face
    {
    public:
        vector<Vertex> Vertices;
    };

    class Mesh
    {
    public:
        int MtlIndex;
        vector<Face> Faces;
    };

    class Group
    {
    public:
        string Name;
        int PosStart, TcdStart, NorStart;
        vector<Mesh> Meshes;
    };

    inline ErrorCode GetCode () const;
	inline const bool GetHasNormals () const;
	inline const bool GetHasTextures () const;
    inline const vector<MtlLoader::Material>& GetMaterials () const;
    inline const vector<Group>& GetGroups () const;
    inline const vector<Float3>& GetPositions () const;
    inline const vector<Float2>& GetTCoords () const;
    inline const vector<Float3>& GetNormals () const;

private:
    void GetTokens (const string& line, vector<string>& tokens);
	unsigned char GetFaceIndices (const string& line, vector<string>& tokens);
    bool GetMaterialLibrary (const string& path, const vector<string>& tokens);
    bool GetDefaultGroup (const vector<string>& tokens);
    bool GetPosition (const vector<string>& tokens);
    bool GetTCoord (const vector<string>& tokens);
    bool GetNormal (const vector<string>& tokens);
    bool GetGroup (const vector<string>& tokens);
    bool GetMaterialAndMesh (const vector<string>& tokens);
    bool GetFace (const vector<string>& tokens);
	void InitDefaultGroup ();
	void InitGroup ();

    ErrorCode mCode;
    FILE* mLogFile;
    vector<MtlLoader::Material> mMaterials;
    int mCurrentGroup, mCurrentPos, mCurrentTcd, mCurrentNor;
    int mCurrentMtl, mCurrentMesh;
    vector<Group> mGroups;
    vector<Float3> mPositions;
    vector<Float2> mTCoords;
    vector<Float3> mNormals;
    static const char* msCodeString[EC_MAX_ERROR_CODES];
	
	//Flags
	bool mDefGroup;
	bool mHasNormals;
	bool mHasTextures;
	bool mMatRead;
	bool mRegGroup;
};

#include "ObjLoader.inl"

#endif
