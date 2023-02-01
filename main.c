#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX (50)
#define POSSIBLE_MOVES (8)

int N;
int fl = 0;
int chess_board[MAX][MAX];

typedef struct 
{
    int x, y;
} pair;

static int Is_in_boundaries(int x_pos, int y_pos) 
{
    return (((x_pos >= 0) && (x_pos < N)) && (y_pos >= 0) && (y_pos < N));
}

static int Is_move_valid(int x_pos, int y_pos) 
{
    return ((Is_in_boundaries(x_pos, y_pos)) && (chess_board[x_pos][y_pos] == -1));
}

typedef struct 
{
    int x;
    int y;
    int count;
    pair path[MAX * MAX];
} vals;


bool Knights_tour_found() 
{
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            if (chess_board[r][c] == -1) return false;
    return true;
}

static const int possible_moves[][2] = { {1,  2}, {1, -2}, {2,  1}, {2, -1}, {-1,  2}, {-1, -2}, {-2,  1}, {-2, -1} };

void *Degree_fn(void *_arg) 
{
    int cnt = 0;
    vals *t = (vals *)_arg;

    for (int i = 0; i < POSSIBLE_MOVES; ++i) 
        if (Is_move_valid((t->x) + possible_moves[i][0], (t->y) + possible_moves[i][1])) 
            ++cnt;

    return (void *)(intptr_t)cnt;
}

int Second_min_deg_fn(void *args, int x_pos, int y_pos, int idx)
{
	int degree = 0;
	int min_degree = N + 1;
	int i = 0;
	int temp_x_pos = 0;
	int temp_y_pos = 0;
	vals *t1 = (vals *)args;
	for (i = 0; i < POSSIBLE_MOVES; ++i)
	{
		temp_x_pos = x_pos + possible_moves[idx][0] + possible_moves[i][0];
		temp_y_pos = y_pos + possible_moves[idx][1] + possible_moves[i][1];

		if (Is_move_valid(temp_x_pos, temp_y_pos))
		{
		    vals *t = (vals *)malloc(sizeof(vals));
		    t->x = temp_x_pos;
		    t->y = temp_y_pos;
		    t->count = t1->count;
			degree = (int)(intptr_t)Degree_fn(t);

            if (degree <= min_degree)
			{
				min_degree = degree;
			}
		}
	}

	return (min_degree);
}

int Tie_breaker(void *args, int x_pos, int y_pos, int idx1, int idx2)
{
    vals *t1 = (vals *)args;
	int degree1 = Second_min_deg_fn(t1, x_pos, y_pos, idx1);
	int degree2 = Second_min_deg_fn(t1, x_pos, y_pos, idx2);

	return ((degree1 <= degree2) ? idx1 : idx2);
}

void *Next_move_fn(void *_args) 
{
    int degree = 0;
    int next_x_pos = 0, next_y_pos = 0;
    int second_min_degree = 0, second_min_deg_index = 0;
    int min_degree = N + 1, min_deg_index = 0;
    int i = 0;

    vals *t = (vals *)_args;
    int x_pos = t->x, y_pos = t->y;
    int moves_count = t->count;
    t->path[moves_count].x = x_pos;
    t->path[moves_count].y = y_pos;
    chess_board[x_pos][y_pos] = moves_count;

    if (Knights_tour_found()) 
    {
        for (int i = 1;i < (N * N) + 1;i++)
            printf("%d,%d|", t->path[i].x, t->path[i].y);
        fl = 1;
        return NULL;
    }

    if (moves_count == (N * N)) 
        return NULL;

    pthread_t *tid = malloc(POSSIBLE_MOVES * sizeof(pthread_t));
    int temp[POSSIBLE_MOVES];

    for (int i = 0; i < POSSIBLE_MOVES; i++)
        temp[i] = 0;

    for (i = 0; i < POSSIBLE_MOVES; i++) 
    {
        next_x_pos = x_pos + possible_moves[i][0];
        next_y_pos = y_pos + possible_moves[i][1];

        if (Is_move_valid(next_x_pos, next_y_pos)) 
        {
            temp[i] = 1;

            vals *t1 = (vals *)malloc(sizeof(vals));
            t1->x = next_x_pos;
            t1->y = next_y_pos;
            t1->count = moves_count;
            for (int i = 0; i <= N * N; i++) 
            {
                t1->path[i].x = t->path[i].x;
                t1->path[i].y = t->path[i].y;
            }
            pthread_create(&tid[i], NULL, Degree_fn, t1);
        }
    }

    for (int i = 0;i < POSSIBLE_MOVES;i++) 
    {
        if (temp[i]) 
        {
            void *rv;
            pthread_join(tid[i], &rv);
            degree = (int)(intptr_t)rv;
            if (degree <= min_degree) 
            {
                second_min_degree = min_degree;
                second_min_deg_index = min_deg_index;
                min_degree = degree;
                min_deg_index = i;
            }
        }
    }

    if (min_degree == second_min_degree)
    {
    	min_deg_index = Tie_breaker(t, x_pos, y_pos, min_deg_index, second_min_deg_index);
    }

    next_x_pos = x_pos + possible_moves[min_deg_index][0];
    next_y_pos = y_pos + possible_moves[min_deg_index][1];
    vals *t2 = (vals *)malloc(sizeof(vals));
    moves_count++;
    t2->x = next_x_pos;
    t2->y = next_y_pos;
    t2->count = moves_count;
    for (int i = 0; i <= N * N; i++) 
    {
        t2->path[i].x = t->path[i].x;
        t2->path[i].y = t->path[i].y;
    }
    Next_move_fn(t2);
    return NULL;
}


void Solve_chessboard(int x_pos, int y_pos) 
{
    pair pos[N * N];
    for (int i = 0;i < N;i++)
        for (int j = 0;j < N;j++)
            chess_board[i][j] = -1;

    for (int i = 0;i < (N * N)+1;i++) 
    {
        pos[i].x = -1;
        pos[i].y = -1;
    }

    vals *t = (vals *)malloc(sizeof(vals));
    t->x = x_pos;
    t->y = y_pos;
    t->count = 1;
    for (int i = 0;i <= N * N;i++) 
    {
        t->path[i].x = pos[i].x;
        t->path[i].y = pos[i].y;
    }
    
    Next_move_fn(t);

    if (!fl) 
        printf("No Possible Tour");
}


int main(int argc, const char *argv[]) 
{
    if (argc != 4) 
    {
        printf("Usage: ./Knight.out grid_size StartX StartY");
        exit(-1);
    }

    N = atoi(argv[1]);
    int Start_X = atoi(argv[2]);
    int Start_Y = atoi(argv[3]);
    Solve_chessboard(Start_X, Start_Y);
    return 0;
}
