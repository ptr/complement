// TestSTLHash.cpp 
// Short demonstrator that helps reproducing a bug in the hash-table implementation 
// of STLPort 5.0.1/5.0.2 when using Microsoft Visual C++ 2005 on Windows XP.
//
// Problem: Fill a hash_multimap with entries of which many have the same key
//          Internally, entries with the same key are kept as one block within the same bucket. 
//          Thus, when calling equal_range(key) the begin/end of that block is returned.
//          However, this code shows that for key =3, that block is destroyed after inserting the 194th element.
//          According to _hashtable.c we will have a rehash from size 193 to size 389 in that situation.
//          After that rehash,  equal_range only returns 2 elements with key = 3 whereas there are 65 in it.
// Reproduction:
//          In the main()-method we fill a hash_multimap as well as a multi_map  with the same <key, data> pairs
//          After each insertion we call CHECK(...) to assure validity of these two containers.
//          This works fine up to the 193th insertion. Insertion 194 generates the bug.
//
//          CHECK() works as follows:
//          (a) we check whether both containers contain the same number of elements.
//          (b) Assuming that the multi_map works correctly, we iterate over all its elements and check 
//              whether we can find that key also in the hash_multimap. We collect all data for that specific
//              key in in a set ("collection"). Notice that data is unique by construction in main(), thus the 
//              number of elements in the set must equal the number of entries in the hash_multimap and in the multimap
//          (c) We check if we have seen as many data elements in collection as we have seen in the multimap.
//              if so, we print "OK", otherwise we print a detailed key/data overview and assert.
// Caution:
//        There are several configurations of the program that will NOT fail. (see comment in main())
//        E.g. it seems that whenever the keys are more or less sorted, the problem does not occur.
//        Also, using numbers from 200 downto 1 or from 300 downto 1 cannot generate the problem,
//        whereas using 400 downto 1 will fail.
//        Finally, if we use key 1 (rather than key 3) we cannot generate a problem.        

// #define _STLP_DEBUG

#include <iostream>
#include <hash_map>
#include <map>
#include <set>
#include <cassert>

using namespace std;

typedef hash_multimap<int, int> hashType ;
typedef multimap<int, int>      mapType  ;


void check(hashType &h, mapType &m)
{
	set<int> collection;

	// (a) check sizes
	assert (h.size() == m.size());
	cout << "Checking Hash-Size: " << static_cast<unsigned>(h.size()) ;

	// (b) iterate over multi_map
	for (mapType::iterator mIter = m.begin(); mIter != m.end(); mIter++)
	{		
		// look up that key in hash-table and keep all data in the set
		pair<hashType::iterator,hashType::iterator> range = h.equal_range(mIter->first);
		for (hashType::iterator h = range.first; h != range.second; h++)
		{	
			collection.insert (h->second);
		}
	}
		
	// (c) we should have seen as many elements as there are in the hash-table
	if (collection.size() == h.size()) cout << " OK" << endl;
	else  
	{
		// if not, please report
		cout << " FAILED: " << endl;
		int lastKey  = -1;		
		// iterate over all elements in multi_map
		for (mapType::iterator mIter = m.begin(); mIter != m.end(); mIter++)
		{
			// new key? print a new status line
			if (mIter->first != lastKey) 
			{									
			    cout << endl << "Key : " << mIter->first << endl;					
				lastKey = mIter->first;				

				// print all hashed values for that key
				cout << " data in hash: ";
				pair<hashType::iterator,hashType::iterator> range = h.equal_range(mIter->first);				
				for (hashType::iterator h = range.first; h != range.second; h++)
				{
					assert (h->first == lastKey);
					cerr << h->second << ", "; // print all data for that key in Hash-Table				
				}								
				cout << endl << " data in map:  ";				
			}	
			// and print all member in multi-map until the next key occurs
			cout << mIter->second << ", " ;  // print all data for that key in Map			
		}
	}		
	assert (collection.size() == h.size());	// stopper

}


int main(int argc, char* argv[])
{
	hashType h;
	mapType  m;

	// CAUTION the following configurations WORKS in our setting
	//      for (int id = 1; id != 400; id ++)   and int key = (id %3 == 0 ? 3 : id)
	//      for (int id = 200; id != 1; id --)   and int key = (id %3 == 0 ? 3 : id)
	//      for (int id = 300; id != 1; id --)   and int key = (id %3 == 0 ? 3 : id)
	//      for (int id = 400; id != 1; id --)   and int key = (id %3 == 0 ? 1 : id)
	//      for (int id = 4000; id != 1; id --)  and int key = (id %3 == 0 ? 1 : id)
	//
	// whereas these will FAIL
	//      for (int id = 400; id != 1; id --)   and int key = (id %3 == 0 ? 3 : id)
	//      for (int id = 4000; id != 1; id --)  and int key = (id %3 == 0 ? 3 : id)
	//      

	for (int id = 400; id != 1; id --) 
	{
		// generate many entries with key 3, fill up with unique keys. Data is unique (needed in check())
		int key = (id %3 == 0 ? 3 : id); 

		// keep hash_multi_map and multimap in sync
		h.insert (make_pair(key, id));
		m.insert (make_pair(key, id));

		// check whether both contain the same elements
		check(h,m);
	}

	return 0;
}


 	  	 
