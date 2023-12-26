#pragma once
#include "CommonStruct.h"
#include "MMD/PmdReader.h"

class SceneNode;

class IKParam
{
public:
	IKParam() {}
	~IKParam() {}

	bool enable;
	bool enableAngleLimitation;
	FBXSDK_NAMESPACE::FbxVector4 limitation;
	FBXSDK_NAMESPACE::FbxVector4 lowerAngles;
	FBXSDK_NAMESPACE::FbxVector4 upperAngles;
};

class IKChain
{
public:
	IKChain() {}
	~IKChain() {}

	SceneNode* effector;
	SceneNode* target;
	unsigned int interation;
	float maxAngle;
	std::vector<SceneNode*> chainList;
};

class LinkConstraint
{
public:
	LinkConstraint() 
	{
		childNode = NULL;
		parentNode = NULL;
		LinkConstraintAffectTranslation = false;
		LinkConstraintAffectRotation = false;
		LinkConstraintAffectLocal = false;
		LinkConstraintRatio = 0.0f;
		Priority = 0;
	}

	~LinkConstraint() {}

	SceneNode*	childNode;
	SceneNode*	parentNode;
	bool		LinkConstraintAffectTranslation;
	bool		LinkConstraintAffectRotation;
	bool		LinkConstraintAffectLocal;
	float		LinkConstraintRatio;
	int			Priority;
};

class PhysicsRigidBody
{
public:
	PhysicsRigidBody() {}
	~PhysicsRigidBody() {}

	std::string name;
	SceneNode* boneLinked;
	unsigned int groupIndex;
	unsigned int groupTarget;
	std::string shapeType;
	float width;
	float height;
	float depth;
	FbxVector4 position;
	FbxVector4 rotation;
	float mass;
	float positionDamping;
	float rotationDamping;
	float restitution;
	float friction;
	std::string dynamicType;
};

class PhysicsJoint
{
public:
	PhysicsJoint() {}
	~PhysicsJoint() {}

	std::string name;
	unsigned int rigidBodyIndexA;
	unsigned int rigidBodyIndexB; 
	FbxVector4 position;
	FbxVector4 rotation;
	FbxVector4 positionLowerLimitation;
	FbxVector4 positionUpperLimitation;
	FbxVector4 rotationLowerLimitation;
	FbxVector4 rotationUpperLimitation;
	FbxVector4 positionSpringStiffness;
	FbxVector4 rotationSpringStiffness;
};

class SceneNode
{
public:
	SceneNode();
	~SceneNode();

	std::string name;
	int			id;

	std::vector<IKChain*>*	ikChains;
	std::vector<LinkConstraint*>*	linkConstraints;
	std::vector<PhysicsRigidBody*>*		physicsRigidBodies;
	std::vector<PhysicsJoint*>*	physicsJoints;

	IKParam*	ikParam;
	FBXSDK_NAMESPACE::FbxVector4 position;
	FBXSDK_NAMESPACE::FbxVector4 rotationEuler;
	FBXSDK_NAMESPACE::FbxQuaternion rotationQuaternion;
	FBXSDK_NAMESPACE::FbxVector4 scale;
	FBXSDK_NAMESPACE::FbxAMatrix local;
	FBXSDK_NAMESPACE::FbxAMatrix world;
	
	int findChild(SceneNode* node);
	bool addChild(SceneNode* node);
	bool removeChild(SceneNode* node);
	void removeAllChildren();
	bool setParent(SceneNode* node);
	int getChildCount();
	SceneNode* getChild(int index);
	void update();

	SceneNode* parent;
private:	
	std::vector<SceneNode*> children;
};