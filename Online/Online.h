#ifndef HG_SERVER
#define HG_SERVER

namespace hg
{
	void checkUpdates();

	void setUpdatesChecked(bool mUpdatesChecked);
	bool getUpdatesChecked();
	void setServerVersion(float mServerVersion);
	float getServerVersion();
}

#endif // HG_SERVER
