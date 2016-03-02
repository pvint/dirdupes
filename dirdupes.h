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

		Directory( string _path )
		{
			path = _path;
		}


		string getPath()
		{
			return path;
		}

		int getSubdirs();

     // Constructors:
     // Destructor:
     // Functions: modifiers (set), selectors (get)
     // itterators:
     // Attributes visible by scope of instantiation and use
  protected:
     // Attributes visible to descendents
  private:
     // Local attributes
};
              
