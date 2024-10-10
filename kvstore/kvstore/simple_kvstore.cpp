#include "simple_kvstore.hpp"

// SimpleKvStore::kvmap istore; // Why don't I have to do this -> how can you just access the elements from inside this bottom function

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  if(req == NULL){
    mx.unlock();
    return false;
  }
  try {
    std::string value = kvmap.at(req->key);
    res->value = value;
    mx.unlock();
    return true;
  } catch (...) {
    mx.unlock();
    return false;
  }
  
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  kvmap[req->key] = req->value;
  mx.unlock();
  return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  if(kvmap.contains(req->key)){
    std::string val = kvmap[req->key];
    kvmap[req->key] = val + req->value;
  } else {
    kvmap[req->key] = req->value;
  }
  mx.unlock();
  return true;
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  if(kvmap.contains(req->key)){
    res->value = kvmap[req->key];
    kvmap.erase(req->key);
  } else{
    mx.unlock();
    return false;
  }
  mx.unlock();
  return true;
}

bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  for(std::string item: req->keys){
    if(kvmap.contains(item)){
      res->values.push_back(kvmap[item]);
    } else {
      mx.unlock();
      return false;
    }
  }
  mx.unlock();
  return true;
  // unsigned int vectorsize = req->keys.size();
  // for (unsigned int i =0; i < vectorsize; i++) {
  //   if(kvmap.contains(req->keys[i])){
  //     res->values.push_back(kvmap.at(req->keys[i]));
  //   } else {
  //     return false;
  //   }
  // }
  // return true;
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  if(req->keys.size() != req->values.size()){
    mx.unlock();
    return false;
  }
  unsigned int vectorsize = req->keys.size();
  for (unsigned int i = 0; i < vectorsize; i++){
    kvmap[req->keys[i]] = req->values[i];
  }
  mx.unlock();
  return true;
}

std::vector<std::string> SimpleKvStore::AllKeys() {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  mx.lock();
  std::vector<std::string> retvector = {};
  for(auto iter = kvmap.begin(); iter != kvmap.end(); iter++){
    retvector.insert(retvector.end(), iter->first);
  }
  mx.unlock();
  return retvector;
}
