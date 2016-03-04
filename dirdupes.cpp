#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/crc.hpp>  // for boost::crc_32_type
#include <boost/bind.hpp>
#include "dirdupes.h"


using namespace std;
using namespace boost::filesystem;

int dirCount = 0;
bool skipEmpty = true;

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
		s += itr->path().filename().string();
	cout << s << endl;
        }
	return s;    
}

void  getDirectorySize(string rootFolder,long & file_size){
        //replace_all(rootFolder, "\\\\", "\\");   
        path folderPath(rootFolder);                      
        if (exists(folderPath)){
            directory_iterator end_itr;

            for (directory_iterator dirIte(rootFolder); dirIte != end_itr; ++dirIte )
            {
                path filePath(dirIte->path());
                try{
                    if (!is_directory(dirIte->status()) )
                    {
cout << filePath << endl;
                        file_size = file_size + boost::filesystem::file_size(filePath);                      
                    }else{
                        //getDirectorySize(filePath.string(),file_size);
                    }
                }catch(exception& e){               
                    cout << e.what() << endl;
                }
            }
        }

    }


int getSubdirs( const char *path, vector<Directory>& v, int count, int depth )
{
	// Get subdirs and add them to the vector
	vector<directory_entry> dv;
	vector<directory_entry> subdv;

	string files;
	int cnt = count;
	string contents;	// use a simple concatenated string to create a quick hash of contents

	if ( is_directory( path ) )
	{
		copy( directory_iterator( path ), directory_iterator (), back_inserter( dv ) );	
		for ( vector<directory_entry>::const_iterator it = dv.begin(); it != dv.end();  ++ it )
		{
			int oldDepth = depth;	// Save the depth value when descending directory so it's correct when coming back up
			if( is_directory(*it) )
			{

				std::cout<< "ZZZ" << (*it).path().string() << endl;
				dirCount ++;

				// Create Directory object
				Directory *d = new Directory( (*it).path().string() );

				// Get directory listing CRC
				// FIXME For now calling two functions, one to get a string, one to get CRC
				// FIXME Need to come up with a nice efficient method, but this works for now
				files = listFiles( (*it).path() );
				d->childCRC = crcString( files );

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
				
				count += getSubdirs ( (*it).path().string().c_str(), v, cnt, depth + 1 );
				//cout << "==== " << (*it).path().string() << endl;
				d->subdirs = cnt;

				d->depth = depth;
				depth = oldDepth;

				v.push_back( *d );
				delete d;

				cnt++;

				cout << "\r" << count ;
			}
			
			// reset the depth 
			depth = oldDepth;	// FIXME
		}

		return cnt;
	}
	return 0;
}

int main(int argc, char** argv) {
    int opt, cnt = 0;
    string baseDir = "";
    bool flagA = false;
    bool flagB = false;


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
cout << "DIRS: " << directories.size();

	vector<Directory>::iterator it; 
	vector<Directory>::iterator dit;

	
	vector<Directory> round1;

	//DirDupes dupes;
	// Testing using std::set rather than vector to store dupe paths
	set<string> dupes;

	// TODO  Try a few different mothods of searching for dupes and time them
	for(it=directories.begin() ; it < directories.end(); it++ )
	{
		cnt++; 

		// Just show the path for now
		cout << (*it).depth << ": " << (*it).childCRC << "|" << (*it).subdirs << " " << (*it).getPath() << endl;
		/* bogus search with boost::bind
		std::vector<Directory>::iterator dit = std::find_if(
				directories.begin(),
				directories.end(),
				boost::bind(&Directory::childCRC, _1) == (*it).childCRC );

		vector<Directory>::iterator foundIt;
		for ( foundIt = dit.begin(); foundIt < dit.end; foundIt++ )
		{
			cout << "Found one!" << endl;
		}
		*/
		for(dit=directories.begin() ; dit < directories.end(); dit++ )
		{
			// Check if crc matches any others - remove item if not
			if ( ( (*it).childCRC == (*dit).childCRC ) && ( (*it).path != (*dit).path ) )
			{
				cout << (*it).path << " has dupe(s)!! " << (*dit).childCRC << " and " << (*dit).path << endl;

				// Flag each as a potential dupe
				(*it).potentialDupe = true;
				(*dit).potentialDupe = true;

				// insert paths into std::set
				dupes.insert( (*it).path );
				dupes.insert( (*dit).path );
				

			}
			else
			{
				//(*dit).potentialDupe = false;	// Do I need to do this? Set fasle by default?? TODO
				// Just erase the iterator object!!! 
				// TODO NOT TESTED YET!!!  it = directories.erase(it);
			}
		}

	}

	cout << dirCount + 1 << " directories found" << endl;
	cout << dupes.size() << " potential dupes found in round 1" << endl << endl;


	// Free the memory - need to save anything first? FIXME
	directories.clear();
	set<string>::iterator sit;

	long dirSize = 0;
	long totSize = 0;
	for ( sit = dupes.begin(); sit != dupes.end(); sit++ )
	{
		// Now get the total filesize of each file in dir (non-recursive)
		// TODO: Better to do it recursively right now maybe?
		//cout << *sit << endl;
		dirSize = 0;
		getDirectorySize( *sit, dirSize ) ;
		cout << *sit << ": " << dirSize << endl;
		totSize += dirSize;
	}
cout << totSize << endl;

	return 0;


    return 0;
}

