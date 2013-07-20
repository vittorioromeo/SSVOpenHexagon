// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <functional>
#include <SFML/Network.hpp>
#include <SSVUtils/SSVUtils.h>
#include <SSVUtilsJson/SSVUtilsJson.h>
#include "SSVOpenHexagon/Online/Online.h"
#include "SSVOpenHexagon/Online/Server.h"
#include "SSVOpenHexagon/Online/Client.h"
#include "SSVOpenHexagon/Global/Config.h"
#include "SSVOpenHexagon/Online/Definitions.h"
#include "SSVOpenHexagon/Utils/Utils.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace ssvs::Utils;
using namespace hg::Utils;
using namespace ssvu;
using namespace ssvu::Encryption;
using namespace ssvuj;
using namespace ssvu::FileSystem;

namespace hg
{
	namespace Online
	{
		using Request = Http::Request;
		using Response = Http::Response;
		using Status = Http::Response::Status;

		const IpAddress hostIp{"209.236.124.147"};
		const unsigned short hostPort{27272};

		bool connected{false};
		bool loggedIn{false};


		PacketHandler clientPacketHandler;
		Uptr<Client> client;

		Packet buildLoginPacket(const string& mUsername, const string& mPassword)
		{
			Packet result;
			result << ClientPackets::Login << mUsername << mPassword;
			return result;
		}

		void initializeServer()
		{
			PacketHandler packetHandler;
			packetHandler[ClientPackets::Ping] = [](ManagedSocket&, sf::Packet& mPacket)
			{
				ssvu::log("Ping", "PacketHandler");
			};

			packetHandler[ClientPackets::Login] = [](ManagedSocket& mManagedSocket, sf::Packet& mPacket)
			{
				string username, password;
				mPacket >> username >> password;

				ssvu::log("Username: " + username + "; Password: " + password, "PacketHandler");

				Packet response;
				response << ServerPackets::LoginResponse << "Login accepted!";
				mManagedSocket.send(response);
			};

			Server server{packetHandler}; server.start(54000);

			while(true)
			{
				this_thread::sleep_for(std::chrono::milliseconds(250));
			}
		}

		void initializeClient()
		{
			clientPacketHandler[ServerPackets::LoginResponse] = [](ManagedSocket&, sf::Packet& mPacket)
			{
				string response;
				mPacket >> response;
				log(response, "SERVER RESPONSE");
			};

			client = Uptr<Client>(new Client(clientPacketHandler));

			thread([]
			{
				while(true)
				{
					if(connected) { client->send(buildPingPacket()); }
					this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}).detach();
		}

		void tryConnectToServer()
		{
			log("Connecting to server...", "hg::Online::connectToServer");

			thread([]
			{
				if(client->connect("127.0.0.1", 54000))
				{
					log("Successfully connected to server", "hg::Online::connectToServer");
					connected = true; return;
				}

				log("Failed to connect to server", "hg::Online::connectToServer");
				connected = false;
			}).detach();
		}
		bool isConnected() { return connected; }

		void tryLogin(const string& mUsername, const string& mPassword)
		{
			log("Logging in...", "hg::Online::tryLogin");

			thread([=]
			{
				this_thread::sleep_for(std::chrono::milliseconds(1000));
				if(!connected) { log("Client isn't connected, aborting", "hg::Online::tryLogin"); return; }
				client->send(buildLoginPacket(mUsername, mPassword));
			}).detach();
		}
		bool isLoggedIn() { return loggedIn; }









		const string host{"http://vittorioromeo.info"};
		const string folder{"Misc/Linked/OHServer/"};
		const string infoFile{"OHInfo.json"};
		const string sendScoreFile{"sendScore.php"};
		const string getScoresFile{"getScores.php"};

		MemoryManager<ThreadWrapper> memoryManager;
		float serverVersion{-1};
		string serverMessage{""};

		void startCheckUpdates()
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }
		}
		void startSendScore(const string&, const string&, float, float)
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }
		}
		void startGetScores(string&, string&, const string&, vector<string>&, const vector<string>&, const string&, float)
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }
		}
		void startGetFriendsScores(vector<string>&, const vector<string>&, const string&, float)
		{
			if(!getOnline()) { log("Online disabled, aborting", "Online"); return; }
		}

		void cleanUp() 		{ for(const auto& t : memoryManager) if(t->getFinished()) memoryManager.del(*t); memoryManager.refresh(); }
		void terminateAll() { for(const auto& t : memoryManager) t->terminate(); memoryManager.refresh(); }

		string getValidator(const string& mPackPath, const string& mLevelId, const string& mLevelRootPath, const string& mStyleRootPath, const string& mLuaScriptPath)
		{
			string luaScriptContents{getFileContents(mLuaScriptPath)};
			unordered_set<string> luaScriptNames;
			recursiveFillIncludedLuaFileNames(luaScriptNames, mPackPath, luaScriptContents);

			string toEncrypt{""};
			toEncrypt.append(mLevelId);
			toEncrypt.append(getFileContents(mLevelRootPath));
			toEncrypt.append(getFileContents(mStyleRootPath));
			toEncrypt.append(luaScriptContents);

			for(const auto& luaScriptName : luaScriptNames)
			{
				string path{mPackPath + "/Scripts/" + luaScriptName};
				string contents{getFileContents(path)};
				toEncrypt.append(contents);
			}

			toEncrypt = getControlStripped(toEncrypt);

			string result{getUrlEncoded(mLevelId) + getMD5Hash(toEncrypt + HG_SKEY1 + HG_SKEY2 + HG_SKEY3)};
			return result;
		}

		float getServerVersion() 							{ return serverVersion; }
		string getServerMessage() 							{ return serverMessage; }
		string getMD5Hash(const string& mString) 			{ return encrypt<Encryption::Type::MD5>(mString); }

		bool isOverloaded() { return memoryManager.getItems().size() > 4; }
		bool isFree() { return memoryManager.getItems().size() < 2; }
	}
}

