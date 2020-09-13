#include <openvr_driver.h>
#include <librealsense2/rs.hpp>

#include <cubemos/engine.h>
#include <cubemos/skeleton_tracking.h>

class RsCameraDriver : public vr::ITrackedDeviceServerDriver
{
public:
    RsCameraDriver();
    virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId) override;
    virtual void Deactivate() override;
    virtual void EnterStandby() override;
    virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
    virtual vr::DriverPose_t GetPose() override;

    void RunFrame();
    void Cleanup();


    std::string GetSerialNumber();
private:
    vr::TrackedDeviceIndex_t m_unObjectId;
    vr::PropertyContainerHandle_t m_ulPropertyContainer;

    std::string m_sSerialNumber;
    std::string m_sModelNumber;

    double cpX = 0, cpY = 0, cpZ = 0;

    rs2::pipeline pipe;
    rs2::config cfg;
    rs2::context ctx;
};
