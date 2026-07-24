#include "graph.h"
#include "fm.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <cstdlib>
#include <chrono>
using namespace std;

/*
// Print hypergraph summary (for manual debugging)
void print_summary(const hypergraph& hg) {
    cout << "=== Summary ==="<<endl;
    cout << "NumCells: " << hg.cell_name.size() <<endl;
    cout << "NumNets : " << hg.net_name.size() <<endl;
    cout << "TotalCellSize: " << hg.total_cell_size <<endl<<endl;

    cout <<"[Cells]"<<endl;
    for (int i = 0; i < hg.cell_name.size(); i++) {
        cout << "  id=" << i
             << " name=" << hg.cell_name[i]
             << " size=" << hg.cell_size[i] << endl;
    }
    cout << "\n[Nets]\n"<<endl;
    for (int i = 0; i < hg.net_name.size(); i++) {
        cout << "  net_id=" << i << " name=" << hg.net_name[i] << " pins:";
        for (int cid : hg.net_pin[i]) cout << " " <<cid;
        cout<<endl;
    }
    cout << endl;
}

// Print cell-to-net adjacency list
void print_cell_to_net(const hypergraph& hg){
    cout<<"[Cell_to_net]"<<endl;
    for(int i=0;i<hg.cell_to_net.size();i++){
        cout<<"cell "<<i<<"("<<hg.cell_name[i]<<") ->";
        for(int j : hg.cell_to_net[i]) cout<<" "<<j;
        cout<<endl;
    }
}
*/

// Print usage message when arguments are incorrect
static void usage(const char* prog){
    cerr << "Usage: " << prog << " <input> <output> <k>\n"
            "  <k> must be 2 or 4\n";
}


static bool write_spec_output(const string& out_path,
                              const hypergraph& hg,
                              const vector<int>& part,
                              int k, int cutsize){
    ofstream fout(out_path);
    if(!fout){
        cerr << "Cannot open out file: " << out_path << "\n";
        return false;
    }

    fout << "CutSize " << cutsize << "\n";

    static const char* G2[2] = {"GroupA", "GroupB"};
    static const char* G4[4] = {"GroupA", "GroupB", "GroupC", "GroupD"};
    const char** G = (k==2)? G2 : G4;

    vector<vector<int>> groups(k);
    for(int i=0;i<(int)part.size();i++){
        int g = part[i];
        if(0 <= g && g < k) groups[g].push_back(i);
    }

    // Write each group and its member cell names
    for(int g=0; g<k; g++){
        fout << G[g] << " " << groups[g].size() << "\n";
        for(int cid : groups[g]){
            fout << hg.cell_name[cid] << "\n";
        }
    }
    return true;
}

// ============================================================
//  Main program (spec-compliant interface)
// ============================================================

int main(int argc, char** argv){
    if (argc != 4) { usage(argv[0]); return 1; }

    string in_path  = argv[1];
    string out_path = argv[2];
    int seed = 0;
    int k = atoi(argv[3]);
    if (k != 2 && k != 4) { usage(argv[0]); return 1; }

    // Read hypergraph input file
    hypergraph hg;
    if (!hg.read_file(in_path)) {
        cerr << "Failed to read: " << in_path << "\n";
        return 1;
    }
    // Run FM partitioning and measure time
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    
    // Run FM partitioning
    FM fm(hg);
    vector<int> best_part;
    int best_cut = fm.run(k, best_part, seed);
    
    
    auto end = high_resolution_clock::now();
    double elapsed = duration_cast<duration<double>>(end - start).count();
    cout << fixed;
    cout.precision(6);
    cout << "[INFO] FM algorithm execution time: " << elapsed << " seconds\n";
    //if still have enough time, run with another seed until 90 second
    const double TOTAL_BUDGET_SEC = 90.0;
	unsigned base_seed = 1;//first seed
	unsigned seed_step = 9973;
	double single_run_sec = elapsed;

	best_cut = hg.calculate_cutsize(best_part, k); 
	cerr << "[INFO] First run: seed=" << base_seed
	     << ", cut=" << best_cut
	     << ", time=" << single_run_sec << "s\n";

	//remain time > run time => run with another seed
	int extra_id = 2;
	auto program_start = start;
	while (true) {
		double used = duration_cast<duration<double>>(high_resolution_clock::now() - program_start).count();
		double remain = TOTAL_BUDGET_SEC - used;
		if (remain < single_run_sec) break;//Don't have enough time

		seed = extra_id;
        //new FM process
		FM fm_more(hg);
		vector<int> part_more;
		auto ts = high_resolution_clock::now();
		int cut_more = fm_more.run(k, part_more, seed);
		auto te = high_resolution_clock::now();
		double took = duration_cast<duration<double>>(te - ts).count();

		cut_more = hg.calculate_cutsize(part_more, k);

		cerr << "[INFO] Extra run #" << extra_id
		     << ": seed=" << seed
		     << ", cut=" << cut_more
		     << ", time=" << took << "s\n";

		if (cut_more < best_cut) {
			best_cut = cut_more;
			best_part.swap(part_more);
			cerr << "[INFO] New best cut = " << best_cut << "\n";
		}
		++extra_id;
	}

    // Recalculate cutsize for consistency
    best_cut = hg.calculate_cutsize(best_part, k);

    // Write result file in required format
    if (!write_spec_output(out_path, hg, best_part, k, best_cut)) return 1;

    return 0;
}
