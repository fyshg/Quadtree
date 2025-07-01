#ifndef REGIONQUADTREE_HPP
#define REGIONQUADTREE_HPP

#include <memory>
#include <math.h>
#include <cassert>
#include <vector>
#include <stack>


class RegionQuadtree {
    private:
        struct Node;
        class Iterator;
    public: 
                
        using Direction = enum{NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};
        using Colour = enum{WHITE, BLACK, GREY};
        using TreePtr = std::shared_ptr<Node>;
        using WeakTreePtr = std::weak_ptr<Node>;

        TreePtr root_node;
        std::size_t dim;



        RegionQuadtree(){
        }
        RegionQuadtree(RegionQuadtree&& other) : RegionQuadtree(){
                swap(*this, other);
        }
        RegionQuadtree(const RegionQuadtree& other) : dim(other.dim), root_node(std::make_shared<Node>(*(other.root_node))) {
        }
        

        RegionQuadtree& operator=(RegionQuadtree other) {
                 swap(*this, other);
                 return *this;
        }
        friend void swap(RegionQuadtree& qt1, RegionQuadtree& qt2) {
                using std::swap;
                swap(qt1.dim, qt2.dim);
                swap(qt1.root_node, qt2.root_node);
        }

        template<typename Iterator>  //only random access iterators please and only iterators on iterators or random Access data structurs like arrays
        RegionQuadtree(const Iterator& it_begin, const Iterator& it_end) :   
                root_node(std::make_shared<Node>(it_begin, it_end, it_end - it_begin, 0, 0)), dim(it_end - it_begin) {  //N must be set for the root node ....
                        assert( dim > 0 &&  ceil(log2(dim) == floor(log2(dim))));

        }
        
        Iterator begin(){
                return Iterator(*this);
        }
        Iterator end(){
                return Iterator();
        }
        
        template<typename T>
        RegionQuadtree(T (&arr)[1][1]) : root_node(arr[1][1] ? BLACK : WHITE) {
        }

        // Methods for static Array
        template<std::size_t N, typename T>
        RegionQuadtree(T (&arr)[N][N]) : RegionQuadtree(std::begin(arr), std::end(arr)){
        }
        
        std::size_t number_of_leaves(){
                return root_node->number_of_leaves();
        }

        std::vector<std::vector<bool>> get_array(){
                std::vector<std::vector<bool>> arr(dim);
                for(std::size_t i = 0; i < dim; ++i ){
                        arr[i] = std::vector<bool>(dim);
                }
                root_node->get_array(dim, 0, 0, arr);
                return arr;
        }
    private:
        struct Node{
            public:
                
                Node() {
                }

                Node(Colour colour) : colour(colour){
                }
                
                Node(const Node& other) : colour(other.colour) {
                        if(other.colour == GREY){
                              north_east = std::make_shared<Node>(*(other.north_east));
                              south_east = std::make_shared<Node>(*(other.south_east));
                              south_west = std::make_shared<Node>(*(other.south_west));
                              north_west = std::make_shared<Node>(*(other.north_west));
                       }   
                }


                template<typename Iterator>  //only random access iterators please
                Node(const Iterator& it_begin, const Iterator& it_end, 
                        std::size_t N, std::size_t x, std::size_t y) {     
                        if (N == 1 ){
                                colour = it_begin[y][x] ? BLACK : WHITE;

                        } else {
                                N = N/2;  //sidelength of subtrees...
                                TreePtr n_e = std::make_shared<Node>(it_begin, it_end, N, N + x , y );   
                                TreePtr s_e = std::make_shared<Node>(it_begin, it_end, N, N + x, N + y ); 
                                TreePtr s_w = std::make_shared<Node>(it_begin, it_end, N, x , N + y );
                                TreePtr n_w = std::make_shared<Node>(it_begin, it_end, N, x, y );

                                if(n_e->colour == s_e->colour && 
                                   s_e->colour == s_w->colour && 
                                   s_w->colour == n_w->colour && 
                                   n_w->colour == n_e->colour && 
                                   (n_e->colour != GREY)){
                                        colour = n_e->colour;
                                } else {
                                        north_east = std::move(n_e);
                                        south_east = std::move(s_e);
                                        south_west = std::move(s_w);
                                        north_west = std::move(n_w);
                                        colour = GREY;
                                }
                        }
                }
                
                void get_array(std::size_t N, std::size_t x, std::size_t y, std::vector<std::vector<bool>>& arr){
                        
                        if( colour == GREY) {
                                        N = N/2;
                                        north_east->get_array(N, N+x, y, arr);
                                        south_east->get_array(N, N + x, N + y, arr);
                                        south_west->get_array(N, x, N+y , arr);
                                        north_west->get_array(N, x, y, arr);
                                } else {
                                        for( std::size_t i = x; i < N+x; ++i){
                                                for(std::size_t j = y; j < N+y; ++j){
                                                        arr[j][i] = colour;
                                        }
                                }
                        }
                }
                
                std::size_t number_of_leaves(){
                        if(colour == BLACK || colour == WHITE){
                                return 1;
                        } else if(colour == GREY){
                                return  north_east->number_of_leaves() +south_east->number_of_leaves() + south_west->number_of_leaves() + north_west->number_of_leaves();
                        }
                }
            private:
                friend class Iterator; 
                TreePtr north_east;
                TreePtr south_east;
                TreePtr south_west; 
                TreePtr north_west;
                Colour colour;
                };      
        class Iterator {  //has to be random acces ...                   
            public: 
                        
                Iterator(RegionQuadtree& pq) : path(
                            std::stack<std::pair<WeakTreePtr,Direction>>()) {
                        path.push(std::make_pair(pq.root_node, NORTH_WEST));
                         // consider root node as sprecial case which is n w
                        //advance to the north eastest child
                        goto_leaf(pq.root_node);
                }               
                Iterator(std::stack<std::pair<WeakTreePtr, Direction>> path) :     path(path) {
                }
                                
                Iterator() : path(
                        std::stack<std::pair<WeakTreePtr, Direction>>()) {
                }
                Colour operator*() {
                        return path.top().first.lock()->colour;
                }   
                const Colour operator*() const {
                       return path.top().first.lock()->colour;
                }
                Colour* operator->() {
                       return &path.top().first.lock()->colour;
                }
                Iterator& operator++() {
                       advance();
                       return *this;
                }
                bool operator==(const Iterator& other){
                        bool end1 = path.empty();
                        bool end2 = other.path.empty();
                         if (end1 && end2) return true;
                                  // consider iterators as ended iterators
                        bool invalid1 = end1 || !path.top().first.lock();
                        bool invalid2 = end2 || !path.top().first.lock();
                        if (invalid1 && invalid2) return true;
                        if (invalid1 || invalid2) return false;

                        return path.top().first.lock() == 
                                other.path.top().first.lock();
                }
                bool operator!=(const Iterator& other){
                        return !(*this == other);
                }
            private: 
                std::stack<std::pair<WeakTreePtr, Direction>> path;  

                void advance() {  
                        auto current = path.top().first.lock(); 
                        Direction current_dir = path.top().second; 
                        bool found = false; 
                        while(!path.empty() && !found) {
                              path.pop();
                              if( path.empty()) break; 
                              auto father = path.top().first.lock();
                              if(current_dir == NORTH_EAST) {
                                   auto s_e = father->south_east;
                                   path.push(std::make_pair(s_e, SOUTH_EAST)); 
                                   goto_leaf(s_e);
                                   found = true;
                              } else if(current_dir == SOUTH_EAST) {
                                   auto s_w = father->south_west;
                                   path.push(std::make_pair(s_w, SOUTH_WEST)); 
                                   goto_leaf(s_w);
                                   found = true;
                              } else if(current_dir == SOUTH_WEST) {
                                   auto n_w = father->north_west;
                                   path.push(std::make_pair(n_w, NORTH_WEST)); 
                                   goto_leaf(n_w);
                                   found = true; 
                              }
                              current_dir = path.top().second;
                              current = father; 
                        }

               }

                void goto_leaf(TreePtr Node){
                        auto n = Node; 
                        while(n->colour == GREY){
                                n = n->north_east; 
                                path.push(std::make_pair(n, NORTH_EAST)); 
                        }
                }        
                        
        };
                
};

#endif