#ifndef OSGFBXCONVERTER_H
#define OSGFBXCONVERTER_H

#include <fbxsdk/fbxsdk_version.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGRefPtr.h>
#include <OpenSG/OSGTransform.h>

namespace FBXSDK_NAMESPACE {
	class FbxNode;
	class FbxScene;
	class FbxTexture;
	class FbxSurfacePhong;
	class FbxMesh;
	class FbxColor;
}

class OsgFbxConverter
{
public:
	OsgFbxConverter(OSG::NodePtr node, FBXSDK_NAMESPACE::FbxScene* scene);
	virtual ~OsgFbxConverter();

	bool convert(std::string name = "FBXObject");
	FBXSDK_NAMESPACE::FbxNode* getNode() const;

	void addUserProperty(const std::string name, const bool value);
	void addUserProperty(const std::string name, const float value);
	void addUserProperty(const std::string name, const int value);
	void addUserProperty(const std::string name, const std::string value);
	void addUserProperty(const std::string name, const FBXSDK_NAMESPACE::FbxColor value);

protected:

private:

	enum {NOT_GIVEN, PER_VERTEX, PER_CELL};

	//For the translation to OpenSG
	OSG::RefPtr<OSG::NodePtr> _osgRoot;
	OSG::RefPtr<OSG::TransformPtr> _osgTransform;
	FBXSDK_NAMESPACE::FbxNode* _node;
	FBXSDK_NAMESPACE::FbxScene* _scene;
	std::string _name;
};

#endif // OSGFBXCONVERTER_H
