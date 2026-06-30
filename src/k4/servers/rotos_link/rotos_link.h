#ifndef ROTOS_LINK_H
#define ROTOS_LINK_H

/*
 * ROTS network link server — AUX mini-UART (QEMU serial1) to host hub.
 *
 *   task --LinkSend("PING")--> [rotos_link] --line--> TCP hub --> peer OS
 */
#define ROTOS_LINK_PRIORITY           6
#define ROTOS_LINK_NOTIFIER_PRIORITY  4
#define ROTOS_LINK_SERVER_NAME        "rotos_link"

#define ROTOS_LINK_CMD_MAX    120
#define ROTOS_LINK_REPLY_MAX  240
#define LINK_HUB_TIMEOUT_MSG  "(hub timeout)"
#define LINK_HUB_OFFLINE_MSG  "(hub offline)"

void rotos_link_server(void);
int  RotosLinkTid(void);
int  LinkSend(const char *cmd, char *out, int outmax);
int  LinkHubUp(void);
int  LinkGetPeers(char *buf, int max);

#endif
