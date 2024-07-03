#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include "primitives.h"



using namespace std;

const sol_t moves = { U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F, F2, F3, B, B2, B3 };
const sol_t drmoves = { U, U2, U3, D, D2, D3, R2, L2, F2, B2 };
  unordered_map<string, move_t> singmaster_map = {
    {"U", U}, {"U2", U2}, {"U'", U3},
    {"D", D}, {"D2", D2}, {"D'", D3},
    {"R", R}, {"R2", R2}, {"R'", R3},
    {"L", L}, {"L2", L2}, {"L'", L3},
    {"F", F}, {"F2", F2}, {"F'", F3},
    {"B", B}, {"B2", B2}, {"B'", B3}
};

void print_binary(uint8_t byte) {
    for(int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

cube_t apply_alg(cube_t cube, sol_t *moves) {
    auto ret = cube;
    for (auto m : *moves)
        ret = cube_move(ret, m);
    return ret;
}

void print_solution_singmaster(sol_t s, bool l) {
    for (int i = 0; i < s.size(); i++) {
        switch(s[i]) {
            case U: printf("U "); break;
            case U2: printf("U2 "); break;
            case U3: printf("U' "); break;
            case D: printf("D "); break;
            case D2: printf("D2 "); break;
            case D3: printf("D' "); break;
            case R: printf("R "); break;
            case R2: printf("R2 "); break;
            case R3: printf("R' "); break;
            case L: printf("L "); break;
            case L2: printf("L2 "); break;
            case L3: printf("L' "); break;
            case F: printf("F "); break;
            case F2: printf("F2 "); break;
            case F3: printf("F' "); break;
            case B: printf("B "); break;
            case B2: printf("B2 "); break;
            case B3: printf("B' "); break;
        }
    }
    if (l) printf("(%lu)", s.size());
    printf("\n");
}

void print_solution_enum(sol_t s) {
    for (int i = 0; i < s.size(); i++) {
        printf("%u ", s[i]);
    }
    printf("\n");
}

int eofb(const cube_t c) {
    for (int i = 0; i < EDGES; i++) {
        if (c.edge[i] & 0xF0) {
            return false;
        }
    }
    return true;
}

int eorl(const cube_t c) {
    cube_t transformed = cube_transform(c, URr);
    return eofb(transformed);
}

int eoud(const cube_t c) {
    cube_t transformed = cube_transform(c, FUr);
    return eofb(transformed);
}

int coud(const cube_t c) {
    for (int i = 0; i < CORNERS; i++) {
        if (c.corner[i] & 0xF0) {
            return false;
        }
    }
    return true;
}

int corl(const cube_t c) {
    cube_t transformed = cube_transform(c, RUr);
    return coud(transformed);
}

int cofb(const cube_t c) {
    cube_t transformed = cube_transform(c, FUr);
    return coud(transformed);
}

int drud(const cube_t c) {
    return eofb(c) && eorl(c) && coud(c);
}

int drrl(const cube_t c) {
    return drud(cube_transform(c, RUr));
}

int drfb(const cube_t c) {
    return drud(cube_transform(c, FUr));
}

int dr(const cube_t c) {
    return drud(c) || drrl(c) || drfb(c);
}

sol_t read_scramble_from_file(const string& filename) {
    ifstream file(filename);
    string line;
    sol_t scramble;
    
    if (file.is_open()) {
        while (getline(file, line)) {
            istringstream iss(line);
            string move_str;
            while (iss >> move_str) {
                scramble.push_back(singmaster_map[move_str]);
            }
        }
        file.close();
    } else {
        cerr << "Unable to open file" << endl;
    }
    
    return scramble;
}

bool isRedundantMove(move_t n, move_t l, move_t s) {
    if (l / 3 == n / 3) {
        return false;
    }
    if ((l / 6 == s / 6) && (n / 6 == s / 6)) {
        return false;
    }
    return true;
}

