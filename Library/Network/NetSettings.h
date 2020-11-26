// Jon Bellamy 26-01-2008
// Common settings that apply accross many net classes, much of this will be debug


#ifndef _NET_SETTINGS_H
#define _NET_SETTINGS_H


namespace net 
{

#define HTTP_DEBUG_MESSAGES 1
#define DNS_DEBUG_MESSAGES 0

#define RUDP_PACKET_DEBUGGING 1
#define SIMULATE_NETWORK_CONDITIONS 1
#if SIMULATE_NETWORK_CONDITIONS
#define NETSIM_ONLY(X) X
#else
#define NETSIM_ONLY(X)
#endif

} // namespace net


#endif 