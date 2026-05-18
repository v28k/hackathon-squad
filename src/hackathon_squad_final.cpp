#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef chrono::steady_clock Clock;

// ─── Globals ─────────────────────────────────────────────────────────────────

int N, M;
vector<ll>          skill;
vector<vector<int>> adj;
vector<double>      netVal;

Clock::time_point g_start;
inline double elapsed() {
    return chrono::duration<double>(Clock::now() - g_start).count();
}

// ─── SA working state ────────────────────────────────────────────────────────

vector<bool> inSet;
vector<int>  conflicts;
ll           curSkill = 0;

ll           bestSkill = 0;
vector<bool> bestInSet;

inline void saAdd(int u) {
    inSet[u]  = true;
    curSkill += skill[u];
    for (int v : adj[u]) conflicts[v]++;
}
inline void saRemove(int u) {
    inSet[u]  = false;
    curSkill -= skill[u];
    for (int v : adj[u]) conflicts[v]--;
}

// ─── Candidate (self-contained; does not touch SA arrays) ────────────────────

struct Candidate {
    vector<bool> inSet;
    vector<int>  conf;
    ll           score = 0;

    Candidate() : inSet(N + 1, false), conf(N + 1, 0), score(0) {}

    Candidate(const vector<bool>& is, const vector<int>& co, ll sc)
        : inSet(is), conf(co), score(sc) {}

    void add(int u) {
        inSet[u]  = true;
        score    += skill[u];
        for (int v : adj[u]) conf[v]++;
    }
    void remove(int u) {
        inSet[u]  = false;
        score    -= skill[u];
        for (int v : adj[u]) conf[v]--;
    }
    bool free(int u) const { return !inSet[u] && conf[u] == 0; }
};

// ─── Greedy construction ─────────────────────────────────────────────────────

Candidate greedyBuild(const vector<int>& order) {
    Candidate c;
    for (int u : order)
        if (c.free(u)) c.add(u);
    return c;
}

// ─── Local search polish ─────────────────────────────────────────────────────

bool lsRound(Candidate& c, mt19937& rng) {
    bool improved = false;

    // Fill pass
    vector<int> fn;
    fn.reserve(N);
    for (int u = 1; u <= N; u++)
        if (c.free(u)) fn.push_back(u);
    shuffle(fn.begin(), fn.end(), rng);
    for (int u : fn)
        if (c.free(u)) { c.add(u); improved = true; }

    // Swap pass
    vector<int> mem;
    mem.reserve(N);
    for (int u = 1; u <= N; u++)
        if (c.inSet[u]) mem.push_back(u);
    shuffle(mem.begin(), mem.end(), rng);

    for (int u : mem) {
        if (!c.inSet[u]) continue;

        vector<int> fr;
        for (int v : adj[u])
            if (!c.inSet[v] && c.conf[v] == 1) fr.push_back(v);
        if (fr.empty()) continue;

        c.remove(u);
        sort(fr.begin(), fr.end(), [](int a, int b){ return skill[a] > skill[b]; });

        vector<int> added;
        for (int v : fr)
            if (c.free(v)) { c.add(v); added.push_back(v); }

        ll gained = 0;
        for (int v : added) gained += skill[v];

        if (gained > skill[u]) {
            improved = true;
        } else {
            for (int v : added) c.remove(v);
            c.add(u);
        }
    }
    return improved;
}

void localSearch(Candidate& c, mt19937& rng, double deadline) {
    while (elapsed() < deadline)
        if (!lsRound(c, rng)) break;
}

// ─── On-discovery polish ─────────────────────────────────────────────────────

const double SA_END_GLOBAL = 285.0;

void snapshotAndPolish() {
    if (curSkill <= bestSkill) return;
    bestSkill = curSkill;
    bestInSet = inSet;

    if (elapsed() > SA_END_GLOBAL - 2.0) return;

    Candidate c(inSet, conflicts, curSkill);
    mt19937   rng2((uint32_t)bestSkill);
    bool improved = lsRound(c, rng2);

    if (improved && c.score > bestSkill) {
        bestSkill = c.score;
        bestInSet = c.inSet;
        inSet     = c.inSet;
        conflicts = c.conf;
        curSkill  = c.score;
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    g_start = Clock::now();

    const double SA_END      = SA_END_GLOBAL;
    const double INIT_BUDGET = 20.0;

    // Input
    cin >> N >> M;
    skill.resize(N + 1);
    adj.resize(N + 1);
    netVal.resize(N + 1);

    ll max_skill = 0;
    for (int i = 1; i <= N; i++) {
        cin >> skill[i];
        max_skill = max(max_skill, skill[i]);
    }
    for (int i = 0; i < M; i++) {
        int u, v; cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Trivial case: no conflicts at all
    if (M == 0) {
        ll total = 0;
        for (int i = 1; i <= N; i++) total += skill[i];
        cout << total << "\n";
        for (int i = 1; i <= N; i++) {
            if (i > 1) cout << " ";
            cout << i;
        }
        cout << "\n";
        return 0;
    }

    mt19937 rng(1337);

    // Ordering 1: density — skill / (degree + 1)
    vector<int> byDensity(N);
    iota(byDensity.begin(), byDensity.end(), 1);
    sort(byDensity.begin(), byDensity.end(), [](int a, int b){
        return (double)skill[a] / (adj[a].size() + 1.0)
             > (double)skill[b] / (adj[b].size() + 1.0);
    });

    // Ordering 2: raw skill
    vector<int> bySkill(N);
    iota(bySkill.begin(), bySkill.end(), 1);
    sort(bySkill.begin(), bySkill.end(),
         [](int a, int b){ return skill[a] > skill[b]; });

    // Ordering 3: degree ascending (fewest conflicts first)
    vector<int> byDegree(N);
    iota(byDegree.begin(), byDegree.end(), 1);
    sort(byDegree.begin(), byDegree.end(),
         [](int a, int b){ return adj[a].size() < adj[b].size(); });

    // Ordering 4: net value — skill minus weighted neighbour penalty
    for (int u = 1; u <= N; u++) {
        double pen = 0.0, denom = (double)(adj[u].size() + 1);
        for (int v : adj[u]) pen += (double)skill[v] / denom;
        netVal[u] = (double)skill[u] - pen;
    }
    vector<int> byNetValue(N);
    iota(byNetValue.begin(), byNetValue.end(), 1);
    sort(byNetValue.begin(), byNetValue.end(),
         [](int a, int b){ return netVal[a] > netVal[b]; });

    // ── Phase 1 + 2: Greedy init + local search polish ────────────────────────
    Candidate bestCandidate;
    {
        double phaseStart = elapsed();
        double sliceSize  = INIT_BUDGET / 4.0;

        auto tryOrdering = [&](vector<int>& order, int idx) {
            double deadline = phaseStart + sliceSize * (idx + 1);
            Candidate c = greedyBuild(order);
            localSearch(c, rng, deadline);
            if (c.score > bestCandidate.score) bestCandidate = c;
        };

        tryOrdering(byDensity,  0);
        tryOrdering(bySkill,    1);
        tryOrdering(byDegree,   2);
        tryOrdering(byNetValue, 3);
    }

    // ── Phase 3: Simulated Annealing ──────────────────────────────────────────
    inSet    = bestCandidate.inSet;
    conflicts.assign(N + 1, 0);
    curSkill = 0;
    for (int u = 1; u <= N; u++) {
        if (inSet[u]) {
            curSkill += skill[u];
            for (int v : adj[u]) conflicts[v]++;
        }
    }
    if (curSkill > bestSkill) {
        bestSkill = curSkill;
        bestInSet = inSet;
    }

    const double T_INIT  = (double)max_skill * 0.5;
    const double T_FINAL = 0.5;

    uniform_real_distribution<double> prob(0.0, 1.0);
    uniform_int_distribution<int>     pickNode(1, N);

    vector<int> toRemove;
    toRemove.reserve(32);

    ll     iteration      = 0;
    ll     itersSinceBest = 0;
    double saStart        = elapsed();
    double saWindow       = max(1.0, SA_END - saStart);
    double curTemp        = T_INIT;
    double lastPerturb    = saStart;
    double heatBoost      = 1.0;

    while (true) {

        // Clock + temperature check every 4096 iterations
        if ((iteration & 4095) == 0) {
            double t = elapsed();
            if (t >= SA_END) break;

            // Decay heat boost, then recompute temperature with boost applied
            heatBoost = max(1.0, heatBoost * 0.95);
            double frac = (t - saStart) / saWindow;
            curTemp = T_INIT * pow(T_FINAL / T_INIT, frac) * heatBoost;

            // Phase 4: random-restart perturbation every 30 seconds
            if (t - lastPerturb > 30.0) {
                lastPerturb = t;

                inSet    = bestInSet;
                conflicts.assign(N + 1, 0);
                curSkill = 0;
                for (int u = 1; u <= N; u++) {
                    if (inSet[u]) {
                        curSkill += skill[u];
                        for (int v : adj[u]) conflicts[v]++;
                    }
                }
                vector<int> members;
                members.reserve(N);
                for (int u = 1; u <= N; u++)
                    if (inSet[u]) members.push_back(u);
                shuffle(members.begin(), members.end(), rng);

                int toDrop = members.empty() ? 0 : max(1, (int)(members.size() / 10));
                for (int i = 0; i < toDrop; i++) saRemove(members[i]);

                itersSinceBest = -1;
            }

            // Adaptive heat boost: nudge temperature if stuck for too long
            if (itersSinceBest > 500000) {
                heatBoost      = min(heatBoost * 2.0, 100.0);
                itersSinceBest = -1;
            }
        }

        iteration++;
        itersSinceBest++;

        int u = pickNode(rng);

        if (inSet[u]) {
            // DROP: remove u probabilistically
            double delta = -(double)skill[u];
            if (prob(rng) < exp(delta / curTemp))
                saRemove(u);

        } else if (conflicts[u] == 0) {
            // ADD: u is conflict-free — always accept
            saAdd(u);
            snapshotAndPolish();
            if (curSkill >= bestSkill) itersSinceBest = 0;

        } else {
            // SWAP: remove all in-set neighbours of u, then add u
            toRemove.clear();
            double delta = (double)skill[u];
            for (int v : adj[u]) {
                if (inSet[v]) {
                    delta -= (double)skill[v];
                    toRemove.push_back(v);
                }
            }

            if (delta > 0.0 || prob(rng) < exp(delta / curTemp)) {
                for (int v : toRemove) saRemove(v);
                saAdd(u);
                snapshotAndPolish();
                if (delta > 0.0) itersSinceBest = 0;
            }
        }
    }

    // Output
    cout << bestSkill << "\n";
    bool first = true;
    for (int i = 1; i <= N; i++) {
        if (bestInSet[i]) {
            if (!first) cout << " ";
            cout << i;
            first = false;
        }
    }
    cout << "\n";

    return 0;
}