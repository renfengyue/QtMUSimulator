#include "profile.h"

#include "all.h"

extern "C" void read_mu_config(MU_CONF *p_mu_conf, const char* conf_file);

Profile::Profile()
{


}
Profile::~Profile()
{


}

void Profile::ReadProfile(const char* pFilePath, MU_CONF& conf)
{
    if(pFilePath == NULL) return;

    read_mu_config(&conf, pFilePath);
}
void Profile::SaveProfile(const char* pFilePath, const MU_CONF& conf)
{
    if(pFilePath == NULL) return;
    if(conf.freq == 0) return;
}
