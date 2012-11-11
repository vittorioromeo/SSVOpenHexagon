#include <vector>
#include <string>
#include <iostream>
#include <random>
#include "SSVEntitySystem.h"
#include "SSVStart.h"
#include "CPlayer.h"
#include "HexagonGame.h"
#include <memory>
#include "Config.h"

using namespace std;
using namespace sses;
using namespace ssvs;
using namespace sf;
using namespace hg;

int main()
{
	srand(unsigned(time(NULL)));

	loadConfig();
	HexagonGame hg;

	return 0;
}
