#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/crc.hpp>  // for boost::crc_32_type
#include <boost/bind.hpp>
#include "dirdupes.h"


#define DEBUGLEVEL 5
#define MD5SAMPLESIZE 1024	// iterations of buffer size


using namespace std;
using namespace boost::filesystem;

int dirCount = 0;
bool skipEmpty = true;
bool followSymlinks = false;

void infoLog ( string s )
{
	cerr << s << endl;
}

int crcString ( string s )
{
	boost::crc_32_type result;
	result.process_bytes( s.data(), s.length());
	return result.checksum();
}	

void print_md5_sum(unsigned char* md) {
    for(unsigned i=0; i <MD5_DIGEST_LENGTH; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md[i]);
    }
}

bool compareMD5( unsigned char* md1, unsigned char* md2 )
{
	for(unsigned i=0; i <MD5_DIGEST_LENGTH; i++)
	{
		if ( md1[i] != md2[i] )
		{
			return false;
		}
	}

	return true;
}

void hashFile ( const char *fname, unsigned char* d ) 
{

        MD5_CTX ctx;
        MD5_Init(&ctx);
	int cnt = 0;

        ifstream ifs(fname, std::ios::binary);

        char file_buffer[4096];
        while ( ( ifs.read(file_buffer, sizeof(file_buffer)) || ifs.gcount() ) && ( cnt < MD5SAMPLESIZE ) ) 
	{
            MD5_Update(&ctx, file_buffer, ifs.gcount());
		cnt++;
        }
        unsigned char digest[MD5_DIGEST_LENGTH] = {};
        MD5_Final(d, &ctx);
	//print_md5_sum( digest );
        return; 
}

// create md5 hash of all files in directory
void hashDir ( const char *dname, unsigned char* d )
{
	MD5_CTX ctx;
	MD5_Init(&ctx);
	int cnt = 0;
	

	char *fname;
	directory_iterator end_itr;
	for (directory_iterator itr( dname ); itr != end_itr; ++itr)
	{
		ifstream ifs(itr->path().string().c_str(), std::ios::binary);

		char file_buffer[4096];
		while ( ( ifs.read(file_buffer, sizeof(file_buffer)) || ifs.gcount() ) && ( cnt < MD5SAMPLESIZE ) )
		{
			MD5_Update(&ctx, file_buffer, ifs.gcount());
		}
		cnt++;
		cerr << cnt << "\r";
	}

	MD5_Final(d, &ctx);
	return;
}

string hashString ( string s ) 
{

        unsigned char digest[MD5_DIGEST_LENGTH] = {};
	char digest_str[2*MD5_DIGEST_LENGTH+1];

	MD5( (const unsigned char*)s.c_str(), s.length(), digest );
        return s; 
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
		//cerr << s << endl;
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
                        file_size = file_size + boost::filesystem::file_size(filePath);                      
                    }else{
                        //getDirectorySize(filePath.string(),file_size);
                    }
                }catch(exception& e){               
                    cerr << "HEY! " << e.what() << endl;
                }
            }
        }

    }


int getSubdirs( const char *path, vector<Directory> &v, int count, int depth )
{
	// Ensure this isn't a symlink if followSymlinks is false
	if ( !followSymlinks && ( is_symlink( path ) ) )
	{
		return 0;
	}
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
				
				//std::cerr<< "ZZZ" << (*it).path().string() << endl;
				dirCount ++;

				// Create Directory object
				Directory d; // = new Directory( (*it).path().string() );
				d.path = (*it).path().string();

				// Get directory listing CRC
				// FIXME For now calling two functions, one to get a string, one to get CRC
				// FIXME Need to come up with a nice efficient method, but this works for now
				files = listFiles( (*it).path() );
				d.childCRC = crcString( files );

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
				if ( followSymlinks )		// BFI!
				{
					count += getSubdirs ( (*it).path().string().c_str(), v, cnt, depth + 1 );
				}
				else 
				{
					if ( !is_symlink( *it ) )
					{
						count += getSubdirs ( (*it).path().string().c_str(), v, cnt, depth + 1 );
					}
				}
				//cerr << "==== " << (*it).path().string() << endl;
				d.subdirs = cnt;

				d.depth = depth;
				depth = oldDepth;

				v.push_back( d );

				cnt++;

				cerr << "\r" << cnt ;
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
    bool includeEmpty = false;
    bool flagB = false;
    bool unique = false;

	unsigned char digest[MD5_DIGEST_LENGTH] = {};

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
    cerr << "Base Dir = " << baseDir << endl;

    // Shut GetOpt error messages down (return '?'): 
    opterr = 0;

    // Retrieve the options:
    while ( (opt = getopt(argc, argv, "eb")) != -1 ) {  // for each option...
        switch ( opt ) {
            case 'e':
                    includeEmpty = true;
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
	cerr << "Directories found:" << endl;

	x = getSubdirs( baseDir.c_str(), directories ,0 ,0);
cerr << "DIRS: " << directories.size();

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
		//cerr << (*it).depth << ": " << (*it).childCRC << "|" << (*it).subdirs << " " << (*it).getPath() << endl;
		/* bogus search with boost::bind
		std::vector<Directory>::iterator dit = std::find_if(
				directories.begin(),
				directories.end(),
				boost::bind(&Directory::childCRC, _1) == (*it).childCRC );

		vector<Directory>::iterator foundIt;
		for ( foundIt = dit.begin(); foundIt < dit.end; foundIt++ )
		{
			cerr << "Found one!" << endl;
		}
		*/
		for(dit=directories.begin() ; dit < directories.end(); dit++ )
		{
			// Check if crc matches any others - remove item if not
			if ( ( (*it).childCRC == (*dit).childCRC ) && ( (*it).path != (*dit).path ) )
			{
				//cerr << (*it).path << " has dupe(s)!! " << (*dit).childCRC << " and " << (*dit).path << endl;

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

	cerr << dirCount + 1 << " directories found" << endl;
	cerr << dupes.size() << " potential dupes found in round 1" << endl << endl;


	// Free the memory - need to save anything first? FIXME
	directories.clear();
	set<string>::iterator sit;

	// Scan dir directory content sizes being equal
	infoLog ( "Scanning directory sizes...." );

	cnt = 0;
	long dirSize = 0;
	long totSize = 0;
	for ( sit = dupes.begin(); sit != dupes.end(); sit++ )
	{
		cerr << "\r" << cnt;
		cnt++;
		// Now get the total filesize of each file in dir (non-recursive)
		// TODO: Better to do it recursively right now maybe?
		//cerr << *sit << endl;
		dirSize = 0;
		getDirectorySize( *sit, dirSize ) ;

		// Check for empty dir
		if ( includeEmpty || ( dirSize > 0 ) )
		{
			Directory d; 
			d.path = *sit;

			d.du = dirSize;

			round1.push_back( d );
			//cerr << *sit << ": " << dirSize << endl;
			totSize += dirSize;
		}
	}
	//cerr << totSize << endl;
	
	infoLog ( "" );

	// Display what's found thus far
	for(it=round1.begin() ; it < round1.end(); it++ )
	{
		//infoLog ( (*it).path + " has duplicate content size of:" );

		for(dit=round1.begin() ; dit < round1.end(); dit++ )
		{
			if ( ( (*it).du == (*dit).du ) && ( (*it).path != (*dit).path ) )
			{
				//infoLog ( (*it).path + " has duplicate content size of:\t" + (*dit).path );
				// save for next round
				directories.push_back( *it);
			}

		}
	}

	round1.clear();

	// Compare md5 sums
	directory_iterator end_itr;

	cerr << endl << "Checking md5 sums in " << directories.size() << " directories:" << endl;

	for(it=directories.begin() ; it < directories.end(); it++ )
	{
		hashDir( (*it).path.c_str(), digest );
		//cerr << "ZZZ: " << (*it).path << endl;; 
		//print_md5_sum( digest );
		//cerr << endl;
		memcpy( (*it).digest, digest, sizeof(digest) );
	}

	// Check for unique md5s
	round1.clear();
	
	for(it=directories.begin() ; it < directories.end(); it++ )
	{
		unique = true;
		cout << "XXX: " << (*it).path; // << endl;
		print_md5_sum( (*it).digest );
		cout << endl;

		for ( dit = directories.begin(); dit < directories.end(); dit++ )
		{	
			if ( (*it).digest == (*dit).digest )
			{
				unique = false;
				//cerr << "Removing " << (*dit).path << endl;
			}
		}

		if ( !unique )
		{
			round1.push_back( (*it) );
			//cerr << "Keeping " << (*dit).path << endl;
		}
	}

	cout << round1.size() << " directories with dupes" << endl;

	for(it=round1.begin() ; it < round1.end(); it++ )
	{
		cerr << endl << (*it).path << ":" << endl;

		for(dit=round1.begin() ; dit < round1.end(); dit++ )
		{
			if( ( compareMD5( (*it).digest, (*dit).digest ) && ( (*it).childCRC == (*dit).childCRC ) && ( (*it).path != (*dit).path ) ) )
			{
				cerr << "\t" << (*dit).path;
				print_md5_sum( (*dit).digest );
				cerr << endl;
			}
		}
	}

	
	// md5 test
	/*
	const char *f = "testdir/subdir3/dirdupes";
	hashFile( f, digest );
	print_md5_sum( digest );
	*/
	return 0;


}

