#include <string>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "md5/md5.h"

using namespace std;


class Directory
{
	public:
		string path, name;
		int inode, numfiles, subdirs;
		long int du;
		md5_byte_t listingMD5;

<<<<<<< HEAD
		int childCRC;
		int depth;

=======
>>>>>>> c534e4a52de02e95210859c7dce1998b4880441b
		Directory( string _path )
		{
			path = _path;
		}


		string getPath()
		{
			return path;
		}

		int getSubdirs();

<<<<<<< HEAD
		// Constructor
  		Directory( int numfiles = 0, int subdirs = 0, int depth = 0 );
=======
     // Constructors:
     // Destructor:
     // Functions: modifiers (set), selectors (get)
     // itterators:
     // Attributes visible by scope of instantiation and use
>>>>>>> c534e4a52de02e95210859c7dce1998b4880441b
  protected:
     // Attributes visible to descendents
  private:
     // Local attributes
};
              
