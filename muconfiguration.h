#ifndef MUCONFIGURATION_H
#define MUCONFIGURATION_H

#include "all.h"


class MUConfiguration
{
public:
    MUConfiguration();
    ~MUConfiguration();
    static MUConfiguration& GetInstance();
    void Save();

    MU_CONF m_Configuration;
    MU_CONF m_cacheConfiguration;
};

#endif // MUCONFIGURATION_H
