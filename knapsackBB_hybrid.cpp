// C++ program to solve knapsack problem using
// branch and bound

/* Copyright 2017 Meritxell Jordana Gavieiro
 * Copyright 2017 Albert Merino Pulido
 * Copyright 2017 Marc Sánchez Fauste
**/

#include <iostream>     // std::cout
#include <algorithm>    // std::sort
#include <vector>       // std::vector
#include <queue>          // std::queue
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#include <stddef.h>

using namespace std;

// Stucture for Item which store weight and corresponding
// value of Item

struct Item {
    float weight;
    int value;
};

typedef enum TAG {
    WORK_REQ,
    NODE,
    NEW_MAX_PROFIT,
    END,
    CONTINUE,
    HELLO
} TAG;

// Node structure to store information of decision
// tree

struct Node {
    // level --> Level of node in decision tree (or index
    //			 in arr[]
    // profit --> Profit of nodes on path from root to this
    //		 node (including this node)
    // bound ---> Upper bound of maximum profit in subtree
    //		 of this node/
    int level, profit, bound;
    float weight;
};

typedef struct Work {
    queue<Node> q;
    int maxProfit;
    Work(queue<Node> q, int maxProfit) : q(q), maxProfit(maxProfit) {};
} Work;

MPI_Datatype mpiNodeStructType;
MPI_Datatype mpiItemStructType;

// Comparison function to sort Item according to
// val/weight ratio

bool cmp(Item a, Item b) {
    double r1 = (double) a.value / a.weight;
    double r2 = (double) b.value / b.weight;
    return r1 > r2;
}

// Returns bound of profit in subtree rooted with u.
// This function mainly uses Greedy solution to find
// an upper bound on maximum profit.

int bound(Node u, int n, int W, Item arr[]) {
    // if weight overcomes the knapsack capacity, return
    // 0 as expected bound
    if (u.weight >= W)
        return 0;

    // initialize bound on profit by current profit
    int profit_bound = u.profit;

    // start including items from index 1 more to current
    // item index
    int j = u.level + 1;
    int totweight = u.weight;

    // checking index condition and knapsack capacity
    // condition
    while ((j < n) && (totweight + arr[j].weight <= W)) {
        totweight += arr[j].weight;
        profit_bound += arr[j].value;
        j++;
    }

    // If k is not n, include last item partially for
    // upper bound on profit
    if (j < n)
        profit_bound += (W - totweight) * arr[j].value /
            arr[j].weight;

    return profit_bound;
}

Work getWork(int W, Item arr[], int n, Node rootNode, int currentMaxProfit,
        unsigned int numberOfNodes) {
    queue<Node> Q;
    Node u, v;
    Q.push(rootNode);
    int maxProfit = currentMaxProfit;
    while (!Q.empty() && Q.size() < numberOfNodes) {
        u = Q.front();
        Q.pop();
        if (u.level == n - 1) continue;
        v.level = u.level + 1;
        v.weight = u.weight + arr[v.level].weight;
        v.profit = u.profit + arr[v.level].value;
        if (v.weight <= W && v.profit > maxProfit) maxProfit = v.profit;
        v.bound = bound(v, n, W, arr);
        if (v.bound > maxProfit) Q.push(v);
        v.weight = u.weight;
        v.profit = u.profit;
        v.bound = bound(v, n, W, arr);
        if (v.bound > maxProfit) Q.push(v);
    }
    return Work(Q, maxProfit);
}

// Returns maximum profit we can get with capacity W

int knapsack(int W, Item arr[], int n, int currentMaxProfit, queue<Node> Q) {
    Node u, v;

    // One by one extract an item from decision tree
    // compute profit of all children of extracted item
    // and keep saving maxProfit
    int maxProfit = currentMaxProfit;
    while (!Q.empty()) {
        // Dequeue a node
        u = Q.front();
        Q.pop();

        // If there is nothing on next level
        if (u.level == n - 1)
            continue;

        // Else if not last node, then increment level,
        // and compute profit of children nodes.
        v.level = u.level + 1;

        // Taking current level's item add current
        // level's weight and value to node u's
        // weight and value
        v.weight = u.weight + arr[v.level].weight;
        v.profit = u.profit + arr[v.level].value;

        // If cumulated weight is less than W and
        // profit is greater than previous profit,
        // update maxprofit
        if (v.weight <= W && v.profit > maxProfit)
            maxProfit = v.profit;

        // Get the upper bound on profit to decide
        // whether to add v to Q or not.
        v.bound = bound(v, n, W, arr);

        // If bound value is greater than profit,
        // then only push into queue for further
        // consideration
        if (v.bound > maxProfit)
            Q.push(v);

        // Do the same thing, but Without taking
        // the item in knapsack
        v.weight = u.weight;
        v.profit = u.profit;
        v.bound = bound(v, n, W, arr);
        if (v.bound > maxProfit)
            Q.push(v);
    }
    return maxProfit;
}

void master(char *filename) {
    double tpivot1 = 0, tpivot2 = 0, tpivot3 = 0; //time counting
    struct timeval tim;

    Item temp, *items;
    int cont;
    long int Nitems; // Number of items
    long int Width; // Max. load to carry

    FILE *test_file;
    if (!(test_file = fopen(filename, "r"))) {
        printf("Error opening Value file: %s\n", filename);
        exit(1);
    }

    //Reading number of items and Maximum width
    fscanf(test_file, "%ld %ld\n", &Nitems, &Width);
    items = (Item *) malloc(Nitems * sizeof (Item));

    //Capture first token time - init execution
    gettimeofday(&tim, NULL);
    tpivot1 = tim.tv_sec + (tim.tv_usec / 1000000.0);

    //Reading value and width for each element
    for (cont = 0; cont < Nitems; cont++) {
        fscanf(test_file, "%d,%f\n", &temp.value, &temp.weight);
        items[cont] = temp;
    }

    MPI_Bcast(&Nitems, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Width, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    sort(items, items + Nitems, cmp);
    MPI_Bcast(items, Nitems, mpiItemStructType, 0, MPI_COMM_WORLD);

    gettimeofday(&tim, NULL);
    tpivot2 = (tim.tv_sec + (tim.tv_usec / 1000000.0));
    /********************************************************************/
    int nWorkers;
    MPI_Comm_size(MPI_COMM_WORLD, &nWorkers);
    nWorkers -= 1;
    Node u;

    // dummy node at starting
    Node node;
    node.level = -1;
    node.profit = node.weight = 0;
    Work work = getWork(Width, items, Nitems, node, 0, nWorkers + 1);

    int maxProfit = work.maxProfit;
    queue<Node> Q = work.q;
    MPI_Bcast(&maxProfit, 1, MPI_INT, 0, MPI_COMM_WORLD);
    for (int i = 1; i < nWorkers + 1; i += 1) {
        if (!Q.empty()) {
            u = Q.front();
            Q.pop();
            MPI_Send(&u, 1, mpiNodeStructType, i,
                    NODE, MPI_COMM_WORLD);
        } else {
            MPI_Send(&u, 1, mpiNodeStructType, i,
                    END, MPI_COMM_WORLD);
        }
    }
    maxProfit =  knapsack(Width, items, Nitems, maxProfit, Q);
    MPI_Status status;
    for (int i = 1; i < nWorkers + 1; i += 1) {
        MPI_Recv(&u, 1, mpiNodeStructType, MPI_ANY_SOURCE, MPI_ANY_TAG,
                MPI_COMM_WORLD, &status);
        if (u.profit > maxProfit) {
            maxProfit = u.profit;
        }
    }
    /********************************************************************/
    cout << Width << ":" << Nitems << ":" << maxProfit;
    gettimeofday(&tim, NULL);
    tpivot3 = (tim.tv_sec + (tim.tv_usec / 1000000.0));
    cout << ":" << tpivot3 - tpivot2 << ":" << tpivot3 - tpivot1 << endl;
}

void worker() {
    long int n; // Number of items
    long int W; // Max. load to carry
    Item *arr;
    MPI_Bcast(&n, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&W, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    arr = (Item *) malloc(n * sizeof (Item));
    MPI_Bcast(arr, n, mpiItemStructType, 0, MPI_COMM_WORLD);

    queue<Node> Q;
    Node u, v;

    MPI_Status status;

    int maxProfit = 0;
    MPI_Bcast(&maxProfit, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Recv(&u, 1, mpiNodeStructType, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == NODE) {
        Q.push(u);
    }
    v.profit = knapsack(W, arr, n, maxProfit, Q);
    MPI_Send(&v, 1, mpiNodeStructType, 0, NEW_MAX_PROFIT, MPI_COMM_WORLD);
}

// driver program to test above function

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    /********************* Declare Node Struct *********************/
    int nodeStructBlockLengths[4] = {1, 1, 1, 1};
    MPI_Datatype nodeStructTypes[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_FLOAT};
    MPI_Aint nodeStructOffsets[4];

    nodeStructOffsets[0] = offsetof(Node, level);
    nodeStructOffsets[1] = offsetof(Node, profit);
    nodeStructOffsets[2] = offsetof(Node, bound);
    nodeStructOffsets[3] = offsetof(Node, weight);

    MPI_Type_create_struct(4, nodeStructBlockLengths, nodeStructOffsets,
            nodeStructTypes, &mpiNodeStructType);
    MPI_Type_commit(&mpiNodeStructType);
    /********************* Declare Item Struct *********************/
    int itemStructBlockLengths[2] = {1, 1};
    MPI_Datatype itemStructTypes[2] = {MPI_FLOAT, MPI_INT};
    MPI_Aint itemStructOffsets[2];

    itemStructOffsets[0] = offsetof(Item, weight);
    itemStructOffsets[1] = offsetof(Item, value);

    MPI_Type_create_struct(2, itemStructBlockLengths, itemStructOffsets,
            itemStructTypes, &mpiItemStructType);
    MPI_Type_commit(&mpiItemStructType);
    /***************************************************************/
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        master(argv[1]);
    } else {
        worker();
    }
    /*********************** Free Node Struct **********************/
    MPI_Type_free(&mpiNodeStructType);
    /***************************************************************/
    /*********************** Free Item Struct **********************/
    MPI_Type_free(&mpiItemStructType);
    /***************************************************************/
    MPI_Finalize();
    return 0;
}
