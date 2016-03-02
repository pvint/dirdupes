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

		int childCRC;
		int depth;

		Directory( string _path )
		{
			path = _path;
		}


		string getPath()
		{
			return path;
		}

		int getSubdirs();

		// Constructor
  		Directory( int numfiles = 0, int subdirs = 0, int depth = 0 );

  protected:
     // Attributes visible to descendents
  private:
     // Local attributes
};
              
