#include<iostream>
#include<string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/crc.hpp>  // for boost::crc_32_type
#include "dirdupes.h"


using namespace std;
using namespace boost::filesystem;

int crcString ( string s )
{
	boost::crc_32_type result;
	result.process_bytes( s.data(), s.length());
	return result.checksum();
}	

string listFiles ( path p )
{
	// just a string containing all the filenames - will not be read by humans
	string s;
	directory_iterator end_itr;
	// cycle through the directory
	for (directory_iterator itr(p); itr != end_itr; ++itr)
	{
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (is_regular_file(itr->path())) {
			// assign current file name to current_file and echo it out to the console.
			s += itr->path().string();
		}
        }
	return s;    
}


int getSubdirs( const char *path, vector<Directory>& v, int count, int depth )
{
	// Get subdirs and add them to the vector
	vector<directory_entry> dv;
	vector<directory_entry> subdv;

	int cnt;
	string contents;	// use a simple concatenated string to create a quick hash of contents

	if ( is_directory( path ) )
	{
		copy( directory_iterator( path ), directory_iterator (), back_inserter( dv ) );	
		for ( vector<directory_entry>::const_iterator it = dv.begin(); it != dv.end();  ++ it )
		{
			int oldDepth = depth;	// Save the depth value when descending directory so it's correct when coming back up
			if( is_directory(*it) )
			{
				//std::cout<< (*it).path().string() << endl;

				// Create Directory object
				Directory *d = new Directory( (*it).path().string() );

				// create hash of names of directory items (to quickly be able to discard non-dupes)
				string s;
				copy( directory_iterator( (*it).path() ), directory_iterator (), back_inserter( subdv ) );
				for ( vector<directory_entry>::const_iterator subit = subdv.begin(); subit != subdv.end();  ++ subit )
				{
					s += (*subit).path().string();
				}
				// create md5 hash of path strings

				// To set the depth of each, need to increment when descending directories
				// and then decrement when coming back up
				
				count += getSubdirs ( (*it).path().string().c_str(), v, count, depth + 1 );

				d->subdirs = cnt;

				d->depth = depth;
				depth = oldDepth;

				v.push_back( *d );
				delete d;


				cout << "\r" << count ;
			}
			
			// reset the depth 
			depth = oldDepth;
		}

		return count;
	}
	return 0;
}

int main(int argc, char** argv) {
    int opt;
    string baseDir = "";
    bool flagA = false;
    bool flagB = false;

    int dirCount = 0;

    struct stat st;


    // Retrieve the (non-option) argument:
    if ( (argc <= 1) || (argv[argc-1] == NULL) || (argv[argc-1][0] == '-') ) {  // there is NO input...
        cerr << "No argument provided!" << endl;
        return 1;
    }
    else {  // there is an input...
        baseDir= argv[argc-1];
	if( !( stat( baseDir.c_str(), &st ) == 0 && S_ISDIR( st.st_mode ) ) )
	{
		cerr << baseDir << " is not a valid directory" << endl;
		return 1;
	}

    }

    // Debug:
    cout << "Base Dir = " << baseDir << endl;

    // Shut GetOpt error messages down (return '?'): 
    opterr = 0;

    // Retrieve the options:
    while ( (opt = getopt(argc, argv, "ab")) != -1 ) {  // for each option...
        switch ( opt ) {
            case 'a':
                    flagA = true;
                break;
            case 'b':
                    flagB = true;
                break;
            case '?':  // unknown option...
                    cerr << "Unknown option: '" << char(optopt) << "'!" << endl;
                break;
        }
    }


	

	// Get all directories under baseDir
	vector<Directory> directories;	
	int x;
	cout << "Directories found:" << endl;

	x = getSubdirs( baseDir.c_str(), directories ,0 ,0);


	vector<Directory>::iterator it; 

	for(it=directories.begin() ; it < directories.end(); it++ )
	{
		// Just show the path for now
		cout << (*it).depth << ": " << (*it).subdirs << " " << (*it).getPath() << endl;
	}

	cout << x << " directories found" << endl;
	//cout << directories.size() << " items in vector" << endl;

	return 0;

	


    return 0;
}
