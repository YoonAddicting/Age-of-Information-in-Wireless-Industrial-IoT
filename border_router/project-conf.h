#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
#define TSCH_LOG_CONF_PERLOT                     0
#define TSCH_LOG_CONF_PER_SLOT		0
#define NBR_TABLE_CONF_MAX_NEIGHBORS	4
#define UIP_CONF_BUFFER_SIZE 200

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 2
#endif

#ifndef BORDER_ROUTER_CONF_WEBSERVER
#define BORDER_ROUTER_CONF_WEBSERVER 1
#endif

#if BORDER_ROUTER_CONF_WEBSERVER
#define UIP_CONF_TCP 1
#endif

#endif /* PROJECT_CONF_H_ */