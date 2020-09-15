#include "iceoryx_ipcf/ipcf_if.hpp"

#include <string>
extern "C"{
	#include "ipc-shm.h"

}

template <typename ipcfchanneltype_t> 
iox::ipcf::IPCFChannel<ipcfchanneltype_t>::IPCFChannel(int channelid,unsigned int cSize)
{
	chan_cfg = {
		.type = 1,                               //To be handled
		.ch = {
			.unmanaged = {
				.size = cSize,
				.rx_cb = this->ChanRxCb,
				.cb_arg = NULL,
			},
		}
	};
    IpcfChannels[channelid] = chan_cfg;
}

template <typename ipcfchanneltype_t>
int iox::ipcf::IPCFChannel<ipcfchanneltype_t>::SendDataOnipcf(int channelid)
{
    int error=-1;
	if ( (IpcfChannelsBaseAddr != NULL) && (channelid <= MAX_IPCF_CHANNELS))
	{error = ipc_shm_unmanaged_tx(channelid);}
	
	return error;
}
template <typename ipcfchanneltype_t>
void* iox::ipcf::IPCFChannel<ipcfchanneltype_t>::GetChannelBaseptr(int channelid)
{
   if (channelid <= MAX_IPCF_CHANNELS)
   {return  IpcfChannelsBaseAddr[channelid];}
   else
   {return NULL;}
}
template <typename ipcfchanneltype_t>
void iox::ipcf::IPCFChannel<ipcfchanneltype_t>::ChanRxCb()
{


}

iox::ipcf::IPCFChannelInit::IPCFChannelInit(unsigned int localAddress,unsigned int remoteAddress)
{
	shm_cfg.local_shm_addr     = localAddress;
	shm_cfg.remote_shm_addr    = localAddress + IPC_SHM_SIZE;
	shm_cfg.shm_size           = IPC_SHM_SIZE;
	shm_cfg.inter_core_tx_irq  = INTER_CORE_TX_IRQ;
	shm_cfg.inter_core_rx_irq  = INTER_CORE_RX_IRQ;
	shm_cfg.remote_core.type   = IPC_CORE_DEFAULT;
	shm_cfg.remote_core.index  = 0;
	shm_cfg.num_channels = MAX_IPCF_CHANNELS;
}

int iox::ipcf::IPCFChannelInit::IPCFChannelInitCfg()
{
     shm_cfg.channels =IpcfChannels;
     auto error = ipc_shm_init(&shm_cfg);

	 for(unsigned int i = 0; i<MAX_IPCF_CHANNELS;i++)
	 {
		 IpcfChannelsBaseAddr[i]=ipc_shm_unmanaged_acquire(i);
	 }
  return error ;
}