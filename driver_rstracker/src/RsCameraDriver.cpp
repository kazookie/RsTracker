#include "RsCameraDriver.h"
#include <Windows.h>
#include <cstdio>

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

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_SerialNumber_String, m_sSerialNumber.c_str());
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

    return vr::VRInitError_None;
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
    vr::HmdQuaternion_t quat;
    quat.w = 1;
    quat.x = 0;
    quat.y = 0;
    quat.z = 0;

    vr::DriverPose_t pose = { 0 };
    // pose.poseIsValid = false;
    pose.poseIsValid = true;
    // pose.result = TrackingResult_Calibrating_OutOfRange;
    pose.result = vr::TrackingResult_Running_OK;
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
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(vr::DriverPose_t));
    }
    UpdateKeypoints();
}

void RsCameraDriver::Cleanup()
{
    cm_skel_destroy_handle(&handle);
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

    // Output all messages with severity level INFO or higher to the console and to a file
    cm_initialise_logging(CM_LogLevel::CM_LL_INFO, true, default_log_dir().c_str());
    DriverLog("Unable to create watchdog thread\n");

    // Cubemos license check and init handle.
    CM_ReturnCode retCode = cm_skel_create_handle(&handle, default_license_dir().c_str());
    if(retCode != CM_SUCCESS) DriverLog("Failed activate license.\n");

    // Cubemos load model.
    std::string modelName = default_model_dir();
    modelName += std::string("/fp32/skeleton-tracking.cubemos");
    retCode = cm_skel_load_model(handle, enInferenceMode, modelName.c_str());
    if (retCode != CM_SUCCESS) DriverLog("Model loading failed.\n");

    // cache the first inference to get started with tracking
    // let some empty frames to run
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

    CM_Image imageLast = {
        capturedFrame.data,         CM_UINT8, capturedFrame.cols, capturedFrame.rows, capturedFrame.channels(),
        (int)capturedFrame.step[0], CM_HWC
    };

    // Get the skeleton keypoints for the first frame
    CM_ReturnCode retCodeFirstFrame = cm_skel_estimate_keypoints(handle, &imageLast, nHeight, skeletonsLast.get());
    if (retCode == CM_SUCCESS) DriverLog("SUCCESS get first skeltal point.\n");
}


CUBEMOS_SKEL_Buffer_Ptr RsCameraDriver::create_skel_buffer()
{
    return CUBEMOS_SKEL_Buffer_Ptr(new CM_SKEL_Buffer(), [](CM_SKEL_Buffer* pb) {
        cm_skel_release_buffer(pb);
        delete pb;
    });
}

void RsCameraDriver::UpdateKeypoints()
{
    // capture image
    rs2::frameset data = pipe.wait_for_frames();
    rs2::align align_to_color(RS2_STREAM_DEPTH);
    data = align_to_color.process(data);

    rs2::frame colorFrame = data.get_color_frame();
    rs2::frame depthFrame = data.get_depth_frame();

    capturedFrame = cv::Mat(
        cv::Size(colorFrame.as<rs2::video_frame>().get_width(), colorFrame.as<rs2::video_frame>().get_height()),
        CV_8UC3,
        (void*)colorFrame.get_data(),
        cv::Mat::AUTO_STEP);


    CM_Image imagePresent = {
    capturedFrame.data,         CM_UINT8, capturedFrame.cols, capturedFrame.rows, capturedFrame.channels(),
    (int)capturedFrame.step[0], CM_HWC
    };

    // Run Skeleton Tracking and display the results
    CM_ReturnCode retCode = cm_skel_estimate_keypoints(handle, &imagePresent, nHeight, skeletonsPresent.get());

    if (retCode == CM_SUCCESS) {
        if (skeletonsPresent->numSkeletons > 0) {
            // Assign tracking ids to the skeletons in the present frame
            cm_skel_update_tracking_id(handle, skeletonsLast.get(), skeletonsPresent.get());

            const cv::Point2f absentKeypoint(-1.0f, -1.0f);
            const std::vector<std::pair<int, int>> limbKeypointsIds = { { 1, 2 },   { 1, 5 },   { 2, 3 }, { 3, 4 },  { 5, 6 },
                                                                    { 6, 7 },   { 1, 8 },   { 8, 9 }, { 9, 10 }, { 1, 11 },
                                                                    { 11, 12 }, { 12, 13 }, { 1, 0 }, { 0, 14 }, { 14, 16 },
                                                                    { 0, 15 },  { 15, 17 } };

            // Get the 3d point and render it on the joints
            CM_SKEL_Buffer* skeletons_buffer = skeletonsLast.get();

            cv::Point2f keyPointHead(skeletons_buffer->skeletons[0].keypoints_coord_x[0],
                skeletons_buffer->skeletons[0].keypoints_coord_y[0]);

            for (size_t keypointIdx = 0; keypointIdx < skeletons_buffer->skeletons[0].numKeyPoints; keypointIdx++) {
                const cv::Point2f keyPoint(skeletons_buffer->skeletons[0].keypoints_coord_x[keypointIdx],
                    skeletons_buffer->skeletons[0].keypoints_coord_y[keypointIdx]);
                if (keyPoint != absentKeypoint) {
                    // get the 3d point and render it on the joints
                    point3d[keypointIdx] = get_skeleton_point_3d(depthFrame, static_cast<int>(keyPoint.x), static_cast<int>(keyPoint.y));
                    //char buffer[256] = "";
                    //snprintf(buffer, sizeof(buffer), "%d, %f, %f, %f\n", static_cast<int>(keypointIdx), point3d[13].point3d[0], point3d[13].point3d[1], point3d[13].point3d[2]);
                    // snprintf(buffer, sizeof(buffer), "%d\n", skeletons_buffer->numSkeletons);
                    // DriverLog(buffer);

                    // FILE* fp;
                    // errno_t error;
                    // error = fopen_s(&fp, "C:\\Users\\kzki\\source\\repos\\RsTracker\\bin\\drivers\\rstracker\\bin\\win64\\smpl.txt", "a");
                    // if (error == 0) {
                    //     fprintf(fp, buffer);
                    //     fclose(fp);
                    // }
                }
            }
            // Set the present frame as last one to track the next frame
            skeletonsLast.swap(skeletonsPresent);
            // Free memory of the latest frame
            cm_skel_release_buffer(skeletonsPresent.get());
        }
    }
}


// cmPoint RsCameraDriver::EstimatePoint3d(const CM_SKEL_Buffer* skeletons_buffer, rs2::depth_frame const& depth_frame, cv::Mat& image)
// {
//     const std::vector<std::pair<int, int>> limbKeypointsIds = { { 1, 2 },   { 1, 5 },   { 2, 3 }, { 3, 4 },  { 5, 6 },
//                                                         { 6, 7 },   { 1, 8 },   { 8, 9 }, { 9, 10 }, { 1, 11 },
//                                                         { 11, 12 }, { 12, 13 }, { 1, 0 }, { 0, 14 }, { 14, 16 },
//                                                         { 0, 15 },  { 15, 17 } };
//     const cv::Point2f absentKeypoint(-1.0f, -1.0f);
// }

cmPoint RsCameraDriver::GetPoint3d(int i)
{
    return point3d[i];
}