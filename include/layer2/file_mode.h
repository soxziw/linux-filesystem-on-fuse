#define GET_OWNER_MODE(mode) (mode & 07)
#define GET_GROUP_MODE(mode) ((mode & 070) >> 3)
#define GET_OTHER_MODE(mode) ((mode & 0700) >> 6)