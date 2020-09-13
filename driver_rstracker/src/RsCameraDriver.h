#include <openvr_driver.h>

#include <opencv2/core.hpp>

#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>

#include <cubemos/engine.h>
#include <cubemos/skeleton_tracking.h>

using CUBEMOS_SKEL_Buffer_Ptr = std::unique_ptr<CM_SKEL_Buffer, void (*)(CM_SKEL_Buffer*)>;

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

    cv::Mat capturedFrame;

    const int nHeight = 192; // height of the image with which the DNN model will run inference
    CM_Image imageLast;
    //CUBEMOS_SKEL_Buffer_Ptr skeletonsPresent;
    //CUBEMOS_SKEL_Buffer_Ptr skeletonsLast;

    void SetupRealsense();
    CUBEMOS_SKEL_Buffer_Ptr create_skel_buffer();
};
