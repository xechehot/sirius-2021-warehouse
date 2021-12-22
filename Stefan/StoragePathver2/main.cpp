#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <random>
#include <chrono>
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

int get_distance_in_one_block(position now, position next, int sections) {
    if (now.row == next.row)
        return abs(next.section - now.section);
    int res = abs(next.row - now.row);
    res += min(next.section + now.section, sections - now.section + sections - next.section);
    return res;
}

int get_distance(position now, position next, int rows, int sections) {
    now.row -= now.row % 2;
    next.row -= next.row % 2;
    if (now.block_x == next.block_x && now.block_y == next.block_y)
        return get_distance_in_one_block(now, next, sections);
    if (now.block_x == next.block_x) {
        if (now.block_y > next.block_y) swap(now, next);
        int res = now.section + sections - next.section;
        res += (next.block_y - now.block_y - 1) * sections + (next.block_y - now.block_y) * 2;
        res += abs(next.row - now.row);
        return res;
    } else if (now.block_y == next.block_y) {
        if (now.block_x > next.block_x) swap(now, next);
        int res = rows - now.block_x + next.block_x;
        res += (next.block_x - now.block_x - 1) * rows + 2 * (next.block_x - now.block_x);
        res += min(next.section + now.section, sections - now.section + sections - next.section);
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
            res += now.section + sections - next.section;
            res += (next.block_y - now.block_y - 1) * sections + (next.block_y - now.block_y) * 2;
        } else {
            res += next.section + sections - now.section;
            res += (now.block_y - next.block_y - 1) * sections + (now.block_y - next.block_y) * 2;
        }

        return res;
    }
}

const int MAX_CAPACITY = 32;


int calc(vector<int> &permutation, vector<product> &items, int rows, int sections,
         int block_x, int block_y) {
    if (permutation.empty()) return 0;
    position start = {-1, block_x / 2, block_y, rows, 1};
    position last = start;
    int res = 0;
    for (int i = 0; i < items.size(); i++) {
        int j = i;
        while (j < items.size() && items[permutation[j]].cnt == 0) j++;
        i = j;
        if (i == items.size()) break;
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
    int pr = rnd() % (int) bucket[chosen].size();
    for (int iter = 0; iter < 100; iter++) {
        int x2 = rnd() % (int) bucket.size();
        if (chosen == x2) continue;
        if (bucket_capacity[x2] + bucket[chosen][pr].cnt <= MAX_CAPACITY) {

            bucket_capacity[x2] += bucket[chosen][pr].cnt;
            bucket_capacity[chosen] -= bucket[chosen][pr].cnt;

            bucket[x2].push_back(bucket[chosen][pr]);
            bucket[chosen].erase(bucket[chosen].begin() + pr);
            int up = permutations[chosen][pr];
            permutations[x2].push_back(permutations[x2].size());
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
mutation_swapping_batch(vector<vector<product>> &batch, int chosen) {
    int chosen2 = chosen;
    while (true) {
        chosen2 = rnd() % (int) batch.size();
        if (chosen2 != chosen) break;
    }
    int pr = rnd() % ((int) batch[chosen].size());
    int pr2 = rnd() % ((int) batch[chosen2].size());
    swap(batch[chosen][pr], batch[chosen2][pr2]);
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


void print_bucket(vector<vector<product>> &bucket, vector<vector<int>> &bestpermutations) {
    json basket;
    basket["buckets"] = {};
    for (int i = 0; i < bucket.size(); i++) {
        json items;
        items["items"] = {};
        for (int j = 0; j < bucket[i].size(); j++) {
            assert(bestpermutations[i].size() == bucket[i].size());
            assert(bucket[i][bestpermutations[i][j]].cnt >= 0);
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
solve_batch(vector<product> &cells2, map<int, vector<product>> &product_position, map<position, int> &used,
            map<position, int> &product_cnt, int floors,
            int rows, int sections, int block_x,
            int block_y, int type) {
    if (cells2.empty()) return 0;
    vector<vector<product>> cells(floors + 1);
    for (auto i: cells2) cells[i.p.floor].push_back(i);
    vector<vector<product>> bucket;
    for (int i = 1; i <= floors; i++) {
        vector<product> cur_bucket;
        int cur_capacity = MAX_CAPACITY;
        for (int j = 0; j < cells[i].size(); j++) {
            if (cur_capacity >= cells[i][j].cnt) {
                cur_bucket.push_back(cells[i][j]);
                cur_capacity -= cells[i][j].cnt;
            } else {
                bucket.push_back(cur_bucket);
                cur_bucket = {cells[i][j]};
                cur_capacity = MAX_CAPACITY - cells[i][j].cnt;
            }
        }
        if (!cur_bucket.empty()) bucket.push_back(cur_bucket);
    }
    vector<int> bucket_capacity(bucket.size());
    for (int i = 0; i < bucket.size(); i++)
        for (auto j: bucket[i])
            bucket_capacity[i] += j.cnt;

    vector<vector<int>> permutations(bucket.size());
    vector<vector<product>> bestbucket = bucket;
    for (int i = 0; i < bucket.size(); i++) {
        permutations[i].resize(bucket[i].size());
        iota(permutations[i].begin(), permutations[i].end(), 0);
    }
    vector<vector<int>> bestpermutations = permutations;


    vector<int> answers(bucket.size()), bestanswers;
    for (int i = 0; i < bucket.size(); i++)
        answers[i] = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);

    bestanswers = answers;
    long double t = 100;
    for (int zigzig = 0; zigzig < 2000; zigzig++) {
        t *= 0.9999;
        // перекладывание товаров из ячейки в ячейку
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket[i].empty()) continue;
            transfer trans = mutation_transfering_bucket(bucket[i], used, product_cnt);
            if (trans.chosen == -1) continue;
            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
                if (cur < bestanswers[i]) {
                    bestanswers[i] = cur;
                    bestpermutations[i] = permutations[i];
                    bestbucket[i] = bucket[i];
                }
                answers[i] = cur;
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
//            cout << answers[i] - cur << '\n';
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
                if (cur < bestanswers[i]) {
                    bestanswers[i] = cur;
                    bestpermutations[i] = permutations[i];
                    bestbucket[i] = bucket[i];
                }
                answers[i] = cur;
            } else
                swap(permutations[i][swapping.first], permutations[i][swapping.second]);
        }

        // реверс отрезка пути
        for (int i = 0; i < bucket.size(); i++) {
            if (permutations[i].empty()) continue;

            pair<int, int> flipping = mutation_flipping_path(permutations[i]);

            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
                if (cur < bestanswers[i]) {
                    bestanswers[i] = cur;
                    bestpermutations[i] = permutations[i];
                    bestbucket[i] = bucket[i];
                }
                answers[i] = cur;
            } else {
                reverse(permutations[i].begin() + flipping.first, permutations[i].begin() + flipping.second + 1);
            }
        }

        // полное перекладывание товара из 1 ячейки в другую
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket[i].empty()) continue;
            auto changing = mutation_changing_cell(bucket[i], product_position, used);
            if (changing.chosen == -1) continue;
            int cur = calc(permutations[i], bucket[i], rows, sections, block_x, block_y);
            if (cur < answers[i] || rnd2() < exp((long double) (answers[i] - cur) / t)) {
                if (cur < bestanswers[i]) {
                    bestanswers[i] = cur;
                    bestbucket[i] = bucket[i];
                    bestpermutations[i] = permutations[i];
                }
                answers[i] = cur;
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

            if (cur + cur2 < answers[i] + answers[changing.chosen] ||
                rnd2() < exp((long double) (answers[i] + answers[changing.chosen] - cur - cur2) / t)) {
                if (cur + cur2 < bestanswers[i] + bestanswers[changing.chosen]) {
                    bestanswers[i] = cur;
                    bestanswers[changing.chosen] = cur2;

                    bestbucket[i] = bucket[i];
                    bestbucket[changing.chosen] = bucket[changing.chosen];

                    bestpermutations[i] = permutations[i];
                    bestpermutations[changing.chosen] = permutations[changing.chosen];
                }
                answers[i] = cur;
                answers[changing.chosen] = cur2;
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
                rnd2() < exp((long double) (answers[i] + answers[merged.chosen] - cur) / t)) {
                if (cur < answers[i] + answers[merged.chosen]) {
                    bestanswers[merged.chosen] = cur;
                    bestanswers[i] = 0;

                    bestbucket[i] = bucket[i];
                    bestbucket[merged.chosen] = bucket[merged.chosen];

                    bestpermutations[i] = permutations[i];
                    bestpermutations[merged.chosen] = permutations[merged.chosen];
                }
                answers[i] = 0;
                answers[merged.chosen] = cur;
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
    int res = 0;
    for (int i = 0; i < bucket.size(); i++) res += bestanswers[i];
    if (type == 2)
        print_bucket(bestbucket, bestpermutations);
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

map<pair<int, int>, vector<vector<product>>> popular;

bool cmp(pair<int, int> a, pair<int, int> b) {
    return popular[a].size() > popular[b].size();
}


signed main() {
    freopen("../../../data_sample/sample_3200_4.json", "r", stdin);
//    freopen("../../../data_sample/output.json", "w", stdout);
    json data;
    cin >> data;
    int batch_size = data["batch_size"];
    int floors = data["warehouse"]["meta"]["floors"];
    int block_x = data["warehouse"]["meta"]["block_y"];
    int block_y = data["warehouse"]["meta"]["block_x"];
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
        products[i].p.block_x = data["warehouse"]["stock"][i]["p"]["block_y"];
        products[i].p.block_y = data["warehouse"]["stock"][i]["p"]["block_x"];
        products[i].p.row = data["warehouse"]["stock"][i]["p"]["row"];
        products[i].p.section = data["warehouse"]["stock"][i]["p"]["section"];
        products[i].cnt = data["warehouse"]["stock"][i]["count"];
        product_position[products[i].id].push_back(products[i]);
        product_cnt[products[i].p] = products[i].cnt;
    }
    for (auto &i: product_position)
        sort(i.second.begin(), i.second.end());
    vector<vector<product>> cells;
    int tt = 0;
    vector<int> answers;


    for (int i = 0; i < orders.size(); i++) {
        vector<item> order;
        for (auto[id, cnt]: orders[i])
            order.push_back({id, cnt});
        sort(order.begin(), order.end());
        parse(order);
        map<pair<int, int>, int> popular_in_order;
        vector<product> order2;
        for (auto[id, cnt]: order) {
            int rem = cnt;
            for (auto &j: product_position[id]) {
                int mi = min(rem, j.cnt - used[j.p]);
                rem -= mi;
                auto need = j;
                need.cnt = mi;
                order2.push_back(need);
                if (need.cnt > 0)
                    popular_in_order[{need.p.block_x, need.p.block_y}]++;
                used[j.p] += mi;
            }
        }
        pair<int, int> most_popular_in_order = {-1, -1};
        for (auto j: popular_in_order)
            if (most_popular_in_order.first == -1 || j.second > popular_in_order[most_popular_in_order])
                most_popular_in_order = j.first;
        popular[most_popular_in_order].push_back(order2);
    }

    vector<pair<int, int>> blocks;
    for (int i = 1; i <= block_x; i++)
        for (int j = 1; j <= block_y; j++)
            blocks.push_back({i, j});
    sort(blocks.begin(), blocks.end(), cmp);

    vector<vector<product>> orders2;
    for (auto block: blocks)
        for (auto &j: popular[block])
            orders2.push_back(j);

    for (int i = 0; i < orders2.size(); i += batch_size) {
        cells.push_back({});
        answers.push_back(0);
        for (int j = i; j < min((int) orders2.size(), i + batch_size); j++)
            for (auto &k: orders2[j])
                cells[tt].push_back(k);
        answers[tt] = solve_batch(cells[tt], product_position, used, product_cnt, floors, rows, sections, block_x,
                                  block_y, 1);
        tt++;
    }
    vector<int> bestanswers = answers;
    vector<vector<product>> bestcells = cells;
    long double t = 100;


    if (tt > 1) {
        for (int zigzag = 0; zigzag < 30; zigzag++) {
            t *= 0.9999;
            for (int i = 0; i < tt; i++) {
                swapping_batch swapping = mutation_swapping_batch(cells, i);
                int cur = solve_batch(cells[i], product_position, used, product_cnt, floors, rows, sections, block_x,
                                      block_y, 1);
                int cur2 = solve_batch(cells[swapping.chosen2], product_position, used, product_cnt, floors, rows,
                                       sections, block_x,
                                       block_y, 1);
                if (cur + cur2 < answers[i] + answers[swapping.chosen2] ||
                    exp((long double) (answers[i] + answers[swapping.chosen2] - cur - cur2) / t)) {
                    if (cur + cur2 < bestanswers[i] + bestanswers[swapping.chosen2]) {
                        bestanswers[i] = cur;
                        bestanswers[swapping.chosen2] = cur2;
                        bestcells[i] = cells[i];
                        bestcells[swapping.chosen2] = cells[swapping.chosen2];
                    }
                    answers[i] = cur;
                    answers[swapping.chosen2] = cur2;
                } else {
                    swap(cells[i][swapping.pr], cells[swapping.chosen2][swapping.pr2]);
                }
            }
        }
    }

    data_res["batches"] = {};
    for (int i = 0; i < tt; i++) {
        int cur = solve_batch(bestcells[i], product_position, used, product_cnt, floors, rows, sections, block_x,
                              block_y, 2);
    }
    int res = 0;
    for (auto i: bestanswers) res += i;
    cout << (long double) res / cntt;
//    cout << data_res;

}

