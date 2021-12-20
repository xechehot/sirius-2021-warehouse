#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <random>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

using namespace std;
mt19937 rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count());

long double rnd2() {
    return (long double)rnd() / UINT32_MAX;
}

double GetTime() { return clock() / (double)CLOCKS_PER_SEC; };


struct position {
    int floor, block_x, block_y, row, section;
};

struct item {
    int id, cnt;

    bool operator<(const item& a) const {
        if (id == a.id) return cnt < a.cnt;
        else return id < a.id;
    }
};

struct product {
    int id;
    position p;
    int cnt;
};

int get_distance_in_one_block(position now, position next, int sections) {
    if (now.row == next.row)
        return abs(next.section - now.section);
    int res = abs(next.row - now.row);
    res += min(next.section + now.section, sections - now.section + 1 + sections - next.section + 1);
    return res;
}

int get_distance(position now, position next, int rows, int sections) {
    now.row -= now.row % 2;
    next.row -= next.row % 2;
    if (now.block_x == next.block_x && now.block_y == next.block_y)
        return get_distance_in_one_block(now, next, sections);
    if (now.block_x == next.block_x) {
        if (now.block_y > next.block_y) swap(now, next);
        int res = now.section + sections - next.section + 1;
        res += (next.block_y - now.block_y - 1) * sections + (next.block_y - now.block_y) * 2;
        res += abs(next.row - now.row);
        return res;
    }
    else if (now.block_y == next.block_y) {
        if (now.block_x > next.block_x) swap(now, next);
        int res = rows - now.block_x + next.block_x;
        res += (next.block_x - now.block_x - 1) * rows + 2 * (next.block_x - now.block_x);
        res += min(next.section + now.section, sections - now.section + 1 + sections - next.section + 1);
        return res;
    }
    else {
        int res = 0;
        if (now.block_x < next.block_x) {
            res = (rows - now.row) + next.row;
            res += (next.block_x - now.block_x - 1) * rows + (next.block_x - now.block_x) * 2;
        }
        else {
            res = (rows - next.row) + now.row;
            res += (now.block_x - next.block_x - 1) * rows + (now.block_x - next.block_x) * 2;
        }

        if (now.block_y < next.block_y) {
            res += now.section + sections - next.section + 1;
            res += (next.block_y - now.block_y - 1) * sections + (next.block_y - now.block_y) * 2;
        }
        else {
            res += next.section + sections - now.section + 1;
            res += (now.block_y - next.block_y - 1) * sections + (now.block_y - next.block_y) * 2;
        }

        return res;
    }
}

const int FLOORS = 5, BLOCK_X = 5, BLOCK_Y = 5, ROWS = 20, SECTIONS = 20, MAX_CAPACITY = 32;

//map<int, int> warehouse[FLOORS][BLOCK_X][BLOCK_Y][ROWS][SECTIONS];
//
//void fill_warehouse(vector<product> &products, int floors, int block_x, int block_y, int rows, int sections) {
//    for (int i = 0; i < products.size(); i++)
//        warehouse[products[i].p.floor][products[i].p.block_x][products[i].p.block_y][products[i].p.row][products[i].p.section][products[i].id] = products[i].cnt;
//}

int calc(vector<int>& permutation, vector<item>& items, map<int, position>& item_position, int rows, int sections,
    int block_x, int block_y) {
    position start = { -1, block_x / 2, block_y, rows, 1 };
    position last = start;
    int res = 0, cur_capacity = MAX_CAPACITY;
    for (int i = 0; i < items.size(); i++) {
        position cur = item_position[items[permutation[i]].id];
        res += get_distance(last, cur, rows, sections);
        int rem = items[permutation[i]].cnt;
        while (rem > 0) {
            int mi = min(rem, cur_capacity);
            rem -= mi;
            cur_capacity -= mi;
            if (rem == 0) break;
            if (cur_capacity == 0) {
                position up = cur;
                up.block_y = 1;
                up.section = sections;
                res += get_distance(cur, up, rows, sections) + 2;
                res += get_distance(up, start, rows, sections) + 2;
                res += get_distance(cur, start, rows, sections) + 2;
                cur_capacity = MAX_CAPACITY;
            }
        }
        if (cur_capacity == 0) {
            position up = cur;
            up.block_y = 1;
            up.section = sections;
            res += get_distance(cur, up, rows, sections) + 2;
            res += get_distance(up, start, rows, sections) + 2;
            cur_capacity = MAX_CAPACITY;
            last = start;
        }
        else last = cur;
    }
    if (cur_capacity != MAX_CAPACITY) {
        position up = last;
        up.block_y = 1;
        up.section = sections;
        res += get_distance(last, up, rows, sections) + 2;
        res += get_distance(up, start, rows, sections) + 2;
    }
    return res;
}


pair<int, vector<int>>
find_best_path(vector<item>& order, map<int, position>& item_position, int rows, int sections, int block_x,
    int block_y) {
    if (order.empty()) return { 0, {} };
    vector<int> permutation(order.size());
    iota(permutation.begin(), permutation.end(), 0);
    shuffle(permutation.begin(), permutation.end(),
        mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    long double t = 1;
    int ans = calc(permutation, order, item_position, rows, sections, block_x, block_y);
    int bestans = ans;
    vector<int> bestperm = permutation;
    for (int i = 0; i < 100000; i++) {
        t *= 0.999;
        int a = rnd() % permutation.size();
        int b = rnd() % permutation.size();
        swap(permutation[a], permutation[b]);
        bool ok = false;
        int cur = calc(permutation, order, item_position, rows, sections, block_x, block_y);
        if (cur < ans || rnd2() < exp((long double)(ans - cur) / t)) {
            if (cur < bestans) {
                bestans = cur;
                bestperm = permutation;
            }
            ans = cur;
            ok = true;
        }
        if (!ok) swap(permutation[a], permutation[b]);
    }
    return { bestans, bestperm };
}


void parse(vector<item>& items) {
    sort(items.begin(), items.end());
    vector<item> items2;
    for (int i = 0; i < items.size(); i++) {
        int cnt = 0, j = i;
        while (j < items.size() && items[j].id == items[i].id) {
            cnt += items[j].cnt;
            j++;
        }
        items2.push_back({ items[i].id, cnt });
        i = j - 1;
    }
    items = items2;
}


signed main() {

    ifstream in;  // Поток in будем использовать для чтения
    ofstream out; // Поток out будем использовать для записи

    in.open("/Users/user/Desktop/Project/sirius-2021-warehouse/data_sample/sample.json");
    out.open("/Users/user/Desktop/Project/sirius-2021-warehouse/data_sample/out.txt");
    json data;
    in >> data;
    int batch_size = data["batch_size"];
    int floors = data["warehouse"]["meta"]["floors"];
    int block_x = data["warehouse"]["meta"]["block_x"];
    int block_y = data["warehouse"]["meta"]["block_y"];
    int rows = data["warehouse"]["meta"]["rows"];
    int sections = data["warehouse"]["meta"]["sections"];
    map<int, position> item_position;

    vector<vector<item>> orders(data["orders"].size());
    for (int i = 0; i < orders.size(); i++)
        for (int j = 0; j < data["orders"][i]["items"].size(); j++)
            orders[i].push_back({ data["orders"][i]["items"][j]["id"], data["orders"][i]["items"][j]["count"] });

    vector<product> products(data["warehouse"]["stock"].size());
    for (int i = 0; i < products.size(); i++) {
        products[i].id = data["warehouse"]["stock"][i]["id"];
        products[i].p.floor = data["warehouse"]["stock"][i]["p"]["floor"];
        products[i].p.block_x = data["warehouse"]["stock"][i]["p"]["block_x"];
        products[i].p.block_y = data["warehouse"]["stock"][i]["p"]["block_y"];
        products[i].p.row = data["warehouse"]["stock"][i]["p"]["row"];
        products[i].p.section = data["warehouse"]["stock"][i]["p"]["section"];
        products[i].cnt = data["warehouse"]["stock"][i]["count"];
        item_position[products[i].id] = products[i].p;
    }

    //    fill_warehouse(products, floors, block_x, block_y, rows, sections);
    int all_result = 0;
    for (int i = 0; i < orders.size(); i += batch_size) {
        vector<vector<item>> order(floors + 1);
        for (int j = i; j < min((int)orders.size(), i + batch_size); j++)
            for (auto [id, cnt] : orders[j])
                order[item_position[id].floor].push_back({ id, cnt });
        for (int floor = 1; floor <= floors; floor++) {
            parse(order[floor]);
            pair<int, vector<int>> cres = find_best_path(order[floor], item_position, rows, sections, block_x, block_y);
            all_result += cres.first;
            // printing path
//            cout << cres.first << '\n';
//            for (auto j: cres.second) cout << order[floor][j].id << ' ' << order[floor][j].cnt << '\n';
//            cout << "\n\n";
        }
    }

    out << all_result;
}
/*
Утверждается, что товар лежит только в 1 определенном месте
Утверждается, что товара хватает на все заказы
Не оптимальное разбиение заказов на батчи
Утверждается что заказы собираются на разных этажах независимо
 */
