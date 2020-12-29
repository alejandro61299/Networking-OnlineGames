#pragma once

// TODO(you): Reliability on top of UDP lab session
#include <list>
class DeliveryManager;

class DeliveryDelegate
{
public:

	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
};



class DeliveryManager
{
public:

	// For senders to write a new seq. numbers into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	// For Receivers to process the seq. number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	// For Recievers to write ack'ed seq. numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	// For Senders to process ack'ed seq. numbers from a packet
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();

	std::list<OutputMemoryStream> packetsSaved;
	std::list<OutputMemoryStream> packetsToSend;


private:

	// Private members (Senser side)
	// - The next outgoing sequence number
	uint32 nextSequenceNumber = 0;

	// - A list of pending deliveries
	std::list<Delivery*> pendingDeliveries;


	// Private members (Receiver side)
	// - The next expected sequence number
	uint32 nextExpectedSequenceNumber = 0;

	// - A list of sequence numbers pending ack
	std::list<Delivery*> sequenceNumbersPendingAck;

};