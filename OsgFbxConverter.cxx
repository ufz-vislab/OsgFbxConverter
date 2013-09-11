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
#include <OpenSG/OSGPrimitiveIterator.h>
#include <OpenSG/OSGTriangleIterator.h>
#include <fbxsdk.h>

OSG_USING_NAMESPACE
using namespace std;

OsgFbxConverter::OsgFbxConverter(NodePtr node, FbxScene* scene)
: _osgRoot(node), _scene(scene), _currentTransformNode(NULL)
{
	_currentNode = _scene->GetRootNode();
}

OsgFbxConverter::~OsgFbxConverter()
{
}

Action::ResultE OsgFbxConverter::onEntry(NodePtr& node)
{
	if(createTransformNode(node))
		return Action::Continue;
	else if (node->getCore()->getType().isDerivedFrom(Geometry::getClassType()))
	{
		FbxNode* newNode = FbxNode::Create(_scene, getName(node));
		_currentNode->AddChild(newNode);
		_currentNode = newNode;

		GeometryPtr geo = GeometryPtr::dcast(node->getCore());
		GeoPTypesPtr osgTypes = geo->getTypes();
		GeoPLengthsPtr osgLengths = geo->getLengths();
		GeoIndicesUI32Ptr osgIndices = GeoIndicesUI32Ptr::dcast(geo->getIndices());
		GeoPositions3fPtr osgVertices = GeoPositions3fPtr::dcast(geo->getPositions());
		GeoNormals3fPtr osgNormals = GeoNormals3fPtr::dcast(geo->getNormals());
		GeoColors3fPtr osgColors = GeoColors3fPtr::dcast(geo->getColors());
		GeoTexCoords2dPtr osgTexCoords = GeoTexCoords2dPtr::dcast(geo->getTexCoords());

		unsigned numTypes = osgTypes->size();
		unsigned numLengths = osgLengths->size();
		unsigned numIndices = osgIndices->size();
		unsigned numVertices = osgVertices->size();
		unsigned numNormals = osgNormals->size();
		unsigned numColors = 0;
		if(osgColors)
			numColors = osgColors->size();
		unsigned numTexCoords = 0;
		if(osgTexCoords)
			numTexCoords = osgTexCoords->size();

		cout << "Types: " << numTypes << "\nLengths: " << numLengths << "\nIndices: " << numIndices
			<< "\nVertices: " << numVertices << "\nNormals: " << numNormals << "\nColors: "
			<< numColors << "\nTex coords: " << numTexCoords << endl;

		string name("Geometry");
		if(getName(node))
			name = getName(node);
		FbxMesh* mesh = FbxMesh::Create(_scene, name.c_str());

		// -- Vertices --
		mesh->InitControlPoints(numVertices);
		FbxVector4* controlPoints = mesh->GetControlPoints();
		for(unsigned i = 0; i < numVertices; ++i)
		{
			Vec3f pos = osgVertices->getValue(i);
			controlPoints[i] = FbxVector4(-pos.x(), pos.z(), pos.y());
		}

		// Compute bounding box and translate all points so
		// that new bounding box equals (0, 0, 0)
		mesh->ComputeBBox();
		FbxDouble3 bbmin = mesh->BBoxMin;
		FbxDouble3 bbmax = mesh->BBoxMax;
		FbxDouble3 boundingBoxCenter((bbmax[0] + bbmin[0]) / 2,
									 (bbmax[1] + bbmin[1]) / 2,
									 (bbmax[2] + bbmin[2]) / 2);
		cout << "    Object Center: " << boundingBoxCenter[0] << ", " << boundingBoxCenter[1] << ", " << boundingBoxCenter[2] << endl;
		for (unsigned i = 0; i < numVertices; i++)
			controlPoints[i] = controlPoints[i] - boundingBoxCenter;

		FbxLayer* layer = mesh->GetLayer(0);
		if (layer == NULL)
		{
			mesh->CreateLayer();
			layer = mesh->GetLayer(0);
		}

		// -- Normals --
		FbxLayerElementNormal* layerElementNormal= FbxLayerElementNormal::Create(mesh, "");
		layerElementNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
		for (unsigned i = 0; i < numNormals; i++)
		{
			Vec3f normal = osgNormals->getValue(i);
			layerElementNormal->GetDirectArray().Add(FbxVector4(-normal.x(), normal.z(), normal.y()));
		}
		layer->SetNormals(layerElementNormal);

		// -- Texture coordinates --
		if(numTexCoords)
		{
			// Create UV for Diffuse channel.
			FbxLayerElementUV* lUVDiffuseLayer = FbxLayerElementUV::Create(mesh, "DiffuseUV");
			lUVDiffuseLayer->SetMappingMode(FbxLayerElement::eByControlPoint);
			lUVDiffuseLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
			layer->SetUVs(lUVDiffuseLayer, FbxLayerElement::eTextureDiffuse);
			for (unsigned i = 0; i < numNormals; i++)
			{
				Vec2f uv = osgTexCoords->getValue(i);
				lUVDiffuseLayer->GetDirectArray().Add(FbxVector2(uv.x(), uv.y()));
			}

			//Now we have set the UVs as eIndexToDirect reference and in eByPolygonVertex  mapping mode
			//we must update the size of the index array.
			lUVDiffuseLayer->GetIndexArray().SetCount(numVertices);
			for (unsigned i = 0; i < numVertices; i++)
				lUVDiffuseLayer->GetIndexArray().SetAt(i, i);
		}

		// -- Vertex Colors --
		if(numColors)
		{
			FbxGeometryElementVertexColor* vertexColorElement = mesh->CreateElementVertexColor();
			vertexColorElement->SetReferenceMode(FbxGeometryElement::eDirect);
			vertexColorElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			for (unsigned i = 0; i < numColors; i++)
			{
				Color3f color = osgColors->getValue(i);
				vertexColorElement->GetDirectArray().Add(FbxColor(color.red(), color.green(), color.blue(), 1.0));
			}
		}

		// -- Polygons --
		TriangleIterator pi;
		for(pi = geo->beginTriangles(); pi != geo->endTriangles(); ++pi)
		{
			mesh->BeginPolygon(-1, -1, -1, false);
			mesh->AddPolygon(pi.getPositionIndex(0));
			mesh->AddPolygon(pi.getPositionIndex(1));
			mesh->AddPolygon(pi.getPositionIndex(2));
			mesh->EndPolygon();
		}

		_currentNode->SetNodeAttribute(mesh);

		// Translate the object back to its originally calculated bounding box centre
		// This and the vertex translation aligns the bounding box centre and the
		// pivot point in Unity
		_currentNode->LclTranslation.Set(boundingBoxCenter);
	}
	else if (node->getCore()->getType().isDerivedFrom(Group::getClassType()))
	{
		if(checkVredIgnoreNodes(node))
			return Action::Skip;

		FbxNode* newNode = FbxNode::Create(_scene, getName(node));
		_currentNode->AddChild(newNode);
		_currentNode = newNode;
	}

	return Action::Continue;
}

Action::ResultE OsgFbxConverter::onLeave(NodePtr& node, Action::ResultE result)
{
	if (node->getCore()->getType().isDerivedFrom(Transform::getClassType()))
	{
		_currentNode = _currentTransformNode->GetParent();
		_currentTransformNode = _currentNode;
		if(_currentNode == NULL)
		{
			cout << "Finished traversal." << endl;
			return Action::Quit;
		}
	}
	if (node->getCore()->getType().isDerivedFrom(Geometry::getClassType()) ||
		node->getCore()->getType().isDerivedFrom(Group::getClassType()))
	{
		if(checkVredIgnoreNodes(node))
			return Action::Skip;
		_currentNode = _currentNode->GetParent();
	}

	return Action::Continue;
}

bool OsgFbxConverter::convert(std::string name)
{
	cout << "\n### Converting ###" << endl;

	traverse(_osgRoot,
		osgTypedMethodFunctor1ObjPtrCPtrRef<Action::ResultE, OsgFbxConverter, NodePtr>
			(this, &OsgFbxConverter::onEntry),
		osgTypedMethodFunctor2ObjPtrCPtrRef<Action::ResultE, OsgFbxConverter, NodePtr,
			Action::ResultE>(this, &OsgFbxConverter::onLeave));
	return true;
}

bool OsgFbxConverter::createTransformNode(NodePtr node)
{
	if (node->getCore()->getType().isDerivedFrom(Transform::getClassType()))
	{
		TransformPtr transform = TransformPtr::dcast(node->getCore());
		Matrix mat = transform->getMatrix();
		Vec3f translation, scaleFactor, center;
		Quaternion quat, scaleOrientation;
		mat.getTransform(translation, quat, scaleFactor, scaleOrientation, center);
		FbxNode* newNode = FbxNode::Create(_scene, getName(node));
		_currentNode->AddChild(newNode);
		_currentNode = newNode;
		_currentTransformNode = newNode;
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

bool OsgFbxConverter::checkVredIgnoreNodes(NodePtr node)
{
	string name("");
	if(getName(node))
		name = getName(node);
	return node->getParent() == _osgRoot && (
		name.find("VREDMaterialPool") != string::npos ||
		name.find("Front") != string::npos ||
		name.find("Perspective") != string::npos ||
		name.find("Side") != string::npos ||
		name.find("Top") != string::npos );
}
