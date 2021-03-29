#include <openvr_driver.h>
#include "RsCameraDriver.h"

class RsTrackerDriver : public vr::ITrackedDeviceServerDriver
{
public:
    RsTrackerDriver(RsCameraDriver* rsCamera_Driver, int joint_Num, std::string deviceName);
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

    std::string device_name;


    double cpX = 0, cpY = 0, cpZ = 0;
    const static int bufsize = 2;
    double xb[bufsize]{ 0 };
    double yb[bufsize]{ 0 };
    double zb[bufsize]{ 0 };
    double factor = 0.7;

    int jointNum = 0;
    RsCameraDriver* rsCameraDriver;

};
