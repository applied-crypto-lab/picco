
#ifndef TRUNC_H_
#define TRUNC_H_

#include "Mod2M.h"
#include "Operation.h"

class Trunc: public Operation {
public:
	Trunc(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]);
	virtual ~Trunc();
	void doOperation(mpz_t* result, mpz_t* shares, int K, int M, int size, int threadID);
private:
	Mod2M *Mod;
};

#endif /* TRUNC_H_ */
