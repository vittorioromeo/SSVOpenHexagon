// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Groups.h"

namespace hg
{
	sses::Group gWall;

	sses::Group getGWall() { return gWall; }
	void initGroups(sses::Manager& mManager) { gWall = mManager.getGroup("wall"); }
}
