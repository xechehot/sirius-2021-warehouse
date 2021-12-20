#include <bits/stdc++.h>
#include <bits/extc++.h>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

using namespace std;
mt19937 rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count());

long double rnd2() {
    return (long double) rnd() / UINT32_MAX;
}

double GetTime() { return clock() / (double) CLOCKS_PER_SEC; };


struct position {
    int floor = 0, block_x = 0, block_y = 0, row = 0, section = 0;
    bool operator<(const position &a) const {
        if (floor == a.floor) {
            if (block_x == a.block_x) {
                if (block_y == a.block_y) {
                    if (row == a.row) {
                        return section < a.section;
                    } else return row < a.row;
                } else return block_y < a.block_y;
            } else return block_x < a.block_x;
        } else return floor < a.floor;
    }
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
        if (cnt == a.cnt) {
            if (id == a.id)
                return p < a.p;
            else return id < a.id;
        } else return cnt > a.cnt;
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

void mutation_swapping_path(vector<vector<int>> &permutations, vector<vector<product>> &bucket, vector<int> &answers,
                            vector<int> &bestanswers, vector<vector<int>> &bestpermutations, long double t, int rows,
                            int sections,
                            int block_x, int block_y) {
    for (int i = 0; i < permutations.size(); i++) {
        int a = rnd() % permutations[i].size();
        int b = rnd() % permutations[i].size();
        if (a == b) continue;
        swap(permutations[i][a], permutations[i][b]);
        bool ok = false;
        int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
        if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
            if (cur < bestanswers[i]) {
                bestanswers[i] = cur;
                bestpermutations[i] = permutations[i];
            }
            answers[i] = cur;
            ok = true;
        }
        if (!ok) swap(permutations[i][a], permutations[i][b]);
    }
}

void mutation_reversing_path(vector<vector<int>> &permutations, vector<vector<product>> &bucket, vector<int> &answers,
                             vector<int> &bestanswers, vector<vector<int>> &bestpermutations, long double t, int rows,
                             int sections,
                             int block_x, int block_y) {
    for (int i = 0; i < permutations.size(); i++) {
        int a = rnd() % permutations[i].size();
        int b = rnd() % permutations[i].size();
        if (a == b) continue;
        if (a > b) swap(a, b);
        reverse(permutations[i].begin() + a, permutations[i].begin() + b + 1);
        bool ok = false;
        int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
        if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
            if (cur < bestanswers[i]) {
                bestanswers[i] = cur;
                bestpermutations[i] = permutations[i];
            }
            answers[i] = cur;
            ok = true;
        }
        if (!ok) reverse(permutations[i].begin() + a, permutations[i].begin() + b + 1);

    }
}

void mutation_cutting_merging_buckets(vector<vector<int>> &permutations, vector<vector<product>> &bucket,
                                      vector<int> &answers,
                                      vector<int> &bestanswers, vector<vector<int>> &bestpermutations, long double t,
                                      int rows,
                                      int sections,
                                      int block_x, int block_y) {

}


void mutation_5(vector<vector<product>> &bucket, map<int, vector<product>> &product_position, map<product, int> &used) {
    assert(!bucket.empty());
    int chosen_bucket = rnd() % bucket.size();
    int chosen_cell = rnd() % bucket[chosen_bucket].size();
    swap(bucket[chosen_bucket][chosen_cell], bucket[chosen_bucket].back());
    vector<product> candidat;
    int sum = 0;
    product cur = bucket[chosen_bucket].back();
//    for (product j: product_position[cur.id]) {
//        if (j.p.floor == cur.p.floor(j.id != cur.id || j.p != cur.p || j.cnt != cur.cnt)) {
//            candidat.push_back(j);
//        }
//    }
}


void print_bucket(vector<vector<product>> &bucket, vector<vector<int>> &bestpermutations, int res) {
    cout << res << "\n";
    for (int i = 0; i < bucket.size(); i++) {
        for (int j = 0; j < bucket[i].size(); j++)
            print(bucket[i][bestpermutations[i][j]]);
        cout << "------------------------------\n";
    }
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
    vector<vector<int>> permutations(bucket.size());
    vector<vector<int>> bestpermutations = permutations;
    for (int i = 0; i < bucket.size(); i++) {
        permutations[i].resize(bucket[i].size());
        iota(permutations[i].begin(), permutations[i].end(), 0);
    }

    vector<int> answers(bucket.size()), bestanswers;
    for (int i = 0; i < bucket.size(); i++)
        answers[i] = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);

    bestanswers = answers;
    long double t = 1;
    for (int zigzig = 0; zigzig < 10000; zigzig++) {
        t *= 0.999;
        mutation_reversing_path(permutations, bucket, answers, bestanswers, bestpermutations, t, rows, sections,
                                block_x,
                                block_y);

        mutation_swapping_path(permutations, bucket, answers, bestanswers, bestpermutations, t, rows, sections, block_x,
                               block_y);
    }
    int res = 0;
    for (int i = 0; i < bucket.size(); i++) res += bestanswers[i];
    //print_bucket(bucket, bestpermutations, res);
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
    freopen("../../../data_sample/sample_1600_1.json", "r", stdin);
    json data;
    cin >> data;
    int batch_size = data["batch_size"];
    int floors = data["warehouse"]["meta"]["floors"];
    int block_x = data["warehouse"]["meta"]["block_x"];
    int block_y = data["warehouse"]["meta"]["block_y"];
    int rows = data["warehouse"]["meta"]["rows"];
    int sections = data["warehouse"]["meta"]["sections"];
    map<int, vector<product>> product_position;
    map<position, int> used;
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
            for (auto &j: product_position[id]) {
                if (rem > 0 && j.cnt - used[j.p] > 0) {
                    int mi = min(rem, j.cnt - used[j.p]);
                    rem -= j.cnt;
                    auto need = j;
                    need.cnt = mi;
                    cells.push_back(need);
                    used[j.p] += mi;
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
