
#ifndef DEBUGHELPER_H
#define DEBUGHELPER_H

#include "../stdafx.h"

class DebugHelper
{
private:

public:
    DebugHelper();
    virtual ~DebugHelper();
    static void showVector4(FbxVector4);
    static void showQuaterion(FbxQuaternion);
};

#endif /* DEBUGHELPER_H */
