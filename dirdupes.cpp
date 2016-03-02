#include<iostream>
#include<string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dirdupes.h"


using namespace std;
using namespace boost::filesystem;

int getSubdirs( const char *path, vector<Directory>& v, int count )
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
			if( is_directory(*it) )
			{
				count++;
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


				cnt = getSubdirs ( (*it).path().string().c_str(), v, 0 );
				count += cnt;

				d->subdirs = cnt;

				v.push_back( *d );
				delete d;


				cout << "\r" << count ;
			}
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

	x = getSubdirs( baseDir.c_str(), directories ,0 );


	vector<Directory>::iterator it; 

	for(it=directories.begin() ; it < directories.end(); it++ )
	{
		// Just show the path for now
		cout << (*it).subdirs << " " << (*it).getPath() << endl;
	}

	cout << x << " directories found" << endl;
	//cout << directories.size() << " items in vector" << endl;

	return 0;

	


    return 0;
}

