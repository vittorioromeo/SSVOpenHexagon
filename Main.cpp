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
#include <json/json.h>
#include <json/reader.h>
#include "Utils.h"
#include "Assets.h"

using namespace std;
using namespace sses;
using namespace ssvs;
using namespace sf;
using namespace hg;

int main()
{
	loadConfig();
	loadAssets();

	srand(unsigned(time(NULL)));

	HexagonGame hg;

	return 0;
}
