# IC — Optimization

## Heuristic & Continuous Optimization
- [`kaggle/santa-2025`](kaggle/santa-2025): Christmas Tree Packing Challenge
- [`ahc/069`](ahc/069): AtCoder Heuristic Contest 069

## Combinatorial, DP & Graph Optimization (Aug–Oct 2026)
- **DP / Knapsack**
  - [`icpc/brazil/2026/esquenta/n`](icpc/brazil/2026/esquenta/n/sol.cpp) ([MOJ `esquenta-uberlandia-2026#kit-do-competidor`](https://moj.naquadah.com.br/treino/problema/?id=esquenta-uberlandia-2026%23kit-do-competidor)): *Kit do Competidor* (0-1 Knapsack Bottom-Up DP)
  - [ICPC `106598L` (*Left or right side*)](https://codeforces.com/gym/106598/submission/387490678): Interval DP (Maratona Paulista 2026)
  - [ICPC `105327K` (*Karamell*)](https://codeforces.com/gym/105327/submission/394874302): Subset Sum / Knapsack DP (Subregional 2024)
  - [ICPC `104603H` (*Robotic Skills*)](https://codeforces.com/gym/104603/submission/394691815): Grid DP Path Maximization (TAP 2023)
  - [CF `1005D` (*Polycarp and Div 3*)](https://codeforces.com/contest/1005/submission/391926549): DP / Greedy Substring Partition Maximization
  - [CF `1389C` (*Good String*)](https://codeforces.com/contest/1389/submission/392369639): DP / Greedy Deletion Minimization
  - [CF `2274A` (*The Projections of Paco*)](https://codeforces.com/contest/2274/submission/394182822): Tree / Combinatorial DP
- **Graph, Tree, Data Structures & Network Optimization**
  - [`icpc/brazil/2026/subreg/e`](icpc/brazil/2026/subreg/e/sol.cpp) ([MOJ `mdp-br-fp-2026#e-edicao`](https://moj.naquadah.com.br/treino/problema/?id=mdp-br-fp-2026%23e-edicao)): *Edição de Rede* (`sol.cpp`, `sol2.cpp`, `sol3.cpp`)
  - [`icpc/brazil/2026/subreg/i`](icpc/brazil/2026/subreg/i/sol.cpp) ([MOJ `mdp-br-fp-2026#i-invations`](https://moj.naquadah.com.br/treino/problema/?id=mdp-br-fp-2026%23i-invations)): *Porquinho-da-Índia* (Dependency Tree DFS + Interval Sweep-line Min-Chairs / Max-Overlap)
  - [MOJ `esquenta-uberlandia-2026#placar-ao-vivo` (*Placar ao Vivo*)](https://moj.naquadah.com.br/treino/problema/?id=esquenta-uberlandia-2026%23placar-ao-vivo): Lazy Segment Tree Range-Add / Range-Max Optimization
  - [`icpc/brazil/2026/mpp/e`](icpc/brazil/2026/mpp/e/sol.cpp) ([`106598E`](https://codeforces.com/gym/106598/submission/388055443)): *Space emergency* (HLD + Lazy Segment Tree)
  - [`icpc/brazil/2024/subreg/b`](icpc/brazil/2024/subreg/b/sol.cpp) ([`105327B`](https://codeforces.com/gym/105327/submission/389778446)): *Bacon Number*
  - [`icpc/brazil/2023/regional/m`](icpc/brazil/2023/regional/m/sol.cpp) ([`104736M`](https://codeforces.com/gym/104736/submission/390501177)): *Meeting Point* (Constrained Shortest Path / Modified Dijkstra)
  - [`icpc/argentina/tap/2023/g`](icpc/argentina/tap/2023/g/sol.cpp) ([`104603G`](https://codeforces.com/gym/104603/submission/395107750)): *Great Heights* (Sweep-line + Kruskal/DSU MST)
  - [CF `1985H1` (*Maximize the Largest Component*)](https://codeforces.com/contest/1985/submission/393970465): DSU Grid Component Maximization
- **Greedy, Binary Search, Geometric & Scheduling Optimization**
  - [`atCoder/begginer/470/b`](atCoder/begginer/470/b/sol.cpp) ([`abc470_b`](https://atcoder.jp/contests/abc470/submissions/78206269)): Replacement Minimization
  - [AtCoder `abc470_c`](https://atcoder.jp/contests/abc470/submissions/78292813): ABC 470 C
  - [`icpc/brazil/2026/esquenta/f`](icpc/brazil/2026/esquenta/f/sol.cpp) ([MOJ `esquenta-uberlandia-2026#fila-de-julgamento`](https://moj.naquadah.com.br/treino/problema/?id=esquenta-uberlandia-2026%23fila-de-julgamento)): *Fila de Julgamento* (Total Completion Time Minimization)
  - [`icpc/brazil/2025/subreg/l`](icpc/brazil/2025/subreg/l/sol.cpp) ([`106073L`](https://codeforces.com/gym/106073/submission/388960555)): *LLMs* (Dot-Product Similarity Maximization)
  - [`icpc/brazil/2016/regional/d`](icpc/brazil/2016/regional/d/sol.cpp) ([Beecrowd `2348`](https://judge.beecrowd.com/pt/runs/code/49926901) / [VJudge `847901D`](https://vjudge.net/contest/847901#problem/D)): *Dating On-Line / Encontros On-Line* (Polygon Area Maximization)
  - [`icpc/brazil/2016/regional/j`](icpc/brazil/2016/regional/j/sol.cpp) ([Beecrowd `2353`](https://judge.beecrowd.com/pt/runs/code/49925919) / [VJudge `847901J`](https://vjudge.net/contest/847901#problem/J)): *Just in Time / Bem na Hora* (Largest Prime $\le N$)
  - [ICPC `104736C` (*Candy Rush*)](https://codeforces.com/gym/104736/submission/391806536): Longest Balanced Subarray
  - [ICPC `104603C` (*Chromatic*)](https://codeforces.com/gym/104603/submission/394056533): Min-Max Difference Minimization
  - [ICPC `105327A` (*Attention to the Meeting*)](https://codeforces.com/gym/105327/submission/394861956): Speech Duration Maximization
  - [ICPC `105327L` (*Lexicographically Maximum*)](https://codeforces.com/gym/105327/submission/394863397): Bitwise Greedy Maximization
  - [`codeforces/round411div1/b`](codeforces/round411div1/b/sol.cpp) ([`804B`](https://codeforces.com/contest/804/submission/392376444)): *Minimum number of steps*
  - [CF `1967A` (*Permutation Counting*)](https://codeforces.com/contest/1967/submission/391944510): Binary Search + Greedy Min-Frequency Maximization
  - [CF `1824A` (*LuoTianyi and the Show*)](https://codeforces.com/contest/1824/submission/392100093): Greedy Seating Maximization
  - [CF `803A` (*Maximal Binary Matrix*)](https://codeforces.com/contest/803/submission/392536969): Lexicographically Maximal Symmetric Binary Matrix
  - [CF `2252C` (*Risky Tower*)](https://codeforces.com/contest/2252/submission/392766336): Binary Search + Greedy Optimization
  - [CF `1733D1` (*Zero-One Easy Version*)](https://codeforces.com/contest/1733/submission/393417539): Greedy Operation Cost Minimization
  - [CF `2274C` (*Lavencia Oranges*)](https://codeforces.com/contest/2274/submission/394334807): Binary Search Optimization
  - [CF `2274B` (*Prefix*)](https://codeforces.com/contest/2274/submission/394339324): Greedy Prefix Optimization
  - [`icpc/argentina/tap/2023/b`](icpc/argentina/tap/2023/b/sol.cpp) ([`104603B`](https://codeforces.com/gym/104603/submission/394056315)): *Black and white* (Grid Tile Packing)
  - [`icpc/argentina/tap/2023/d`](icpc/argentina/tap/2023/d/sol.cpp) ([`104603D`](https://codeforces.com/gym/104603/submission/394059062)): *Assigning problems* (Contest Round Maximization)
  - [`icpc/argentina/tap/2023/m`](icpc/argentina/tap/2023/m/sol.cpp) ([`104603M`](https://codeforces.com/gym/104603/submission/394064515)): *Multiple Downloads* (Priority Bandwidth Scheduling)
  - [`icpc/argentina/tap/2023/n`](icpc/argentina/tap/2023/n/sol.cpp) ([`104603N`](https://codeforces.com/gym/104603/submission/394313914)): *Lucky Number* (Divisible-by-5 Group Maximization)
