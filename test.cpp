#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <stack>
#include <queue>
#include <list>
#include <cstring>
#include <deque>
#include <bitset>
#include <iterator>
#include <algorithm>
#include <iostream>


using namespace std;


// typedef struct board{
//     vector<pair<int,int>> *b_boxes;              // pointer to current boxes coordinates
//     int h;                                       // current total board heuristic value
//     int x,y;                                     // current x y coordinates of the player
//     //unordered_map<int,int> * b_mapping;          // pointer to current mapping of box_id to location_id. If a box is not mapped, it is not in it. 
//     int steps;                                   // how many steps is now
    
//     pair<int,int> prev_box;                      // box A being pushed in previous board resulting in current board
//     pair<int,int> cur_box;                       // box A's new location in current board
//     board * prev_board;                          // pointer to previous board
//     bool operator==(const board* b) const{
//         if (b_boxes->size() != b->b_boxes->size()) return false;
//         for(int i = 0; i<b_boxes->size();i++){
//             if( (*b_boxes)[i] != (*b->b_boxes)[i] ) return false;
//         }
//         return true;
//     }
// } board;

// struct board_hash{
//     size_t operator()(const board *b) const{
//         ll x_value, y_value,hash_value ;
//         for(ll i = 0; i< b->b_boxes->size();i++){
//             x_value += (*b->b_boxes)[i].first * i;
//             y_value += (*b->b_boxes)[i].second * i;
//         }
//         hash<ll> hash_ll;
//         hash_value = hash_ll(x_value+y_value);
//         return hash_value;
//     }
// };



typedef struct chess{
    vector<pair<int,int>> *b_chess;              // pointer to current boxes coordinates
    int h;                                       // current total board heuristic value
    int x,y;                                     // current x y coordinates of the player
    //unordered_map<int,int> * b_mapping;          // pointer to current mapping of box_id to location_id. If a box is not mapped, it is not in it. 
    int steps;                                   // how many steps is now
    
    pair<int,int> prev_box;                      // box A being pushed in previous board resulting in current board
    pair<int,int> cur_box;                       // box A's new location in current board
    bool operator==(const  chess* b) const{
        if (b_chess->size() != b->b_chess->size()) {
            cout<<"size not same\n";
            return false;
        }
        for(int i = 0; i<b_chess->size();i++){
            if( (*b_chess)[i] != (*b->b_chess)[i] ) {
                cout <<"things diff\n";
                return false;
            }
        }
        
        return true;
    }
} chess;

struct chess_hash{
    size_t operator()(const chess *b) const{
        size_t x_value = 0, y_value = 0 ;
        for(size_t i = 0; i< b->b_chess->size();i++){
            x_value += (*b->b_chess)[i].first * i;
            y_value += (*b->b_chess)[i].second * i;
        }
        
        return x_value ^ y_value;
    }
};



int main(int argc, char ** argv) {

    vector<pair<int,int>> v = {{1,2},{3,4},{5,6}};
    vector<pair<int,int>> *v1 = new vector<pair<int,int>> (v);
    vector<pair<int,int>> *v2 = new vector<pair<int,int>> (*v1);


    chess *c1 = new chess;
    chess *c2 = new chess;
    c1->b_chess = v1;
    c2->b_chess = v2;

    unordered_set<chess*, chess_hash> s1;
    unordered_set<chess*, chess_hash>::hasher fn = s1.hash_function();
    cout << "c1's hashvalue " << fn(c1) <<" c2's hashvalue "<< fn(c2) << endl;

    s1.insert(c1);
    if(s1.count(c1)){
        cout <<"Found chess c1\n";
    }
    if(s1.count(c2)){
        cout <<"Found chess c2\n";
    }
    cout <<"end\n";
}


