#include <boost/format.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/range/algorithm.hpp>
#include <mpi.h>
#include <vector> 
#include <fstream>
#include <iostream>
#define Buff_SIZE 100000


#define SERIAL

using namespace std;

  class index_element{
    public:
    long index;
    double element;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & index;
      ar &element;
    }
    friend ostream& operator<<(ostream& os, const index_element& onepiece) {
      os << onepiece.index<< "\t\t\t" << onepiece.element;
      return os;
    }
    bool operator<(const index_element& onepiece2) const{
      if(index < onepiece2.index) return true;
      if(index > onepiece2.index) return false;
      if(index == onepiece2.index){
        cout << "there are elements with same index"<<endl;
        if(element !=onepiece2.element) 
        {
          cout << " and their values are different, abort"<<endl;
          abort();
        }

      }
    }
  };

//int main(int argc, char* argv[]){
//
//  //std::fstream file(argv[1],std::ios::in|std::ios::binary);
//  FILE* datafile;
//  datafile=fopen(argv[1],"rb");
//  //boost::archive::binary_iarchive loaddata(file);
//  std::vector<index_element> data;
//  long read_size;
//  bool read_success = true;
//  while(read_success){
//    index_element onepiece[buff_size];
//    read_size=fread(onepiece,sizeof(index_element),buff_size,datafile);
//    if(read_size == 0) break;
//    else if( read_size != buff_size) read_success =false;
//  //  loaddata >> onepiece;
//    //file >> onepiece.index;
//    //file >> onepiece.element;
//    for(int i =0 ; i< buff_size;i++)
//      cout << onepiece[i]<<endl;
//    //data.push_back(onepiece[0]);
//
//  };
//
//}

template< class T> 
class cache
{
  public:
  typedef T valuetype;
  //cache(char* filename ){}
//  cache(char* filename, size_t buff_size) {
//    begin_pionter=malloc(sizeof(valuetype*buff_size));
//
//    inputfile=fopen(finename,"rb"); 
//    if(inputfile==NULL){
//      current=pointer = NULL;
//      cout << "cannot open \n";
//      return;
//    }
//    size_t real_size=fread(begin_pointer,sizeof(valuetype),buff_size,inputfile);
//    if(real_size ==0) {fclose(inputfile); current_pointer=NULL;}
//    else if ( real_size< buff_size){
//      buff_size = real_size;
//      fclose(inputfile);
//      finishedread=true;
//    }
//    end_pointer = begin_pointer+buff_size;
//    current_pointer = begin_pointer;
//    return current_pointer++;
//
//  }
    
  //seek_set of file makes it possible to seperate one file into different parts.
  //Now, it is not implented because, there is no option to detect the end of pieces of one file.
  cache(char* filename, size_t buff_size, size_t seek_set_of_file =0) {
    begin_pointer=(valuetype*) malloc(sizeof(valuetype)*buff_size);
    if(begin_pointer==NULL){
      cout << "cannot allocate memory for cache, abort\n";
      abort();
    }

    inputfile=fopen(filename,"rb"); 
    if(inputfile==NULL){
      current_pointer = NULL;
      cout << "cannot open :"<<filename<<endl;
      return;
    }
    fseek(inputfile,sizeof(valuetype)*seek_set_of_file,SEEK_SET);
    size_t real_size=fread(begin_pointer,sizeof(valuetype),buff_size,inputfile);
    if(real_size ==0) {fclose(inputfile); current_pointer=NULL;}
    else if ( real_size< buff_size){
      buff_size = real_size;
      fclose(inputfile);
      finishedread=true;
    }
    end_pointer = begin_pointer+buff_size;
    current_pointer = begin_pointer;

  }
  ~cache(){
    free(begin_pointer);
  }
  valuetype* pop(){
    if(current_pointer!=end_pointer){
      return current_pointer++;
    }
    else{
      if(finishedread) return NULL;
      size_t real_size=fread(begin_pointer,sizeof(valuetype),buff_size,inputfile);
      if(real_size ==0) {fclose(inputfile); return NULL;}
      else if ( real_size< buff_size){
        buff_size = real_size;
        fclose(inputfile);
        finishedread=true;
      }
      end_pointer = begin_pointer+buff_size;
      current_pointer = begin_pointer;
      return current_pointer++;

    }
  }
  valuetype* current_position(){
    return current_pointer;
  }
  valuetype& value(){
    return *current_pointer;
  }

  private:
    bool finishedread;
    valuetype* current_pointer;
    valuetype* begin_pointer;
    valuetype* end_pointer;
    size_t buff_size;
    FILE* inputfile;

  

};


template<class T> 
void externalsort(char*, char* );

int main(int argc, char* argv[]){

  char sortedfile[100]= "outfile";
  externalsort<index_element>(argv[1],sortedfile);

}

//outfilename and inputfilename can be the same. There is no conflict.
template<class T> 
void externalsort(char* inputfilename, char* outputfilename){
  FILE* datafile;
  FILE* outputfile;
  long read_size;
  bool read_success =true;
  datafile=fopen(inputfilename,"rb");
  int piecesnumber=0;
  // sort within different pieces;
  while(read_success){
    char tmpfile[100];
#ifndef SERIAL
    sprintf (tmpfile, "%s%d%s%d","tmpsortingfile", piecesnumber,".",mpigetrank());
#else
    sprintf (tmpfile, "%s%d","tmpsortingfile", piecesnumber);
#endif
    outputfile=fopen(tmpfile,"wb");
    //index_element onepiece[Buff_SIZE];
    T onepiece[Buff_SIZE];
    read_size=fread(onepiece,sizeof(T),Buff_SIZE,datafile);
    if(read_size == 0) break;
    else if( read_size != Buff_SIZE) read_success =false;

    std::vector<index_element> sortedpiece(onepiece,onepiece+read_size);
    boost::sort(sortedpiece);
    std::copy(sortedpiece.begin(),sortedpiece.end(),onepiece);
  //  loaddata >> onepiece;
    //file >> onepiece.index;
    //file >> onepiece.element;
    //for(int i =0 ; i< Buff_SIZE;i++)
    //  cout << onepiece[i]<<endl;
    //data.push_back(onepiece[0]);
    fwrite(onepiece,sizeof(T),read_size,outputfile);
    piecesnumber++;
    fclose(outputfile);
    cout << "piecenumber: "<<piecesnumber<<endl;

  };
  fclose(datafile);
  // Merge these pieces;
  
  //std::vector<FILE*> mergepieces;
  //mergepieces.resize(piecesnumber);
  //for(int i=0; i<piecesnumber;i++){
  //  mergepieces[i]= fopen("tmpsortfile","rb");
  //  fseekp(mergepieces[i],i*buffsize*sizeof(T),SEEK_SET);
  //}
  //std::vector<T*> cach;
  //for(int i=0; i<piecesnumber;i++){
  //  cach[i]=malloc((buffsize/piecesnumbr)*sizeof(T));
  //  fread(cach[i],sizeof(T),buffsize/piecesnumber,mergepieces[i]);
  //}
  std::vector<cache<T>> filecache;
  char tmpfile[100];
  for(int i =0; i<= piecesnumber; i++){
#ifndef SERIAL
    sprintf (tmpfile, "%s%d%s%d","tmpsortingfile", i,".",mpigetrank());
#else
    sprintf (tmpfile, "%s%d","tmpsortingfile", i);
#endif

    cache<T> tmpcache( tmpfile, Buff_SIZE/piecesnumber);
    if(tmpcache.current_position()!= NULL)
      filecache.push_back(tmpcache);
  }

  //outputfile = fopen(outputfilename,"wb");
  outputfile = fopen(outputfilename,"w");

  cout << "Okay?"<<endl;
  T outputbuff[Buff_SIZE];
  long outputbuff_position=0;
  // select the smallest one in the current positions of different caches.
  for(;;){
    int smallest = 0;
    for(int i=0 ; i< filecache.size(); i++){
      if(filecache[i].value() < filecache[smallest].value())
        smallest =i;
    }
    cout << filecache[smallest].value()<<endl;
    outputbuff[outputbuff_position++]=*(filecache[smallest].pop());
    if(outputbuff_position == Buff_SIZE){
      fwrite(outputbuff,sizeof(T),Buff_SIZE,outputfile);
      outputbuff_position=0;
    }
    if(filecache[smallest].current_position() ==NULL){
      filecache.erase(filecache.begin()+smallest);
      if (filecache.size()==0) break;
    }
  }
  fclose(outputfile);



}

