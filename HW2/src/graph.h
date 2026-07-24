#pragma once
#include<string>
#include<vector>
#include<unordered_map>

class hypergraph{
    public:
    //cell
    std::vector<std::string> cell_name;
    std::vector<int> cell_size;
    //net
    std::vector<std::string> net_name;
    std::vector<std::vector<int>> net_pin; //net connect to which cell
    //adjacency
    std::vector<std::vector<int>> cell_to_net; //cell connect to which net

    long long total_cell_size = 0;

    std::unordered_map<std::string,int> cell_name_to_id;
    std::unordered_map<std::string,int> net_name_to_id;

    void clear();

    void build_cell_to_net();

    //part : partition number
    int calculate_cutsize(std::vector<int>& part, int k) const;
    //return size of one group
    //vector<int> group_size(std::vector<int>& part, int k);

    bool is_balanced(std::vector<int>& part, int k) const;

    //for 2 way, initialize the count
    void init_net_part_count2(const std::vector<int>& part,std::vector<int>& net_A_count,std::vector<int>& net_B_count) const;

    //for 4 way, initialize the count
    void init_net_part_count4(const std::vector<int>& part,std::vector<int>& net_A_count,std::vector<int>& net_B_count,std::vector<int>& net_C_count,std::vector<int>& net_D_count) const;

    bool read_file(const std::string& filename);
};