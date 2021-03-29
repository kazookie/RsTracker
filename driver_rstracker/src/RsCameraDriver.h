#include <openvr_driver.h>
#include "driverlog.h"

#include <opencv2/core.hpp>

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>

#include <cubemos/engine.h>
#include <cubemos/skeleton_tracking.h>

#pragma once

using CUBEMOS_SKEL_Buffer_Ptr = std::unique_ptr <CM_SKEL_Buffer, void (*)(CM_SKEL_Buffer*)>;

struct cmPoint {
    float color_pixel[2];
    float point3d[3];
};


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

    cmPoint get_skeleton_point_3d(rs2::depth_frame const& depthFrame, int x, int y);

    void UpdateKeypoints();

    cmPoint GetPoint3d(int i);
    CUBEMOS_SKEL_Buffer_Ptr create_skel_buffer();

    // cmPoint EstimatePoint3d(const CM_SKEL_Buffer* skeletons_buffer, rs2::depth_frame const& depth_frame, cv::Mat& image);

    std::string get_localdata_path()
    {
        const int size = 256;
        char buf[size] = "";
        size_t requiredSize;

        getenv_s(&requiredSize, buf, size, "LOCALAPPDATA");
        return std::string(buf);
    }

    std::string default_log_dir()
    {
        std::string cubemosLogDir = "";
        cubemosLogDir = get_localdata_path() + "\\Cubemos\\SkeletonTracking\\logs";
        return cubemosLogDir;
    }

    std::string default_license_dir()
    {
        std::string cubemosLicenseDir = "";
        cubemosLicenseDir = get_localdata_path() + "\\Cubemos\\SkeletonTracking\\license";
        return cubemosLicenseDir;
    }

    std::string default_model_dir()
    {
        std::string cubemosModelDir = "";
        cubemosModelDir = get_localdata_path() + "\\Cubemos\\SkeletonTracking\\models";
        return cubemosModelDir;
    }
private:
    vr::TrackedDeviceIndex_t m_unObjectId;
    vr::PropertyContainerHandle_t m_ulPropertyContainer;

    std::string m_sSerialNumber;
    std::string m_sModelNumber;

    double cpX = 0, cpY = 0, cpZ = 0;

    // RealSense
    rs2::pipeline pipe;
    rs2::config cfg;
    rs2::context ctx;
    rs2::pipeline_profile profile;
    rs2::device sensor;
    rs2::align align_to_color = rs2::align(RS2_STREAM_DEPTH);

    // Cubemos
    CM_SKEL_Handle* handle = nullptr;   // set up the cubemos skeleton tracking api pipeline

    cv::Mat capturedFrame;
    const int nHeight = 192; // height of the image with which the DNN model will run inference

    CUBEMOS_SKEL_Buffer_Ptr skeletonsPresent = create_skel_buffer();
    CUBEMOS_SKEL_Buffer_Ptr skeletonsLast = create_skel_buffer();

    cmPoint point3d[18];

    // RealSence filter
    rs2::decimation_filter dec_filter;
    rs2::disparity_transform depth_to_disparity;
    rs2::disparity_transform disparity_to_depth;
    rs2::spatial_filter spat_filter;
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hf_filter;

    void SetupRealsense();
};
