#pragma once
#include "MemoryStream.h"

// TODO(done): World state replication lab session

class ReplicationManagerClient
{
public:
    void read(const InputMemoryStream& packet);
};