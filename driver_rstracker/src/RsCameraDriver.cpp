#include "RsCameraDriver.h"
#include <Windows.h>

using namespace vr;

RsCameraDriver::RsCameraDriver()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

    m_sSerialNumber = "841512070493";
    m_sModelNumber = "D435i";
}

vr::EVRInitError RsCameraDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
{
    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, m_sSerialNumber.c_str());
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, m_sModelNumber.c_str());

    // Set icons
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{rstracker}realsense_off.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{rstracker}realsense_ready.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{rstracker}realsense_ready_alert.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{rstracker}realsense_ready.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{rstracker}realsense_ready_alert.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{rstracker}realsense_error.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{rstracker}realsense_standby.png");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{rstracker}realsense_ready_low.png");

    return VRInitError_None;
}

void RsCameraDriver::Deactivate()
{

}

void RsCameraDriver::EnterStandby()
{

}

void* RsCameraDriver::GetComponent(const char* pchComponentNameAndVersion)
{
    return nullptr;
}

void RsCameraDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{

}

vr::DriverPose_t RsCameraDriver::GetPose()
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

    pose.vecPosition[0] = cpX;
    pose.vecPosition[1] = cpY;
    pose.vecPosition[2] = cpZ;

    return pose;
}

void RsCameraDriver::RunFrame()
{
    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
    }
}

void RsCameraDriver::Cleanup()
{

}

std::string RsCameraDriver::GetSerialNumber()
{
    return m_sSerialNumber;
}