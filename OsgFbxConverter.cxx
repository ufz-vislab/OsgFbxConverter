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
#include <OpenSG/OSGComponentTransform.h>

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

Action::ResultE OsgFbxConverter::onEntry(NodePtr& node)
{
	createTransformNode(node);
	if (node->getCore()->getType().isDerivedFrom(Geometry::getClassType()))
	{
		if (getName(node))
			cout << "Found a geometry core stored in " << getName(node) << endl;
		else
			cout << "Found a geometry core but node has no name" << endl;
	}

	return Action::Continue;
}

Action::ResultE OsgFbxConverter::onLeave(NodePtr& node, Action::ResultE result)
{
	if (node->getCore()->getType().isDerivedFrom(Transform::getClassType()))
	{
		if (getName(node))
			cout << "Found a geometry core stored in " << getName(node) << endl;
		else
			cout << "Found a geometry core but node has no name" << endl;
	}

	return Action::Continue;
}

bool OsgFbxConverter::convert(std::string name)
{
	cout << "Converting ..." << endl;

	traverse(_osgRoot,
		osgTypedMethodFunctor1ObjPtrCPtrRef<Action::ResultE, OsgFbxConverter, NodePtr>
			(this, &OsgFbxConverter::onEntry),
		osgTypedMethodFunctor2ObjPtrCPtrRef<Action::ResultE, OsgFbxConverter, NodePtr,
			Action::ResultE>(this, &OsgFbxConverter::onLeave));
	return true;
}

bool OsgFbxConverter::createTransformNode(OSG::NodePtr node)
{
	if (node->getCore()->getType().isDerivedFrom(Transform::getClassType()))
	{
		TransformPtr transform = TransformPtr::dcast(node->getCore());
		Matrix mat = transform->getMatrix();
		Vec3f translation, scaleFactor, center;
		Quaternion quat, scaleOrientation;
		mat.getTransform(translation, quat, scaleFactor, scaleOrientation, center);
		_currentNode = FbxNode::Create(_scene, getName(node));
		_currentNode->LclTranslation.Set(FbxDouble3(translation.x(), translation.y(), translation.z()));
		// TODO Rotation
		// _currentNode->LclRotation.Set(FbxDouble3(translation.x(), translation.y(), translation.z()));
		_currentNode->LclScaling.Set(FbxDouble3(scaleFactor.x(), scaleFactor.y(), scaleFactor.z()));
		cout << "Creating FbxNode with translation (" << translation.x() << ", " << translation.y()
			<< ", " << translation.z() << ") and scaling (" << scaleFactor.x() << ", "
			<< scaleFactor.y() << ", " << scaleFactor.z() << ")" << endl;
		return true;
	}
	return false;
}
