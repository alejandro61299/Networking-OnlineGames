#pragma once
#include "ModuleNetworking.h"
#include "ReplicationManagerServer.h"
#include "DeliveryManager.h"

struct ClientProxy
{
	bool connected = false;
	sockaddr_in address;
	uint32 clientId;

	std::string name;
	GameObject* gameObject = nullptr;

	struct PlayerData
	{
		uint8 spaceshipType = 0u;
		bool spawned = true;
		int points = 0;
		float timeToSpawn = 0.f;
	} playerData;

	// TODO(done): UDP virtual connection lab session
	double lastPacketRecivedTime = 0.0f;

	// TODO(done): World state replication lab session
	float secondsSinceLastReplication = 0;
	ReplicationManagerServer replicationServer;

	// TODO(you): Reliability on top of UDP lab session (DONE)
	uint32 lastInputRecived = 0;
	DeliveryManager deliveryManager;

	uint32 nextExpectedInputSequenceNumber = 0;
	InputController gamepad;
};

class ModuleNetworkingServer : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer public methods
	//////////////////////////////////////////////////////////////////////

	void setListenPort(int port);



private:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isServer() const override { return true; }

	void onStart() override;

	void onGui() override;

	void onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress) override;

	void onUpdate() override;

	void onConnectionReset(const sockaddr_in &fromAddress) override;

	void onDisconnect() override;



	//////////////////////////////////////////////////////////////////////
	// Client proxies
	//////////////////////////////////////////////////////////////////////

	uint32 nextClientId = 0;

	ClientProxy clientProxies[MAX_CLIENTS];

	ClientProxy * createClientProxy();

	ClientProxy * getClientProxy(const sockaddr_in &clientAddress);

    void destroyClientProxy(ClientProxy *clientProxy);

public:

	//////////////////////////////////////////////////////////////////////
	// Spawning network objects
	//////////////////////////////////////////////////////////////////////
	friend class GameManager;
	GameManager gameManager;

private:

	//////////////////////////////////////////////////////////////////////
	// Updating / destroying network objects
	//////////////////////////////////////////////////////////////////////

	GameObject * instantiateNetworkObject();
	friend GameObject *(NetworkInstantiate)();

	void updateNetworkObject(GameObject *gameObject);
	friend void (NetworkUpdate)(GameObject *);

	void destroyNetworkObject(GameObject *gameObject);
	void destroyNetworkObject(GameObject *gameObject, float delaySeconds);
	friend void (NetworkDestroy)(GameObject *);
	friend void (NetworkDestroy)(GameObject *, float delaySeconds);

	struct DelayedDestroyEntry
	{
		float delaySeconds = 0.0f;
		GameObject *object = nullptr;
	};

	DelayedDestroyEntry netGameObjectsToDestroyWithDelay[MAX_GAME_OBJECTS] = {};

	//////////////////////////////////////////////////////////////////////
	// State
	//////////////////////////////////////////////////////////////////////

	enum class ServerState
	{
		Stopped,
		Listening
	};

	ServerState state = ServerState::Stopped;

	uint16 listenPort = 0;


	// TODO(done): UDP virtual connection lab session
	float secondsSinceLastPingDelivery = 0.0f;

};


// NOTE(jesus): It creates a game object into the network. Use
// this method instead of Instantiate() to create network objects.
// It makes sure the object creation is replicated over the network.
GameObject * NetworkInstantiate();

// NOTE(jesus): It marks an object for replication update.
void NetworkUpdate(GameObject *gameObject);

// NOTE(jesus): For network objects, use this version instead of
// the default Destroy(GameObject *gameObject) one. This one makes
// sure to notify the destruction of the object to all connected
// machines.
void NetworkDestroy(GameObject *gameObject);
void NetworkDestroy(GameObject *gameObject, float delaySeconds);
void NetworkPlayAudio(AudioClip* clip);
