#include"fm.h"
#include"graph.h"
#include<iostream>
#include<random>
#include<algorithm>
#include<climits>
#include<chrono>
using namespace std;

void bucket_list::init(int gain_min,int gain_max, int n){
    this->gain_min = gain_min;
    this->gain_max = gain_max;
    width = gain_max - gain_min + 1;
    buckets.clear();
    buckets.resize(width);          
    current_max_index = -1;       
    cell_gain.assign(n, 0);
    in_bucket.assign(n, 0);  
    
    pos_in_bucket.assign(n, -1);  //initialize position container
    bucket_of.assign(n, -1);

    
}

void bucket_list::insert(int cell, int gain){
    int idx = gain-gain_min;
    buckets[idx].push_back(cell);
    
    // record the position in the bucket
    pos_in_bucket[cell] = (int)buckets[idx].size() - 1;
    bucket_of[cell] = idx;
    
    cell_gain[cell] = gain;
    in_bucket[cell] = true;
    if (current_max_index < idx || current_max_index == -1)
        current_max_index = idx;
}

void bucket_list::remove(int cell){
    if (!in_bucket[cell]) return;

    int idx = bucket_of[cell];
    if (idx < 0 || idx >= width) return;

    int p = pos_in_bucket[cell];

    int last_index = (int)buckets[idx].size() - 1;
    int last_cell  = buckets[idx][last_index];

    if (p != last_index) {
        buckets[idx][p] = last_cell;
        pos_in_bucket[last_cell] = p; //update removed cell's position
    }

    //remove last item
    buckets[idx].pop_back();

    //reset condition
    in_bucket[cell]    = 0;
    pos_in_bucket[cell]= -1;
    bucket_of[cell]    = -1;

    if (idx == current_max_index && buckets[idx].empty()) {
        update_current_max(idx);
    }
}

void bucket_list::update_current_max(int index) {
    if (index >= width) index = width - 1;
    if (index < 0) { 
        current_max_index = -1;
        return;
    }

    for (int i = index; i >= 0; --i) {
        if (!buckets[i].empty()) {
            current_max_index = i;
            return;
        }
    }
    current_max_index = -1; // all empty
}

pair<int,int> bucket_list::pop_max(){
    int idx = current_max_index;
    if (current_max_index == -1 || buckets[idx].empty()) {
        return make_pair(-1, 0);
    }
    int cell = buckets[idx].back();
    int gain = gain_min + idx;
    buckets[idx].pop_back();
    //reset condition
    in_bucket[cell] = 0;
    pos_in_bucket[cell]  = -1;
    bucket_of[cell]      = -1;
    
    if(buckets[idx].size() == 0)
        update_current_max(current_max_index);

    return make_pair(cell, gain);
}

pair<int,int> bucket_list::peek_max() const {
    int idx = current_max_index;
    if (current_max_index == -1 || buckets[idx].empty()) {
        return make_pair(-1, 0);
    }
    
    int cell = buckets[idx].back();
    int gain = gain_min + idx;
    return make_pair(cell, gain);
}

bool bucket_list::contain(int cell) const{
    return cell >= 0 && cell < (int)in_bucket.size() && in_bucket[cell];
}

void bucket_list::clear(){
    for (int i = 0; i < (int)buckets.size(); i++) {
        buckets[i].clear();
    }
    for (int i = 0; i < (int)in_bucket.size(); i++) {
        in_bucket[i] = 0;
        pos_in_bucket[i] = -1;
        bucket_of[i] = -1;
    }
    current_max_index = -1;
}

FM::FM(const hypergraph& hg) : hg(hg){
    int n = hg.cell_name.size();
    int m = hg.net_name.size();

    part.assign(n,0);
    locked.assign(n,0);

    cell_best_to.assign(n,-1);
    cell_best_gain.assign(n,-1);
    cell_gain.assign(n,0);

    //assign the net
    net_A_count.assign(m, 0);
    net_B_count.assign(m, 0);
    net_C_count.assign(m, 0);
    net_D_count.assign(m, 0);

    pass_id = 0;
    hard_stop = false;
    time_budget_sec = 95.0; //95 second for execution
    buckets.clear();

    seen_stamp.assign(hg.cell_name.size(), 0);
    tmp_neighbors.reserve(1024);//allocate a large size to avoid dynamic expansion
    stamp = 1;
}

int FM::run(int k, std::vector<int>& best_part, unsigned seed){
    this->k = k;
    group_size_sum.assign(k,0);
    //reassign buckets
    buckets.clear();
    buckets.resize(k);
    int best_cut = 0;
    if(k==2){
        best_cut = run_two_way(best_part, seed);
    }
    else if(k==4){
        best_cut = run_four_way(best_part, seed);
    }
    else{
        cout<<"Don't support "<<k<<" way partition"<<endl;
        best_cut = hg.calculate_cutsize(best_part, k);
    }

    return best_cut;
}

int FM::run_two_way (vector<int>& best_part, unsigned seed){
    init_partition(seed);
    pass_id = 1;
    hard_stop = false;
    t_program_start = std::chrono::high_resolution_clock::now();


    int global_best_cut = hg.calculate_cutsize(part, k);
    best_part = part;

    while(1){
        if (hard_stop) break;
        vector<int> temp_best_part;
        int temp_best_cut = pass(temp_best_part);
        pass_id++;
        if (temp_best_cut < global_best_cut) {
            global_best_cut = temp_best_cut;
            best_part = temp_best_part;
            part = temp_best_part;
            recompute_group_sizes(); // group size
        } else {
            break; //no improvement, stop
        }
    }
    return global_best_cut;
}
int FM::run_four_way(vector<int>& best_part, unsigned seed){
    init_partition(seed);

    pass_id = 1;
    hard_stop = false;
    t_program_start = std::chrono::high_resolution_clock::now();

    int global_best_cut = hg.calculate_cutsize(part, k);
    best_part = part;

    while(1){
        if (hard_stop) break;
        vector<int> temp_best_part;
        int temp_best_cut = pass(temp_best_part);
        pass_id++;
        if (temp_best_cut < global_best_cut) {
            global_best_cut = temp_best_cut;
            best_part = temp_best_part;
            part = temp_best_part;
            recompute_group_sizes(); //update group sizes
        } else {
            break; //no improvement, stop
        }
    }
    return global_best_cut;
}
   
int FM::pass(vector<int>& best_part) {
    compute_all_gains_and_insert();
    reset_lock();

    vector<int> start_part = part;
    vector<FMmove> seq;
    int best_cut        = hg.calculate_cutsize(part, k);
    int current_cut     = best_cut;
    int best_prefix_len = 0;

    int moves = 0; // how many moves in this pass

    while (1) {
        FMmove m = select_best_move();
        if (m.cell == -1) break;

        seq.push_back(m);
        apply_move_update(m.cell, m.origin_part, m.new_part);

        current_cut -= m.gain;

        if (current_cut < best_cut) {
            best_cut        = current_cut;
            best_prefix_len = (int)seq.size();
        }

        // -------------------- move cap per pass --------------------
        // First pass: allow more moves (N/2). Later passes: fewer (N/40).
        // This keeps each pass short to avoid time explosion.
        moves++;
        int cap;
        if (pass_id <=2) cap = (int)part.size();
        else              cap = (int)part.size() / 4;
        if (cap < 1) cap = 1; // safety for tiny cases
        if (moves >= cap) {
            // Reached per-pass move cap; stop this pass and let rollback decide
            break;
        }
        // -----------------------------------------------------------

        // -------------------- global time budget -------------------
        // Stop all remaining passes if total elapsed time exceeds the budget.
        chrono::high_resolution_clock::time_point now =
        chrono::high_resolution_clock::now();
        chrono::duration<double> diff = now - t_program_start;
        double elapsed = diff.count();
        if (elapsed > time_budget_sec) {
            hard_stop = true; // outer run_* while() will break
            break;
        }
        // -----------------------------------------------------------
    }

    restore_by_max_prefix(seq, best_prefix_len, start_part, best_part);
    return best_cut;
}




void FM::init_partition(unsigned seed){
    const int n = hg.cell_name.size();
    part.assign(n,0);
    group_size_sum.assign(k,0LL); //long long zero

    // build index array 0,1,2,...,n-1
    if ((int)imp.size() != n) build_importance();

    vector<int> indices;
    order_by_importance(indices, seed);

    //distribute all the cells
    int least_size_group = 0;
    for(int i=0;i<n;i++){
        int idx = indices[i];
        part[idx] = least_size_group;
        group_size_sum[least_size_group] += hg.cell_size[idx];
        //update least size group index
        for(int j=0;j<k;j++){
            if(group_size_sum[j]<group_size_sum[least_size_group])
                least_size_group = j;
        }
    }
}
void FM::init_net_counts() {
    if (k == 2) {
        net_A_count.assign(hg.net_pin.size(), 0);
        net_B_count.assign(hg.net_pin.size(), 0);
        hg.init_net_part_count2(part, net_A_count, net_B_count);
    } else if (k == 4) {
        net_A_count.assign(hg.net_pin.size(), 0);
        net_B_count.assign(hg.net_pin.size(), 0);
        net_C_count.assign(hg.net_pin.size(), 0);
        net_D_count.assign(hg.net_pin.size(), 0);
        hg.init_net_part_count4(part, net_A_count, net_B_count, net_C_count, net_D_count);
    }
}

int FM::compute_gain(int cell, int target_group){
    int origin = part[cell];
    if(origin == target_group) return 0;

    int gain = 0;
    
    if(k==2){
        // for every cell, test each hyperedge that adjacent to it, move it to the other side
        // then calculate the new cutsize(gain)
        for(int edge_index=0;edge_index < (int)hg.cell_to_net[cell].size();edge_index++){
            int edge = hg.cell_to_net[cell][edge_index];

            int A = net_A_count[edge];
            int B = net_B_count[edge];

            bool before_cut = (A > 0 && B > 0);
            //test move the cell
            if (origin == 0) { // A -> B
                A -= 1;
                B += 1;
            } else {          // B -> A
                B -= 1;
                A += 1;
            }

            bool after_cut = (A > 0 && B > 0);

            if (before_cut && !after_cut)      gain += 1;
            else if (!before_cut && after_cut) gain -= 1;
        }
    }
    else if(k==4){
        // for every cell, test each hyperedge that adjacent to it, move it to the other side
        // then calculate the new cutsize(gain)
        for(int edge_index=0;edge_index < (int)hg.cell_to_net[cell].size();edge_index++){
            int edge = hg.cell_to_net[cell][edge_index];

            int A = net_A_count[edge];
            int B = net_B_count[edge];
            int C = net_C_count[edge];
            int D = net_D_count[edge];

            //before cut state
            
            int before_nonzero_group = (A>0) + (B>0) + (C>0) + (D>0);
            bool before_cut = (before_nonzero_group>1);
            //test move cell to target group
            if(origin == 0) A--;
            else if(origin == 1) B--;
            else if(origin == 2) C--;
            else if(origin == 3) D--;

            if(target_group == 0) A++;
            else if(target_group == 1) B++;
            else if(target_group == 2) C++;
            else if(target_group == 3) D++;

            //after cut state
            bool after_cut = 0;
            int after_nonzero_group = (A>0) + (B>0) + (C>0) + (D>0);
            if(after_nonzero_group>1) after_cut = true;  
            
            if (before_cut && !after_cut)      gain += 1;
            else if (!before_cut && after_cut) gain -= 1;
        }
    }
    return gain;
}
void FM::init_bucket(){
    buckets.clear();
    buckets.resize(k); //how many partition
    const int n = hg.cell_name.size();
    int max_deg = 0;
    for(int i=0;i<(int)hg.cell_to_net.size();i++){
        if((int)hg.cell_to_net[i].size() > max_deg) max_deg = (int)hg.cell_to_net[i].size();
    }
    for (int i = 0; i < k; i++) {
        buckets[i].init(-max_deg, +max_deg, n);
    }

}

void FM::compute_all_gains_and_insert(){
    const int n = hg.cell_name.size();

    init_net_counts();
    init_bucket();

    locked.assign(n,0);
    cell_best_to.assign(n, -1);
    cell_best_gain.assign(n,INT_MIN);
    cell_gain.assign(n, 0);

    for(int cell_id=0;cell_id<n;cell_id++){
        int origin = part[cell_id];
        int to;
        //cell doesn't connect to other net
        if(hg.cell_to_net[cell_id].empty()){
            cell_best_to[cell_id] = -1;
            cell_best_gain[cell_id] = 0;
            cell_gain[cell_id] = 0;
            buckets[origin].insert(cell_id, 0);
            continue;
        }
        
        int best_to = -1;
        int best_gain = INT_MIN;

        if(k==2){
            to = 1-origin;
            int g = compute_gain(cell_id, to);
            best_gain = g;
            best_to   = to;
        }
        else if(k==4){
            for(to = 0;to<k;to++){
                if(to == origin) continue;
                if (!legal(cell_id, to)) continue;// not legal, don't compute it
                int g = compute_gain(cell_id, to);
                if(g > best_gain){
                    best_gain = g;
                    best_to = to;
                }
                
            }
            if (best_to == -1) {
                best_gain = 0; // no legal move
            }
        }
        
        cell_best_to[cell_id] = best_to;
        cell_best_gain[cell_id] = best_gain;
        cell_gain[cell_id] = best_gain;

        buckets[origin].insert(cell_id, best_gain);
    }
}

void FM::apply_move_update(int cell, int origin_part, int new_part){
    if (buckets[origin_part].contain(cell)) {
        buckets[origin_part].remove(cell);
    }
    
    locked[cell] = 1;
    long long w = hg.cell_size[cell];
    group_size_sum[origin_part] -= w; 
    group_size_sum[new_part] += w;

    for(int edge : hg.cell_to_net[cell]){
        if(k==2){
            if(origin_part == 0) net_A_count[edge]--;
            else if(origin_part == 1) net_B_count[edge]--;

            if(new_part == 0) net_A_count[edge]++;
            else if(new_part == 1) net_B_count[edge]++;
        }
        else if(k==4){
            if(origin_part == 0) net_A_count[edge]--;
            else if(origin_part == 1) net_B_count[edge]--;
            else if(origin_part == 2) net_C_count[edge]--;
            else if(origin_part == 3) net_D_count[edge]--;

            if(new_part == 0) net_A_count[edge]++;
            else if(new_part == 1) net_B_count[edge]++;
            else if(new_part == 2) net_C_count[edge]++;
            else if(new_part == 3) net_D_count[edge]++;
        }
    }
    //move to new part
    part[cell] = new_part;
    //clear the original data
    cell_best_to[cell]   = -1;
    cell_best_gain[cell] = 0;
    cell_gain[cell]      = 0;

    update_neighbor_gains(cell, origin_part, new_part);
}





void FM::update_neighbor_gains(int moved_cell, int origin_part, int new_part){

    // 2-way FM: apply fast incremental update rules on incident nets
    if (k == 2) {
        for (int edge_index = 0; edge_index < (int)hg.cell_to_net[moved_cell].size(); ++edge_index) {
            int net = hg.cell_to_net[moved_cell][edge_index];

            // A_post / B_post are the pin counts on each side AFTER the move of 'moved_cell'
            int A_post = net_A_count[net];
            int B_post = net_B_count[net];
            
            //pass this net
            if (A_post >= 2 && B_post >= 2) continue;
            
            // T_post / T_pre: pin count on target side (new_part) after/before the move
            int new_part_pin_post_move = (new_part == 0) ? A_post : B_post; // T_post
            int new_part_pin_pre_move  = new_part_pin_post_move - 1;        // T_pre = T_post - 1

            // F_post: pin count on source side (origin_part) after the move
            int origin_part_pin_post_move = (origin_part == 0) ? A_post : B_post;

            // Rule 1: T_pre == 0 -> all cells on source side gain +1 for this net
            if (new_part_pin_pre_move == 0) {
                for (int j = 0; j < (int)hg.net_pin[net].size(); ++j) {
                    int u = hg.net_pin[net][j];
                    if (u == moved_cell) continue;
                    if (locked[u])       continue;
                    if (part[u] != origin_part) continue;

                    int from_side = part[u];
                    if (buckets[from_side].contain(u)) {
                        buckets[from_side].remove(u);
                    }

                    cell_gain[u]     += 1;
                    cell_best_to[u]   = 1 - from_side; // in 2-way, only one alternative side
                    cell_best_gain[u] = cell_gain[u];

                    buckets[from_side].insert(u, cell_gain[u]);
                }
            }
            // Rule 2: T_pre == 1 -> the unique cell on target side gets -1
            else if (new_part_pin_pre_move == 1) {
                for (int j = 0; j < (int)hg.net_pin[net].size(); ++j) {
                    int u = hg.net_pin[net][j];
                    if (u == moved_cell) continue;
                    if (locked[u])       continue;
                    if (part[u] != new_part) continue; // the unique one on target side

                    int from_side = part[u];
                    if (buckets[from_side].contain(u)) {
                        buckets[from_side].remove(u);
                    }

                    cell_gain[u]     -= 1;
                    cell_best_to[u]   = 1 - from_side;
                    cell_best_gain[u] = cell_gain[u];

                    buckets[from_side].insert(u, cell_gain[u]);
                    break; // unique one
                }
            }

            // Rule 3: F_post == 0 -> all cells on target side get -1
            if (origin_part_pin_post_move == 0) {
                for (int j = 0; j < (int)hg.net_pin[net].size(); ++j) {
                    int u = hg.net_pin[net][j];
                    if (u == moved_cell) continue;
                    if (locked[u])       continue;
                    if (part[u] != new_part) continue;

                    int from_side = part[u];
                    if (buckets[from_side].contain(u)) {
                        buckets[from_side].remove(u);
                    }

                    cell_gain[u]     -= 1;
                    cell_best_to[u]   = 1 - from_side;
                    cell_best_gain[u] = cell_gain[u];

                    buckets[from_side].insert(u, cell_gain[u]);
                }
            }
            // Rule 4: F_post == 1 -> the unique cell on source side gets +1
            else if (origin_part_pin_post_move == 1) {
                for (int j = 0; j < (int)hg.net_pin[net].size(); ++j) {
                    int u = hg.net_pin[net][j];
                    if (u == moved_cell) continue;
                    if (locked[u])       continue;
                    if (part[u] != origin_part) continue; // the unique one on source side

                    int from_side = part[u];
                    if (buckets[from_side].contain(u)) {
                        buckets[from_side].remove(u);
                    }

                    cell_gain[u]     += 1;
                    cell_best_to[u]   = 1 - from_side;
                    cell_best_gain[u] = cell_gain[u];

                    buckets[from_side].insert(u, cell_gain[u]);
                    break; // unique one
                }
            }
        }
        return;
    }



    // k-way (k >= 3): recompute best move for touched neighbors (stamping + reuse buffer)
    tmp_neighbors.clear();

    // neighbor collect a new stamp
    if (++stamp == INT_MAX) {
        fill(seen_stamp.begin(), seen_stamp.end(), 0);
        stamp = 1;
    }

    // Collect all unlocked neighbors sharing any net with 'moved_cell'
    for (int edge_index = 0; edge_index < (int)hg.cell_to_net[moved_cell].size(); ++edge_index) {
        int net = hg.cell_to_net[moved_cell][edge_index];
        for (int j = 0; j < (int)hg.net_pin[net].size(); ++j) {
            int u = hg.net_pin[net][j];
            if (u == moved_cell) continue;
            if (locked[u])       continue;
            if (seen_stamp[u] == stamp) continue; //already added
            seen_stamp[u] = stamp;
            tmp_neighbors.push_back(u);
        }
    }

    // For each neighbor: remove from its current bucket, recompute best (to, gain), reinsert
    for (int idx = 0; idx < (int)tmp_neighbors.size(); idx++) {
        int u = tmp_neighbors[idx];
        int from_side = part[u];

        if (buckets[from_side].contain(u)) {
            buckets[from_side].remove(u);
        }

        int best_to   = -1;
        int best_gain = INT_MIN;

        // Evaluate all alternative parts for k-way
        for (int to = 0; to < k; ++to) {
            if (to == from_side) continue;
            if (!legal(u, to))   continue; //add legality test
            int g = compute_gain(u, to);
            if (g > best_gain) {
                best_gain = g;
                best_to   = to;
            }
        }
        if (best_to == -1) best_gain = 0;   //avoid throw int_min to bucket
        cell_best_to[u]   = best_to;
        cell_best_gain[u] = best_gain;
        cell_gain[u]      = best_gain;

        buckets[from_side].insert(u, best_gain);
    }
}


FMmove FM::select_best_move(){
    FMmove no_move;
    no_move.cell = -1;
    no_move.origin_part = -1;
    no_move.new_part = -1;
    no_move.gain = INT_MIN;

    if (k == 2) {
        vector<pair<int,int>> popped0, popped1;

        while (1) {
            pair<int,int> t0 = buckets[0].peek_max();
            pair<int,int> t1 = buckets[1].peek_max();

            int best_from = -1;
            int best_cell = -1;
            int best_gain = INT_MIN;

            if (t0.first != -1 && t0.second > best_gain) {
                best_from = 0;
                best_cell = t0.first;
                best_gain = t0.second;
            }
            if (t1.first != -1 && t1.second > best_gain) {
                best_from = 1;
                best_cell = t1.first;
                best_gain = t1.second;
            }

            if (best_cell == -1) {
                for (pair<int,int>& p : popped0) buckets[0].insert(p.first, p.second);
                for (pair<int,int>& p : popped1) buckets[1].insert(p.first, p.second);
                return no_move;
            }

            int to = 1 - best_from;
            int u_peek = best_cell;

            if (legal(u_peek, to)) {
                pair<int,int> popped_pair = buckets[best_from].pop_max();
                int u = popped_pair.first;
                int g = popped_pair.second;

                for (pair<int,int>& p : popped0) buckets[0].insert(p.first, p.second);
                for (pair<int,int>& p : popped1) buckets[1].insert(p.first, p.second);

                FMmove best_move;
                best_move.cell = u;
                best_move.origin_part = best_from;
                best_move.new_part = to;
                best_move.gain = g;
                return best_move;
            } else {
                pair<int,int> popped_pair = buckets[best_from].pop_max();
                if (best_from == 0) popped0.push_back(popped_pair);
                else                popped1.push_back(popped_pair);
            }
        }
    }
    else if (k == 4) {
        vector<pair<int,int>> popped[4];

        while (1) {
            int best_from = -1;
            int best_cell = -1;
            int best_gain = INT_MIN;

            for (int from = 0; from < 4; ++from) {
                pair<int,int> t = buckets[from].peek_max();
                if (t.first == -1) continue;
                if (t.second > best_gain) {
                    best_gain = t.second;
                    best_cell = t.first;
                    best_from = from;
                }
            }

            if (best_cell == -1) {
                for (int i = 0; i < 4; ++i)
                    for (pair<int,int>& p : popped[i]) buckets[i].insert(p.first, p.second);
                return no_move;
            }

            int u_peek = best_cell;
            int to = cell_best_to[u_peek];

            
            /* 
            If the precomputed best_to is illegal at this moment, 
            recompute a *new best legal destination* on the spot.
            prevents the same illegal cells from being poppedrepeatedly, 
            which previously caused massive slowdown.
            */
            if (to == -1 || !legal(u_peek, to)) {
                // Temporarily remove this cell to change its key (gain)
                pair<int,int> popped_pair = buckets[best_from].pop_max();
                int u = popped_pair.first;
                int old_gain = popped_pair.second;

                int best_to_legal = -1;
                int best_gain_legal = INT_MIN;

                // Re-evaluate possible destinations among the other 3 partitions
                for (int cand = 0; cand < 4; ++cand) {
                    if (cand == best_from) continue;
                    if (!legal(u, cand)) continue;                 // ? ??? u
                    int g2 = compute_gain(u, cand);                // ? ??? u
                    if (g2 > best_gain_legal) {
                        best_gain_legal = g2;
                        best_to_legal = cand;
                    }
                }

                if (best_to_legal == -1) {
                    // Still no legal destination ? temporarily shelve this cell
                    popped[best_from].push_back(make_pair(u, old_gain));
                    continue;
                } else {
                    // Found a legal destination ? update info and reinsert (rekey)
                    cell_best_to[u_peek]   = best_to_legal;
                    cell_best_gain[u_peek] = best_gain_legal;
                    cell_gain[u_peek]      = best_gain_legal;

                    // Reinsert with updated gain so it's reconsidered in next iteration
                    buckets[best_from].insert(u_peek, best_gain_legal);
                    continue; // Skip move now, reselect globally best next round
                }
            }

            // =============================================================
            // If we reach here: current top cell has a legal move , commit it
            // =============================================================
            pair<int,int> popped_pair = buckets[best_from].pop_max();
            int u = popped_pair.first;
            int g = popped_pair.second;

            // Restore previously popped (non-chosen) cells
            for (int i = 0; i < 4; ++i)
                for (pair<int,int>& p : popped[i]) buckets[i].insert(p.first, p.second);

            FMmove best_move;
            best_move.cell = u;
            best_move.origin_part = best_from;
            best_move.new_part = to;
            best_move.gain = g;
            return best_move;
         
        }
    }

    return no_move;
}


bool FM::legal(int cell, int new_part){
    int from = part[cell];
    if(from == new_part) return false;

    long long int w;
    if(!hg.cell_size.empty()) w = (long long int)hg.cell_size[cell];
    else w = 1LL;

    long long int total;
    if(hg.total_cell_size > 0) total = hg.total_cell_size;
    else total = (long long int)hg.cell_name.size();

    long long int from_after = group_size_sum[from] - w;
    long long int to_after   = group_size_sum[new_part] + w;

    double range_min,range_max;
    if(k==2){
        range_min = 0.45;
        range_max = 0.55;
    }
    else if(k==4){
        range_min = 0.225;
        range_max = 0.275;
    }
    else{
        double target = 1.0/(double)k;
        double tol = 0.05;
        range_min = target - tol;
        range_max = target + tol;
    }

    //in the first few pass, allow more unbalanced partition
    double relax = 0.0;
    /*
    if(pass_id == 1){
        if(k == 2) relax = 0.1;
        else if(k == 4) relax = 0.05;
    }
    if(pass_id == 2){
        if(k == 2) relax = 0.05;
        else if(k == 4) relax = 0.025;
    }
    */
    double adj_min = range_min - relax;
    double adj_max = range_max + relax;

    double r_from = (double)from_after/(double)total;
    double r_to   = (double)to_after  /(double)total;

    if( (r_from < adj_min) || (r_from > adj_max) ) return false;
    if( (r_to   < adj_min) || (r_to   > adj_max) ) return false;
    return true;
}

void FM::set_lock(int cell) {
    if (locked[cell]) return;
    int from = part[cell];
    if (buckets[from].contain(cell)) {
        buckets[from].remove(cell);
    }
    locked[cell] = 1; 
}

void FM::reset_lock() {
    fill(locked.begin(), locked.end(), 0);
}

void FM::restore_by_max_prefix(const std::vector<FMmove>& seq, int best_prefix_len,vector<int>& base_part, vector<int>& out_part) const{
    out_part = base_part;
    if(best_prefix_len < 0) best_prefix_len = 0;
    for (int i = 0; i < best_prefix_len; ++i) {
        const FMmove& m = seq[i];
        out_part[m.cell] = m.new_part;
    }
}


void FM::recompute_group_sizes(){
    fill(group_size_sum.begin(), group_size_sum.end(), 0LL);
    for (int i = 0; i < (int)part.size(); ++i) {
        long long w = hg.cell_size.empty() ? 1LL : (long long)hg.cell_size[i];
        group_size_sum[ part[i] ] += w;
    }
}


void FM::build_importance(){
    int n = (int)hg.cell_name.size();
    imp.assign(n, 0.0);

    int max_net_id = -1;
    for (int v = 0; v < n; ++v) {
        const vector<int>& nets = hg.cell_to_net[v];
        for (int i = 0; i < (int)nets.size(); ++i) {
            if (nets[i] > max_net_id) max_net_id = nets[i];
        }
    }
    int m = max_net_id + 1;

    vector<int> net_deg(m, 0);
    for (int v = 0; v < n; ++v) {
        const vector<int>& nets = hg.cell_to_net[v];
        for (int i = 0; i < (int)nets.size(); ++i) {
            int e = nets[i];
            if (e >= 0 && e < m) net_deg[e] += 1;
        }
    }

    vector<double> inc_by_net(m, 0.0);
    for (int e = 0; e < m; ++e) {
        int denom = net_deg[e] - 1;
        if (denom < 1) denom = 1;
        inc_by_net[e] = 1.0 / (double)denom;
    }

    for (int v = 0; v < n; ++v) {
        const vector<int>& nets = hg.cell_to_net[v];
        double s = 0.0;
        for (int i = 0; i < (int)nets.size(); ++i) {
            int e = nets[i];
            if (e >= 0 && e < m) s += inc_by_net[e];
        }
        imp[v] = s;
    }
}

void FM::order_by_importance(vector<int>& idx, unsigned seed) const{
    int n = (int)hg.cell_name.size();
    idx.resize(n);
    for (int i = 0; i < n; ++i) idx[i] = i;

    //first time -> used by importance , but if the testcase is smaller, use random
    //to discover more probability
    static bool used_importance_once = false;

    std::mt19937 rng(seed);

    if (!used_importance_once) {
        shuffle(idx.begin(), idx.end(), rng);
        stable_sort(idx.begin(), idx.end(), [&](int a, int b){
            return imp[a] > imp[b];
        });
        int head = (int)(0.30 * (double)n);
        if (head < 0) head = 0;
        if (head > n) head = n;
        if (head < n) shuffle(idx.begin() + head, idx.end(), rng);
        used_importance_once = true;
    } else {
        //the second time or after -> random
        shuffle(idx.begin(), idx.end(), rng);
    }
}


