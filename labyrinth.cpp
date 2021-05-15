#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <omp.h>
#include <math.h>

using namespace std;

class Maze {
public:
    explicit Maze(char *winner) {
        this->winner = winner;
        omp_init_lock(&this->ompLock);
    };
    vector<pair<pair<int, int>, char>> position_info(pair<int, int> position) {
        vector<pair<pair<int, int>, char>> positions;
        if (position.second-1 > 0) {
            auto symbol = field[position.first][position.second - 1];
            if (symbol == ' ' || symbol == '$' || symbol == 'E'){
                positions.emplace_back(make_pair(position.first, position.second - 1), symbol);
            }
        }

        if (position.second+1 < field[position.first].size()) {
            auto symbol = field[position.first][position.second + 1];
            if (symbol == ' ' || symbol == '$' || symbol == 'E'){
                positions.emplace_back(make_pair(position.first, position.second + 1), symbol);
            }
        }

        if (position.first + 1 < field.size()) {
            auto symbol = field[position.first + 1][position.second];
            if (symbol == ' ' || symbol == '$' || symbol == 'E'){
                positions.emplace_back(make_pair(position.first + 1, position.second), symbol);
            }
        }

        if (position.first - 1 > 0) {
            auto symbol = field[position.first - 1][position.second];
            if (symbol == ' ' || symbol == '$' || symbol == 'E'){
                positions.emplace_back(make_pair(position.first - 1, position.second), symbol);
            }
        }
        return positions;
    };
    pair<bool,int> move_player(pair<int, int> from, pair<int, int> to) {
        bool moved;
        int money = 0;
        //omp_set_lock(&ompLock);
        if (field[to.first][to.second] == '$') {
            srand( time(0) );
            money = 10 + (rand() % static_cast<int>(25 - 10 + 1));
        }
        if (field[to.first][to.second] == ' ' || field[to.first][to.second] == '$' || field[to.first][to.second] == 'E') {
            if (field[to.first][to.second] == 'E') {
                *this->winner = field[from.first][from.second];
            }
            field[to.first][to.second] = field[from.first][from.second];
            field[from.first][from.second] = ' ';
            moved = true;
            this->print();
        } else {
            moved = false;
        }
        //omp_unset_lock(&ompLock);
        return make_pair(moved, money);
    };
    pair<int,int> get_pos(char symbol) {
        for (auto i = 0;i<field.size();i++) {
            for (auto j = 0;j<field[i].size();j++) {
                if (field[i][j] == symbol) {
                    return make_pair(i,j);
                }
            }
        }
        return make_pair(-1,-1);
    };
    vector<pair<int,int>> get_all_golds() {
        vector<pair<int, int>> golds;
        for (auto i = 0;i<field.size();i++) {
            for (auto j = 0;j<field[i].size();j++) {
                if (field[i][j] == '$') {
                    golds.emplace_back(make_pair(i, j));
                }
            }
        }
        return golds;
    }
    void result() {
        cout << "---------------"<< endl;
        if (*this->winner == ' ') {
            cout << "No winner" << endl;
        } else {
            cout << "Player " << *this->winner << " is WINNER!" << endl;
        }
    }
    void print() {
        cout << "---------------"<< endl;
        for (auto &row : field) {
            for (auto &position : row) {
                cout << position;
            }
            cout << endl;
        }
    };
    void init(const string& filename) {
        ifstream in(filename);
        if (in.is_open())
        {
            string line;
            while(getline(in, line))
            {
                vector<char> row;
                for(char & j : line)
                {
                    row.push_back(j);
                }
                field.push_back(row);
            }
        }
        in.close();
    };
private:
    vector<vector<char>> field;
    omp_lock_t ompLock;
    char *winner;
};

class Player {
public:
    int GetScore() {
        return this->gold;
    }
    void Start() {
        auto start = maze->get_pos(player_name);
        coord = start;
        while (true) {
            if (*this->winner != ' ' || this->gold == 0) {
                break;
            }
            Turn();
        }
    }
    virtual void Turn() = 0;
    Player(char *winner, char player_name, Maze *maze) {
        this->maze = maze;
        this->winner = winner;
        this->player_name = player_name;
        this->gold = 100;
    };
protected:
    Maze *maze;
    int gold;
    pair<int,int> coord;
    char *winner;
    char player_name;
};

class Player1: public Player {
private:
    int k;
    pair<int, int> exit;
    vector<pair<int,int>> golds;
    bool point_in_vision(pair<int, int> point) {
        if (coord.first >= point.first - k  && coord.first <= point.first + k  && coord.second >= point.second - k && coord.second <= point.second + k) {
            return true;
        }
        return false;
    }
public:
    Player1(char *winner, char player_name, Maze *maze) : Player(winner, player_name, maze) {
        this->k=3;
        this->exit = maze->get_pos('E');
        this->golds = maze->get_all_golds();
    };
    void Turn() override {
        pair<int,int> current_move;
        pair<int, int> move_to = make_pair(-1,-1);
        vector<pair<int,int>> possible_moves;
        auto moves = maze->position_info(coord);
        if (moves.size() == 0) {
            return;
        }
        if (point_in_vision(exit)) {
            move_to = make_pair(exit.first, exit.second);
        }
        for (auto gold: golds) {
            if (point_in_vision(gold)) {
                move_to = make_pair(gold.first, gold.second);
            }
        }
        if (move_to.first != -1) {
            double min_score = 10000000; // TODO: remove const
            vector<pair<int,int>> best_moves;
            for(auto move: moves) {
                auto score = sqrt(pow(move_to.first-move.first.first,2) + pow(move_to.second-move.first.second,2));
                if( score == min_score ) {
                    best_moves.push_back(move.first);
                } else if (score < min_score) {
                    best_moves.clear();
                    best_moves.push_back(move.first);
                    min_score = score;
                }
            }
            srand( time(0) );
            current_move = best_moves[rand() % best_moves.size()];
        } else {
            srand( time(0) );
            current_move = moves[rand() % moves.size()].first;
        }
        auto result = maze->move_player(coord, current_move);
        if(result.first) {
            coord = current_move;
            gold = gold + result.second - 1;
            this->golds = maze->get_all_golds();
        }
    };
};

class Player2: public Player {
private:
    vector<pair<int,int>> route;
    pair<int, int> search_move(vector<pair<int, int>> moves) {
        pair<int,int> result;
        result = make_pair(-1,-1);
        for (auto move: moves) {
            if (move.first == coord.first && move.second == coord.second + 1) {
                result = move;
            }
        }
        if (result.first == -1 && result.second == -1) {
            srand( time(0) );
            result = moves[rand() % moves.size()];
        }
        return result;
    }
public:
    Player2(char *winner, char player_name, Maze *maze) : Player(winner, player_name, maze) {};
    void Turn() override {
        pair<int,int> current_move;
        auto moves = maze->position_info(coord);
        if (moves.size() == 0) {
            return;
        }
        vector<pair<int, int>> moves_not_in_routes;
        if (route.empty()) {
            for (auto move: moves) {
                moves_not_in_routes.push_back(move.first);
            }
        } else {
            for (auto move: moves) {
                bool flag = true;
                for (auto pos: route) {
                    if(move.first.first == pos.first && move.first.second == pos.second) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    moves_not_in_routes.push_back(move.first);
                }
            }
        }
        if (moves_not_in_routes.empty()) {
            srand( time(0) );
            current_move = moves[rand() % moves.size()].first;
        } else {
            current_move = search_move(moves_not_in_routes);
        }
        auto result = maze->move_player(coord, current_move);
        if(result.first) {
            route.push_back(coord);
            coord = current_move;
            gold = gold + result.second - 1;
        }
    };
};

class Player3: public Player {
private:
    vector<pair<int,int>> route;
    pair<int, int> search_move(vector<pair<int, int>> moves) {
        pair<int,int> result;
        result = make_pair(-1,-1);
        for (auto move: moves) {
            if (move.first == coord.first && move.second == coord.second - 1) {
                result = move;
            }
        }
        if (result.first == -1 && result.second == -1) {
            srand( time(0) );
            result = moves[rand() % moves.size()];
        }
        return result;
    }
public:
    Player3(char *winner, char player_name, Maze *maze) : Player(winner, player_name, maze) {};
    void Turn() override {
        pair<int,int> current_move;
        auto moves = maze->position_info(coord);
        if (moves.size() == 0) {
            return;
        }
        vector<pair<int, int>> moves_not_in_routes;
        if (route.empty()) {
            for (auto move: moves) {
                moves_not_in_routes.push_back(move.first);
            }
        } else {
            for (auto move: moves) {
                bool flag = true;
                for (auto pos: route) {
                    if(move.first.first == pos.first && move.first.second == pos.second) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    moves_not_in_routes.push_back(move.first);
                }
            }
        }
        if (moves_not_in_routes.empty()) {
            srand( time(0) );
            current_move = moves[rand() % moves.size()].first;
        } else {
            current_move = search_move(moves_not_in_routes);
        }
        auto result = maze->move_player(coord, current_move);
        if(result.first) {
            route.push_back(coord);
            coord = current_move;
            gold = gold + result.second - 1;
        }
    };
};

class Player4: public Player {
private:
    bool back;
    int fork;
    pair<int, int> exit;
    vector<pair<int,int>> route;
    pair<int,int> get_best(const vector<pair<pair<int, int>, char>>& moves) {
        pair<int,int> current_move;
        double min_score = 10000000; // TODO: remove const
        vector<pair<int,int>> best_moves;
        for(auto move: moves) {
            auto score = sqrt(pow(exit.first-move.first.first,2) + pow(exit.second-move.first.second,2));
            if( score == min_score ) {
                best_moves.push_back(move.first);
            } else if (score < min_score) {
                best_moves.clear();
                best_moves.push_back(move.first);
                min_score = score;
            }
        }
        srand( time(0) );
        current_move = best_moves[rand() % best_moves.size()];
        return current_move;
    }
public:
    Player4(char *winner, char player_name, Maze *maze) : Player(winner, player_name, maze) {
        this->exit = maze->get_pos('E');
        back = false;
    };
    void Turn() override {
        pair<int,int> current_move;
        auto moves = maze->position_info(coord);
        if (moves.size() == 0) {
            return;
        }
        if (moves.size() > 2) {
            fork = route.size();
        }
        if (route.empty()) {
           current_move = get_best(moves);
        } else {
            if(moves.size() == 1) {
                back = true;
                current_move = moves[0].first;
            } else {
                if (back) {
                    vector<pair<int, int>> moves_not_in_routes;
                    if (route[fork] == coord) {
                        back = false;
                        for (auto move: moves) {
                            bool flag = true;
                            for (auto pos: route) {
                                if(move.first.first == pos.first && move.first.second == pos.second) {
                                    flag = false;
                                    break;
                                }
                            }
                            if (flag) {
                                moves_not_in_routes.push_back(move.first);
                            }
                        }
                        if (moves_not_in_routes.empty()) {
                            srand( time(0) );
                            current_move = moves[rand() % moves.size()].first;
                        } else {
                            current_move = moves_not_in_routes[rand() % moves_not_in_routes.size()];
                        }
                    } else {
                        bool flag = false;
                        for(auto move: moves) {
                            for(auto i = fork; i < route.size(); i++) {
                                if (route[i] == move.first) {
                                    current_move = move.first;
                                    flag = true;
                                    break;
                                }
                            }
                            if (flag) {
                                break;
                            }
                        }
                    }
                } else {
                    current_move = get_best(moves);
                }
            }
        }
        auto result = maze->move_player(coord, current_move);
        if(result.first) {
            route.push_back(coord);
            coord = current_move;
            gold = gold + result.second - 1;
        }
    };
};

int main()
{
    char winner = ' ';
    Maze maze(&winner);
    maze.init("lab1");
    maze.print();
    Player1 player1 = Player1(&winner, '1', &maze);
    Player2 player2 = Player2(&winner, '2', &maze);
    Player3 player3 = Player3(&winner, '3', &maze);
    Player4 player4 = Player4(&winner, '4', &maze);
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task
            player1.Start();
            #pragma omp task
            player2.Start();
            #pragma omp task
            player3.Start();
            #pragma omp task
            player4.Start();
            #pragma omp taskwait
        }
    }
    maze.result();
    std::cout << "Score Player1: " << player1.GetScore() << std::endl;
    std::cout << "Score Player2: " << player2.GetScore() << std::endl;
    std::cout << "Score Player3: " << player3.GetScore() << std::endl;
    std::cout << "Score Player4: " << player4.GetScore() << std::endl;
    return 0;
}