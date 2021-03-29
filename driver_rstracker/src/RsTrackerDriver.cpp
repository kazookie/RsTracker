#include "RsTrackerDriver.h"
#include <Windows.h>
#include <cstdio>
#include <cmath>

using namespace vr;

RsTrackerDriver::RsTrackerDriver(RsCameraDriver* rsCamera_Driver, int joint_Num, std::string deviceName)
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

    m_sSerialNumber = "841512070493";
    m_sModelNumber = "EmuTracker";

    rsCameraDriver = rsCamera_Driver;
    jointNum = joint_Num;

    device_name = deviceName;
}

vr::EVRInitError RsTrackerDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
{
    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, "841512070493_Tracker_foot");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "{rstracker}/rendermodels/vr_tracker_emu_1_0");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "Tracker_foot");

    return VRInitError_None;
}

void RsTrackerDriver::Deactivate()
{

}

void RsTrackerDriver::EnterStandby()
{

}

void* RsTrackerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
    return nullptr;
}

void RsTrackerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{

}

vr::DriverPose_t RsTrackerDriver::GetPose()
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

    cmPoint point3d = rsCameraDriver->GetPoint3d(jointNum);
    
    
    if ((double)point3d.point3d[2] > 0.5){

        double x_raw = (double)point3d.point3d[0];
        double y_raw = (double)point3d.point3d[1];
        double z_raw = (double)point3d.point3d[2];


        if (x_raw != -1 && y_raw != -1 && x_raw != 0) {

            double x = x_raw * -1;
            double y = y_raw * -1 + 0.68;
            double z = z_raw - 2.0;

            // check spike
            double x_diff = std::abs(x) - std::abs(xb[bufsize - 2]);
            double y_diff = std::abs(y) - std::abs(yb[bufsize - 2]);

            double x_lpf = factor * xb[bufsize - 2] + (1 - factor) * x;
            double y_lpf = factor * yb[bufsize - 2] + (1 - factor) * y;
            double z_lpf = factor * zb[bufsize - 2] + (1 - factor) * z;


            // calc sma
            for (int i = 0; i < bufsize - 1; i++) {
                xb[i] = xb[i + 1];
                yb[i] = yb[i + 1];
                zb[i] = zb[i + 1];
            }
            xb[bufsize - 1] = x_lpf;
            yb[bufsize - 1] = y_lpf;
            zb[bufsize - 1] = z_lpf;

            pose.vecPosition[0] = x_lpf;
            pose.vecPosition[1] = y_lpf;
            pose.vecPosition[2] = z_lpf;
            return pose;
        }
    }
//    double sum_x = 0, sum_y = 0, sum_z = 0;
//    for (int i = 0; i < bufsize; i++) {
//        sum_x += xb[i];
//        sum_y += yb[i];
//        sum_z += zb[i];
//    }
//    double ave_x = sum_x / bufsize;
//    double ave_y = sum_y / bufsize;
//    double ave_z = sum_z / bufsize;

    pose.vecPosition[0] = xb[bufsize - 1];
    pose.vecPosition[1] = yb[bufsize - 1];
    pose.vecPosition[2] = zb[bufsize - 1];
    return pose;
}

void RsTrackerDriver::RunFrame()
{
    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
    }
}

void RsTrackerDriver::Cleanup()
{

}

std::string RsTrackerDriver::GetSerialNumber()
{
    return "841512070493_Tracker_" + device_name;
}
