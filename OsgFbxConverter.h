#ifndef OSGFBXCONVERTER_H
#define OSGFBXCONVERTER_H

#include <fbxsdk.h>
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

	void addUserProperty(const std::string name, const bool value);
	// void addUserProperty(const std::string name, const float value);
	// void addUserProperty(const std::string name, const int value);
	// void addUserProperty(const std::string name, const std::string value);
	// void addUserProperty(const std::string name, const FBXSDK_NAMESPACE::FbxColor value);

protected:
	OSG::Action::ResultE onEntry(OSG::NodePtr& node);
	OSG::Action::ResultE onLeave(OSG::NodePtr& node, OSG::Action::ResultE result);
	FBXSDK_NAMESPACE::FbxNode* _currentNode;
	FBXSDK_NAMESPACE::FbxNode* _currentTransformNode;

private:
	bool createTransformNode(OSG::NodePtr node);
	bool checkVredIgnoreNodes(OSG::NodePtr node);
	unsigned geometryCounter;

	enum {NOT_GIVEN, PER_VERTEX, PER_CELL};

	//For the translation to OpenSG
	OSG::RefPtr<OSG::NodePtr> _osgRoot;
	FBXSDK_NAMESPACE::FbxScene* _scene;
};

#if FBX_VERSION_MAJOR>2014
FBXSDK_NAMESPACE::FbxPropertyFlags::EFlags getUserPropertyFlag();
#else
FBXSDK_NAMESPACE::FbxPropertyAttr::EFlags getUserPropertyFlag();
#endif

#endif // OSGFBXCONVERTER_H
