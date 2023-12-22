#pragma once

#include <math.h>
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "utils/logger.h"

// 此处均表示 extrinsic 形式的euler
//   一般大写用于表示 extrinsic, 小写表示 intrinsic
enum kEulerMode {
    kEulerMode_XYZ,
    kEulerMode_YXZ,
    kEulerMode_ZXY,
    kEulerMode_ZYX,
    kEulerMode_YZX,
    kEulerMode_XZY
};

// all angle values in EulerAngle are radians
class EulerAngle {
 public:
    kEulerMode mode = kEulerMode_ZYX;
    glm::vec3 xyz;
    std::pair<glm::vec3, kEulerMode> GetValue() const {
        switch (mode) {
            case kEulerMode_XYZ:
                return {glm::vec3(xyz.x, xyz.y, xyz.z), mode};
            case kEulerMode_YXZ:
                return {glm::vec3(xyz.y, xyz.x, xyz.z), mode};
            case kEulerMode_ZXY:
                return {glm::vec3(xyz.z, xyz.x, xyz.y), mode};
            case kEulerMode_ZYX:
                return {glm::vec3(xyz.z, xyz.y, xyz.x), mode};
            case kEulerMode_YZX:
                return {glm::vec3(xyz.y, xyz.z, xyz.x), mode};
            case kEulerMode_XZY:
                return {glm::vec3(xyz.x, xyz.z, xyz.y), mode};
            default:
                return {glm::vec3(0.0f), mode};
        }
    }
    void SetXYZ(const glm::vec3& in_xyz, kEulerMode in_mode) { 
        mode = in_mode;
        xyz = in_xyz;
    }
    void SetValue(const glm::vec3& in, kEulerMode in_mode) {
        mode = in_mode;
        switch (mode) {
            case kEulerMode_XYZ:
                xyz = glm::vec3(in[0], in[1], in[2]);
                break;
            case kEulerMode_YXZ:
                xyz = glm::vec3(in[1], in[0], in[2]);
                break;
            case kEulerMode_ZXY:
                xyz = glm::vec3(in[1], in[2], in[0]);
                break;
            case kEulerMode_ZYX:
                xyz = glm::vec3(in[2], in[1], in[0]);
                break;
            case kEulerMode_YZX:
                xyz = glm::vec3(in[2], in[0], in[1]);
                break;
            case kEulerMode_XZY:
                xyz = glm::vec3(in[0], in[2], in[1]);
                break;
            default:
                xyz = glm::vec3(0.0f);
                break;
        }
    }
    glm::mat4 ToMatrix() const {
        auto angles = GetValue();
        switch (angles.second) {
            case kEulerMode_XYZ:
                return glm::eulerAngleXYZ(angles.first[0], angles.first[1], angles.first[2]);
            case kEulerMode_YXZ:
                return glm::eulerAngleYXZ(angles.first[0], angles.first[1], angles.first[2]);
            case kEulerMode_ZXY:
                return glm::eulerAngleZXY(angles.first[0], angles.first[1], angles.first[2]);
            case kEulerMode_ZYX:
                return glm::eulerAngleZYX(angles.first[0], angles.first[1], angles.first[2]);
            case kEulerMode_YZX:
                return glm::eulerAngleYZX(angles.first[0], angles.first[1], angles.first[2]);
            case kEulerMode_XZY:
                return glm::eulerAngleXZY(angles.first[0], angles.first[1], angles.first[2]);
            default:
                return glm::mat4(0.0f);
        }
        //// or say:
        //return glm::toMat4(ToQuat());
    }
    void FromMatrix(const glm::mat4& mat, kEulerMode in_mode) {
        mode = in_mode;
        glm::vec3 values(0.0f);
        if (mode == kEulerMode_XYZ) {
            glm::extractEulerAngleXYZ(mat, values[0], values[1], values[2]);
        } else if (mode == kEulerMode_YXZ) {
            glm::extractEulerAngleYXZ(mat, values[0], values[1], values[2]);
        } else if (mode == kEulerMode_ZXY) {
            glm::extractEulerAngleZXY(mat, values[0], values[1], values[2]);
        } else if (mode == kEulerMode_ZYX) {
            glm::extractEulerAngleZYX(mat, values[0], values[1], values[2]);
        } else if (mode == kEulerMode_YZX) {
            glm::extractEulerAngleYZX(mat, values[0], values[1], values[2]);
        } else if (mode == kEulerMode_XZY) {
            glm::extractEulerAngleXZY(mat, values[0], values[1], values[2]);
        }
        SetValue(values, mode);
    }
    glm::quat ToQuat() const {
        glm::quat ret(0.0f, 0.0f, 0.0f, 1.0f);
        // http://www.mathworks.com/matlabcentral/fileexchange/
        // 	20696-function-to-convert-between-dcm-euler-angles-quaternions-and-euler-vectors/
        //	content/SpinCalc.m
        float c0 = glm::cos(xyz.x / 2.0f);
        float c1 = glm::cos(xyz.y / 2.0f);
        float c2 = glm::cos(xyz.z / 2.0f);
        float s0 = glm::sin(xyz.x / 2.0f);
        float s1 = glm::sin(xyz.y / 2.0f);
        float s2 = glm::sin(xyz.z / 2.0f);

        if (mode == kEulerMode_XYZ) {
            ret.x = s0 * c1 * c2 + c0 * s1 * s2;
            ret.y = c0 * s1 * c2 - s0 * c1 * s2;
            ret.z = c0 * c1 * s2 + s0 * s1 * c2;
            ret.w = c0 * c1 * c2 - s0 * s1 * s2;
        } else if (mode == kEulerMode_YXZ) {
            ret.x = s0 * c1 * c2 + c0 * s1 * s2;
            ret.y = c0 * s1 * c2 - s0 * c1 * s2;
            ret.z = c0 * c1 * s2 - s0 * s1 * c2;
            ret.w = c0 * c1 * c2 + s0 * s1 * s2;
        } else if (mode == kEulerMode_ZXY) {
            ret.x = s0 * c1 * c2 - c0 * s1 * s2;
            ret.y = c0 * s1 * c2 + s0 * c1 * s2;
            ret.z = c0 * c1 * s2 + s0 * s1 * c2;
            ret.w = c0 * c1 * c2 - s0 * s1 * s2;
        } else if (mode == kEulerMode_ZYX) {
            ret.x = s0 * c1 * c2 - c0 * s1 * s2;
            ret.y = c0 * s1 * c2 + s0 * c1 * s2;
            ret.z = c0 * c1 * s2 - s0 * s1 * c2;
            ret.w = c0 * c1 * c2 + s0 * s1 * s2;
        } else if (mode == kEulerMode_YZX) {
            ret.x = s0 * c1 * c2 + c0 * s1 * s2;
            ret.y = c0 * s1 * c2 + s0 * c1 * s2;
            ret.z = c0 * c1 * s2 - s0 * s1 * c2;
            ret.w = c0 * c1 * c2 - s0 * s1 * s2;
        } else if (mode == kEulerMode_XZY) {
            ret.x = s0 * c1 * c2 - c0 * s1 * s2;
            ret.y = c0 * s1 * c2 - s0 * c1 * s2;
            ret.z = c0 * c1 * s2 + s0 * s1 * c2;
            ret.w = c0 * c1 * c2 + s0 * s1 * s2;
        }
        return ret;
    }
};

class Rotation {
 public:
    void FromEuler(kEulerMode mode, glm::vec3 values, bool is_degree = false) {
        EulerAngle angle;
        if (is_degree) {
            values = glm::radians(values);
        }
        angle.SetValue(values, mode);
        mat = angle.ToMatrix();
    }
    void FromMatrix(const glm::mat4& in_mat) { mat = glm::toMat4(glm::toQuat(in_mat)); }
    void FromMatrix(const glm::mat3& in_mat) { mat = glm::toMat4(glm::toQuat(in_mat)); }
    void FromQuat(const glm::quat& quat) { mat = glm::toMat4(quat); }

    glm::quat AsQuat() const { return glm::toQuat(mat); }
    glm::mat4 AsMat() const { return mat; }
    glm::mat3 AsMat3() const { return mat; }
    EulerAngle AsEuler(kEulerMode mode) const {
        EulerAngle ret;
        ret.FromMatrix(mat, mode);
        return ret;
    }

 protected:
    glm::mat4 mat;
};

// values are radians
inline glm::mat4 MatFromEulerXYZ(const glm::vec3& xyz, kEulerMode mode) {
    EulerAngle e;
    e.SetXYZ(xyz, mode);
    return e.ToMatrix();
}
inline glm::mat4 MatFromEuler(const glm::vec3& values, kEulerMode mode) { 
    EulerAngle e; 
    e.SetValue(values, mode);
    return e.ToMatrix();
}
inline EulerAngle EulerFromMatrix(const glm::mat4& mat, kEulerMode mode) {
    EulerAngle e;
    e.FromMatrix(mat, mode);
    return e;
}

template <typename ValueType = float, int N = 3>
glm::vec<N, ValueType, glm::defaultp> ChangeSign(const glm::vec<N, ValueType, glm::defaultp>& x,
                                                 const glm::vec<N, ValueType, glm::defaultp>& y) {
    glm::vec<N, ValueType, glm::defaultp> ret;
    for (int i = 0; i < N; ++i) {
        ret[i] = (y[i] < 0) ? -x[i] : x[i];
    }
    return ret;
}

template <typename ValueType = float>
glm::qua<ValueType, glm::defaultp> ChangeSign(const glm::qua<ValueType, glm::defaultp>& x,
                                              const glm::vec<4, ValueType, glm::defaultp>& y) {
    glm::qua<ValueType, glm::defaultp> ret;
    for (int i = 0; i < 4; ++i) {
        ret[i] = (y[i] < 0) ? -x[i] : x[i];
    }
    return ret;
}

// change sign only
inline glm::quat ScaleMulQuat(const glm::vec3& scale, const glm::quat& in) {
    glm::vec4 uniform(1.0, 1.0, 1.0, 1.0);
    glm::vec4 tmp(scale, 1.0);
    glm::vec4 sign = ChangeSign(uniform, tmp);
    tmp = glm::vec4(sign.y * sign.z, sign.x * sign.z, sign.x * sign.y, 1);
    return ChangeSign(in, tmp);
}

template <typename ValueType = float>
glm::vec<3, ValueType> DecomposeTranslation(const glm::mat<4, 4, ValueType>& matrix_local) {
    glm::vec3 tmp_scale;
    glm::quat tmp_quat;
    glm::vec3 tmp_translation;
    glm::vec3 tmp_skew;
    glm::vec4 tmp_perspective;
    glm::decompose(matrix_local, tmp_scale, tmp_quat, tmp_translation, tmp_skew, tmp_perspective);
    return tmp_translation;
}

template <typename ValueType = float>
glm::vec<3, ValueType> DecomposeScale(const glm::mat<4, 4, ValueType>& matrix_local) {
    glm::vec3 tmp_scale;
    glm::quat tmp_quat;
    glm::vec3 tmp_translation;
    glm::vec3 tmp_skew;
    glm::vec4 tmp_perspective;
    glm::decompose(matrix_local, tmp_scale, tmp_quat, tmp_translation, tmp_skew, tmp_perspective);
    return tmp_scale;
}

template <typename ValueType = float>
glm::quat DecomposeQuat(const glm::mat<4, 4, ValueType>& matrix_local) {
    glm::vec3 tmp_scale;
    glm::quat tmp_quat;
    glm::vec3 tmp_translation;
    glm::vec3 tmp_skew;
    glm::vec4 tmp_perspective;
    glm::decompose(matrix_local, tmp_scale, tmp_quat, tmp_translation, tmp_skew, tmp_perspective);
    return glm::conjugate(tmp_quat);
}

template <typename ValueType = float>
glm::vec<3, ValueType> DecomposeSkew(const glm::mat<4, 4, ValueType>& matrix_local) {
    glm::vec3 tmp_scale;
    glm::quat tmp_quat;
    glm::vec3 tmp_translation;
    glm::vec3 tmp_skew;
    glm::vec4 tmp_perspective;
    glm::decompose(matrix_local, tmp_scale, tmp_quat, tmp_translation, tmp_skew, tmp_perspective);
    return tmp_skew;
}

template <typename ValueType = float>
glm::vec<4, ValueType> DecomposePerspective(const glm::mat<4, 4, ValueType>& matrix_local) {
    glm::vec3 tmp_scale;
    glm::quat tmp_quat;
    glm::vec3 tmp_translation;
    glm::vec3 tmp_skew;
    glm::vec4 tmp_perspective;
    glm::decompose(matrix_local, tmp_scale, tmp_quat, tmp_translation, tmp_skew, tmp_perspective);
    return tmp_perspective;
}