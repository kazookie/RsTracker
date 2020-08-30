#include <openvr_driver.h>


class EmuTrackerDriver : public vr::ITrackedDeviceServerDriver
{
public:
    EmuTrackerDriver();
    virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId);

    void RunFrame();
    void Cleanup();
private:
    vr::TrackedDeviceIndex_t m_unObjectId;

};
