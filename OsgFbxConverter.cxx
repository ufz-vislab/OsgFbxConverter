#include "OsgFbxConverter.h"

#include <fbxsdk.h>

OSG_USING_NAMESPACE

OsgFbxConverter::OsgFbxConverter(NodePtr node, FbxScene* scene)
: _osgRoot(node), _scene(scene)
{

}

OsgFbxConverter::~OsgFbxConverter()
{
}

bool OsgFbxConverter::convert(std::string name)
{
	return true;
}
