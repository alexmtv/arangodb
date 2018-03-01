////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Tobias Gödderz
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB3_CLUSTERREPAIRS_H
#define ARANGODB3_CLUSTERREPAIRS_H

#include <arangod/Agency/AgencyComm.h>
#include <velocypack/Compare.h>
#include <velocypack/Slice.h>
#include <velocypack/velocypack-common.h>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "ClusterInfo.h"

namespace arangodb {
namespace velocypack {
class Slice;
}

namespace cluster_repairs {

using DBServers = std::vector<ServerID>;
using VPackBufferPtr = std::shared_ptr<VPackBuffer<uint8_t>>;

template<typename T>
class TResult : public arangodb::Result {
 public:

  TResult static success(T val) {
    return TResult(val, 0);
  }

  TResult static error(int errorNumber) {
    return TResult(boost::none, errorNumber);
  }

  TResult static error(int errorNumber, std::__cxx11::string const &errorMessage) {
    return TResult(boost::none, errorNumber, errorMessage);
  }

  explicit TResult(Result const &other)
    : Result(other) {}

  T get() {
    return _val.get();
  }

 protected:
  boost::optional<T> _val;

  TResult(boost::optional<T> val_, int errorNumber)
    : Result(errorNumber), _val(val_) {}

  TResult(boost::optional<T> val_, int errorNumber, std::__cxx11::string const &errorMessage)
    : Result(errorNumber, errorMessage), _val(val_) {}
};


class VersionSort {
  using CharOrInt = boost::variant<char, uint64_t>;

 public:

  bool operator()(std::string const &a, std::string const &b) const;

 private:

  std::vector<CharOrInt> static splitVersion(std::string const &str);
};


struct Collection {
  DatabaseID database;
  std::string name;
  CollectionID id;
  uint64_t replicationFactor;
  bool deleted;
  boost::optional<CollectionID> distributeShardsLike;
  boost::optional<CollectionID> repairingDistributeShardsLike;
  std::map<ShardID, DBServers, VersionSort> shardsById;

  std::string inline fullName() const {
    return this->database + "/" + this->name;
  }

  std::string inline agencyCollectionId() const {
    return "Plan/Collections/" + this->database + "/" + this->id;
  }

  VPackBufferPtr
  createShardDbServerArray(
    ShardID const &shardId
  ) const;

  // maybe more?
  // isSystem
  // numberOfShards
  // deleted
};

struct MoveShardOperation {
  DatabaseID database;
  CollectionID collection;
  ShardID shard;
  ServerID from;
  ServerID to;
  bool isLeader;

  MoveShardOperation() = delete;

  VPackBufferPtr
  toVpackTodo(uint64_t jobId) const;
};

bool operator==(MoveShardOperation const &left, MoveShardOperation const &other);
std::ostream& operator<<(std::ostream& ostream, MoveShardOperation const& operation);

using RepairOperation = boost::variant<MoveShardOperation, AgencyWriteTransaction>;

class DistributeShardsLikeRepairer {
 public:
  std::list<RepairOperation> repairDistributeShardsLike(
    velocypack::Slice const& planCollections,
    velocypack::Slice const& supervisionHealth
  );

 private:
  std::vector< VPackBufferPtr > _vPackBuffers;

  std::map<ShardID, DBServers, VersionSort> static
  readShards(velocypack::Slice const& shards);

  DBServers static
  readDatabases(velocypack::Slice const& planDbServers);

  std::map<CollectionID, struct Collection> static
  readCollections(velocypack::Slice const& collectionsByDatabase);

  boost::optional<ServerID const> static
  findFreeServer(
    DBServers const& availableDbServers,
    DBServers const& shardDbServers
  );

  std::vector<CollectionID> static
  findCollectionsToFix(std::map<CollectionID, struct Collection> collections);

  DBServers static serverSetDifference(
    DBServers setA,
    DBServers setB
  );

  DBServers static serverSetSymmetricDifference(
    DBServers setA,
    DBServers setB
  );

  MoveShardOperation
  createMoveShardOperation(
    Collection& collection,
    ShardID const& shardId,
    ServerID const& fromServerId,
    ServerID const& toServerId,
    bool isLeader
  );

  std::list<RepairOperation>
  fixLeader(
    DBServers const& availableDbServers,
    Collection& collection,
    Collection& proto,
    ShardID const& shardId,
    ShardID const& protoShardId
  );

  std::list<RepairOperation>
  fixShard(
    DBServers const& availableDbServers,
    Collection& collection,
    Collection& proto,
    ShardID const& shardId,
    ShardID const& protoShardId
  );

  boost::optional<AgencyWriteTransaction>
  createFixServerOrderTransaction(
    Collection& collection,
    Collection const& proto,
    ShardID const& shardId,
    ShardID const& protoShardId
  );

  AgencyWriteTransaction
  createRenameAttributeTransaction(
    Collection const& collection,
    velocypack::Slice const& value,
    std::string const& from,
    std::string const& to
  );

  AgencyWriteTransaction
  createRenameDistributeShardsLikeAttributeTransaction(
    Collection &collection
  );

  AgencyWriteTransaction
  createRestoreDistributeShardsLikeAttributeTransaction(
    Collection &collection
  );
};


}
}

#endif  // ARANGODB3_CLUSTERREPAIRS_H
