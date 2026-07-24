#pragma once
#include"graph.h"
#include<vector>
#include<limits>
#include<string>
#include<utility>
#include<chrono>

struct FMmove{
    int cell;
    int origin_part;
    int new_part;
    int gain;
};

class bucket_list{
public:
    void init(int gain_min,int gain_max, int n);

    void insert(int cell, int gain);

    void remove(int cell);
    
    std::pair<int,int> pop_max();
    std::pair<int,int> peek_max() const;
    //see if one cell is in the bucket
    bool contain(int cell) const;
    
    void clear();

private:
    std::vector<std::vector<int>> buckets;
    std::vector<int> pos_in_bucket;     
    std::vector<int> bucket_of; // cell belong to which index
    std::vector<int> cell_gain;
    std::vector<char> in_bucket;
    int gain_min = 0;
    int gain_max = -1;
    int width = 0;
    int current_max_index = -1;

    void update_current_max(int index);
};  


class FM{
public:
    FM(const hypergraph& hg);

    int run(int k, std::vector<int>& best_part, unsigned seed = 0);

    void output(const std::string& out_path,
                const std::vector<int>& part,
                int k, int cutsize) const;

private:
    const hypergraph& hg;
    std::vector<int> cell_best_to;    // best target partition for this cell
    std::vector<int> cell_best_gain;  // best gain value
    std::vector<int> part;
    std::vector<char> locked;
    std::vector<int> cell_gain;
    //vector to store importance
    std::vector<double> imp;
    
    int k = 2;

    std::vector<long long> group_size_sum;

    std::vector<bucket_list> buckets;
    std::vector<int> net_A_count, net_B_count, net_C_count, net_D_count;

    //variable about time
    int pass_id;     // which pass
    bool hard_stop;  // exceeed the total execution time
    double time_budget_sec; //total time budget
    std::chrono::high_resolution_clock::time_point t_program_start;
    
    //variable about stamp(avoid unnecesssary initialization during updating gain)
    std::vector<int> seen_stamp;
    std::vector<int> tmp_neighbors;
    int stamp = 1;
    
    //initialization
    void init_partition(unsigned seed);

    //main function
    int run_two_way (std::vector<int>& best_part, unsigned seed);
    int run_four_way(std::vector<int>& best_part, unsigned seed);
    
    int pass(std::vector<int>& best_part);

    void init_net_counts();

    void init_bucket();
    
    int compute_gain(int cell, int target_group);
    
    void compute_all_gains_and_insert();

    void apply_move_update(int cell, int origin_part, int new_part);

    void update_neighbor_gains(int moved_cell, int origin_part, int new_part);

    FMmove select_best_move();

    bool legal(int cell, int new_part);

    void set_lock(int cell);

    void reset_lock();

    void restore_by_max_prefix(const std::vector<FMmove>& seq,
                               int best_prefix_len,
                               std::vector<int>& base_part,
                               std::vector<int>& out_part) const;

    void recompute_group_sizes();

    void build_importance();
    void order_by_importance(std::vector<int>& idx, unsigned seed) const;
};   
