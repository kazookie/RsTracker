#include "ServerDriver.h"

vr::EVRInitError ServerDriver::Init(vr::IVRDriverContext* pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    InitDriverLog(vr::VRDriverLog());

    m_pRsCamera = new RsCameraDriver();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pRsCamera->GetSerialNumber().c_str(), vr::TrackedDeviceClass_TrackingReference, m_pRsCamera);

    m_pTracker_foot_left = new RsTrackerDriver(m_pRsCamera, 13, "left");
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pTracker_foot_left->GetSerialNumber().c_str(), vr::TrackedDeviceClass_GenericTracker, m_pTracker_foot_left);

    m_pTracker_foot_right = new RsTrackerDriver(m_pRsCamera, 10, "right");
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_pTracker_foot_right->GetSerialNumber().c_str(), vr::TrackedDeviceClass_GenericTracker, m_pTracker_foot_right);

    return vr::VRInitError_None;
}

void ServerDriver::RunFrame()
{
    if (m_pRsCamera)
    {
        m_pRsCamera->RunFrame();
    }
    if (m_pTracker_foot_left)
    {
        m_pTracker_foot_left->RunFrame();
    }
    if (m_pTracker_foot_right)
    {
        m_pTracker_foot_right->RunFrame();
    }
}

void ServerDriver::Cleanup()
{
    CleanupDriverLog();
    delete m_pRsCamera;
    m_pRsCamera = NULL;
    delete m_pTracker_foot_left;
    m_pTracker_foot_left = NULL;
    delete m_pTracker_foot_right;
    m_pTracker_foot_right = NULL;
}