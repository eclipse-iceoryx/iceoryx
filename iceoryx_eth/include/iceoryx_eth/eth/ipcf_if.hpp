#include <string>
extern "C"{
    #include "ipc-shm.h"
}

namespace iox
{
namespace ipcf
{

class IPCFChannelInit
{

public :

    static const unsigned MAX_IPCF_CHANNELS =10;
	/* IPC SHM configuration defines */
	static const unsigned int LOCAL_SHM_ADDR = 0x3E900000 ;
	static const unsigned int IPC_SHM_SIZE = 0x100000 ;/* 1M local shm, 1M remote shm */
	static const unsigned int REMOTE_SHM_ADDR;
	static const unsigned int INTER_CORE_TX_IRQ=2u;
	static const unsigned int INTER_CORE_RX_IRQ=1u;
	static const unsigned int MAX_NUM_IPCFCHS=10;

    static struct ipc_shm_cfg shm_cfg;  
    static ipc_shm_channel_cfg IpcfChannels[MAX_IPCF_CHANNELS];
    static void*  IpcfChannelsBaseAddr[MAX_IPCF_CHANNELS];

    IPCFChannelInit(unsigned int ,unsigned int );
    int IPCFChannelInitCfg();

private :


};

template <typename ipcfchanneltype_t>
class IPCFChannel : public IPCFChannelInit
{

public :

IPCFChannel(int ,unsigned int );
IPCFChannel()= delete;

void* GetChannelBaseptr(int);
int   SendDataOnipcf(int);
void  ChanRxCb();

private : 
struct ipc_shm_channel_cfg chan_cfg;

};

} // namespace ipcf
} // namespace iox
