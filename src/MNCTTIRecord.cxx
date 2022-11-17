#include "MNCTTIRecord.h"

MNCTTIRecord::MNCTTIRecord(size_t length)
{
	m_Length = length;
}

bool MNCTTIRecord::Add(long int t, int64_t PPS)
{
	bool Ret = false;
	if(m_Record.count(t) == 1){
		Ret = false;
	} else {
		if(m_Record.size() > 1){
			auto i = m_Record.lower_bound(t);
			if(i == m_Record.end()) --i;
			long int dt = t - i->first;
			MTime mdt(dt,(long int)0); //MTime UTC second delta
			int64_t PPSLSBs = PPS & 0xffffffff;
			int64_t PPSMSBs = PPS & 0x0000ffff00000000;
			int64_t NewPPS = 0;
			bool Success = false;
			int x,y;
			for(x = -1; x <= 1; ++x){ //tweak MSBs
				for(y = -2; y <= 2; ++y){ //tweak LSBs
					NewPPS = (PPSMSBs + (0x0000000100000000)*x) | PPSLSBs;
					NewPPS += (10000000)*y;
					int64_t dPPS = NewPPS - i->second;
					long int sec = (long int)(dPPS/10000000);
					long int nsec = (long int)((dPPS % 10000000)*100);
					MTime mdPPS(sec,nsec); //MTime PPS delta
					double diff = fabs(mdPPS.GetAsDouble() - mdt.GetAsDouble());
					//cout << "diff = " << diff << endl;
					if(diff <= 0.100){
						Success = true;
						break;
					}
				}
				if(Success) break;
			}
			if(Success){
				if(NewPPS != PPS){
					cout << "TIRecord:corrected PPS, x = " << x << ", y = " << y << " for t = " << t << " and PPS = " << PPS << endl;
				}
				m_Record[t] = NewPPS;
				Ret = true;
			} else {
				cout << "TIRecord:failed, expect TI issues!!!" << endl;
				Ret = false;
			}
		} else {
			m_Record[t] = PPS;
			if(m_Record.size() == 2){
				//check if record is consistent
				auto i1 = m_Record.begin();
				map<long int,int64_t>::iterator i2 = i1++;
				MTime dUTC(i2->first - i1->first,(long int)0);
				MTime dPPS((long int)((i2->second - i1->second)/10000000),(long int)((i2->second - i1->second)%10000000)*100);
				double diff = fabs(dUTC.GetAsDouble() - dPPS.GetAsDouble());
				if(diff >= 0.100) m_Record.erase(m_Record.begin());
			}
			Ret = true;
		}
	}

	while(m_Record.size() > m_Length) m_Record.erase(m_Record.begin()); //trim record down to size
	return Ret;
}

bool MNCTTIRecord::AddCorrect(long int& t, int64_t& PPS)
{
	if(m_Record.size() < 2){
		m_Record[t] = PPS;
		if(m_Record.size() == 2){
			auto i1 = m_Record.begin();
			map<long int,int64_t>::iterator i2 = i1++;
			MTime dUTC(i2->first - i1->first,(long int)0);
			MTime dPPS((long int)((i2->second - i1->second)/10000000),(long int)((i2->second - i1->second)%10000000)*100);
			double diff = fabs(dUTC.GetAsDouble() - dPPS.GetAsDouble());
			if(diff >= 0.100) m_Record.erase(m_Record.begin());
		}
		return false;
	} else {
		auto i = m_Record.lower_bound(t);
		if(i == m_Record.begin()){
			return false;
		} else {
			--i;
		}

		//check a few common cases
		if(t == i->first){ //GPS second is the same
			if(labs((PPS - i->second) - 10000000) < 100){//PPS differs by ~ 1 second
				++t;
				cout << "TIRecord: incrementing GPS second by 1: (" << i->first << "," << i->second << ") ---> (" << t << "," << PPS << ")" << endl;
				m_Record[t] = PPS;
				return true;
			} else {
				return false;
			}
		} else {
			int64_t PPSLSBs = PPS & 0xffffffff;
			int64_t PPSMSBs = PPS & 0x0000ffff00000000;
			int64_t NewPPS = 0;
			bool Success = false;
			int x,y;
			long int dt = t - i->first;
			MTime mdt(dt,(long int)0); //MTime UTC second delta
			for(x = -1; x <= 1; ++x){ //tweak MSBs
				for(y = -2; y <= 2; ++y){ //tweak LSBs
					NewPPS = (PPSMSBs + (0x0000000100000000)*x) | PPSLSBs;
					NewPPS += (10000000)*y;
					int64_t dPPS = NewPPS - i->second;
					long int sec = (long int)(dPPS/10000000);
					long int nsec = (long int)((dPPS % 10000000)*100);
					MTime mdPPS(sec,nsec); //MTime PPS delta
					double diff = fabs(mdPPS.GetAsDouble() - mdt.GetAsDouble());
					//cout << "diff = " << diff << endl;
					if(diff <= 0.100){
						Success = true;
						break;
					}
				}
				if(Success) break;
			}
			if(Success){
				if(NewPPS != PPS){
					cout << "TIRecord:corrected PPS, x = " << x << ", y = " << y << " for t = " << t << " and PPS = " << PPS << endl;
				}
				m_Record[t] = NewPPS;
				PPS = NewPPS;
				return true;
			} else {
				cout << "TIRecord:failed, expect TI issues!!!" << endl;
				return false;
			}
		}
	}
}

bool MNCTTIRecord::Get(long int t, long int& UTCSecond, int64_t& PPS){
	if(m_Record.size() < 2){
		PPS = 0;
		return false;
	} else {
		if(m_Record.count(t) == 1){
			UTCSecond = t;
			PPS = m_Record[t];
			return true;
		} else {
			auto i = m_Record.lower_bound(t);
			if(i == m_Record.end()) --i;
			UTCSecond = i->first;
			PPS = i->second;
			return true;
		}
	}
}
