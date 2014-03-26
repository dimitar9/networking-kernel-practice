#ifndef _NLEXAMPLE_H_
#define _NLEXAMPLE_H_

enum nlexample_msg_types {
   NLEX_CMD_UPD = 0,
   NLEX_CMD_GET,
   NLEX_CMD_MAX
};

enum nlexample_attr {
   NLE_UNSPEC,
   NLE_MYVAR,
   __NLE_MAX,
};
#define NLE_MAX (__NLE_MAX - 1)

#define NLEX_GRP_MYVAR 1

#endif
