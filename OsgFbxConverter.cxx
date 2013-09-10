#include "OsgFbxConverter.h"

#include <OpenSG/OSGSimpleAttachments.h>
#include <OpenSG/OSGGeoFunctions.h>
#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGLineChunk.h>
#include <OpenSG/OSGMaterialChunk.h>
#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGPointChunk.h>
#include <OpenSG/OSGPolygonChunk.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGTwoSidedLightingChunk.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGBlendChunk.h>
#include <OpenSG/OSGGL.h>

#include <fbxsdk.h>

OSG_USING_NAMESPACE
using namespace std;

OsgFbxConverter::OsgFbxConverter(NodePtr node, FbxScene* scene)
: _osgRoot(node), _scene(scene)
{

}

OsgFbxConverter::~OsgFbxConverter()
{
}

Action::ResultE isGeometry(NodePtr& node)
{
	// this tests if the core is derived from geometry
	if (node->getCore()->getType().isDerivedFrom(Geometry::getClassType()))
		if (getName(node))
			cout << "Found a geometry core stored in " << getName(node) << endl;
		else
			cout << "Found a geometry core but node has no name" << endl;

	return Action::Continue;
}

bool OsgFbxConverter::convert(std::string name)
{
	cout << "Converting ..." << endl;
	traverse(_osgRoot,
		osgTypedFunctionFunctor1CPtrRef<Action::ResultE, NodePtr>(isGeometry));
	return true;
}
