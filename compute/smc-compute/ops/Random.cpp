/*   
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Random.h"

int Random::isFirst = 0;
int* Random::isFirst_thread; 
int Random::isInitialized = 0; 

gmp_randstate_t* Random::rstates; 
gmp_randstate_t** Random::rstates_thread; 
pthread_mutex_t Random::mutex;
 
void Random::getNextRandValue(int id, int bits, std::map<std::string, std::vector<int> > poly, mpz_t value)
{
	if(isFirst == 0)
	{
		std::map<std::string, std::vector<int> >::iterator it;
		mpz_t* keys = (mpz_t*)malloc(sizeof(mpz_t) * poly.size()); 
		int k = 0;
		for(it = poly.begin(); it != poly.end(); it++)
		{
			mpz_init(keys[k]); 
			mpz_set_str(keys[k], ((*it).first).c_str(), 10);
			k++; 
		}
		rstates = (gmp_randstate_t*)malloc(sizeof(gmp_randstate_t) * poly.size());  
		for(int i = 0; i < poly.size(); i++)
		{
			gmp_randinit_default(rstates[i]); 
			gmp_randseed(rstates[i], keys[i]); 
		}
		isFirst = 1; 
	}	
	else
		mpz_urandomb(value, rstates[id], bits); 
}

void Random::getNextRandValue(int id, int bits, std::map<std::string, std::vector<int> > poly, mpz_t value, int threadID)
{
	if(threadID == -1){
		getNextRandValue(id, bits, poly, value); 
		return; 
	}
	if(isFirst_thread[threadID] == 0)
	{
		std::map<std::string, std::vector<int> >::iterator it;
		mpz_t* keys = (mpz_t*)malloc(sizeof(mpz_t) * poly.size()); 
		int k = 0; 
		for(it = poly.begin(); it != poly.end(); it++)
		{
			mpz_init(keys[k]); 
			mpz_set_str(keys[k], ((*it).first).c_str(), 10); 
			k++; 
		}
		for(int i = 0; i < poly.size(); i++)
		{
			gmp_randinit_default(rstates_thread[i][threadID]); 
			gmp_randseed(rstates_thread[i][threadID], keys[i]); 
		}
		isFirst_thread[threadID] = 1; 
	}	
	else
		mpz_urandomb(value, rstates_thread[id][threadID], bits); 
}

void Random::getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int> > poly, mpz_t value)
{
	if(isFirst == 0)
	{
		std::map<std::string, std::vector<int> >::iterator it;
		mpz_t* keys = (mpz_t*)malloc(sizeof(mpz_t) * poly.size()); 
		int k = 0; 
		for(it = poly.begin(); it != poly.end(); it++)
		{
			mpz_init(keys[k]); 
			mpz_set_str(keys[k], ((*it).first).c_str(), 10); 
			k++; 
		}
		rstates = (gmp_randstate_t*)malloc(sizeof(gmp_randstate_t) * poly.size());  
		for(int i = 0; i < poly.size(); i++)
		{
			gmp_randinit_default(rstates[i]); 
			gmp_randseed(rstates[i], keys[i]); 
		}
		isFirst = 1; 
	}	
	else
		mpz_urandomm(value, rstates[id], mod); 
}


void Random::getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int> > poly, mpz_t value, int threadID)
{
	if(threadID == -1){
		getNextRandValue(id, mod, poly, value); 
		return; 
	}
	if(isFirst_thread[threadID] == 0)
	{
		std::map<std::string, std::vector<int> >::iterator it;
		mpz_t* keys = (mpz_t*)malloc(sizeof(mpz_t) * poly.size()); 
		int k = 0; 
		for(it = poly.begin(); it != poly.end(); it++)
		{
			mpz_init(keys[k]); 
			mpz_set_str(keys[k], ((*it).first).c_str(), 10);
			k++; 
		}
		for(int i = 0; i < poly.size(); i++)
		{
			gmp_randinit_default(rstates_thread[i][threadID]); 
			gmp_randseed(rstates_thread[i][threadID], keys[i]); 
		}
		isFirst_thread[threadID] = 1; 
	}	
	else
		mpz_urandomm(value, rstates_thread[id][threadID], mod); 
}


long long Random::nChoosek(long long n,  long long k)
{
    if (k > n) return 0;
    if (k * 2 > n) k = n-k;
    if (k == 0) return 1;

    int result = n;
    for( int i = 2; i <= k; ++i ) {
        result *= (n-i+1);
        result /= i;
    }
    return result;
}

Random::Random(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s) {
    ss = s;
    id = nodeID;
    net = nodeNet;
    polynomials = poly;
    numOfThreads = nodeNet.getNumOfThreads();     
    if(!isInitialized){
	isInitialized = 1; 
	pthread_mutex_init(&mutex, NULL); 
    	isFirst_thread = (int*)malloc(sizeof(int) * numOfThreads); 
    	for(int i = 0; i < numOfThreads; i++)
		isFirst_thread[i] = 0; 
    	rstates_thread = (gmp_randstate_t**)malloc(sizeof(gmp_randstate_t*) * poly.size());  
    	for(int i = 0; i < poly.size(); i++)
		rstates_thread[i] = (gmp_randstate_t*)malloc(sizeof(gmp_randstate_t) * numOfThreads); 
    }
}

Random::~Random() {
	// TODO Auto-generated destructor stub
}

int Random::computePolynomials(std::vector<int> polys, int point){
	int result = 0;
	for(unsigned int i = 0; i < polys.size(); i++)
		result += pow(point, (polys.size()-1-i)) * polys[i];
	return result; 
}

void Random::generateRandValue(int nodeID, int bits, int size, mpz_t* results){
	std::map<std::string, std::vector<int> >::iterator it;
	mpz_t zero, rand, temp;
	int polysize = polynomials.size(); 
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_init(zero);
    	mpz_init(rand);
	mpz_init(temp); 
	mpz_set_ui(zero, 0);
	for(int i = 0; i < polysize; i++)
	{
		mpz_init(temp1[i]); 
		mpz_init(temp2[i]); 
		mpz_init(temp3[i]); 
	}

	int index = 0; 
	std::vector<long> polyOutput;
	std::vector<long> denominator; 
	long long combinations = nChoosek(ss->getPeers(), ss->getThreshold());
	
	if(isFirst == 0)
		getNextRandValue(0, 0, polynomials, NULL);
 
	/*************** Evaluate the polynomials on points ******************/
	for(it = polynomials.begin(); it != polynomials.end(); it++)
	{
		std::vector<int> polys = (*it).second; 
		denominator.push_back(polys[polys.size()-1]); 
		polyOutput.push_back(computePolynomials((*it).second, nodeID));
	}
		
	for(int m = 0; m < polysize; m++)
	{
		mpz_set_si(temp2[m], denominator[m]);
		mpz_set_si(temp3[m], polyOutput[m]); 
		if(denominator[m] < 0)
			ss->modAdd(temp2[m], zero, temp2[m]); 
		if(polyOutput[m] < 0)
			ss->modAdd(temp3[m], zero, temp3[m]);
		ss->modInv(temp1[m], temp2[m]);  
		ss->modMul(temp3[m], temp3[m], temp1[m]); 
	}

	/************* Generate the random values ******************/
	for(int i = 0; i < size; i++){
		for(int m = 0; m < polysize; m++){
            		getNextRandValue(m, bits, polynomials, rand); 
			mpz_div_ui(rand, rand, combinations); 
			ss->modMul(temp, rand, temp3[m]);
            		ss->modAdd(results[i], results[i], temp);
		}
		
	}
	/*********** Free the memory **************/	
	for(int i = 0; i < polysize; i++)
	{	
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]); 
		mpz_clear(temp3[i]);
	} 
	free(temp1); 
	free(temp2); 
	free(temp3); 
	
	mpz_clear(zero); 
	mpz_clear(rand); 
}


void Random::generateRandValue(int nodeID, int bits, int size, mpz_t* results, int threadID){
	if(threadID == -1){
		generateRandValue(nodeID, bits, size, results);
		return; 
	} 
	std::map<std::string, std::vector<int> >::iterator it;
	mpz_t zero, rand, temp;
	int polysize = polynomials.size(); 
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_init(zero);
    	mpz_init(rand);
	mpz_init(temp); 
	mpz_set_ui(zero, 0);
	for(int i = 0; i < polysize; i++)
	{
		mpz_init(temp1[i]); 
		mpz_init(temp2[i]); 
		mpz_init(temp3[i]); 
	}

	int index = 0; 
	std::vector<long> polyOutput;
	std::vector<long> denominator; 
	long long combinations = nChoosek(ss->getPeers(), ss->getThreshold());
	
	if(isFirst_thread[threadID] == 0)
		getNextRandValue(0, 0, polynomials, NULL, threadID);
 
	/*************** Evaluate the polynomials on points ******************/
	for(it = polynomials.begin(); it != polynomials.end(); it++)
	{
		std::vector<int> polys = (*it).second; 
		denominator.push_back(polys[polys.size()-1]);
		polyOutput.push_back(computePolynomials((*it).second, nodeID));
	}

	for(int m = 0; m < polysize; m++)
	{
		mpz_set_si(temp2[m], denominator[m]);
		mpz_set_si(temp3[m], polyOutput[m]); 
		if(denominator[m] < 0)
			ss->modAdd(temp2[m], zero, temp2[m]); 
		if(polyOutput[m] < 0)
			ss->modAdd(temp3[m], zero, temp3[m]);
		ss->modInv(temp1[m], temp2[m]);  
		ss->modMul(temp3[m], temp3[m], temp1[m]); 
	}

	/************* Generate the random values ******************/
	for(int i = 0; i < size; i++){
		for(int m = 0; m < polysize; m++){
            		getNextRandValue(m, bits, polynomials, rand, threadID); 
			mpz_div_ui(rand, rand, combinations); 
			ss->modMul(temp, rand, temp3[m]);
            		ss->modAdd(results[i], results[i], temp);
		}
		
	}
	/*********** Free the memory **************/	
	for(int i = 0; i < polysize; i++)
	{	
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]); 
		mpz_clear(temp3[i]);
	} 
	free(temp1); 
	free(temp2); 
	free(temp3); 
	
	mpz_clear(zero); 
	mpz_clear(rand); 
	/* When multiple compute nodes attempt to access polynomials at the same time, there may exisit a conflict that results in the memory problem */    
	/*for(unsigned int i = 0; i < 2 ; i++){
		polys = polynomials.find(oldseed[i])->second;
		polynomials.erase(oldseed[i]);
		polynomials.insert(std::pair<uint64_t, std::vector<int> >(newseed[i], polys));
	}*/
}

void Random::generateRandValue(int nodeID, mpz_t mod, int size, mpz_t* results){
	std::map<std::string, std::vector<int> >::iterator it;
	mpz_t zero, rand, temp;
	int polysize = polynomials.size(); 
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_init(zero);
    	mpz_init(rand);
	mpz_init(temp); 
	mpz_set_ui(zero, 0);
	for(int i = 0; i < polysize; i++)
	{
		mpz_init(temp1[i]); 
		mpz_init(temp2[i]); 
		mpz_init(temp3[i]); 
	}

	int index = 0; 
	std::vector<long> polyOutput;
	std::vector<long> denominator; 
	long long combinations = nChoosek(ss->getPeers(), ss->getThreshold());
	
	if(isFirst == 0)
		getNextRandValue(0, 0, polynomials, NULL);
 
	/*************** Evaluate the polynomials on points ******************/
	for(it = polynomials.begin(); it != polynomials.end(); it++)
	{
		std::vector<int> polys = (*it).second; 
		denominator.push_back(polys[polys.size()-1]); 
		polyOutput.push_back(computePolynomials((*it).second, nodeID));
	}
		
	for(int m = 0; m < polysize; m++)
	{
		mpz_set_si(temp2[m], denominator[m]);
		mpz_set_si(temp3[m], polyOutput[m]); 
		if(denominator[m] < 0)
			ss->modAdd(temp2[m], zero, temp2[m]); 
		if(polyOutput[m] < 0)
			ss->modAdd(temp3[m], zero, temp3[m]);
		ss->modInv(temp1[m], temp2[m]);  
		ss->modMul(temp3[m], temp3[m], temp1[m]); 
	}

	/************* Generate the random values ******************/
	for(int i = 0; i < size; i++){
		for(int m = 0; m < polysize; m++){
            		getNextRandValue(m, mod, polynomials, rand);
			mpz_div_ui(rand, rand, combinations); 
			ss->modMul(temp, rand, temp3[m]);
            		ss->modAdd(results[i], results[i], temp);
		}
		
	}
	/*********** Free the memory **************/	
	for(int i = 0; i < polysize; i++)
	{	
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]); 
		mpz_clear(temp3[i]);
	} 
	free(temp1); 
	free(temp2); 
	free(temp3); 
	
	mpz_clear(zero); 
	mpz_clear(rand); 
}


void Random::generateRandValue(int nodeID, mpz_t mod, int size, mpz_t* results, int threadID){ 
	if(threadID == -1){
		generateRandValue(nodeID, mod, size, results); 
		return; 
	} 
	std::map<std::string, std::vector<int> >::iterator it;
	mpz_t zero, rand, temp;
	int polysize = polynomials.size(); 
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * polysize); 
	mpz_init(zero);
    	mpz_init(rand);
	mpz_init(temp); 
	mpz_set_ui(zero, 0);
	for(int i = 0; i < polysize; i++)
	{
		mpz_init(temp1[i]); 
		mpz_init(temp2[i]); 
		mpz_init(temp3[i]); 
	}

	int index = 0; 
	std::vector<long> polyOutput;
	std::vector<long> denominator; 
	long long combinations = nChoosek(ss->getPeers(), ss->getThreshold());
	
	if(isFirst_thread[threadID] == 0)
		getNextRandValue(0, 0, polynomials, NULL, threadID);
 
	/*************** Evaluate the polynomials on points ******************/
	for(it = polynomials.begin(); it != polynomials.end(); it++)
	{
		std::vector<int> polys = (*it).second; 
		denominator.push_back(polys[polys.size()-1]);
		polyOutput.push_back(computePolynomials((*it).second, nodeID));
	}

	for(int m = 0; m < polysize; m++)
	{
		mpz_set_si(temp2[m], denominator[m]);
		mpz_set_si(temp3[m], polyOutput[m]); 
		if(denominator[m] < 0)
			ss->modAdd(temp2[m], zero, temp2[m]); 
		if(polyOutput[m] < 0)
			ss->modAdd(temp3[m], zero, temp3[m]);
		ss->modInv(temp1[m], temp2[m]);  
		ss->modMul(temp3[m], temp3[m], temp1[m]); 
	}

	/************* Generate the random values ******************/
	for(int i = 0; i < size; i++){
		for(int m = 0; m < polysize; m++){
            		getNextRandValue(m, mod, polynomials, rand, threadID);
			mpz_div_ui(rand, rand, combinations); 
			ss->modMul(temp, rand, temp3[m]);
            		ss->modAdd(results[i], results[i], temp);
		}
		
	}
	/*********** Free the memory **************/	
	for(int i = 0; i < polysize; i++)
	{	
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]); 
		mpz_clear(temp3[i]);
	} 
	free(temp1); 
	free(temp2); 
	free(temp3); 
	
	mpz_clear(zero); 
	mpz_clear(rand); 
	/* When multiple compute nodes attempt to access polynomials at the same time, there may exisit a conflict that results in the memory problem */    
	/*for(unsigned int i = 0; i < 2 ; i++){
		polys = polynomials.find(oldseed[i])->second;
		polynomials.erase(oldseed[i]);
		polynomials.insert(std::pair<uint64_t, std::vector<int> >(newseed[i], polys));
	}*/
}

/*
 * this assumes that M is <= 64
 */
void Random::PRandM(int K, int M, int size, mpz_t** result){
	mpz_t* tempResult = (mpz_t*) malloc(sizeof(mpz_t)  * size * M);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	unsigned long pow = 1;
	unsigned long i, j;
	
	//do initialization
	for (i = 0; i < size * M; i++)
		mpz_init(tempResult[i]);

	for (i = 0; i < size; i++)
		mpz_init(temp[i]);

	PRandBit(size*M, tempResult);
	
	for (i = 0; i < size; i++) {
	  mpz_set(result[M][i], tempResult[i]); 
	  mpz_set(result[0][i], tempResult[i]); 
	}
	
	for (i = 1; i < M; i++) {
	  pow = pow << 1;
	  for (j = 0; j < size; j++) {
	    mpz_set(result[i][j], tempResult[i * size + j]);
	    mpz_mul_ui(temp[j], result[i][j], pow);
	  }
	  ss->modAdd(result[M], result[M], temp, size);
	}
	
	//free the memory
	for (i = 0; i < size * M; i++)
		mpz_clear(tempResult[i]);
	free(tempResult);
 
	for (i = 0; i < size; i++)
		mpz_clear(temp[i]);
	free(temp); 

}

void Random::PRandM(int K, int M, int size, mpz_t** result, int threadID){
	if(threadID == -1){
		PRandM(K, M, size, result);
		return; 
	} 
	mpz_t* tempResult = (mpz_t*) malloc(sizeof(mpz_t)  * size * M);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	unsigned long pow = 1;
	unsigned long i, j;
	
	//do initialization
	for(i = 0; i < size * M; i++)
		mpz_init(tempResult[i]);

	for(i = 0; i < size; i++)
		mpz_init(temp[i]);

	PRandBit(size*M, tempResult, threadID);
	
	for (i = 0; i < size; i++)
		mpz_set(result[M][i], tempResult[i]); 

	for (i = 1; i < M; i++) {
	  pow = pow << 1;
	  for (j = 0; j < size; j++) {
	    mpz_set(result[i][j], tempResult[i * size + j]);
	    mpz_mul_ui(temp[j], result[i][j], pow);
	  }
	  ss->modAdd(result[M], result[M], temp, size);
	}
	
	//free the memory
	for (i = 0; i < size * M; i++)
		mpz_clear(tempResult[i]);
	free(tempResult);
 
	for (i = 0; i < size; i++)
		mpz_clear(temp[i]);
	free(temp); 
}

void Random::PRandInt(int K, int M, int size, mpz_t* result)
{
	int bits = 48+K-M;
	generateRandValue(id, bits, size, result); 
}


void Random::PRandInt(int K, int M, int size, mpz_t* result, int threadID)
{
	if(threadID == -1){
		PRandInt(K, M, size, result); 
		return; 
	}
	int bits = 48+K-M;
	generateRandValue(id, bits, size, result, threadID); 
}

void Random::PRandBit(int size, mpz_t* results){
    int peers = ss->getPeers();
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	mpz_t* u = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* v = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t inv2, const1;

	//initialization
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(inv2, 2);
	ss->modInv(inv2, inv2);
 	
	for(int i = 0; i < peers; i++){
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(resultShares[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(u[i]);
		mpz_init(v[i]);
	}
	mpz_t field;  
	mpz_init(field); 
	ss->getFieldSize(field);
	generateRandValue(id, field, size, u); 
	ss->modMul(v, u, u, size);
	net.broadcastToPeers(v, size, resultShares, -1);
	ss->reconstructSecret(v, resultShares, size, true);
	ss->modSqrt(v, v, size);
	ss->modInv(v, v, size);
	ss->modMul(results, v, u, size);
	ss->modAdd(results, results, const1, size);
	ss->modMul(results, results, inv2, size);

	//free the memory
	mpz_clear(inv2);
	mpz_clear(const1); 
	for(int i = 0; i < peers; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(resultShares[i][j]);
		free(resultShares[i]); 
	}
	free(resultShares); 
	for(int i = 0; i < size; i++){
		mpz_clear(u[i]);
		mpz_clear(v[i]);
	}
	free(u); 
	free(v); 
}

void Random::PRandBit(int size, mpz_t* results, int threadID){
	if(threadID == -1){
		PRandBit(size, results); 
		return; 
	}
        int peers = ss->getPeers();
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	mpz_t* u = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* v = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t const1, inv2;

	//initialization
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(inv2, 2);
	ss->modInv(inv2, inv2);
 
	for(int i = 0; i < peers; i++){
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(resultShares[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(u[i]);
		mpz_init(v[i]);
	}
	/***********************************************************/
	mpz_t field;  
	mpz_init(field); 
	ss->getFieldSize(field);
	generateRandValue(id, field, size, u, threadID); 
	ss->modMul(v, u, u, size);
	net.broadcastToPeers(v, size, resultShares, threadID);
	ss->reconstructSecret(v, resultShares, size, true);
	ss->modSqrt(v, v, size);
	ss->modInv(v, v, size);
	ss->modMul(results, v, u, size);
	ss->modAdd(results, results, const1, size);
	ss->modMul(results, results, inv2, size);
	
	//free the memory
	mpz_clear(inv2);
	mpz_clear(const1); 
	for(int i = 0; i < peers; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(resultShares[i][j]);
		free(resultShares[i]); 
	}
	free(resultShares); 

	for(int i = 0; i < size; i++){
		mpz_clear(u[i]);
		mpz_clear(v[i]);
	}
	free(u); 
	free(v); 
}
