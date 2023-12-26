#include "stdafx.h"
#include "SceneNode.h"

SceneNode::SceneNode()
{
	name = "";
	id = -1;	

	ikChains = NULL;
	linkConstraints = NULL;
	physicsRigidBodies = NULL;
	physicsJoints = NULL;

	ikParam = NULL;
	position.Set(0.0, 0.0, 0.0, 1.0);
	rotationEuler.Set(0.0, 0.0, 0.0, 0.0);
	rotationQuaternion.Set(0.0, 0.0, 0.0, 1.0);
	scale.Set(1.0, 1.0, 1.0, 1.0);
	local.SetIdentity();
	world.SetIdentity();
	parent = NULL;
}

SceneNode::~SceneNode()
{

}

int SceneNode::findChild(SceneNode* node)
{
	int i = -1;
	for (size_t s = 0; s < this->children.size(); ++s)
	{
		if (node == this->children[s])
		{
			i = (int)s;
			break;
		}
	}

	return i;
}

bool SceneNode::addChild(SceneNode* node)
{
	int i = findChild(node);
	if (i == -1)
	{
		this->children.push_back(node);
		node->parent = this;
		return true;
	}
	else
	{
		FBXSDK_printf("%s", "Error: Failed to add node to children, because the node is already in the child list.\n");
		return false;
	}
}

bool SceneNode::removeChild(SceneNode* node)
{
	int i = findChild(node);
	if (i == -1)
	{
		FBXSDK_printf("%s", "Error: Failed to remove the node from children, because the node is not in the child list.\n");
		return false;
	}
	else
	{
		if (node->parent != this || node != this->children[i])
		{
			FBXSDK_printf("%s", "Error: Scene graph structure is corrupted.\n");
			return false;
		}

		node->parent = NULL;
		this->children.erase(this->children.begin() + i);
		return true;
	}
}

void SceneNode::removeAllChildren()
{
	for (size_t s = 0; s < this->children.size(); ++s)
	{
		SceneNode* child = this->children[s];
		child->setParent(NULL);
	}
}

bool SceneNode::setParent(SceneNode* node)
{
	if (node != this->parent)
	{
		if (this->parent != NULL)
		{
			this->parent->removeChild(this);
		}

		this->parent = node;

		if (this->parent != NULL)
		{
			this->parent->addChild(this);
		}

		return true;
	}
	else
	{
		FBXSDK_printf("%s", "Error: Failed to remove the node from children, because the node is not in the child list.\n");
		return false;
	}
}

int SceneNode::getChildCount()
{
	return (int)this->children.size();
}

SceneNode* SceneNode::getChild(int index)
{
	if (index >= 0 && index < (int)this->children.size())
	{
		return this->children[index];
	}
	else
	{
		return NULL;
	}
}

void makeRotationFromQST(
	const FBXSDK_NAMESPACE::FbxQuaternion& Q,
	const FBXSDK_NAMESPACE::FbxVector4& S,
	const FBXSDK_NAMESPACE::FbxVector4& T,
	FBXSDK_NAMESPACE::FbxAMatrix& outMatrix)
{
	//FBX MATRIX is row major in memory, row first in the low address.
	FBXSDK_NAMESPACE::FbxAMatrix test;
	test[1][3] = 5.0;
	test.SetT(FBXSDK_NAMESPACE::FbxVector4(7, 8, 9));
	double a = test.Get(1, 3);

	double x = Q[0], y = Q[1], z = Q[2], w = Q[3];
	double x2 = x + x, y2 = y + y, z2 = z + z;
	double xx = x * x2, xy = x * y2, xz = x * z2;
	double yy = y * y2, yz = y * z2, zz = z * z2;
	double wx = w * x2, wy = w * y2, wz = w * z2;

	outMatrix[0][0] = 1.0 - (yy + zz);
	outMatrix[1][0] = xy - wz;
	outMatrix[2][0] = xz + wy;

	outMatrix[0][1] = xy + wz;
	outMatrix[1][1] = 1.0 - (xx + zz);
	outMatrix[2][1] = yz - wx;

	outMatrix[0][2] = xz - wy;
	outMatrix[1][2] = yz + wx;
	outMatrix[2][2] = 1.0 - (xx + yy);

	//
	outMatrix[0][0] *= S[0];

	// last column
	outMatrix[0][3] = 0.0;
	outMatrix[1][3] = 0.0;
	outMatrix[2][3] = 0.0;

	// bottom row
	outMatrix[3][0] = T[0];
	outMatrix[3][1] = T[1];
	outMatrix[3][2] = T[2];
	outMatrix[3][3] = 1.0;
}

void SceneNode::update()
{
	local.SetT(position); //PMD AND PMX bind bone do not have any scale and rotation

	if (parent != NULL)
	{
		world = parent->world * local;
	}
	else
	{
		world = local;
	}

	for (size_t s = 0; s < children.size(); ++s)
	{
		children[s]->update();
	}
}