#include "muconfiguration.h"

MUConfiguration::MUConfiguration()
{
    memset(&m_Configuration, 0 , sizeof(MU_CONF));
    memset(&m_cacheConfiguration, 0 , sizeof(MU_CONF));
}

MUConfiguration& MUConfiguration::GetInstance()
{
    static MUConfiguration Instance;
    return Instance;
}

MUConfiguration::~MUConfiguration()
{
}

void MUConfiguration::Save()
{
    memcpy(&m_Configuration, &m_cacheConfiguration, sizeof(MU_CONF));
}
