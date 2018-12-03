#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <map>
#include <algorithm>
#include <omp.h>
//#include <glib.h>


using namespace std;

const int NOISE = -2;
const int NOT_CLASSIFIED = -1;
static int NUM_THREADS = 4;

class Point {
    public:
        double x, y;
        int ptsCnt, cluster;
        double getDis(const Point & ot) {
            return sqrt((x-ot.x)*(x-ot.x)+(y-ot.y)*(y-ot.y));
        }
};

class DBCAN {
    public:
        int n, minPts;
        double eps;
        vector<Point> points;
        int size;
        vector<vector<int> > adjPoints;
        vector<bool> visited;
        vector<vector<int> > cluster;
        int clusterIdx;
        
        DBCAN(int n, double eps, int minPts, vector<Point> points) {
            this->n = n;
            this->eps = eps;
            this->minPts = minPts;
            this->points = points;
            this->size = (int)points.size();
            adjPoints.resize(size);
            this->clusterIdx=-1;
        }
        void runSequential () {
            checkNearPoints();
            
            for(int i=0;i<size;i++) {
                if(points[i].cluster != NOT_CLASSIFIED) continue;
                
                if(isCoreObject(i)) {
                    dfs(i, ++clusterIdx);
                } else {
                    points[i].cluster = NOISE;
                }
            }
            
            cluster.resize(clusterIdx+1);
            for(int i=0;i<size;i++) {
                if(points[i].cluster != NOISE) {
                    cluster[points[i].cluster].push_back(i);
                }
            }
        }

        void runParallel () {
            checkNearPointsParallel();
            
            for(int i=0;i<size;i++) {
                if(points[i].cluster != NOT_CLASSIFIED) continue;
                
                if(isCoreObject(i)) {
                    dfs(i, ++clusterIdx);
                } else {
                    points[i].cluster = NOISE;
                }
            }
          
            cluster.resize(clusterIdx+1);
            for(int i=0;i<size;i++) {
                if(points[i].cluster != NOISE) {
                    cluster[points[i].cluster].push_back(i);
                }
            }
            
        }
        
        void dfs (int now, int c) {
            points[now].cluster = c;
            if(!isCoreObject(now)) return;
            
            for(auto&next:adjPoints[now]) {
                if(points[next].cluster != NOT_CLASSIFIED) continue;
                dfs(next, c);
            }
        }
        
        void checkNearPointsParallel() {
            #pragma omp parallel for
            {
                for(int i=0;i<size;i++) {
                    for(int j=0;j<size;j++) {
                        if(i==j) continue;
                        if(points[i].getDis(points[j]) <= eps) {
                            points[i].ptsCnt++;
                            adjPoints[i].push_back(j);
                        }
                    }
                }
            }
        }
        
        void checkNearPoints() {
            for(int i=0;i<size;i++) {
                for(int j=0;j<size;j++) {
                    if(i==j) continue;
                    if(points[i].getDis(points[j]) <= eps) {
                        points[i].ptsCnt++;
                        adjPoints[i].push_back(j);
                    }
                }
            }
        }
        // is idx'th point core object?
        bool isCoreObject(int idx) {
            return points[idx].ptsCnt >= minPts;
        }
        
        vector<vector<int> > getCluster() {
            return cluster;
        }
};

class InputReader {
    private:
        ifstream fin;
        vector<Point> points;
    public:
        InputReader(string filename) {
            fin.open(filename);
            if(!fin) {
                cout << filename << " file could not be opened\n";
                exit(0);
            }
            parse();
        }
        void parse() {
            int idx;
            double x, y;
            while(!fin.eof()) {
                fin >> idx >> x >> y;
                points.push_back({x,y,0, NOT_CLASSIFIED});
            }
            points.pop_back();
        }
        vector<Point> getPoints() {
            return points;
        }
};

class OutputPrinter {
    private:
        ofstream fout;
        vector<vector<int> > cluster;
        string filename;
        int n;
    public:
        OutputPrinter(int n, string filename, vector<vector<int> > cluster) {
            this->n = n;
            this->cluster = cluster;
            
            // remove ".txt" from filename
            if(filename.size()<4){
                cout << filename << "input file name's format is wrong\n";
                exit(0);
            }
            for(int i=0;i<4;i++) filename.pop_back();
            this->filename = filename;
            
            // sort by size decending order
            sort(cluster.begin(), cluster.end(), [&](const vector<int> i, const vector<int> j) {
                return (int)i.size() > (int)j.size();
            });
        }
        void print() {
            for(int i=0;i<n;i++) {
                fout.open(filename+"_cluster_"+to_string(i)+".txt");
                
                for(int j=0;j<cluster[i].size();j++) {
                    fout << cluster[i][j] << endl;
                }
                
                fout.close();
            }
        }
};

int main(int argc, const char * argv[]) {
    if(argc!=5) {
        cout << "Please follow this format. clustering.exe [intput] [n] [eps] [minPts]";
        return 0;
    }

    string inputFileName(argv[1]);
    string n(argv[2]);
    string eps(argv[3]);
    string minPts(argv[4]);
   
    ofstream file;
    file.open("comparacao.csv", ios_base::app);

    double start_time, elapsed_time_parallel=0, elapsed_time_sequential=0;
    
    InputReader inputReader(inputFileName);
    
    DBCAN dbScan(stoi(n), stod(eps), stoi(minPts), inputReader.getPoints());

    for( int n_threads = 1; n_threads <= 100; n_threads++ ) { 
        omp_set_num_threads(n_threads);
 /*       
        // -------------------------------SEQUENCIAL------------------------------- //
        start_time = omp_get_wtime();
        dbScan.runSequential();
        elapsed_time_sequential = omp_get_wtime() - start_time;

        printf("Sequencial:\t %lf seconds \n", elapsed_time_sequential);
        // ----------------------------------------------------------------------- //
*/
                            
        // -------------------------------PARALELO-------------------------------- //
        start_time = omp_get_wtime();
        dbScan.runParallel();
        elapsed_time_parallel = omp_get_wtime() - start_time;

        printf("Paralelo:\t %lf seconds \n\n", elapsed_time_parallel);
        // ----------------------------------------------------------------------- //
        
        file << n_threads << ";" << "50000" << ";" << elapsed_time_sequential << ";" << elapsed_time_parallel << endl;
        
    }

    //OutputPrinter outputPrinter(stoi(n), inputFileName, dbScan.getCluster());
    //outputPrinter.print();
    
    return 0;
}