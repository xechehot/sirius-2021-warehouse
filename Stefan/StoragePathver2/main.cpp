#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <random>

using namespace std;
mt19937 rnd(std::chrono::high_resolution_clock::now().time_since_epoch().count());
const int INF = 1000000007;

long double rnd2() {
    return (long double) rnd() / UINT32_MAX;
}

int calc(vector<int> &permutation, vector<vector<int>> &graph, vector<pair<int, int>> &location2, int h, int w) {
    int result = graph[0][location2[permutation[0]].first * w + location2[permutation[0]].second];
    for (int i = 1; i < permutation.size(); i++) {
        int previous = location2[permutation[i - 1]].first * w + location2[permutation[i - 1]].second;
        int next = location2[permutation[i]].first * w + location2[permutation[i]].second;
        result += graph[previous][next];
    }
    int minimal = INF;
    for (int i = 0; i <= w; i++) {
        int previous = location2[permutation.back()].first * w + location2[permutation.back()].second;
//        cerr << previous << ' ' << h * w + i << '\n';
        minimal = min(minimal, graph[previous][(h - 1) * w + i]);
    }
    if (minimal >= INF)
        cerr << location2[permutation.back()].first << ' ' << location2[permutation.back()].second;
    assert(minimal < INF);
    return result + minimal;
}

void testing() {
    int h, w, n;
//    cin >> h >> w >> n;
    h = 17, w = 12, n = 10;
    map<string, int> cnt;
    map<string, pair<int, int>> location;
    vector<vector<vector<pair<string, int>>>> order(h + 2, vector<vector<pair<string, int>>>(w + 2));
    for (int i = 0; i < n; i++) {
        string name;
        int len = rnd() % 10 + 1;
        name.resize(len);
        for (int j = 0; j < len; j++) name[j] = rnd() % 26 + 'a';
        cnt[name]++;
    }
    for (auto[name, count]: cnt) {
        location[name] = make_pair(rnd() % h + 1, rnd() % w + 1);
//        cout << location[name].first << ' ' << location[name].second << '\n';
    }

    vector<vector<int>> graph((w + 1) * (h + 2) + 5, vector<int>((w + 1) * (h + 2) + 5, INF));
    for (int i = 0; i <= h; i++)
        for (int j = 0; j < w; j++) {
            int x = i * w + j;
            if (i == 0 || i == h) {
                graph[x + 1][x] = 200;
                graph[x][x + 1] = 200;
            } else {
                graph[x - w][x] = 100;
                graph[x][x - w] = 100;
            }
        }

    for (int k = 0; k < graph.size(); k++)
        for (int i = 0; i < graph.size(); i++)
            for (int j = 0; j < graph.size(); j++)
                graph[i][j] = min(graph[i][j], graph[i][k] + graph[k][j]);

    for (auto[name, count]: cnt)
        order[location[name].first][location[name].second].push_back({name, count});


    vector<int> permutation(location.size());

    iota(permutation.begin(), permutation.end(), 0);
    shuffle(permutation.begin(), permutation.end(),
            mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    vector<pair<int, int>> location2;
    for (auto &i: location) location2.push_back(i.second);
    for (auto &i: location2) {
        i.first--;
        i.second--;
    }
    long double t = 1;
    int ans = calc(permutation, graph, location2, h, w);
    int bestans = ans;
    vector<int> bestpermutation = permutation;
//    return;
    for (int i = 0; i < 100000; i++) {
        t *= 0.999;
        vector<int> nextpermutation = permutation;
        swap(nextpermutation[rnd() % nextpermutation.size()], nextpermutation[rnd() % nextpermutation.size()]);
        int cur = calc(nextpermutation, graph, location2, h, w);
        if (cur < ans || rnd2() < exp((long double) (ans - cur) / t)) {
            if (cur < bestans) {
                bestpermutation = nextpermutation;
                bestans = cur;
            }
            permutation = nextpermutation;
            ans = cur;
        }
    }
    vector<string> locationname;
    for (auto &i: location) locationname.push_back(i.first);
    cout << ans / 100 << '\n';
    for (auto v: bestpermutation)
        cout << locationname[v] << '\n';
}


signed main() {
    testing();
}
