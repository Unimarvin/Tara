/*!
\file SuffixTreeStore.h
\author Christian Koch
\status new
\brief VectorStore implementation using binary suffix trees. Based on BinStore. Relies on the assumption that different input vectors (possibly of different length) are not prefix of another.
*/

#pragma once
#include <Core/Dimensions.h>
#include <Stores/VectorStores/VectorStore.h>
#include <pthread.h>

template<typename P>
class VectorStoreCreator {
public:
  virtual VectorStore<P>* operator() ( void ) const  = 0;
};

template<typename P,typename T>
class NullaryVectorStoreCreator : public VectorStoreCreator<P> {
public:
	VectorStore<P>* operator() ( void ) const {
    return new T();
  }
};

template<typename P,typename T,typename A1>
class UnaryVectorStoreCreator : public VectorStoreCreator<P> {
  A1 arg1;
public:
  UnaryVectorStoreCreator(A1 _arg1) {
    arg1 = _arg1;
  }
  VectorStore<P>* operator() ( void ) const {
    return new T(arg1);
  }
};

template<typename P,typename T,typename A1,typename A2>
class BinaryVectorStoreCreator : public VectorStoreCreator<P> {
	  A1 arg1;
	  A2 arg2;
public:
	  BinaryVectorStoreCreator(A1 _arg1,A2 _arg2) {
    arg1 = _arg1;
    arg2 = _arg2;
  }
  VectorStore<P>* operator() ( void ) const {
    return new T(arg1,arg2);
  }
};



template <typename T>
class HashingWrapperStore : public VectorStore<T>
{
public:
    /// constructor
	HashingWrapperStore(VectorStoreCreator<T>* _storeCreator);
    /// destructor
    virtual ~HashingWrapperStore();

    /// searches for a vector and inserts if not found
    /// @param in vector to be seached for or inserted
    /// @param bitlen length of vector
    /// @param hash of current NetState
    /// @param payload pointer to be set to the place where the payload of this state will be held
    /// @param threadIndex the index of the thread that requests this call. Values will range from 0 to (number_of_threads - 1). Used to allow using thread-local auxiliary data structures without locking any variables.
    /// @return true, if the marking was found in the store, otherwise false.
    virtual bool searchAndInsert(const vectordata_t* in, bitindex_t bitlen, hash_t hash, T** payload, index_t threadIndex);

    /// gets and removes a vector from the store
    /// @param out place where the returned vector will be written to
    /// @return false, if the store was already empty, otherwise true
    virtual bool popVector(vectordata_t * & out);
private:

    VectorStoreCreator<T>* storeCreator;

        // first branch in decision tree; NULL as long as less than two elements in bucket
    VectorStore<T>** buckets;

    // the read-write mutexes
    pthread_rwlock_t* rwlocks;

    index_t currentPopBucket;
};

#include <Stores/VectorStores/HashingWrapperStore.inc>
