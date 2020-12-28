#include "Networks.h"
#include "DeliveryManager.h"
// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
    Delivery* delivery = new Delivery();

    delivery->sequenceNumber = nextSequenceNumber;
    packet << delivery->sequenceNumber;
    pendingDeliveries.push_back(delivery);

    ++nextSequenceNumber;

    return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
    // If the order of the packet is not correct, discard!
    bool ret = false;

    uint32 sequenceNumberPacket;
    packet >> sequenceNumberPacket;


    if (nextExpectedSequenceNumber == sequenceNumberPacket)
    {
        Delivery* delivery = new Delivery();
        delivery->sequenceNumber = sequenceNumberPacket;
        sequenceNumbersPendingAck.push_back(delivery);
        ++nextExpectedSequenceNumber;

        ret = true;
    }
    else
        LOG("Not in the correct order!!");

    return ret;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
    return sequenceNumbersPendingAck.empty() == false;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
    packet << PROTOCOL_ID;
    packet << ClientMessage::Confirmation;
    int size = sequenceNumbersPendingAck.size();
    packet << size;

    for (Delivery* currentDelivery : sequenceNumbersPendingAck)
    {
        packet << currentDelivery->sequenceNumber;

        currentDelivery = nullptr;
        delete(currentDelivery);
    }
    sequenceNumbersPendingAck.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
    int size = -1;
    packet >> size;

    for (int i = 0; i < size; ++i)
    {
        uint32 currentSequenceNumber;
        packet >> currentSequenceNumber;

        for (Delivery* currentDelivery : pendingDeliveries)
        {
            if (currentDelivery->sequenceNumber == currentSequenceNumber)
            {
                //TODO Succes!
                pendingDeliveries.remove(currentDelivery); // Problems with the for??
                break;
            }
        }
    }

}

void DeliveryManager::processTimedOutPackets()
{
    for (Delivery* currentDelivery : pendingDeliveries)
    {
        currentDelivery->dispatchTime += Time.deltaTime;
        
        if (currentDelivery->dispatchTime >= PACKET_DELIVERY_TIMEOUT_SECONDS)
        {
            //TODO Failure!
            LOG("Failure!!!!");

        }
    }
}
