#ifndef PROFILE_H
#define PROFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globaltype.h"
#include "frame.h"

class Profile
{
public:
    Profile();
    ~Profile();
    static void ReadProfile(const char* pFilePath, MU_CONF& conf);
    static void SaveProfile(const char* pFilePath, const MU_CONF& conf);
};

#endif // PROFILE_H
