#include "ServerDriver.h"

using namespace vr;

EVRInitError ServerDriver::Init(vr::IVRDriverContext* pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    // InitDriverLog(vr::VRDriverLog());

    return VRInitError_None;
}

void ServerDriver::RunFrame()
{
    
}

void ServerDriver::Cleanup()
{

}