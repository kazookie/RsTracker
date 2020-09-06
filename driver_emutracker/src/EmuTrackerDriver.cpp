#include "EmuTrackerDriver.h"
#include <Windows.h>

using namespace vr;

EmuTrackerDriver::EmuTrackerDriver()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

    m_sSerialNumber = "841512070493";
    m_sModelNumber = "EmuTracker";
}

vr::EVRInitError EmuTrackerDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
{
    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str());
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{emutracker}/rendermodels/vr_tracker_emu_1_0");

    return VRInitError_None;
}

void EmuTrackerDriver::Deactivate()
{

}

void EmuTrackerDriver::EnterStandby()
{

}

void* EmuTrackerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
    return nullptr;
}

void EmuTrackerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{

}

vr::DriverPose_t EmuTrackerDriver::GetPose()
{
    HmdQuaternion_t quat;
    quat.w = 1;
    quat.x = 0;
    quat.y = 0;
    quat.z = 0;

    DriverPose_t pose = { 0 };
    // pose.poseIsValid = false;
    pose.poseIsValid = true;
    // pose.result = TrackingResult_Calibrating_OutOfRange;
    pose.result = TrackingResult_Running_OK;
    pose.deviceIsConnected = true;

    pose.qWorldFromDriverRotation = quat;
    pose.qDriverFromHeadRotation = quat;

    if ((GetAsyncKeyState(87) & 0x8000) != 0) {
        cpZ += -0.01;                                       //W
    }
    if ((GetAsyncKeyState(83) & 0x8000) != 0) {
        cpZ += 0.01;                                       //S
    }
    if ((GetAsyncKeyState(65) & 0x8000) != 0) {
        cpX += -0.01;                                       //A
    }
    if ((GetAsyncKeyState(68) & 0x8000) != 0) {
        cpX += 0.01;                                       //D
    }
    if ((GetAsyncKeyState(81) & 0x8000) != 0) {
        cpY += 0.01;                                       //Q
    }
    if ((GetAsyncKeyState(69) & 0x8000) != 0) {
        cpY += -0.01;                                       //E
    }
    if ((GetAsyncKeyState(82) & 0x8000) != 0) {
        cpX = 0;                                            //R
        cpY = 0;
        cpZ = 0;
    }

    pose.vecPosition[0] = cpX;
    pose.vecPosition[1] = cpY;
    pose.vecPosition[2] = cpZ;

    return pose;
}

void EmuTrackerDriver::RunFrame()
{
    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
    }
}

void EmuTrackerDriver::Cleanup()
{

}

std::string EmuTrackerDriver::GetSerialNumber()
{
    return m_sSerialNumber;
}