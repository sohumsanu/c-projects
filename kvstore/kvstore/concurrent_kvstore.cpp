#include "concurrent_kvstore.hpp"

#include <optional>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
  store.mutexcount[store.bucket(req->key)].lock_shared();
  std::optional<DbItem> item = store.getIfExists(store.bucket(req->key), req->key);
  if(item == std::nullopt){
    store.mutexcount[store.bucket(req->key)].unlock_shared();
    return false;
  } else {
    res->value = item->value;
    store.mutexcount[store.bucket(req->key)].unlock_shared();
  }
  return true;
}

bool ConcurrentKvStore::Put(const PutRequest* req, PutResponse*) {
  store.mutexcount[store.bucket(req->key)].lock();
  store.insertItem(store.bucket(req->key), req->key, req->value);
  store.mutexcount[store.bucket(req->key)].unlock();
  return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
  store.mutexcount[store.bucket(req->key)].lock();
  // std::optional<DbItem> item = store.getIfExists(store.bucket(req->key), req->key);
  // if (item != std::nullopt){
  //   std::string items = item.value;
  //   item.value = items+req->value;
  //   store.mutexcount[store.bucket(req->key)].unlock();
  // }
  auto& bucket = store.buckets[store.bucket(req->key)];
  for (auto it = bucket.begin(); it != bucket.end(); it++){
    if(it->key == req->key){
      it->value.append(req->value);
      store.mutexcount[store.bucket(req->key)].unlock();
      return true;
    }
  }
  store.insertItem(store.bucket(req->key), req->key, req->value);
  store.mutexcount[store.bucket(req->key)].unlock();

  return true;
}

bool ConcurrentKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  store.mutexcount[store.bucket(req->key)].lock();
  res->value = store.getIfExists(store.bucket(req->key), req->key)->value;
  store.removeItem(store.bucket(req->key), req->key);
  store.mutexcount[store.bucket(req->key)].unlock();
  return true;


  // std::optional<DbItem> item = store.getIfExists(store.bucket(req->key), req->key);
  // if (item == std::nullopt){
  //   store.mutexcount[store.bucket(req->key)].unlock();
  //   return false;
  // } else {
  //   res->value = item->value;
  //   store.removeItem(store.bucket(req->key), req->key);
  //   store.mutexcount[store.bucket(req->key)].unlock();
  // }
  // return true;
}

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
  for(size_t i = 0; i < store.BUCKET_COUNT; i++){
    store.mutexcount[i].lock_shared();
  }
  int size = req->keys.size();
  bool iftrue = false;
  for(int j = 0; j < size; j++){
    std::optional<DbItem> item = store.getIfExists(store.bucket(req->keys[j]), req->keys[j]);
    if(item != std::nullopt){
      res->values.push_back(item->value);
    } else {
      iftrue = true;
    }
  }
  for (size_t x = 0; x < store.BUCKET_COUNT; x++){
        store.mutexcount[x].unlock_shared();
  }
  if(iftrue){
    return false;
  }
  return true;
}

bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
  if(req->keys.size() != req->values.size()){
    return false;
  }
  for(size_t i = 0; i < store.BUCKET_COUNT; i++){
    store.mutexcount[i].lock();
  }
  
  int size = req->keys.size();
  for (int x = 0; x < size; x++){
    store.insertItem(store.bucket(req->keys[x]), req->keys[x], req->values[x]);
  }
  for(size_t j = 0; j < store.BUCKET_COUNT; j++){
    store.mutexcount[j].unlock();
  }
  return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
  for(size_t i = 0; i < store.BUCKET_COUNT; i++){
    store.mutexcount[i].lock_shared();
  }
  std::vector<std::string> finalreturn = {};
  for(size_t x = 0; x < store.BUCKET_COUNT; x++){
    for(auto bucket : store.buckets[x]){
      finalreturn.push_back(bucket.key);
    }
  }
  for(size_t i = 0; i < store.BUCKET_COUNT; i++){
    store.mutexcount[i].unlock_shared();
  }
  return finalreturn;
}

