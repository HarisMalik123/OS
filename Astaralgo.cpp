#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define ROWS 4
#define COLS 4

typedef struct Node Node;

typedef struct 
{
    int row;
    int col;
    int distance;
} State;

struct Node 
{
    State state;
    Node* next;
};

typedef struct 
{
    Node* front;
    Node* rear;
} Queue;

Queue* createQueue() 
{
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

void enqueue(Queue* queue, State state) 
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->state = state;
    newNode->next = NULL;

    if (queue->rear == NULL)
    {
        queue->front = newNode;
        queue->rear = newNode;
    } 
    else 
    {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

State dequeue(Queue* queue) 
{
    if (queue->front == NULL) 
    {
        State emptyState = {-1, -1, -1};
        return emptyState;
    }

    State state = queue->front->state;
    Node* temp = queue->front;

    if (queue->front == queue->rear) 
    {
        queue->front = NULL;
        queue->rear = NULL;
    } 
    else 
    {
        queue->front = queue->front->next;
    }

    free(temp);
    return state;
}

bool isSafe(int row, int col) 
{
    return (row >= 0 && row < ROWS && col >= 0 && col < COLS);
}

bool isGoalState(State state) 
{
    return (state.row == 3 && state.col == 3);
}

int heuristic(State state) 
{
    return abs(state.row - 3) + abs(state.col - 3);
}

void printPath(State path[], int length) 
{  
    printf("Path: ");
    for (int i = 0; i < length; i++) {
        printf("(%d, %d) ", path[i].row, path[i].col);
    }
    printf("\n");
}

Queue* queues[4];
pthread_mutex_t mutex;

void* exploreStates(void* direction) 
{
    int dir = *(int*)direction;
    int moveRow[] = {-1, 0, 1, 0}; 
    int moveCol[] = {0, 1, 0, -1};

    while (true) 
    {
        State currentStates[4]; 
        int hValues[4]; 
        int directions[4]; 

        pthread_mutex_lock(&mutex);
        State currentState = dequeue(queues[dir]);
        pthread_mutex_unlock(&mutex);

        currentStates[dir] = currentState;
        hValues[dir] = heuristic(currentState);
        directions[dir] = dir;
        
        if (isGoalState(currentState)) 
        {
            printf("Goal state found!\n");
            printPath(currentStates, currentState.distance);
            exit(0);
        }

        for (int j = 0; j < 4; j++) 
        {
            int newRow = currentState.row + moveRow[j];
            int newCol = currentState.col + moveCol[j];

        if (isSafe(newRow, newCol)) 
        {
            State newState = {newRow, newCol, currentState.distance + 1};

            int hValue = heuristic(newState);

            pthread_mutex_lock(&mutex);
            enqueue(queues[dir], newState);
            pthread_mutex_unlock(&mutex);
        }
    }

    int minHValue = hValues[0];
    int minIndex = 0;

    for (int j = 1; j < 4; j++) 
    {
        if (hValues[j] < minHValue) 
        {
            minHValue = hValues[j];
            minIndex = j;
        }
    }

    if (!isGoalState(currentState) && minHValue < hValues[dir]) 
    {
        pthread_mutex_lock(&mutex);
        State tempState = dequeue(queues[dir]);
        pthread_mutex_unlock(&mutex);

        currentStates[minIndex] = tempState;
        hValues[minIndex] = heuristic(tempState);
        directions[minIndex] = dir;

        pthread_mutex_lock(&mutex);
        enqueue(queues[minIndex], currentState);
        pthread_mutex_unlock(&mutex);
    }

    }

    return NULL;
}


int main() 
{

    for (int i = 0; i < 4; i++) 
    {   
        queues[i] = createQueue();
    }

    State startState = {0, 0, 0};


    for (int i = 0; i < 4; i++) 
    {
        enqueue(queues[i], startState);
    }

    pthread_mutex_init(&mutex, NULL); 

    pthread_t threads[4];
    int directions[4] = {0, 1, 2, 3};

    for (int i = 0; i < 4; i++) 
    {
        pthread_create(&threads[i], NULL, exploreStates, &directions[i]);
    }


    for (int i = 0; i < 4; i++) 
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}