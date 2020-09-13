#include <openvr_driver.h>
#include "RsCameraDriver.h"
#include "RsTrackerDriver.h"

class ServerDriver : public vr::IServerTrackedDeviceProvider
{
public:
    virtual vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext);
    virtual void Cleanup();
    virtual const char* const* GetInterfaceVersions() { return vr::k_InterfaceVersions; }
    virtual void RunFrame();
    virtual bool ShouldBlockStandbyMode() { return false; }
    virtual void EnterStandby() {}
    virtual void LeaveStandby() {}

private:
    RsCameraDriver* m_pRsCamera = nullptr;
    RsTrackerDriver *m_pTracker_foot_left = nullptr;
    RsTrackerDriver* m_pTracker_foot_right = nullptr;

};
