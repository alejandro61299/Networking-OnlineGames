#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(done): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet, uint32& lastInputRecivied)
{
	size_t size;
	packet >> size;

	packet >> lastInputRecivied;

	for (int i = 0; i < size; ++i)
	{
		uint32 id = 0;
		packet >> id;
		int action = -1;
		packet >> action;

		if (action == (int)ReplicationAction::None)
		{
			continue;
		}
		else if (action == (int)ReplicationAction::Destroy) {
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(id);
			if (gameObject != nullptr) {
				App->modLinkingContext->unregisterNetworkGameObject(gameObject);
				App->modGameObject->Destroy(gameObject);
			}
		}

		GameObject* gameObject = nullptr;

		if (action == (int)ReplicationAction::Update)
		{
			gameObject = App->modLinkingContext->getNetworkGameObject(id);

			if (gameObject != nullptr)
				gameObject->read(packet, true);
		}
		if (action == (int)ReplicationAction::Create)
		{
			gameObject = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, id);
			gameObject->read(packet, false);
		}
	}
}
