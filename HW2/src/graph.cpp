#include "graph.h"  
#include <iostream>
#include <fstream>
using namespace std;

void hypergraph::clear(){
    cell_name.clear();
    cell_size.clear();
    net_name.clear();
    net_pin.clear();
    cell_to_net.clear();
    total_cell_size = 0;
    cell_name_to_id.clear();
    net_name_to_id.clear();
}

// find the cell belong to which net
void hypergraph::build_cell_to_net(){
    cell_to_net.assign(cell_name.size(),{}); //initilaize
    // visit every net
    for(int i=0;i<net_pin.size();i++){
        vector<int> pins = net_pin[i];
        for(int j=0;j<pins.size();j++){
            cell_to_net[ pins[j] ].push_back(i); // push to net's id
        }
    }

}

int hypergraph::calculate_cutsize(vector<int>& part, int k)const{//k:partition number
    int cutsize = 0;
    int cross;//cross in each net
    int seen[k]; //k groups
    int g;
    for(int i=0;i<net_pin.size();i++){
        cross = 0;
        for(int x=0;x<k;x++){ //reset the seen array
            seen[x] = 0;
        }
        for(int j=0;j<net_pin[i].size();j++){
            int cid = net_pin[i][j];
            g = part[cid];
            if(seen[g] == 0){
                seen[g] = 1;
                cross++;
            }
            if(cross >= 2){
                cutsize++;
                break;
            }
        }

    }
    return cutsize;
} 

bool hypergraph::is_balanced(vector<int>& part, int k)const{
    double range_min = 0.0, range_max = 1.0;
    std::vector<long long> group_size(k, 0);
    for(int i=0;i<part.size();i++){
        group_size[ part[i] ] += cell_size[i];
    }
    if(k==2){
        range_min = 0.45;
        range_max = 0.55;
    }
    else if(k==4){
        range_min = 0.225;
        range_max = 0.275;
    }

    for(int i=0;i<k;i++){ // make sure set to double
        if( ( (double)group_size[i]/total_cell_size < range_min ) || ( (double)group_size[i]/total_cell_size > range_max ) )
            return false;
    }
    
    return true;
}

void hypergraph::init_net_part_count2(const vector<int>& part,vector<int>& net_A_count,vector<int>& net_B_count) const{
    net_A_count.assign(net_pin.size(),0);
    net_B_count.assign(net_pin.size(),0);

    for(int i=0;i<net_pin.size();i++){
        for(int j=0;j<net_pin[i].size();j++){
            int cell = net_pin[i][j];
            if(part[cell] == 0){
                net_A_count[i]++;
            }
            else if(part[cell] == 1){
                net_B_count[i]++;
            }
        }
    }
}

void hypergraph::init_net_part_count4(const vector<int>& part,vector<int>& net_A_count,vector<int>& net_B_count,vector<int>& net_C_count,vector<int>& net_D_count) const{
    net_A_count.assign(net_pin.size(),0);
    net_B_count.assign(net_pin.size(),0);
    net_C_count.assign(net_pin.size(),0);
    net_D_count.assign(net_pin.size(),0);

    for(int i=0;i<net_pin.size();i++){
        for(int j=0;j<net_pin[i].size();j++){
            int cell = net_pin[i][j];
            if(part[cell] == 0){
                net_A_count[i]++;
            }
            else if(part[cell] == 1){
                net_B_count[i]++;
            }
            else if(part[cell] == 2){
                net_C_count[i]++;
            }
            else if(part[cell] == 3){
                net_D_count[i]++;
            }
        }
    }
}

bool hypergraph::read_file(const string& filename){
    clear();

    ifstream fin(filename);
    if(!fin){
        cerr << "Error: fail to open the file " << filename << endl;
        return false;
    }

    string token;

    // NumCells
    int num_cell = 0;
    if(!(fin >> token >> num_cell) || token != "NumCells"){
        cerr << "Error: expect 'NumCells <N>'" << endl;
        return false;
    }

    for(int i=0; i<num_cell; i++){
        string dummy, name;
        int size = 0;
        if(!(fin >> dummy >> name >> size) || dummy != "Cell"){
            cerr << "Error: expect 'Cell <name> <size>' at idx " << i << endl;
            return false;
        }
        cell_name.push_back(name);
        cell_size.push_back(size);
        cell_name_to_id[name] = i;
        total_cell_size += size;
    }

    // NumNets
    int num_net = 0;
    if(!(fin >> token >> num_net) || token != "NumNets"){
        cerr << "Error: expect 'NumNets <M>'" << endl;
        return false;
    }

    for(int i=0; i<num_net; i++){
        string dummy, name;
        int count = 0; // number of pins
        if(!(fin >> dummy >> name >> count) || dummy != "Net"){
            cerr << "Error: expect 'Net <name> <#pins>' at net " << i << endl;
            return false;
        }

        net_name.push_back(name);
        net_name_to_id[name] = i;

        vector<int> pins;
        pins.reserve(count);
        for(int j=0; j<count; j++){
            string dummy2, cname;
            if(!(fin >> dummy2 >> cname) || dummy2 != "Cell"){
                cerr << "Error: expect 'Cell <name>' in net " << name << endl;
                return false;
            }
            auto it = cell_name_to_id.find(cname);
            if(it == cell_name_to_id.end()){
                cerr << "Error: cell '" << cname << "' not defined (net " << name << ")" << endl;
                return false;
            }
            pins.push_back(it->second);
        }
        net_pin.push_back(pins);
    }

    fin.close();
    build_cell_to_net();
    return true;
}
