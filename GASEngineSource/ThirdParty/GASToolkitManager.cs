using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;
using System.IO;
using System.Runtime.InteropServices;
using Newtonsoft.Json.Linq;
using System.Text;
using System;

[StructLayout(LayoutKind.Explicit)]
public struct MESH_BIN_FILE_HEADER
{
	[FieldOffset(0)]
	public char FLAG0;

	[FieldOffset(1)]
	public char FLAG1;

	[FieldOffset(2)]
	public char FLAG2;

	[FieldOffset(3)]
	public char FLAG3;

	[FieldOffset(4)]
	public uint OBJECT_FILE_SIZE;

	[FieldOffset(8)]
	public uint ENDIAN_FLAG;

	[FieldOffset(12)]
	public uint VERSION;

	[FieldOffset(16)]
	public uint SECTION_TABLE_OFFSET;

	[FieldOffset(20)]
	public int PARENT_UNIQUE_ID;

	[FieldOffset(24)]
	public int UNIQUE_ID;

	[FieldOffset(28)]
	[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 12)]
	public string OBJECT_TYPE;
}

public struct GASBone
{
	public string name0;
	public string name1;
	public int uniqueID;
	public float[] inverseMatrix;
};

public class GASToolkitManager : MonoBehaviour {

	public Material mMaterial = null;
	public Texture2D mTexture = null;
	public Transform mRootNode = null;

    // Use this for initialization
    void Start()
    {
        StartCoroutine(GetStructure("http://localhost/tm/dog/dog.fbx.structure.json"));
    }

    IEnumerator GetStructure(string url)
    {
        using (UnityWebRequest www = UnityWebRequest.Get(url))
        {
            yield return www.Send();

            if (www.isError)
            {
                Debug.LogError("Network error.");
                yield break;
            }
            else if (www.responseCode == 200)
            {
                string results = www.downloadHandler.text;
                JObject structure = JObject.Parse(results);
                string root = structure["nodeTree"].ToString();
                analyseModelStructure_r(JObject.Parse(root), null);

				GameObject rootNode = GameObject.Find ("RootNode");
				setNodeTransform_r (rootNode);

				mRootNode = rootNode.transform;

				yield return StartCoroutine(GetMesh("http://localhost/tm/dog/Dog_0001.168.mesh.bin"));
				yield return StartCoroutine(GetAnimation("http://localhost/tm/dog/Take_001.166.animation.bin"));
				yield return StartCoroutine(GetTexture("http://localhost/tm/dog/Dog_0001_D_01.png"));
            }
        }
    }

    double DegreeToRadian(double angle)
    {
        return (Math.PI / 180.0) * angle;
    }

    //this function returns intrinsic Euler quaternion
    Quaternion getQuaternionFromEuler(float _ex, float _ey, float _ez, string order)
    {

        // http://www.mathworks.com/matlabcentral/fileexchange/
        // 	20696-function-to-convert-between-dcm-euler-angles-quaternions-and-euler-vectors/
        //	content/SpinCalc.m

        _ex = (float)DegreeToRadian(_ex);
        _ey = (float)DegreeToRadian(_ey);
        _ez = (float)DegreeToRadian(_ez);

        float c1 = (float)Math.Cos(_ex / 2);
        float c2 = (float)Math.Cos(_ey / 2);
        float c3 = (float)Math.Cos(_ez / 2);
        float s1 = (float)Math.Sin(_ex / 2);
        float s2 = (float)Math.Sin(_ey / 2);
        float s3 = (float)Math.Sin(_ez / 2);

        Quaternion quat = new Quaternion();

        if (order == "XYZ")
        {
            quat.x = s1 * c2 * c3 + c1 * s2 * s3;
            quat.y = c1 * s2 * c3 - s1 * c2 * s3;
            quat.z = c1 * c2 * s3 + s1 * s2 * c3;
            quat.w = c1 * c2 * c3 - s1 * s2 * s3;
        }
        else if (order == "YXZ")
        {
            quat.x = s1 * c2 * c3 + c1 * s2 * s3;
            quat.y = c1 * s2 * c3 - s1 * c2 * s3;
            quat.z = c1 * c2 * s3 - s1 * s2 * c3;
            quat.w = c1 * c2 * c3 + s1 * s2 * s3;
        }
        else if (order == "ZXY")
        {
            quat.x = s1 * c2 * c3 - c1 * s2 * s3;
            quat.y = c1 * s2 * c3 + s1 * c2 * s3;
            quat.z = c1 * c2 * s3 + s1 * s2 * c3;
            quat.w = c1 * c2 * c3 - s1 * s2 * s3;
        }
        else if (order == "ZYX")
        {
            quat.x = s1 * c2 * c3 - c1 * s2 * s3;
            quat.y = c1 * s2 * c3 + s1 * c2 * s3;
            quat.z = c1 * c2 * s3 - s1 * s2 * c3;
            quat.w = c1 * c2 * c3 + s1 * s2 * s3;
        }
        else if (order == "YZX")
        {
            quat.x = s1 * c2 * c3 + c1 * s2 * s3;
            quat.y = c1 * s2 * c3 + s1 * c2 * s3;
            quat.z = c1 * c2 * s3 - s1 * s2 * c3;
            quat.w = c1 * c2 * c3 - s1 * s2 * s3;

        }
        else if (order == "XZY")
        {
            quat.x = s1 * c2 * c3 - c1 * s2 * s3;
            quat.y = c1 * s2 * c3 - s1 * c2 * s3;
            quat.z = c1 * c2 * s3 + s1 * s2 * c3;
            quat.w = c1 * c2 * c3 + s1 * s2 * s3;
        }

        return quat;
    }

//	Quaternion getQuaternionFromEuler(float _ex, float _ey, float _ez, string order)
//	{
//		return Quaternion.AngleAxis (_ex, Vector3.right) * Quaternion.AngleAxis (_ey, Vector3.up) * Quaternion.AngleAxis (_ez, Vector3.forward);
//	}

	void setNodeTransform_r(GameObject node)
	{
		NeonateNode nn = node.GetComponent<NeonateNode> ();

		node.transform.localPosition = new Vector3 (nn.position.x, nn.position.y, -nn.position.z);
        //3DS MAX Euler Right XYZ EXTRINSIC -> Unity Euler LEFT XYZ EXTRINSIC
        //ZYX Intrinsic equals XYZ extrinsic Euler
        Quaternion rightQuaternion = getQuaternionFromEuler(-nn.rotationEuler.x, -nn.rotationEuler.y, nn.rotationEuler.z, "ZYX");
        node.transform.localRotation = rightQuaternion;

        node.transform.localScale = new Vector3(nn.scale.x, nn.scale.y, nn.scale.z);

        for (int i = 0; i < node.transform.childCount; ++i) 
		{
			Transform child = node.transform.GetChild (i);
			if (child != null) 
			{
				setNodeTransform_r (child.gameObject);
			}
		}
	}

	void analyseModelStructure_r(JObject JNode, GameObject parent)
    {
		int uniqueID = (int)JNode["uniqueID"];
		string name = (string)JNode["name"];
		string guid = (string)JNode["guid"];
		float[] translation = JNode["translation"].ToObject<float[]>();
		float[] rotation = JNode["rotation"].ToObject<float[]>();
		float[] scaling = JNode["scaling"].ToObject<float[]>();

		JObject MB_PROPS = JObject.Parse (JNode ["MB_PROPS"].ToString ());
		float[] ScalingPivot = MB_PROPS["ScalingPivot"].ToObject<float[]>();
		float[] ScalingOffset = MB_PROPS["ScalingOffset"].ToObject<float[]>();
		float[] RotationPivot = MB_PROPS["RotationPivot"].ToObject<float[]>();
		float[] RotationOffset = MB_PROPS["RotationOffset"].ToObject<float[]>();
		float[] PreRotation = MB_PROPS["PreRotation"].ToObject<float[]>();
		float[] PostRotation = MB_PROPS["PostRotation"].ToObject<float[]>();

		//GameObject gbNode = new GameObject ();
		GameObject gbNode = GameObject.CreatePrimitive(PrimitiveType.Sphere);

		if (parent != null) 
		{
			gbNode.transform.parent = parent.transform;
		}

		NeonateNode nn = gbNode.AddComponent<NeonateNode> ();
		gbNode.name = name;

		nn.name = name;
		nn.uniqueID = uniqueID;
		nn.guid = guid;
		nn.position = new Vector3 (translation [0], translation [1], translation [2]);
		nn.rotationEuler = new Vector3 (rotation [0], rotation [1], rotation [2]); //right hand to left hand euler
		nn.scale = new Vector3 (scaling [0], scaling [1], scaling [2]);

		nn.ScalingPivot = new Vector3 (ScalingPivot [0], ScalingPivot [1], ScalingPivot [2]);
		nn.ScalingOffset = new Vector3 (ScalingOffset [0], ScalingOffset [1], ScalingOffset [2]);
		nn.RotationPivot = new Vector3 (RotationPivot [0], RotationPivot [1], RotationPivot [2]);
		nn.RotationOffset = new Vector3 (RotationOffset [0], RotationOffset [1], RotationOffset [2]);
		nn.PreRotation = new Vector3 (PreRotation [0], PreRotation [1], PreRotation [2]);
		nn.PostRotation = new Vector3 (PostRotation [0], PostRotation [1], PostRotation [2]);

		JArray children = JArray.Parse (JNode["children"].ToString());
		for (int i = 0; i < children.Count; ++i) 
		{
			JToken childNode = children [i];
			analyseModelStructure_r ((JObject)childNode, gbNode);
		}
    }

    IEnumerator GetTexture(string url) 
	{
		Texture2D tex = new Texture2D (512, 512, TextureFormat.ARGB32, true);
		using (WWW www = new WWW (url)) 
		{
			yield return www;
			www.LoadImageIntoTexture (tex);

			mTexture = tex;

			if (mMaterial != null) 
			{
				mMaterial.mainTexture = mTexture;
			}
		}
	}

	IEnumerator GetAnimation(string url) 
	{
		using (UnityWebRequest www = UnityWebRequest.Get (url)) 
		{			
			yield return www.Send ();

			if (www.isError) 
			{
				Debug.LogError ("Network error.");
				yield break;
			} 
			else if (www.responseCode == 200)
			{
				byte[] results = www.downloadHandler.data;
                
                MemoryStream ms = new MemoryStream (results);
                using (BinaryReader br = new BinaryReader(ms))
                {
					GameObject rootNode = GameObject.Find("RootNode");
					Animation anim = rootNode.AddComponent<Animation>(); 

                    AnimationClip clip = new AnimationClip();
                    clip.legacy = true;

                    br.BaseStream.Seek(36, SeekOrigin.Begin);
                    int animationNameID = br.ReadInt32();

                    br.BaseStream.Seek(40, SeekOrigin.Begin);
                    float FPS = br.ReadSingle();

                    br.BaseStream.Seek(44, SeekOrigin.Begin);
                    float startFrame = br.ReadSingle();

                    br.BaseStream.Seek(48, SeekOrigin.Begin);
                    float endFrame = br.ReadSingle();

                    br.BaseStream.Seek(84, SeekOrigin.Begin);

                    int stringTableOffset = br.ReadInt32();
                    string[] stringTable = null;
                    if (stringTableOffset > 0)
                    {
                        br.BaseStream.Seek(stringTableOffset, SeekOrigin.Begin);
                        int stringCount = br.ReadInt32();
                        stringTable = new string[stringCount];
                        for (int stringIndex = 0; stringIndex < stringCount; ++stringIndex)
                        {
                            int stringSize = br.ReadInt32();
                            byte[] buffer = br.ReadBytes(stringSize + 1);
                            stringTable[stringIndex] = Encoding.UTF8.GetString(buffer);
                        }
                    }

                    string animationName = "Unknown";
                    if(stringTable != null)
                    {
                        animationName = stringTable[animationNameID];
                    }

                    br.BaseStream.Seek(16, SeekOrigin.Begin);
                    int sectionTableOffset = br.ReadInt32();

                    br.BaseStream.Seek(sectionTableOffset, SeekOrigin.Begin);
                    int animationNodeCount = br.ReadInt32();

                    //
                    uint sectionTableEntrySize = 28;
                    for (uint i = 0; i < animationNodeCount; ++i)
                    {
                        long offset = sectionTableOffset + 4 + sectionTableEntrySize * i;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint sectionType = br.ReadUInt32();

                        offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 4;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint dataAttribute = br.ReadUInt32();

                        offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 8;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint sectionOffset = br.ReadUInt32();

                        offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 12;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint sectionLength = br.ReadUInt32();

                        offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 16;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint curveCount = br.ReadUInt32();

                        offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 20;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint elementCount = br.ReadUInt32();

                        offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 24;
                        br.BaseStream.Seek(offset, SeekOrigin.Begin);
                        uint elementType = br.ReadUInt32();

                        br.BaseStream.Seek(sectionOffset, SeekOrigin.Begin);
                        uint objectNameIndex = br.ReadUInt32();

                        string objectName = stringTable[objectNameIndex];
                        GameObject targetObject = GameObject.Find(objectName);
                        string objectPath = getFullPath(targetObject);

                        br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);
                        uint objectUniqueID = br.ReadUInt32();

                        uint keyframeDataOffset = sectionOffset + 20; //20 bytes = nameIndex16 + uniqueID(4)

                        AnimationCurve eulerX = null;
                        AnimationCurve eulerY = null;
                        AnimationCurve eulerZ = null;

                        for (int j = 0; j < curveCount; ++j)
                        {
                            br.BaseStream.Seek(keyframeDataOffset + 0, SeekOrigin.Begin);
                            byte targetType = br.ReadByte();

                            br.BaseStream.Seek(keyframeDataOffset + 1, SeekOrigin.Begin);
                            byte keyValueType = br.ReadByte();

                            br.BaseStream.Seek(keyframeDataOffset + 2, SeekOrigin.Begin);
                            byte keyIndexType = br.ReadByte();

                            br.BaseStream.Seek(keyframeDataOffset + 3, SeekOrigin.Begin);
                            byte keySize = br.ReadByte();

                            br.BaseStream.Seek(keyframeDataOffset + 4, SeekOrigin.Begin);
                            uint keyCount = br.ReadUInt32();

                            br.BaseStream.Seek(keyframeDataOffset + 8, SeekOrigin.Begin);
                            uint animationDataSize = br.ReadUInt32();

                            br.BaseStream.Seek(keyframeDataOffset + 12, SeekOrigin.Begin);
                            uint propertyNameIndex = br.ReadUInt32();

                            keyframeDataOffset += 16;

                            string targetName = "";

                            //ANIMATION_POSITION_X = 0,
	                        //ANIMATION_POSITION_Y = 1,
	                        //ANIMATION_POSITION_Z = 2,
	                        //ANIMATION_ROTATION_EX = 3,
	                        //ANIMATION_ROTATION_EY = 4,
	                        //ANIMATION_ROTATION_EZ = 5,
	                        //ANIMATION_SCALING_X = 6,
	                        //ANIMATION_SCALING_Y = 7,
	                        //ANIMATION_SCALING_Z = 8,
	                        //ANIMATION_MORPHWEIGHT = 9,
	                        //ANIMATION_VISIBILITY = 10,
	                        //ANIMATION_ROTATION_QUATERNION = 11,
	                        //ANIMATION_CUSTOMIZED_PROPERTY = 12,
	                        //ANIMATION_TYPE_COUNT = 13
                            if ((targetType == 9 || targetType == 12) && stringTable != null)
                            {
                                targetName = stringTable[propertyNameIndex];
                            }
                            else
                            {
                                switch(targetType)
                                {
                                    case 0:
                                        targetName = "localPosition.x";
                                        break;
                                    case 1:
                                        targetName = "localPosition.y";
                                        break;
                                    case 2:
                                        targetName = "localPosition.z";
                                        break;
                                    case 3:
										targetName = "localEulerAngles.x";
                                        break;
                                    case 4:
										targetName = "localEulerAngles.y";
                                        break;
                                    case 5:
										targetName = "localEulerAngles.z";
                                        break;
									case 6:
										targetName = "localScale.x";
										break;
									case 7:
										targetName = "localScale.y";
										break;
									case 8:
										targetName = "localScale.z";
										break;
                                    default:
                                        break;
                                }
                            }

                            //
                            //KEYFRAME_INDEX_FLOAT = 0,
                            //KEYFRAME_INDEX_INT = 1,
                            //KEYFRAME_INDEX_UINT = 2,
                            //KEYFRAME_INDEX_SHORT = 3,
                            //KEYFRAME_INDEX_USHORT = 4,
                            //KEYFRAME_INDEX_BYTE = 5,
                            //KEYFRAME_INDEX_UBYTE = 6,
                            //KEYFRAME_INDEX_VARIABLE = 7
                            float[] frames = null;
                            if (keyIndexType == 0) //old float data
                            {
                                frames = new float[keyCount];
                                for (int k = 0; k < keyCount; ++k)
                                {
                                    br.BaseStream.Seek(keyframeDataOffset + 4 * k, SeekOrigin.Begin);
                                    frames[k] = br.ReadSingle() / FPS; //Unity animation use time as key frame index but GAS use frame index
                                }
                            }
                            else if (keyIndexType == 1) //mmd data
                            {
                                //There are 16 bytes(4 ints) in front of data which indicate data element size and data element count
                                //(data elements are frame index and frame data). The rest two bytes are zero placeholder.
                                //var keyframeIndexDataSize = dataView.getUint32(dataOffset + 0, true);
                                //var keyframeIndexCount = dataView.getUint32(dataOffset + 4, true);

                                //var frames = new Int32Array(dataView.buffer, dataOffset + 16, keyCount);
                            }

                            //KEY_FLOAT = 1,
                            //KEY_FLOAT_BEZIER_MMD = 2,
                            //KEY_QUATERNION_LINEAR = 9,
                            //KEY_QUATERNION_BEZIER_MMD = 10,
                            //KEY_INT = 20,
                            //KEY_UINT = 30,
                            //KEY_VARIABLE = 40
                            float[] values = null;
                            if (keyValueType == 1) //old float data
                            {
                                values = new float[keyCount];
                                for (int k = 0; k < keyCount; ++k)
                                {
                                    br.BaseStream.Seek(keyframeDataOffset + animationDataSize / 2 + 4 * k, SeekOrigin.Begin);
									if(targetName == "localPosition.z" || targetName == "localEulerAngles.x" || targetName == "localEulerAngles.y")
                                    	values[k] = -br.ReadSingle();
									else
										values[k] = br.ReadSingle();
                                }
                                //var values = new Float32Array(dataView.buffer, dataOffset + animationDataSize / 2, keyCount);
                            }
                            else if (keyValueType == 2) //KEY_FLOAT_BEZIER_MMD
                            {
                                //var offset4 = dataOffset + 16 + keyframeIndexDataSize * keyframeIndexCount;
                                //var keyframeDataSize = dataView.getUint32(offset4 + 0, true);
                                //var keyframeDataCount = dataView.getUint32(offset4 + 4, true);

                                //var values = [];
                                //offset4 += 16;
                                //for (int dataIndex = 0; dataIndex < keyframeDataCount; ++dataIndex)
                                //{
                                    //var value = dataView.getFloat32(offset4, true);
                                    //offset4 += 4;

                                    //var x1 = dataView.getUint8(offset4, true);
                                    //offset4 += 1;

                                    //var y1 = dataView.getUint8(offset4, true);
                                    //offset4 += 1;

                                    //var x2 = dataView.getUint8(offset4, true);
                                    //offset4 += 1;

                                    //var y2 = dataView.getUint8(offset4, true);
                                    //offset4 += 1;

                                    //var frameData = 
                                    //{
                                    //            "value": value,
                                    //            "mmd_bezier_x1": x1,
                                    //            "mmd_bezier_y1": y1,
                                    //            "mmd_bezier_x2": x2,
                                    //            "mmd_bezier_y2": y2,
                                    //};

                                    //values.push(frameData);
                                //}
                            }
                            else if (keyValueType == 10) //KEY_QUATERNION_BEZIER_MMD
                            {
                                //var offset5 = dataOffset + 16 + keyframeIndexDataSize * keyframeIndexCount;

                                //var keyframeDataSize = dataView.getUint32(offset5, true);
                                //offset5 += 4;

                                //var keyframeDataCount = dataView.getUint32(offset5, true);
                                //offset5 += 4;

                                //offset5 += 8; //ununsed

                                //var values = [];

                                //for (var dataIndex = 0; dataIndex < keyframeDataCount; ++dataIndex)
                                //{
                                //    var qx = dataView.getFloat32(offset5, true);
                                //    offset5 += 4;

                                //    var qy = dataView.getFloat32(offset5, true);
                                //    offset5 += 4;

                                //    var qz = dataView.getFloat32(offset5, true);
                                //    offset5 += 4;

                                //    var qw = dataView.getFloat32(offset5, true);
                                //    offset5 += 4;

                                //    var x1 = dataView.getUint8(offset5, true);
                                //    offset5 += 1;

                                //    var y1 = dataView.getUint8(offset5, true);
                                //    offset5 += 1;

                                //    var x2 = dataView.getUint8(offset5, true);
                                //    offset5 += 1;

                                //    var y2 = dataView.getUint8(offset5, true);
                                //    offset5 += 1;

                                //    var frameData = 
                                //    {
                                //        "value": new GASEngine.Quaternion(qx, qy, qz, qw),
                                //        "mmd_bezier_x1": x1,
                                //        "mmd_bezier_y1": y1,
                                //        "mmd_bezier_x2": x2,
                                //        "mmd_bezier_y2": y2,
                                //    };

                                //    values.push(frameData);
                                //}
                            }

                            if(frames != null && values != null)
                            {
                                AnimationCurve myCurve = new AnimationCurve();
                                myCurve.postWrapMode = WrapMode.ClampForever;
                                myCurve.preWrapMode = WrapMode.ClampForever;

                                for (int m = 0; m < keyCount; ++m)
                                {
                                    myCurve.AddKey(frames[m], values[m]);
                                }

                                if (targetName == "localEulerAngles.x")
                                    eulerX = myCurve;
                                else if (targetName == "localEulerAngles.y")
                                    eulerY = myCurve;
                                else if (targetName == "localEulerAngles.z")
                                    eulerZ = myCurve;
                                else
                                    clip.SetCurve(objectPath, typeof(Transform), targetName, myCurve);
                            }
                            //

                            keyframeDataOffset += animationDataSize;
                        }

                        transformEulerRightXYZToLeftQuaternion(clip, targetObject, objectPath, eulerX, eulerY, eulerZ);
                    }

                    //  
					anim.wrapMode = WrapMode.Loop;
                    anim.AddClip(clip, "test");
					anim.Play("test");

                    //AssetDatabase.SaveAssets
                }
            }
        }
		//
	}

    void transformEulerRightXYZToLeftQuaternion(
        AnimationClip clip, 
        GameObject targetObject,
        string objectPath,
        AnimationCurve eulerCurveX,
        AnimationCurve eulerCurveY, 
        AnimationCurve eulerCurveZ)
    {
		NeonateNode nn = targetObject.GetComponent<NeonateNode> ();
		float _ex = -nn.rotationEuler.x;
		float _ey = -nn.rotationEuler.y;
		float _ez = -nn.rotationEuler.z;

        if(eulerCurveX == null)
        {
            eulerCurveX = new AnimationCurve();
            eulerCurveX.AddKey(0.0f, _ex);
            eulerCurveX.preWrapMode = WrapMode.ClampForever;
            eulerCurveX.postWrapMode = WrapMode.ClampForever;
        }

        if (eulerCurveY == null)
        {
            eulerCurveY = new AnimationCurve();
            eulerCurveY.AddKey(0.0f, _ey);
            eulerCurveY.preWrapMode = WrapMode.ClampForever;
            eulerCurveY.postWrapMode = WrapMode.ClampForever;
        }

        if (eulerCurveZ == null)
        {
            eulerCurveZ = new AnimationCurve();
            eulerCurveZ.AddKey(0.0f, _ez);
            eulerCurveZ.preWrapMode = WrapMode.ClampForever;
            eulerCurveZ.postWrapMode = WrapMode.ClampForever;
        }

        AnimationCurve totalCurve = new AnimationCurve();
        int maxKeyCount = Math.Max(eulerCurveX.length, Math.Max(eulerCurveY.length, eulerCurveZ.length));
        for(int i = 0; i < maxKeyCount; ++i)
        {
            if (i < eulerCurveX.length)
                totalCurve.AddKey(eulerCurveX[i].time, 0.0f);

            if (i < eulerCurveY.length)
                totalCurve.AddKey(eulerCurveY[i].time, 0.0f);

            if (i < eulerCurveZ.length)
                totalCurve.AddKey(eulerCurveZ[i].time, 0.0f);
        }

        AnimationCurve quatX = new AnimationCurve();
        AnimationCurve quatY = new AnimationCurve();
        AnimationCurve quatZ = new AnimationCurve();
        AnimationCurve quatW = new AnimationCurve();

        for (int i = 0; i < totalCurve.length; ++i)
        {
            float t = totalCurve[i].time;
            float ex = eulerCurveX.Evaluate(t);
            float ey = eulerCurveY.Evaluate(t);
            float ez = eulerCurveZ.Evaluate(t);

            Quaternion rightQuaternion = getQuaternionFromEuler(ex, ey, ez, "ZYX");

            quatX.AddKey(t, rightQuaternion.x);
            quatY.AddKey(t, rightQuaternion.y);
            quatZ.AddKey(t, rightQuaternion.z);
            quatW.AddKey(t, rightQuaternion.w);
        }

        clip.SetCurve(objectPath, typeof(Transform), "localRotation.x", quatX);
        clip.SetCurve(objectPath, typeof(Transform), "localRotation.y", quatY);
        clip.SetCurve(objectPath, typeof(Transform), "localRotation.z", quatZ);
        clip.SetCurve(objectPath, typeof(Transform), "localRotation.w", quatW);

        clip.EnsureQuaternionContinuity();
    }

    string getFullPath(GameObject obj)
    {
        string path = obj.name;
		while(obj.transform.parent != null && obj.transform.parent.GetComponent<Animation>() == null)
        {
            obj = obj.transform.parent.gameObject;
            path = obj.name + "/" + path;
        }

		if(obj.transform.parent == null)
			path += "/";

        return path;
    }

	IEnumerator GetMesh(string url) 
	{  
		using(UnityWebRequest www = UnityWebRequest.Get(url))
		{			
			yield return www.Send();

			if (www.isError) 
			{
				Debug.LogError("Network error.");
				yield break;
			} 
			else if (www.responseCode == 200) 
			{
				byte[] results = www.downloadHandler.data;

				MemoryStream ms = new MemoryStream (results);

				Vector3[] vertices = null;
				Vector3[] normals = null;
				Vector2[] uv0 = null;
				BoneWeight[] weights = null;
				int[] indices = null;

				GASBone[] bones = null;
				Transform[] boneTransforms = null;
				Matrix4x4[] bindPoses = null;

				using (BinaryReader br = new BinaryReader (ms)) 
				{
					br.BaseStream.Seek(84, SeekOrigin.Begin);
					int stringTableOffset = br.ReadInt32 ();
					br.BaseStream.Seek(stringTableOffset, SeekOrigin.Begin);
					int stringCount = br.ReadInt32 ();
					string[] stringTable = new string[stringCount];
					for (int stringIndex = 0; stringIndex < stringCount; ++stringIndex)
					{
						int stringSize = br.ReadInt32 ();
						//byte[] buffer = new byte[stringSize];
						byte[] buffer = br.ReadBytes(stringSize + 1);
						stringTable [stringIndex] = Encoding.UTF8.GetString (buffer);

					}
//					int fileHeaderSize = Marshal.SizeOf (typeof(MESH_BIN_FILE_HEADER));
//					byte[] headerData = br.ReadBytes (fileHeaderSize);
//					GCHandle handle = GCHandle.Alloc (headerData, GCHandleType.Pinned);
//					MESH_BIN_FILE_HEADER header = (MESH_BIN_FILE_HEADER)Marshal.PtrToStructure (handle.AddrOfPinnedObject(), typeof(MESH_BIN_FILE_HEADER));
//					handle.Free ();
					br.BaseStream.Seek(16, SeekOrigin.Begin);
					uint sectionTableOffset = br.ReadUInt32 ();

					br.BaseStream.Seek(sectionTableOffset, SeekOrigin.Begin);
					uint sectionTableEntryCount = br.ReadUInt32 ();

					uint sectionTableEntrySize = 28;
					for (uint i = 0; i < sectionTableEntryCount; ++i) 
					{
						uint offset = sectionTableOffset + 4 + sectionTableEntrySize * i;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint sectionType = br.ReadUInt32 ();

						offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 4;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint dataAttribute = br.ReadUInt32 ();

						offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 8;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint sectionOffset = br.ReadUInt32 ();

						offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 12;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint sectionLength = br.ReadUInt32 ();

						offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 16;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint attributeCount = br.ReadUInt32 ();

						offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 20;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint elementCount = br.ReadUInt32 ();

						offset = sectionTableOffset + 4 + sectionTableEntrySize * i + 24;
						br.BaseStream.Seek(offset, SeekOrigin.Begin);
						uint elementType = br.ReadUInt32 ();

						//Position
						if (sectionType == 0) 
						{
							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);

							vertices = new Vector3[attributeCount];
							for (uint index = 0; index < attributeCount; ++index) 
							{
								float x = br.ReadSingle ();
								float y = br.ReadSingle ();
								float z = br.ReadSingle ();
								vertices [index].Set (x, y, -z);
							}
						}

						//Normal
						if (sectionType == 1) 
						{
							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);
							normals = new Vector3[attributeCount];
							for (uint index = 0; index < attributeCount; ++index) 
							{
								float x = br.ReadSingle ();
								float y = br.ReadSingle ();
								float z = br.ReadSingle ();
								normals [index].Set (x, y, -z);
							}
						}

						//BlendWeight
						if (sectionType == 11) 
						{
							if (weights == null)
							{
								weights = new BoneWeight[attributeCount];
							}

							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);
							for (uint index = 0; index < attributeCount; ++index) 
							{
								float w0 = br.ReadSingle ();
								float w1 = br.ReadSingle ();
								float w2 = br.ReadSingle ();
								float w3 = br.ReadSingle ();

								weights [index].weight0 = w0;
								weights [index].weight1 = w1;
								weights [index].weight2 = w2;
								weights [index].weight3 = w3;
							}
						}

						//BlendIndex
						if (sectionType == 12) 
						{
							if (weights == null)
							{
								weights = new BoneWeight[attributeCount];
							}

							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);
							for (uint index = 0; index < attributeCount; ++index) 
							{
								float i0 = br.ReadSingle ();
								float i1 = br.ReadSingle ();
								float i2 = br.ReadSingle ();
								float i3 = br.ReadSingle ();

								weights [index].boneIndex0 = (int)i0;
								weights [index].boneIndex1 = (int)i1;
								weights [index].boneIndex2 = (int)i2;
								weights [index].boneIndex3 = (int)i3;
							}
						}

						//UV
						if (sectionType == 7) 
						{
							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);
							uv0 = new Vector2[attributeCount];
							for (uint index = 0; index < attributeCount; ++index) 
							{
								float u = br.ReadSingle ();
								float v = br.ReadSingle ();
								uv0 [index].Set (u, v);
							}
						}

						//Index
						if (sectionType == 13) 
						{
							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);

							indices = new int[attributeCount*elementCount];
							for (uint index = 0; index < attributeCount; ++index) 
							{
								int v0 = br.ReadInt32 ();
								int v1 = br.ReadInt32 ();
								int v2 = br.ReadInt32 ();
								indices [index * 3 + 0] = v2;
								indices [index * 3 + 1] = v1;
								indices [index * 3 + 2] = v0;
							}
						}

						//Bone
						if (sectionType == 15) 
						{
							br.BaseStream.Seek(sectionOffset + 16, SeekOrigin.Begin);

							bones = new GASBone[attributeCount];
							boneTransforms = new Transform[attributeCount];
							bindPoses = new Matrix4x4[attributeCount];

							for (uint index = 0; index < attributeCount; ++index) 
							{
								int uniqueID = br.ReadInt32 ();
								int nameID0 = br.ReadInt32 ();
								int nameID1 = br.ReadInt32 ();
								int dummy = br.ReadInt32 ();

								bones[index].uniqueID = uniqueID;
								bones[index].name0 = stringTable[nameID0];
								bones[index].name1 = stringTable[nameID1];

								GameObject gbBone = GameObject.Find (bones [index].name0);
								if (gbBone != null) 
								{
									boneTransforms [index] = gbBone.transform;
								} 
								else 
								{
									Debug.Log ("Can not find bone, bind failed.");
								}

								float[] matrix = new float[16];
								for (uint elementIndex = 0; elementIndex < 16; elementIndex++) 
								{
									float element = br.ReadSingle ();
									matrix [elementIndex] = element;
                                }
								bones [index].inverseMatrix = matrix;
								bindPoses [index].SetRow(0, new Vector4 (matrix [0], matrix [1], -matrix [2], matrix [3]));
								bindPoses [index].SetRow(1, new Vector4 (matrix [4], matrix [5], -matrix [6], matrix [7]));
								bindPoses [index].SetRow(2, new Vector4 (-matrix [8], -matrix [9], matrix [10], -matrix [11]));
								bindPoses [index].SetRow(3, new Vector4 (matrix [12], matrix [13], -matrix [14], matrix [15]));
								//
							}

						}
					}

					br.Close ();

					if(vertices != null && indices != null)
					{
						GameObject gb = new GameObject ("Dog");
						MeshFilter mf = gb.AddComponent<MeshFilter> ();
						mf.mesh.vertices = vertices;
						mf.mesh.triangles = indices;
						mf.mesh.RecalculateBounds ();

						if (uv0 != null) 
						{
							mf.mesh.uv = uv0;
						}

						if (normals != null) 
						{
							mf.mesh.normals = normals;
						} 
						else 
						{
							mf.mesh.RecalculateNormals ();
						}

						if (weights != null) 
						{
							mf.mesh.boneWeights = weights;
						}

						if (bindPoses != null) 
						{
							mf.mesh.bindposes = bindPoses;
						}

						mMaterial = new Material (Shader.Find("Standard"));
						mMaterial.color = Color.white;

						if (mTexture != null) 
						{
							mMaterial.mainTexture = mTexture;
						}

						if (weights != null && bones != null) 
						{
							SkinnedMeshRenderer render = gb.AddComponent<SkinnedMeshRenderer> ();
							render.bones = boneTransforms;
							render.sharedMesh = mf.mesh;
							render.material = mMaterial;
						} 
						else 
						{
							MeshRenderer render = gb.AddComponent<MeshRenderer> ();
							render.material = mMaterial;
						}
					}
				}
		
				yield break;
			}
		}  
	}

	void debugHierarckies_r(Transform t)
	{
		for (int i = 0; i < t.childCount; ++i) 
		{
			Transform child = t.GetChild (i);
			if (child != null) 
			{
				Debug.DrawLine (t.position, child.position, Color.red, 0.5f, false);
				debugHierarckies_r (child);
			}
		}
	}
	
	// Update is called once per frame
	void Update ()
    {
		if (mRootNode != null) 
		{
			//debugHierarckies_r (mRootNode);
		}
	}
}
