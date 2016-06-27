#include "TH1.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"
#include <vector>
#include <iostream>
using namespace std;

int main(int argc, char ** argv){
	if(argc < 4){
		cout << "need at least 3 args, out file name, then at least two input files" << endl;
		return -1;
	}

	TFile* fout = new TFile(argv[1],"recreate");
	TFile* f1 = new TFile(argv[2]);
	TList* l = f1->GetListOfKeys();
	vector<TFile*> fn;
	for(int i = 3; i < argc; ++i){
		TFile* f = new TFile(argv[i]);
		fn.push_back(f);
	}

	for(int i = 0; i < l->GetSize(); ++i){
		TH1* h1 = (TH1*) f1->Get(l->At(i)->GetName());
		if(h1 != NULL){
			cout << "summing TH1's with name " << h1->GetName() << endl;
			for(const auto f:fn){
				TH1* h = (TH1*) f->Get(h1->GetName());
				if(h != NULL){
					h1->Add(h);
				} else {
					cout << "error" << endl;
				}
			}
		}
		fout->WriteTObject(h1);
	}

	fout->Close();
	f1->Close();
	////
	for(const auto f:fn){
		f->Close();
	}
	return 0;
}
				

