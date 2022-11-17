/*
 * MNCTArray.cxx
 *
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTArray
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTArray.h"

// Standard libs:
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTArray)
#endif


////////////////////////////////////////////////////////////////////////////////


double_array4 & MNCTArray::Array_loader4(MString filename,int a,int b,int c,int d)
{
  //int n_element=a*b*c*d;
  cout << "filename: " << filename << '\n';
  double_array4* response_array=new double_array4(boost::extents[a][b][c][d]);  
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);  
  if(!f1.is_open()){
    cerr << " file does not exist!!!\n";
    exit(1);
  }
  
  istream_iterator<double> fbegin(f1);
  istream_iterator<double> fend;//end-of-stream
  
  //assign array values from input file
  (*response_array).assign(fbegin,fend);//this function do not check element number in istream
  f1.close();
  return *response_array;
}


////////////////////////////////////////////////////////////////////////////////


double_array3 & MNCTArray::Array_loader3(MString filename,int a,int b,int c)
{
  //int n_element=a*b*c;
  cout << "filename: " << filename << '\n';
  double_array3* response_array=new double_array3(boost::extents[a][b][c]);
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  if(!f1.is_open()){
    cerr << " file does not exist!!!\n";
    exit(1);
  }
  istream_iterator<double> fbegin(f1);
  istream_iterator<double> fend;//end-of-stream
  
  //assign array values from input file
  (*response_array).assign(fbegin,fend);//this function do not check element number in istream
  f1.close();
  return *response_array;
}


////////////////////////////////////////////////////////////////////////////////


double_array2 & MNCTArray::Array_loader2(MString filename,int a,int b)
{
  //int n_element=a*b;
  cout << "filename: " << filename << '\n';
  double_array2* response_array=new double_array2(boost::extents[a][b]);
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  if(!f1.is_open()){
    cerr << " file does not exist!!!\n";
    exit(1);
  }
  istream_iterator<double> fbegin(f1);
  istream_iterator<double> fend;
  
  //assign array values from input file 
  (*response_array).assign(fbegin,fend);//this function do not check element number in istream
  f1.close();
  return *response_array;
}


////////////////////////////////////////////////////////////////////////////////


double_array4& MNCTArray::csv_loader4(MString filename,int a,int b,int c,int d, string mode)
{
  char buffer[512];
  string sbuffer;
  int ia=0,ib=0,ic=0,id=0;
  int n=0;
  double_array4* response_array=new double_array4(boost::extents[a][b][c][d]);
  
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading .csv file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    
    sbuffer=buffer;
    //cout << "#debug: " << sbuffer << '\n';//debug
    istringstream isbuffer(sbuffer);
    
    
    if(buffer[0]!='#')
    {
      isbuffer >> ia
      >> ib
      >> ic;
      //cout << ' ' << ia << ' ' << ib << ' ' << ic;//debug 
      double temp=0;
      if(mode=="one")
      {
        //cout << "debug: one\n";//debug
        isbuffer >> id;
        isbuffer >> temp;
        (*response_array)[ia][ib][ic][id]=temp;
        n++;
      }
      else
      {
        //cout << "debug: many\n";//debug
        for(int i=0;i<d;i++)
        {
          isbuffer >> temp;
          (*response_array)[ia][ib][ic][i]=temp;
          //cout << '\n' << temp;//debug
          n++;
        }
      }
    }
    //cout << m_DetectorNumber;//debug
  }
  if(n!=a*b*c*d)cerr<<"Error: Total elements is not correct!!!"<< '\n';
  return *response_array;
}


////////////////////////////////////////////////////////////////////////////////


double_array3& MNCTArray::csv_loader3(MString filename,int a,int b,int c, string mode)
{
  char buffer[512];
  string sbuffer;
  int ia=0,ib=0,ic=0;
  int n=0;
  double_array3* response_array=new double_array3(boost::extents[a][b][c]);
  
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading .csv file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    
    sbuffer=buffer;
    //cout << "#debug: " << sbuffer << '\n';//debug
    istringstream isbuffer(sbuffer);
    
    if(buffer[0]!='#')
    {
      isbuffer >> ia
      >> ib;
      double temp=0;
      if(mode=="one")
      {
        isbuffer >> ic;
        isbuffer >> temp;
        (*response_array)[ia][ib][ic]=temp;
        n++;
      }
      else
      {
        for(int i=0;i<c;i++)
        {
          isbuffer >> temp;
          (*response_array)[ia][ib][i]=temp;
          n++;
        }
      }
    }
    //cout << m_DetectorNumber;//debug
  }
  
  if(n!=a*b*c)cerr<<"Error: Total elements is not correct!!!"<< '\n';
  return *response_array;
}


////////////////////////////////////////////////////////////////////////////////


double_array2& MNCTArray::csv_loader2(MString filename,int a,int b, string mode)
{
  char buffer[512];
  string sbuffer;
  int ia=0,ib=0;
  int n=0;
  double_array2* response_array=new double_array2(boost::extents[a][b]);
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading .csv file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    
    sbuffer=buffer;
    //cout << "#debug: " << sbuffer << '\n';//debug
    istringstream isbuffer(sbuffer);
    
    if(buffer[0]!='#')
    {
      isbuffer >> ia;
      double temp=0;
      if(mode=="one")
      {
        isbuffer >> ib;
        isbuffer >> temp;
        (*response_array)[ia][ib]=temp;
        n++;
      }
      else
      {
        for(int i=0;i<b;i++)
        {
          isbuffer >> temp;
          (*response_array)[ia][i]=temp;
          n++;
        }
      }
    }
    //cout << m_DetectorNumber;//debug
  }
  if(n!=a*b)cerr<<"Error: Total elements is not correct!!!"<< '\n';
  return *response_array;
}
////////////////////////////////////////////////////////////////////////////////
// MNCTArray.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

