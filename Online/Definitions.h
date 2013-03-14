// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef HG_DEFINITIONS
#define HG_DEFINITIONS

#ifndef HG_SKEY1
#define HG_SKEY1 "dev1"
#endif

#ifndef HG_SKEY2
#define HG_SKEY2 "dev2"
#endif

#ifndef HG_SKEY3
#define HG_SKEY3 "dev3"
#endif

#ifndef HG_NKEY1
#define HG_NKEY1 123456
#endif

#ifndef HG_ENCRYPTIONKEY
#define HG_ENCRYPTIONKEY getMD5Hash(mName + mValidator + scoreString + getMD5Hash(HG_SKEY1) + getMD5Hash(HG_SKEY2) + getMD5Hash(HG_SKEY3) + getMD5Hash(toStr(HG_NKEY1)))
#endif

#endif
