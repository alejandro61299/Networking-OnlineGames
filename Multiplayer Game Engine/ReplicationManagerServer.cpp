#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(done): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	actions.emplace(networkId, ReplicationAction::Create);
}

void ReplicationManagerServer::update(uint32 networkId)
{
	if (actions.find(networkId) != actions.end())
	{
		if (actions[networkId] == ReplicationAction::Create || actions[networkId] == ReplicationAction::Destroy)
		{
			return;
		}

		actions[networkId] = ReplicationAction::Update;
	}
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	actions[networkId] = ReplicationAction::Destroy;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ClientProxy &proxy)
{
	packet << PROTOCOL_ID;
	packet << ServerMessage::Replication;

	proxy.deliveryManager.writeSequenceNumber(packet);

	packet << actions.size();

	// TODO(you): Reliability on top of UDP lab session
	//Send the last input processed
	packet << proxy.lastInputRecived;


	for (auto item = actions.begin(); item != actions.end();) 
	{
		const uint32 id = item->first;
		ReplicationAction &action = item->second;

		packet << id;
		packet << (int)action;

		if (action == ReplicationAction::None)
		{
			++item;
			continue;
		}
		else if (action == ReplicationAction::Destroy)
		{
			item = actions.erase(item);
			continue;
		}

		GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(id);

		if (action == ReplicationAction::Create)
		{
			gameObject->write(packet, false);
			action = ReplicationAction::Update;
		}
		else if (action == ReplicationAction::Update)
		{
			gameObject->write(packet, true);
			action = ReplicationAction::None;
		}

		++item; // Increased here by erase 
	}
	proxy.deliveryManager.packetsSaved.push_back(packet);

}
