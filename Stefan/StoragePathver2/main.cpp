#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <nlohmann/json.hpp>


using json = nlohmann::json;
#define errv(x) {cerr << "["#x"]  ["; for (const auto& ___ : (x)) cerr << ___ << ", "; cerr << "]\n";}

using namespace std;
mt19937 rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count());

long double rnd2() {
    return (long double) rnd() / UINT32_MAX;
}

double GetTime() { return clock() / (double) CLOCKS_PER_SEC; };


struct position {
    int floor = 0, block_x = 0, block_y = 0, row = 0, section = 0;

    bool operator==(const position &a) const {
        return floor == a.floor && block_x == a.block_x && block_y == a.block_y && row == a.row && section == a.section;
    }

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

    bool operator==(const product &a) const {
        return id == a.id && p == a.p && cnt == a.cnt;
    }

    bool operator<(const product &a) const {
        if (cnt == a.cnt) {
            if (id == a.id)
                return p < a.p;
            else return id < a.id;
        } else return cnt > a.cnt;
    }
};


const int MAX_CAPACITY = 32;


int dist_to_start(position self, position place) {
    int ans = 0;
    int dy = int((double) fabs((double) self.block_y / 2 + (double) 0.5 - (double) place.block_y) + (double) 0.5) - 1;
    ans += dy * (self.row + 2);  // расстояние по горизонтали к центру
    ans += (self.block_x - place.block_x) * (self.section + 2); // расстояние по вертикали к низу
    if (place.block_y <= self.block_y / 2)
        ans += (self.row - place.row);  // расстояние внутри блока
    else
        ans += place.row;
    ans += (self.section + 1 - place.section);
    return ans;
}

int dist_to_tape(position self, position place) {
    int ans = place.section + (place.block_x - 1) * self.section;
    return ans;
}

int get_dist_2_places(position self, position p1, position p2) {
    int ans = 0;
    if (p1.block_x != p2.block_x || p1.block_y != p2.block_y || p1.row != p2.row) {
        if (p1.block_x > p2.block_x)
            swap(p1, p2);
        if (p1.block_x != p2.block_x)
            ans += (p2.block_x - p1.block_x - 1) * self.section;
        ans += (p2.block_x - p1.block_x) * 2 - 1;
        ans += min(p1.section + p2.section, 2 * self.section - p1.section - p2.section + 2);
        if (p1.block_y > p2.block_y) swap(p1, p2);

        ans += (self.row - p1.row + 1) + p2.row;
        if (p1.block_y != p2.block_y)
            ans += (p2.block_y - p1.block_y - 1) * self.row;
        ans += (p2.block_y - p1.block_y) * 2 - 1;
        return ans;
    } else return abs(p1.section - p2.section);
}

int calc(vector<int> &permutation, vector<product> &items, int rows, int sections,
         int block_x, int block_y) {
    if (permutation.empty()) return 0;
    int res = 0;
    int i = 0;
    position self = {-1, block_x, block_y, rows, sections};
    while (i < items.size() && items[permutation[i]].cnt == 0) i++;
    if (i == items.size()) return 0;
    res += dist_to_start(self, items[permutation[i]].p);
    position last = items[permutation[i]].p;
    i++;
    for (; i < items.size(); i++) {
        int j = i;
        while (j < items.size() && items[permutation[j]].cnt == 0) j++;
        i = j;
        if (i == items.size()) break;
        position cur = items[permutation[i]].p;
        res += get_dist_2_places(self, last, cur);
        last = cur;
    }
    res += dist_to_tape(self, last) * 2;
    res += dist_to_start(self, last);
    return res;
}

void print(product a) {
    cout << "id: " << a.id << '\n';
    cout << "floor: " << a.p.floor << '\n';
    cout << "block_x: " << a.p.block_y << '\n';
    cout << "block_y: " << a.p.block_x << '\n';
    cout << "row: " << a.p.row << '\n';
    cout << "section: " << a.p.section << '\n';
    cout << "count: " << a.cnt << '\n';
    cout << "\n\n\n";
}

pair<int, int> mutation_swapping_path(vector<int> &permutation) {
    int a = rnd() % permutation.size();
    int b = rnd() % permutation.size();
    swap(permutation[a], permutation[b]);
    return {a, b};
}

pair<int, int> mutation_flipping_path(vector<int> &permutation) {
    int a = rnd() % permutation.size();
    int b = rnd() % permutation.size();
    if (a > b) swap(a, b);
    reverse(permutation.begin() + a, permutation.begin() + b + 1);
    return {a, b};
}

struct changing_cell {
    int chosen;
    product rep, cur;
};

changing_cell mutation_changing_cell(vector<product> &bucket, map<int, vector<product>> &product_position,
                                     map<position, int> &used) {
    int chosen_cell = rnd() % bucket.size();
    product cur = bucket[chosen_cell];
    vector<product> candidat;
    for (product j: product_position[cur.id])
        if (j.p.floor == cur.p.floor && j.p != cur.p && j.cnt - used[j.p] >= cur.cnt)
            candidat.push_back(j);
    product useless = {-1, {-1, -1, -1, -1, -1}, -1};
    if (candidat.empty()) return {-1, useless, useless};
    product chosen = candidat[rnd() % candidat.size()];
    product rep = cur;
    used[cur.p] -= cur.cnt;
    used[chosen.p] += cur.cnt;
    cur.id = chosen.id;
    cur.p = chosen.p;
    return {chosen_cell, rep, cur};
}

struct changing_bucket {
    int up, pr, chosen;
};


changing_bucket mutation_changing_bucket(vector<vector<product>> &bucket, vector<int> &bucket_capacity,
                                         vector<vector<int>> &permutations, int chosen) {
    if (bucket[chosen].empty()) return {-1, -1, -1};
    int pr = -1;
    for (int iter = 0; iter < 1; iter++) {
        int pr2 = (int) bucket[chosen].size() - 1;
        if (bucket[chosen][pr2].cnt > 0) {
            pr = pr2;
            break;
        }
    }
    if (pr == -1) return {-1, -1, -1};
    for (int iter = 0; iter < 100; iter++) {
        int x2 = rnd() % (int) bucket.size();
        if (chosen == x2) continue;
        if (bucket_capacity[x2] + bucket[chosen][pr].cnt <= MAX_CAPACITY) {
            bucket_capacity[x2] += bucket[chosen][pr].cnt;
            bucket_capacity[chosen] -= bucket[chosen][pr].cnt;

            bucket[x2].push_back(bucket[chosen][pr]);
            bucket[chosen].erase(bucket[chosen].begin() + pr);

            int up = permutations[chosen][pr];
            permutations[x2].push_back((int) permutations[x2].size());
            permutations[chosen].erase(permutations[chosen].begin() + pr);

            for (auto &j: permutations[chosen]) {
                assert(j != up);
                if (j > up)
                    j--;
            }

            return {up, pr, x2};
        }
    }
    return {-1, -1, -1};
}

struct swapping_batch {
    int chosen, chosen2, pr, pr2;
};

swapping_batch
mutation_swapping_batch(vector<vector<vector<product>>> &batches, int chosen) {
    if (batches[chosen].empty()) return {-1, -1, -1, -1};
    int chosen2 = -1;
    for (int iter = 0; iter < 100; iter++) {
        int cur_chosen2 = rnd() % (int) batches.size();
        if (cur_chosen2 != chosen) {
            chosen2 = cur_chosen2;
            break;
        }
    }
    if (chosen2 == -1) return {-1, -1, -1, -1};
    int pr = rnd() % ((int) batches[chosen].size());
    int pr2 = rnd() % ((int) batches[chosen2].size());
    swap(batches[chosen][pr], batches[chosen2][pr2]);
    return {chosen, chosen2, pr, pr2};
}

struct merge_bucket {
    int chosen, cut, capacity;
};


merge_bucket
mutation_merging_bucket(vector<vector<product>> &bucket, vector<vector<int>> &permutations,
                        vector<int> &bucket_capacity,
                        int chosen) {
    int chosen2 = -1;
    for (int iter = 0; iter < 50; iter++) {
        int try_chose = rnd() % (int) bucket.size();
        if (try_chose == chosen || bucket_capacity[try_chose] == 0) continue;
        if (bucket_capacity[chosen] + bucket_capacity[try_chose] <= MAX_CAPACITY) {
            chosen2 = try_chose;
            break;
        }
    }
    if (chosen2 == -1) return {-1, -1, -1};
    merge_bucket res = {chosen2, (int) bucket[chosen2].size(), bucket_capacity[chosen]};
    for (int i = 0; i < bucket[chosen].size(); i++) {
        bucket[chosen2].push_back(bucket[chosen][i]);
        permutations[chosen2].push_back(permutations[chosen][i] + res.cut);
    }
    bucket_capacity[chosen2] += bucket_capacity[chosen];
    bucket_capacity[chosen] = 0;
    bucket[chosen].clear();
    permutations[chosen].clear();
    return res;
}


struct transfer {
    int chosen, chosen2, taken;
};

transfer
mutation_transfering_bucket(vector<product> &bucket, map<position, int> &used, map<position, int> &product_cnt) {
    if (bucket.empty()) return {-1, -1, -1};
    vector<int> candidats;
    int chosen = -1;
    for (int iter = 0; iter < 50; iter++) {
        int cur_chosen = rnd() % (int) bucket.size();
        if (bucket[cur_chosen].cnt != 0) {
            chosen = cur_chosen;
            break;
        }
    }
    if (chosen == -1)
        return {-1, -1, -1};
    for (int i = 0; i < bucket.size(); i++) {
        if (i == chosen) continue;
        if (bucket[i].id != bucket[chosen].id || product_cnt[bucket[i].p] - used[bucket[i].p] == 0) continue;
        candidats.push_back(i);
    }
    if (candidats.empty()) return {-1, -1, -1};
    int chosen2 = candidats[rnd() % (int) candidats.size()];
    int mx = min(bucket[chosen].cnt, product_cnt[bucket[chosen2].p] - used[bucket[chosen2].p]);
    mx = min(mx, 4);
    assert(mx > 0);
    int taken = rnd() % mx + 1;
    bucket[chosen].cnt -= taken;
    bucket[chosen2].cnt += taken;
    used[bucket[chosen].p] -= taken;
    used[bucket[chosen2].p] += taken;
    assert(bucket[chosen].cnt >= 0);
    assert(bucket[chosen2].cnt >= 0);
    transfer res = {chosen, chosen2, taken};
    return res;
}


json data_res;
map<position, int> fill_up;

void print_bucket2(vector<vector<product>> &bucket, vector<vector<int>> &bestpermutations) {
    for (int i = 0; i < bucket.size(); i++) {
        for (int j = 0; j < bucket[i].size(); j++) {
            print(bucket[i][bestpermutations[i][j]]);
        }
        cout << "------------------------------\n";
    }
}

int RES = 0;

void print_bucket(vector<vector<product>> &bucket, vector<vector<int>> &bestpermutations) {
    json basket;
    basket["buckets"] = {};
    for (int i = 0; i < bucket.size(); i++) {
        json items;
        items["items"] = {};
        for (int j = 0; j < bucket[i].size(); j++) {
            assert(bestpermutations[i].size() == bucket[i].size());
            RES += bucket[i][bestpermutations[i][j]].cnt;
            assert(bucket[i][bestpermutations[i][j]].cnt >= 0);
            fill_up[bucket[i][bestpermutations[i][j]].p] += bucket[i][bestpermutations[i][j]].cnt;
            if (bucket[i][bestpermutations[i][j]].cnt == 0) continue;
            json cur = {
                    {"id",      bucket[i][bestpermutations[i][j]].id},
                    {"floor",   bucket[i][bestpermutations[i][j]].p.floor},
                    {"block_x", bucket[i][bestpermutations[i][j]].p.block_x},
                    {"block_y", bucket[i][bestpermutations[i][j]].p.block_y},
                    {"row",     bucket[i][bestpermutations[i][j]].p.row},
                    {"section", bucket[i][bestpermutations[i][j]].p.section},
                    {"cnt",     bucket[i][bestpermutations[i][j]].cnt}};
            items["items"].push_back(cur);
        }
        if (!items.empty())
            basket["buckets"].push_back(items);
        //        cout << "------------------------------\n";
    }
    if (!basket.empty())
        data_res["batches"].push_back(basket);
}


int
solve_batch(vector<vector<product>> bucket, map<int, vector<product>> &product_position,
            map<position, int> &used,
            map<position, int> &product_cnt, int floors,
            int rows, int sections, int block_x,
            int block_y, int type) {

    vector<int> bucket_capacity(bucket.size());
    for (int i = 0; i < bucket.size(); i++)
        for (auto j: bucket[i])
            bucket_capacity[i] += j.cnt;

    vector<vector<int>> permutations(bucket.size());
    for (int i = 0; i < bucket.size(); i++) {
        permutations[i].resize(bucket[i].size());
        iota(permutations[i].begin(), permutations[i].end(), 0);
    }

    vector<vector<int>> bestpermutations = permutations;
    vector<vector<product>> bestbucket = bucket;

    vector<int> answers(bucket.size());
    for (int i = 0; i < bucket.size(); i++)
        answers[i] = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);

    long double t = 0.01, t2 = 0.2, t3 = 0.999, t4 = 0.19, t5 = 0.92342, t6 = 1.99900012;
    int sumres = 0, sumbestres = 0;
    for (auto i: answers) sumres += i, sumbestres += i;
    for (int zigzig = 0; zigzig < 5000 * 20; zigzig++) {
        t *= 0.9696;
        t2 *= 0.9696;
        t3 *= 0.9596;
        t4 *= 0.9996;
        t5 *= 0.9696;
        t6 *= 0.9696;


        // перекладывание товаров из ячейки в ячейку
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket[i].empty()) continue;
            transfer trans = mutation_transfering_bucket(bucket[i], used, product_cnt);
            if (trans.chosen == -1) continue;
            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
                sumres -= answers[i];
                sumres += cur;
                answers[i] = cur;
                if (sumres < sumbestres) {
                    sumbestres = sumres;
                    bestpermutations = permutations;
                    bestbucket = bucket;
                }
            } else {
                bucket[i][trans.chosen].cnt += trans.taken;
                bucket[i][trans.chosen2].cnt -= trans.taken;
                used[bucket[i][trans.chosen].p] += trans.taken;
                used[bucket[i][trans.chosen2].p] -= trans.taken;
            }
        }

        // свап ячеек на пути
        for (int i = 0; i < bucket.size(); i++) {
            if (permutations[i].empty()) continue;
            pair<int, int> swapping = mutation_swapping_path(permutations[i]);

            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t2)) {
                sumres -= answers[i];
                sumres += cur;
                answers[i] = cur;
                if (sumres < sumbestres) {
                    sumbestres = sumres;
                    bestpermutations = permutations;
                    bestbucket = bucket;
                }
            } else swap(permutations[i][swapping.first], permutations[i][swapping.second]);
        }

        // реверс отрезка пути
        for (int i = 0; i < bucket.size(); i++) {
            if (permutations[i].empty()) continue;

            pair<int, int> flipping = mutation_flipping_path(permutations[i]);

            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t3)) {
                sumres -= answers[i];
                sumres += cur;
                answers[i] = cur;
                if (sumres < sumbestres) {
                    sumbestres = sumres;
                    bestpermutations = permutations;
                    bestbucket = bucket;
                }
            } else reverse(permutations[i].begin() + flipping.first, permutations[i].begin() + flipping.second + 1);

        }
        // полное перекладывание товара из 1 ячейки в другую
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket[i].empty()) continue;
            auto changing = mutation_changing_cell(bucket[i], product_position, used);
            if (changing.chosen == -1) continue;
            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t4)) {
                sumres -= answers[i];
                sumres += cur;
                answers[i] = cur;
                if (sumres < sumbestres) {
                    sumbestres = sumres;
                    bestpermutations = permutations;
                    bestbucket = bucket;
                }
            } else {
                used[changing.rep.p] -= changing.rep.cnt;
                used[changing.cur.p] += changing.cur.cnt;
                bucket[i][changing.chosen] = changing.rep;
            }
        }


        // перекладывание товара в другую корзину
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket[i].empty()) continue;
            changing_bucket changing = mutation_changing_bucket(bucket, bucket_capacity, permutations, i);
            if (changing.up == -1) continue;
            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            int cur2 = calc(permutations[changing.chosen], bucket[changing.chosen], rows, sections, block_x, block_y);
            if ((cur + cur2 < answers[i] + answers[changing.chosen] ||
                 rnd2() < exp((long double) (answers[i] + answers[changing.chosen] - cur - cur2) / t5))) {
                sumres -= answers[i];
                sumres -= answers[changing.chosen];
                sumres += cur;
                sumres += cur2;
                answers[i] = cur;
                answers[changing.chosen] = cur2;
                if (sumres < sumbestres) {
                    sumbestres = sumres;
                    bestpermutations = permutations;
                    bestbucket = bucket;
                }
            } else {
                product was_erased = bucket[changing.chosen].back();
                bucket[changing.chosen].pop_back();
                permutations[changing.chosen].pop_back();
                bucket_capacity[i] += was_erased.cnt;
                bucket_capacity[changing.chosen] -= was_erased.cnt;
                for (auto &j: permutations[i])
                    if (j >= changing.up)
                        j++;
                permutations[i].insert(permutations[i].begin() + changing.pr, changing.up);
                bucket[i].insert(bucket[i].begin() + changing.pr, was_erased);
            }
        }

        // соединение двух корзин
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket[i].empty()) continue;
            merge_bucket merged = mutation_merging_bucket(bucket, permutations, bucket_capacity, i);
            if (merged.chosen == -1) continue;
            int cur = calc(permutations[merged.chosen], bucket[merged.chosen], rows, sections, block_x, block_y);

            if (cur < answers[i] + answers[merged.chosen] ||
                rnd2() < exp((long double) (answers[i] + answers[merged.chosen] - cur) / t6)) {
                sumres -= answers[i];
                sumres -= answers[merged.chosen];
                sumres += cur;
                sumres += 0;
                answers[merged.chosen] = cur;
                answers[i] = 0;
                if (sumres < sumbestres) {
                    sumbestres = sumres;
                    bestpermutations = permutations;
                    bestbucket = bucket;
                }
            } else {
                int len = 0;
                for (int j = merged.cut; j < bucket[merged.chosen].size(); j++) {
                    len++;
                    bucket[i].push_back(bucket[merged.chosen][j]);
                    permutations[i].push_back(permutations[merged.chosen][j] - merged.cut);
                }
                for (int j = 0; j < len; j++) {
                    bucket[merged.chosen].pop_back();
                    permutations[merged.chosen].pop_back();
                }
                bucket_capacity[merged.chosen] -= merged.capacity;
                bucket_capacity[i] += merged.capacity;
                reverse(bucket[i].begin(), bucket[i].end());
                reverse(permutations[i].begin(), permutations[i].end());
            }
        }

    }
    if (type == 2) print_bucket(bestbucket, bestpermutations);
    return sumbestres;
}

signed main() {
    freopen("../../../data_sample/sample_64_1.json", "r", stdin);
    freopen("../../../data_sample_output/output_64_1_stepan.json", "w", stdout);
    json data;
    cin >> data;
    freopen("../../../data_sample_output/output_64_1_Maksim.json", "r", stdin);
    json data2;
    cin >> data2;
    int batch_size = data["batch_size"];
    int floors = data["warehouse"]["meta"]["floors"];
    int block_x = data["warehouse"]["meta"]["block_x"];
    int block_y = data["warehouse"]["meta"]["block_y"];
    int rows = data["warehouse"]["meta"]["rows"];
    int sections = data["warehouse"]["meta"]["sections"];
    map<int, vector<product>> product_position;
    map<position, int> product_cnt;
    map<position, int> used;
    vector<vector<item>> orders(data["orders"].size());
    int cntt = 0;
    for (int i = 0; i < orders.size(); i++) {
        for (int j = 0; j < data["orders"][i]["items"].size(); j++) {
            orders[i].push_back({data["orders"][i]["items"][j]["id"], data["orders"][i]["items"][j]["count"]});
            cntt += (int) data["orders"][i]["items"][j]["count"];
        }
    }
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
        product_cnt[products[i].p] = products[i].cnt;
    }
    for (auto &i: product_position)
        sort(i.second.begin(), i.second.end());
//    cout << data2;
    vector<vector<vector<product>>> cells;
    vector<int> answers;
    for (auto &i: data2["batches"]) {
        vector<vector<product>> cell;
        for (auto &j: i["buckets"]) {
            vector<product> bucket;
            for (auto &k: j["items"]) {
                product cur;
                cur.id = k["id"];
                cur.cnt = k["cnt"];
                cur.p.floor = k["floor"];
                cur.p.block_x = k["block_x"];
                cur.p.block_y = k["block_y"];
                cur.p.row = k["row"];
                cur.p.section = k["section"];
                bucket.push_back(cur);
                used[cur.p] += cur.cnt;
                RES += cur.cnt;
            }
            cell.push_back(bucket);
        }
        cells.push_back(cell);
    }

    data_res["batches"] = {};
    int res = 0;
    for (int j = 0; j < 1; j++) {
        fill_up.clear();
        for (int i = 0; i < cells.size(); i++)
            res += solve_batch(cells[i], product_position, used, product_cnt, floors, rows, sections, block_x, block_y, 2);
        for (auto i: products) assert(fill_up[i.p] <= i.cnt);
    }
    cout << (long double) res / cntt << '\n';
    cout << data_res;

}

