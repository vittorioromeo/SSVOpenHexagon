// Copyright (c) 2013-2014 Vittorio Romeo
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

#ifndef HG_ENCRYPT
	#define HG_ENCRYPT(X) toStr(HG_NKEY1) + X + HG_SKEY1 + HG_SKEY2 + HG_SKEY3
#endif

#endif
