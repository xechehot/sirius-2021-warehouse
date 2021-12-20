#include <iostream>
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
    return (long double) rnd() / UINT32_MAX;
}

double GetTime() { return clock() / (double) CLOCKS_PER_SEC; };


struct position {
    int floor, block_x, block_y, row, section;
};

struct item {
    int id, cnt;

    bool operator<(const item &a) const {
        if (id == a.id) return cnt < a.cnt;
        else return id < a.id;
    }
};

struct product {
    int id;
    position p;
    int cnt;

    bool operator<(const product &a) const {
        return cnt > a.cnt;
    }
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
    } else if (now.block_y == next.block_y) {
        if (now.block_x > next.block_x) swap(now, next);
        int res = rows - now.block_x + next.block_x;
        res += (next.block_x - now.block_x - 1) * rows + 2 * (next.block_x - now.block_x);
        res += min(next.section + now.section, sections - now.section + 1 + sections - next.section + 1);
        return res;
    } else {
        int res = 0;
        if (now.block_x < next.block_x) {
            res = (rows - now.row) + next.row;
            res += (next.block_x - now.block_x - 1) * rows + (next.block_x - now.block_x) * 2;
        } else {
            res = (rows - next.row) + now.row;
            res += (now.block_x - next.block_x - 1) * rows + (now.block_x - next.block_x) * 2;
        }

        if (now.block_y < next.block_y) {
            res += now.section + sections - next.section + 1;
            res += (next.block_y - now.block_y - 1) * sections + (next.block_y - now.block_y) * 2;
        } else {
            res += next.section + sections - now.section + 1;
            res += (now.block_y - next.block_y - 1) * sections + (now.block_y - next.block_y) * 2;
        }

        return res;
    }
}

const int FLOORS = 5, BLOCK_X = 5, BLOCK_Y = 5, ROWS = 20, SECTIONS = 20, MAX_CAPACITY = 32;

int calc(vector<int> &permutation, vector<product> &items, int rows, int sections,
         int block_x, int block_y) {
    if (permutation.empty()) return 0;
    position start = {-1, block_x / 2, block_y, rows, 1};
    position last = start;
    int res = 0;
    for (int i = 0; i < items.size(); i++) {
        position cur = items[permutation[i]].p;
        res += get_distance(last, cur, rows, sections);
        last = cur;
    }
    position up = last;
    up.block_y = 1;
    up.section = sections;
    res += get_distance(last, up, rows, sections) + 2;
    res += get_distance(up, start, rows, sections);
    return res;
}

void print(product a) {
    cout << "id: " << a.id << '\n';
    cout << "floor: " << a.p.floor << '\n';
    cout << "block_x: " << a.p.block_x << '\n';
    cout << "block_y: " << a.p.block_y << '\n';
    cout << "row: " << a.p.row << '\n';
    cout << "section: " << a.p.section << '\n';
    cout << "count: " << a.cnt << '\n';
    cout << "\n\n\n";
}

int solve_batch(vector<product> &cells2, int floors, int rows, int sections, int block_x,
                int block_y) {
    if (cells2.empty()) return 0;
    vector<vector<product>> cells(floors + 1);
    for (auto i: cells2) cells[i.p.floor].push_back(i);
    vector<vector<product>> bucket;
    for (int i = 1; i <= floors; i++) {
        vector<product> cur_bucket;
        int cur_capacity = MAX_CAPACITY;
        for (int j = 0; j < cells[i].size(); j++) {
            if (cur_capacity >= cells[i][j].cnt)
                cur_bucket.push_back(cells[i][j]);
            else {
                bucket.push_back(cur_bucket);
                cur_bucket = {cells[i][j]};
                cur_capacity = MAX_CAPACITY - cells[i][j].cnt;
            }
        }
        if (!cur_bucket.empty()) bucket.push_back(cur_bucket);
    }
    int res = 0;
    for (auto &cur_bucket: bucket) {
        vector<int> permutation(cur_bucket.size());
        iota(permutation.begin(), permutation.end(), 0);
        for (auto j: cur_bucket)
            print(j);
        cout << "---------------------\n";
        res += calc(permutation, cur_bucket, rows, sections, block_x, block_y);
    }
    return res;
}


void parse(vector<item> &items) {
    sort(items.begin(), items.end());
    vector<item> items2;
    for (int i = 0; i < items.size(); i++) {
        int cnt = 0, j = i;
        while (j < items.size() && items[j].id == items[i].id) {
            cnt += items[j].cnt;
            j++;
        }
        while (cnt > 0) {
            int mi = min(cnt, MAX_CAPACITY);
            items2.push_back({items[i].id, mi});
            cnt -= mi;
        }
        i = j - 1;
    }
    items = items2;
}


signed main() {
    freopen("../../../data_sample/sample.json", "r", stdin);
    json data;
    cin >> data;
    int batch_size = data["batch_size"];
    int floors = data["warehouse"]["meta"]["floors"];
    int block_x = data["warehouse"]["meta"]["block_x"];
    int block_y = data["warehouse"]["meta"]["block_y"];
    int rows = data["warehouse"]["meta"]["rows"];
    int sections = data["warehouse"]["meta"]["sections"];
    map<int, vector<product>> product_position;

    vector<vector<item>> orders(data["orders"].size());
    for (int i = 0; i < orders.size(); i++)
        for (int j = 0; j < data["orders"][i]["items"].size(); j++)
            orders[i].push_back({data["orders"][i]["items"][j]["id"], data["orders"][i]["items"][j]["count"]});

    vector<product> products(data["warehouse"]["stock"].size());
    for (int i = 0; i < products.size(); i++) {
        products[i].id = data["warehouse"]["stock"][i]["id"];
        products[i].p.floor = data["warehouse"]["stock"][i]["p"]["floor"];
        products[i].p.block_x = data["warehouse"]["stock"][i]["p"]["block_x"];
        products[i].p.block_y = data["warehouse"]["stock"][i]["p"]["block_y"];
        products[i].p.row = data["warehouse"]["stock"][i]["p"]["row"];
        products[i].p.section = data["warehouse"]["stock"][i]["p"]["section"];
        products[i].cnt = data["warehouse"]["stock"][i]["count"];
        product_position[products[i].id].push_back(products[i]);
    }
    for (auto &i: product_position)
        sort(i.second.begin(), i.second.end());

    int all_result = 0;

    for (int i = 0; i < orders.size(); i += batch_size) {

        vector<item> order;
        for (int j = i; j < min((int) orders.size(), i + batch_size); j++)
            for (auto[id, cnt]: orders[j])
                order.push_back({id, cnt});
        sort(order.begin(), order.end());
        parse(order);

        vector<product> cells;
        for (auto[id, cnt]: order) {
            int rem = cnt;
            for (auto j: product_position[id]) {
                if (rem > 0) {
                    int mi = min(rem, j.cnt);
                    rem -= j.cnt;
                    auto j2 = j;
                    j2.cnt = mi;
                    cells.push_back(j2);
                }
            }
        }
        all_result += solve_batch(cells, floors, rows, sections, block_x, block_y);
    }
    cout << all_result;
}
/*
Не оптимальное разбиение заказов на батчи
 */
