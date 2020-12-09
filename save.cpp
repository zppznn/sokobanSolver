#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator> 
#include <limits.h>
#include <unistd.h>

#define ll uint64_t

using namespace std;


typedef struct board{
    vector<pair<int,int>> *b_boxes;              // pointer to current boxes coordinates
    int h;                                       // current total board heuristic value
    int x,y;                                     // current x y coordinates of the player
    //unordered_map<int,int> * b_mapping;          // pointer to current mapping of box_id to location_id. If a box is not mapped, it is not in it. 
    int steps;                                   // how many steps is now
    
    pair<int,int> prev_box;                      // box A being pushed in previous board resulting in current board
    pair<int,int> cur_box;                       // box A's new location in current board
    board * prev_board;                          // pointer to previous board
    bool operator==(const board& b) const{
        if (b_boxes->size() != b.b_boxes->size()) return false;
        for(int i = 0; i<b_boxes->size();i++){
            if( (*b_boxes)[i] != (*b.b_boxes)[i] ) return false;
        }
        return true;
    }
} board;

struct pair_hash{
    size_t operator () (const pair<int,int> & p) const{
        unsigned h = p.first;
        h = h * p.second;
        h = h ^ p.first;
        return  h;
    }
};

struct board_hash{
    size_t operator()(const board &b) const{
        ll x_value, y_value,hash ;
        for(int i = 0; i< b.b_boxes->size();i++){
            x_value += (*b.b_boxes)[i].first * i;
            y_value += (*b.b_boxes)[i].second * i;
        }
        hash = x_value ^ y_value;
        return size_t(hash);
    }
};


struct pq_compare{
    bool operator()(const board &b1, const board &b2){
        return b1.h > b2.h;
    }

};

// global variables 
unsigned board_height, board_width, box_num, location_num, init_player_x, init_player_y, init_heuristic;
vector<pair<int,int>> init_boxes, init_locations;         // boxs may move, but the ID is not changed
unordered_set<pair<int,int>, pair_hash> walls;
unordered_map<int,int> mapping;             // map  box id to location id. constant. will not change after initialization


// read board information from kInputFile. coordinate index stars from 0. BUt input file stars from 1
// only read 5 lines
// Do initialization 
void ReadInput(string input_file ){
    ifstream file(input_file);
	string line;
    istringstream iss;
    vector<int> results;
    // get 1st line. size of board
    getline(file,line);
    iss = istringstream(line);
    results = vector<int>(istream_iterator<int>{iss}, istream_iterator<int>());
    board_width = results[0];                          
    board_height = results[1];

    // get 2rd line. walls
    getline(file,line);
    iss = istringstream(line);
    results = vector<int>(istream_iterator<int>{iss}, istream_iterator<int>());
    for(int i = 1; i < results.size(); i += 2){
        walls.insert({results[i] - 1, results[i+1] - 1});
    }

    // get 3nd line. box coordinates
    getline(file,line);
    iss = istringstream(line);
    results = vector<int>(istream_iterator<int>{iss}, istream_iterator<int>());
    box_num = results[0];
    for(int i = 1; i < results.size(); i += 2){
        init_boxes.push_back({results[i] - 1, results[i+1] - 1});
    }

    // get 4th line. storage coordinates
    getline(file,line);
    iss = istringstream(line);
    results = vector<int>(istream_iterator<int>{iss}, istream_iterator<int>());
    location_num = results[0];
    for(int i = 1; i < results.size(); i += 2){
        init_locations.push_back({results[i] - 1, results[i+1] - 1});
    }

    // get 5th line. player coordinates 
    getline(file,line);
    iss = istringstream(line);
    results = vector<int>(istream_iterator<int>{iss}, istream_iterator<int>());
    init_player_x = results[0] - 1; init_player_y = results[1] - 1;
}


// calculate reachable distance from box_id to every other location. 
// -1 if can not reach
void _MapBox2LocationHelper1(vector<vector<int>> &m, int box_id){
    vector<vector<int>> directions = {{1,0},{0,1}};
    // hashmap | location pair<int,int > to its location id
    unordered_map<pair<int,int>, int, pair_hash> loc_map;
    for(int i = 0 ;i < location_num; i++) loc_map[init_locations[i]] = i;
    int x = init_boxes[box_id].first, y = init_boxes[box_id].second;
    int dist = 0;
    queue<pair<int,int>> q;
    q.push({x,y});
    unordered_set<pair<int,int>, pair_hash>  visited;
    while(!q.empty()){
        for(int i = q.size() ; i > 0;i--){
            auto cur_loc = q.front();q.pop();
            if(loc_map.count(cur_loc)) m[box_id][loc_map[cur_loc]] = dist;
            visited.insert(cur_loc);
            for(auto dir : directions){
                auto new_loc1 = cur_loc, new_loc2 = cur_loc;
                new_loc1.first += dir[0];
                new_loc1.second += dir[1];
                new_loc2.first -= dir[0];
                new_loc2.second -= dir[1];
                // if new location is not wall and not visited, add to queue
                if(!walls.count( new_loc1) && !(walls.count(new_loc2) ) && !visited.count(new_loc1)){
                    q.push(new_loc1);
                }
                if(!walls.count( new_loc1) && !(walls.count(new_loc2) ) && !visited.count(new_loc2)){
                    q.push(new_loc2);
                }
            }
        }
        dist++; 
    }

}

// recursively find the best mapping solution if there is one.
// return -1 if failed to find one. else return the sum of distance (heuristic value)
void _dfs(vector<vector<int>> &m, unordered_map<int,int> & path, unordered_set<int> &filled, int location_id, int cur_sum, int &min_sum){
    if(location_id >= location_num){
    
        if(cur_sum < min_sum){
            mapping = path;
            init_heuristic = cur_sum;
            min_sum = cur_sum;
        }
        return ;
    }
    // for each location id, find its best box id. Beucase we could have more boxes than locations
    for(int j = 0; j < box_num;j++){
        for(int i = 0 ;i<m[j].size();i++){
            if(i == location_id && !filled.count(j)){
                path[j] = i;
                filled.insert(j);
                _dfs(m, path, filled, location_id + 1, cur_sum + m[j][i], min_sum);
                path.erase(j);
                filled.erase(j);
            }   
        }
    }

}

// pick a local best solution for box->storage mapping
// use dfs to get best workable solution
// mapping is a global variable that will not change after this function
void _MapBox2LocationHelper2(vector<vector<int>> &m){
    unordered_set<int> filled_location;
    unordered_map<int,int> map_path;
    int min_sum = INT_MAX;
    _dfs(m, map_path, filled_location,0,0,min_sum);
    if(mapping.empty()){
        cout <<"Mapping creation failed\n";
    }

    //printing
    for(auto i : mapping){
        cout <<"Box " << i.first <<" maps to location " << i.second <<endl;
    }

}

// for each box. use bfs to calculate its reachable location distance. Form a 2-d adjcency matrix
// then select the best mapping from 2-d matrix and form a hashmap for future fast lookup
void MapBox2Location(){
    //calculate reachable distance from box_id to every other location. 
    vector<vector<int>> adj_matrix (box_num, vector<int> (location_num,-1)); // adjcent matrix : distance of box id to location id
    for(int i = 0 ; i < box_num; i++){
        _MapBox2LocationHelper1(adj_matrix, i);                              
    }

    //printing
    for(int i = 0 ; i< box_num;i++){
        for(int j = 0 ; j < location_num;j++){
            cout << adj_matrix[i][j] <<" ";
        }
        cout << endl;
    }

    // test if there is a solution for each storage location 
    for(int j = 0 ;j< location_num ;j++){
        bool has_solution = false;
        for(int i = 0 ; i < box_num; i++){
            if(adj_matrix[i][j] != -1) has_solution = true;
        }
        if(!has_solution ){
            cout <<"There is a storage box that has no solution \n";
            return;
        }
    }
    
    // pick a best solution for box->storage mapping
    _MapBox2LocationHelper2(adj_matrix);

}

// check if this is the final state
bool SolutionFound(const board & b){
    unordered_set<pair<int,int>, pair_hash> s((*b.b_boxes).begin(), (*b.b_boxes).end());
    for(auto loc : init_locations) if(!s.count(loc)) return false;
    return true;
}


void CleanBoard(board &b){
    delete b.b_boxes;
}

void CleanEverything(unordered_set<board, board_hash> &s, priority_queue<board,vector<board>,pq_compare> &pq){
    for( int i = pq.size();i>0; i--){
        board b = pq.top();pq.pop();
        if(s.count(b)){ s.erase(b); }
        CleanBoard(b);
    }
    for(auto i : s) CleanBoard(i);
}

inline int ManDistance(const pair<int,int> &b1, const pair<int,int> &b2){
    return abs(b1.first - b2.first) + abs(b1.second - b2.second);
}

// if current box moved is not mapped to any location, H does not change.
// if it is a mapped box.  So we calculate H by comparing manhattan distance of (box A's new location , storage_loc} and (Box A's old location, storage_loc)
// since new pushed location may cause this box A to reach a dead state. However, other unmapped box B may replace box A to 
// be pushed to A's mapped location. 
int CalculateNewH(pair<int,int> old_box, pair<int,int> new_box, int box_id , int old_h){
    if(!mapping.count(box_id)) return old_h;         
    int dest_loc_id = mapping[box_id];
    pair<int,int> des_loc = init_locations[dest_loc_id];
    if( ManDistance(old_box, des_loc) > ManDistance(new_box, des_loc)) return old_h-1;
    else return old_h+1;
}
// use bfs to find all next available moves. And calculate hueristics.
vector<board> FindNextBoards(board &b){
    vector<pair<int,int>> directions = {{1,0},{-1,0},{0,1},{0,-1}};
    vector<board> res;
    unordered_set<pair<int,int>, pair_hash> box_set ((*b.b_boxes).begin() ,(*b.b_boxes).end() );
    unordered_set<pair<int,int>, pair_hash> visited;
    queue<pair<int,int>> q;            // represent current player reachable locations.
    q.push({b.x,b.y});
    while(!q.empty()){
        pair<int,int> cur = q.front();q.pop();

        visited.insert(cur);
        int new_x, new_y;
        for(auto dir : directions){
            new_x = cur.first + dir.first;
            new_y = cur.second + dir.second;
            // a new non-visited, non-wall, non-box location
            if( !visited.count({new_x,new_y}) && !walls.count({new_x,new_y}) && !box_set.count({new_x,new_y})){
                q.push({new_x,new_y});
            }
            // if we can push this box from this side. // and the other side is blank
            else if(box_set.count({new_x,new_y})){
                if( !box_set.count({new_x + dir.first, new_y + dir.second}) && !walls.count({new_x + dir.first, new_y + dir.second})){  
                    // push this box to new location and create new board
                    board new_board;
                    vector<pair<int,int>> *temp_boxes = new vector<pair<int,int>> (*b.b_boxes);
                    pair<int,int> old_box_loc = {new_x,new_y};
                    pair<int,int> new_box_loc = {new_x + dir.first, new_y + dir.second};
                    int box_id;
                    for(int i = 0 ;i < (*b.b_boxes).size(); i++){
                        if((*(b.b_boxes))[i] == old_box_loc){
                            box_id = i;
                            (*temp_boxes)[i] = new_box_loc;
                            break;
                        }
                    }
                    new_board.b_boxes = temp_boxes;
                    new_board.x = new_x;
                    new_board.y = new_y;
                    new_board.h = CalculateNewH(old_box_loc,new_box_loc,box_id, b.h);
                    new_board.steps = b.steps + 1;
                    res.push_back(new_board);
                }
            }
        }

    }
    return res;

}

void printallboard(priority_queue<board,vector<board>,pq_compare> &pq){

    vector<vector<char>> e,v = {
        {'*','*','*','*','*'},
        {'*',' ',' ',' ','*'},
        {'*',' ',' ',' ','*'},
        {'*',' ',' ',' ','*'},
        {'*','*','*','*','*'}
    };
    e = v;
    for(auto i : (init_boxes) ){
        v[i.first][i.second] = 'B'; 
        cout << i.first <<"->" <<i.second<<endl;
    }
    for(int i =board_width -1; i>=0 ; i--){
        for(int j = 0;j< board_height;j++){
            cout <<v[i][j]<<" ";
        }
        cout <<endl;
    }
    v = e;
    cout <<"----------------------\n";
    for(int i = pq.size();i>0;i--){
        board b = pq.top(); pq.pop();
        for(auto i : (*b.b_boxes) ){
            v[i.first][i.second] = 'B'; 
        }
        for(int i =board_width -1; i>=0 ; i--){
            for(int j = 0;j< board_height;j++){
                cout <<v[i][j]<<" ";
            }
            cout <<endl;
        }
        cout <<"----------------------\n";
        v = e;
    }

}

void printboard(board &b){
    vector<vector<char>> e,v = {
        {'*','*','*','*','*'},
        {'*',' ',' ',' ','*'},
        {'*',' ',' ',' ','*'},
        {'*',' ',' ',' ','*'},
        {'*','*','*','*','*'}
    };
    e = v;
    for(auto i : (*b.b_boxes) ){
            v[i.first][i.second] = 'B'; 
        }
    for(int i =board_width -1; i>=0 ; i--){
        for(int j = 0;j< board_height;j++){
            cout <<v[i][j]<<" ";
        }
        cout <<endl;
    }
}
// The main logic. A* with heuristic
void run(){
    // initialization
    board init_board;
    vector<pair<int,int>> * temp_boxes = new vector<pair<int,int>> (init_boxes);
    //unordered_map<int,int> * temp_mapping = new unordered_map<int,int> (mapping);
    init_board.b_boxes = temp_boxes;
    // init_board.b_mapping = &;
    init_board.x = init_player_x;
    init_board.y = init_player_y;
    init_board.h = init_heuristic;
    init_board.steps = 0;
    priority_queue<board,vector<board>,pq_compare> pq;
    pq.push(init_board);
    unordered_set<board, board_hash> visited;
    int steps = 0;

    while(!pq.empty()){
        board curr_board = pq.top(); pq.pop();
        if( SolutionFound(curr_board)){
            cout <<"Solution found with number of pushes "<<curr_board.steps<<endl;
            CleanEverything(visited, pq);
            return ;
        }
        visited.insert(curr_board);
        vector<board> ret = FindNextBoards(curr_board);     // debug
                                                            // add deadstate detection
        for( auto new_board : ret){
            if ( !visited.count(new_board)){
                pq.push(new_board);
            }
        }


    }

    cout <<"Solution not found\n";
}
int main(int argc, char *argv[]){

    // read input to initialize board information
    string input_file(argv[1]);
    ReadInput(input_file);
    if (location_num > box_num){
        cout <<"Initialization failed. location number > box number\n"; 
        return 0;
    }
    
    // map each box to its best location, and produce a smallest heuristic
    MapBox2Location();
    cout << "init_heuristic "<< init_heuristic<<endl;
    run();
    return 0;
}