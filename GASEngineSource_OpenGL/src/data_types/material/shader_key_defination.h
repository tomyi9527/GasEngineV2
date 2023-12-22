#pragma once
namespace Dielectric {

enum kVSMacros {
    kVSMacros_POSITION = 1 << 0,
    kVSMacros_NORMAL = 1 << 1,
    kVSMacros_TANGENT = 1 << 2,
    kVSMacros_UV0 = 1 << 3,
    kVSMacros_UV1 = 1 << 4,
    kVSMacros_COLOR0 = 1 << 5,
    kVSMacros_COLOR1 = 1 << 6,
    kVSMacros_SKINNING = 1 << 7,
    kVSMacros_MORPHPOSITION = 1 << 8,
    kVSMacros_MORPHNORMAL = 1 << 9,
    kVSMacros_DEPTHBIAS = 1 << 10,
    kVSMacros_VERTEXTEXTURE = 1 << 11,
    kVSMacros_FLOATTEXTURE = 1 << 12,
    kVSMacros_WORLDNORMAL = 1 << 13,
    kVSMacros_DISPLACEMENT = 1 << 14,
    kVSMacros_DISPLACEMENTMAP = 1 << 15
};

enum kFSMacros {
    kFSMacros_ALBEDO = 1 << 0,
    kFSMacros_ALBEDOMAP = 1 << 1,
    kFSMacros_METALNESS = 1 << 2,
    kFSMacros_METALNESSMAP = 1 << 3,
    kFSMacros_SPECULARF0 = 1 << 4,
    kFSMacros_SPECULARF0MAP = 1 << 5,
    kFSMacros_SPECULAR = 1 << 6,
    kFSMacros_SPECULARMAP = 1 << 7,
    kFSMacros_ROUGHNESS = 1 << 8,
    kFSMacros_ROUGHNESSMAP = 1 << 9,
    kFSMacros_NORMAL = 1 << 10,
    kFSMacros_NORMALMAP = 1 << 11,
    kFSMacros_AO = 1 << 12,
    kFSMacros_AOMAP = 1 << 13,
    kFSMacros_CAVITY = 1 << 14,
    kFSMacros_CAVITYMAP = 1 << 15,
    kFSMacros_TRANSPARENCY = 1 << 16,
    kFSMacros_TRANSPARENCYMAP = 1 << 17,
    kFSMacros_EMISSIVE = 1 << 18,
    kFSMacros_EMISSIVEMAP = 1 << 19,
    kFSMacros_DIELECTRIC = 1 << 20,
    kFSMacros_ELECTRIC = 1 << 21,
    kFSMacros_ENVIRONMENTALLIGHTING = 1 << 22,
    kFSMacros_PUNCTUALLIGHTING = 1 << 23,
    kFSMacros_CUBEMAP = 1 << 24,
    kFSMacros_PANORAMA = 1 << 25,
    kFSMacros_MOBILE = 1 << 26,
    kFSMacros_OUTPUTALBEDO = 1 << 27,
    kFSMacros_OUTPUTNORMALS = 1 << 28,
    kFSMacros_OUTPUTLIT = 1 << 29,
    kFSMacros_HIGHLIGHTMASK = 1 << 30
};

}  // namespace Dielectric

namespace BlinnPhong {

enum kFSMacros {
    kFSMacros_ALBEDO = 1 << 0,
    kFSMacros_ALBEDOMAP = 1 << 1,
    kFSMacros_SPECULAR = 1 << 2,
    kFSMacros_SPECULARMAP = 1 << 3,
    kFSMacros_GLOSSINES = 1 << 4,
    kFSMacros_GLOSSINESMAP = 1 << 5,
    kFSMacros_DISPLACEMENT = 1 << 6,
    kFSMacros_DISPLACEMENTMAP = 1 << 7,
    kFSMacros_NORMAL = 1 << 8,
    kFSMacros_NORMALMAP = 1 << 9,
    kFSMacros_TRANSPARENCY = 1 << 10,
    kFSMacros_TRANSPARENCYMAP = 1 << 11,
    kFSMacros_EMISSIVE = 1 << 12,
    kFSMacros_EMISSIVEMAP = 1 << 13,

    kFSMacros_DIRECTIONALLIGHTING = 1 << 14,
    kFSMacros_SPOTLIGHTING = 1 << 15,
    kFSMacros_POINTLIGHTING = 1 << 16,
    kFSMacros_ENVIRONMENTALLIGHTING = 1 << 17,
    kFSMacros_CUBEMAP = 1 << 18,
    kFSMacros_PANORAMA = 1 << 19
};

}

namespace MatCap {

enum kFSMacros {
    kFSMacros_MATCAP = 1 << 0,
    kFSMacros_MATCAPMAP = 1 << 1,
    kFSMacros_NORMAL = 1 << 2,
    kFSMacros_NORMALMAP = 1 << 3,
    kFSMacros_TRANSPARENCY = 1 << 4,
    kFSMacros_TRANSPARENCYMAP = 1 << 5,
    kFSMacros_DISPLACEMENT = 1 << 6,
    kFSMacros_DISPLACEMENTMAP = 1 << 7,
};

}

namespace SkyBox {

enum kFSMacros {
    kFSMacros_SOLIDCOLOR = 1 << 0,
    kFSMacros_IMAGE = 1 << 1,
    kFSMacros_CUBEMAP = 1 << 2,
    kFSMacros_AMBIENT = 1 << 3
};

}