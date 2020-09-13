#include "RsCameraDriver.h"
#include <Windows.h>

using namespace vr;

RsCameraDriver::RsCameraDriver()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

    m_sSerialNumber = "841512070493";
    m_sModelNumber = "D435i";

    SetupRealsense();
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

cmPoint RsCameraDriver::get_skeleton_point_3d(rs2::depth_frame const& depthFrame, int x, int y)
{
    // Get the distance at the given pixel
    auto distance = depthFrame.get_distance(x, y);

    cmPoint point;
    point.color_pixel[0] = static_cast<float>(x);
    point.color_pixel[1] = static_cast<float>(y);

    // Deproject from pixel to point in 3D
    rs2_intrinsics intr = depthFrame.get_profile().as<rs2::video_stream_profile>().get_intrinsics(); // Calibration data
    rs2_deproject_pixel_to_point(point.point3d, &intr, point.color_pixel, distance);

    return point;
}

void RsCameraDriver::SetupRealsense()
{
    cfg.enable_stream(RS2_STREAM_COLOR, -1, 1280, 720, RS2_FORMAT_BGR8, 30);
    cfg.enable_stream(RS2_STREAM_DEPTH, -1, 1280, 720, RS2_FORMAT_ANY, 30);
    rs2::align align_to_color(RS2_STREAM_DEPTH);

    profile = pipe.start(cfg);
    auto sensor = profile.get_device().first<rs2::depth_sensor>();

    auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
    for (auto i = range.min; i < range.max; i += range.step)
        if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == "High Density")
            sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

    CM_TargetComputeDevice enInferenceMode = CM_TargetComputeDevice::CM_CPU;

    // set up the cubemos skeleton tracking api pipeline
    CM_SKEL_Handle* handle = nullptr;
    // Output all messages with severity level INFO or higher to the console and to a file
    std::string default_log_dir = "%LOCALAPPDATA%\\Cubemos\\SkeletonTracking\\logs";
    cm_initialise_logging(CM_LogLevel::CM_LL_INFO, true, default_log_dir.c_str());

    std::string default_license_dir = "%LOCALAPPDATA%\\Cubemos\\SkeletonTracking\\license";
    //CM_ReturnCode retCode = cm_skel_create_handle(&handle, default_license_dir.c_str());
    CM_ReturnCode retCode = cm_skel_create_handle(&handle, default_license_dir.c_str());
    //CHECK_HANDLE_CREATION(retCode);

    std::string modelName = "%LOCALAPPDATA%\\Cubemos\\SkeletonTracking\\models";
    if (enInferenceMode == CM_TargetComputeDevice::CM_CPU) {
        modelName += std::string("/fp32/skeleton-tracking.cubemos");
    }
    else {
        modelName += std::string("/fp16/skeleton-tracking.cubemos");
    }
    cm_skel_load_model(handle, enInferenceMode, modelName.c_str());
    //if (retCode != CM_SUCCESS) {
        // EXIT_PROGRAM("Model loading failed.");
    //}

    for (int k = 0; k < 30; k++) {
        rs2::frameset data = pipe.wait_for_frames();
        rs2::frame colorFrame = data.get_color_frame();
        rs2::frame depthFrame = data.get_depth_frame();
        capturedFrame = cv::Mat(
            cv::Size(colorFrame.as<rs2::video_frame>().get_width(), colorFrame.as<rs2::video_frame>().get_height()),
            CV_8UC3,
            (void*)colorFrame.get_data(),
            cv::Mat::AUTO_STEP);
    }

    imageLast = {
        capturedFrame.data,         CM_UINT8, capturedFrame.cols, capturedFrame.rows, capturedFrame.channels(),
        (int)capturedFrame.step[0], CM_HWC
    };

    //skeletonsPresent = create_skel_buffer();
   //skeletonsLast = create_skel_buffer();

    // Get the skeleton keypoints for the first frame
   //cm_skel_estimate_keypoints(handle, &imageLast, nHeight, skeletonsLast.get());
}

CUBEMOS_SKEL_Buffer_Ptr RsCameraDriver::create_skel_buffer()
{
    return CUBEMOS_SKEL_Buffer_Ptr(new CM_SKEL_Buffer(), [](CM_SKEL_Buffer* pb) {
        cm_skel_release_buffer(pb);
        delete pb;
    });
}