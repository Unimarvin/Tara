
#pragma once


#include <Formula/CTL/AUFormula.h>
#include <Formula/CTL/TrueFormula.h>


struct AFFormula : public DFSFormula {
private:
	AUFormula auFormula;
	TrueFormula trueFormula;
public:
	CTLFormula* inner;

	void updateAtomics(NetState& ns, index_t t) {
		inner->updateAtomics(ns,t);
	}
	void revertAtomics(NetState& ns, index_t t) {
		inner->revertAtomics(ns,t);
	}

	// AF phi A (T U phi)
	bool check(Store<void*>& s, NetState& ns, Firelist& firelist, std::vector<int>* witness) {
		auFormula.phi = &trueFormula;
		auFormula.psi = inner;
		auFormula.index = index;
		auFormula.dfsindex = dfsindex;
		auFormula.payloadsize = payloadsize;
		return auFormula.check(s,ns,firelist,witness);
	}

	void DEBUG_print() {
		printf("[%u,%u,%lu]AF(",index,dfsindex,payloadsize);
		inner->DEBUG_print();
		printf(")");
	}

	void gatherPayloadInformation(index_t* numDFS, index_t* numCachedResults) {
		dfsindex = ((*numDFS)++) * sizeof(statenumber_t);
		index = ((*numCachedResults)++) * 2;
		inner->gatherPayloadInformation(numDFS,numCachedResults);
	}
	void setPayloadInformation(index_t cachedResultOffset, size_t payloadSize) {
		index += cachedResultOffset * 8;
		payloadsize = payloadSize;
		inner->setPayloadInformation(cachedResultOffset,payloadSize);
	}
};
