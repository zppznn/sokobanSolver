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

using namespace std;

const string kInputFile = "./input";

struct pair_hash{
    std::size_t operator () (const pair<int,int> & p) const{
        unsigned h = p.first;
        h = h * p.second;
        h = h ^ p.first;
        return  h;
    }
};

typedef struct{
    unordered_set<pair<int,int>> *b_walls;       // pointer to global vairable walls. constant walls
    vector<pair<int,int>> *b_boxes;              // pointer to current boxes coordinates
    int h;                                       // current total board heuristic value
    int x,y;                                     // current x y coordinates of the player
    unordered_map<int,int> * b_mapping;          // pointer to current mapping of box_id to location_id. If a box is not mapped, it is not in it. 

} board;

// global variables 
unsigned board_height, board_width, box_num, location_num, init_player_x, init_player_y, init_heuristic;
vector<pair<int,int>> init_boxes, init_locations;         // boxs may move, but the ID is not changed
unordered_set<pair<int,int>, pair_hash> walls;
unordered_map<int,int> mapping;             // map  box id to location id


// read board information from kInputFile. coordinate index stars from 0. BUt input file stars from 1
// only read 5 lines
// Do initialization 
void ReadInput(){
    ifstream file(kInputFile);
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
void _dfs(vector<vector<int>> &m, unordered_map<int,int> & path, unordered_set<int> &filled, int location_id, int cur_sum, int min_sum){
    if(location_id >= location_num){
        if(cur_sum < min_sum){
            mapping = path;
            init_heuristic = cur_sum;
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
                path.erase(location_id);
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

    // printing
    // for(int i = 0 ; i< box_num;i++){
    //     for(int j = 0 ; j < location_num;j++){
    //         cout << adj_matrix[i][j] <<" ";
    //     }
    //     cout << endl;
    // }

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

int main(int argc, char *argv[]){

    // read input to initialize board information
    ReadInput();
    if (location_num > box_num){
        cout <<"Initialization failed. location number > box number\n"; 
        return 0;
    }

    // map each box to its best location, and produce a smallest heuristic
    MapBox2Location();

    // TODO 
    // start the main game logic
    return 0;
}