#include <openvr_driver.h>
#include <thread>

class WatchdogDriver : public vr::IVRWatchdogProvider
{
public:
    WatchdogDriver()
    {
        m_pWatchdogThread = nullptr;
    }

    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    virtual void Cleanup();

private:
    std::thread *m_pWatchdogThread;
};

void WatchdogThreadFunction();
