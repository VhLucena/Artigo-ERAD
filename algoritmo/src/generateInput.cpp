#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

int main() {
    ofstream file_txt;
    file_txt.open("input100000.txt");

    srand(time(nullptr));


    for( int i = 0; i < 100000; i++ ) { 
        
        if ( i%3 == 0 ) 
            file_txt << i << "\t" << 2000 + rand()%1000 << "\t" << 2000 + rand()%1000 << endl;
        else if( i%3 == 1 )
            file_txt << i << "\t" << 1000 + rand()%1000 << "\t" << rand()%1000 << endl;
        else
            file_txt << i << "\t" << rand()%1000 << "\t" << 1000 + rand()%1000 << endl;

    }
    
    return 0;
}