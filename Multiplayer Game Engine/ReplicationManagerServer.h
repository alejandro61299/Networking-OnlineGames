#pragma once
// TODO(done): World state replication lab session

#include <unordered_map>

enum class ReplicationAction;

class ReplicationManagerServer
{
public:
    void create(uint32 networkId);
    void update(uint32 networkId);
    void destroy(uint32 networkId);
    void write(OutputMemoryStream& packet);

private:
    std::unordered_map<uint32, ReplicationAction> actions;
};