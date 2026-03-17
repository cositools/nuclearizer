#include <map>
#include "MTime.h"


#ifndef __MTIRecord__
#define __MTIRecord__

class MTIRecord
{
	public:
		MTIRecord(size_t length = 1000);
    virtual ~MTIRecord() {};
		bool Add(long int t, int64_t PPS);
		bool Get(long int UTCIn, long int& UTCOut, int64_t& PPS);
		bool AddCorrect(long int& T, int64_t& PPS);
	private:
		size_t m_Length;
		map<long int,int64_t> m_Record;
};

#endif