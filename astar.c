#include "heap.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

char *test = {
"#    #   # #    #\n"
"  #    #   # #  #\n"
"    #      #   ##\n"
"  #    #        #\n"
"  #    #    #  ##\n"
"#    #      #   #\n"
};

//Algorithm code

#define EDGE_COST 1
#define DIAG_COST sqrt(2)
#define LEAK_COST INFINITY

enum {MAX_NUM_NEIGHBORS = 8};
enum node_state {CLOSED, UNVISITED, VISITED};
struct node;
typedef struct node {
	bool traversible;
	enum node_state state;
	int x, y;
	float cost_to_self;
	float cost_to_goal;
	struct node *pi;
} NODE, *PNODE;

float path_cost_heuristic(int x1, int y1, int x2, int y2)
{
	float x = abs(x1-x2), y = abs(y1-y2);
	return fmaf(DIAG_COST, fmin(x, y), fabs(x - y));
}

int compare_path_cost(const void *node1, const void *node2)
{
	PNODE n1 = (PNODE)node1, n2 = (PNODE)node2;
	float n1_cost = (n1->cost_to_self + n1->cost_to_goal);
	float n2_cost = (n2->cost_to_self + n2->cost_to_goal);

	if (n1_cost < n2_cost)
		return 1;
	if (n1_cost > n2_cost)
		return -1;
	return 0;
}

void visit_neighbors(HEAP *h, PNODE current, PNODE goal, PNODE neighbors[], float edge_costs[], int num_neighbors)
{
	for (int i = 0; i < num_neighbors; i++) {
		PNODE n = neighbors[i];
		if (current->cost_to_self + edge_costs[i] >= n->cost_to_self)
			continue;

		n->pi = current;
		n->cost_to_self = current->cost_to_self + edge_costs[i];
		n->cost_to_goal = current->cost_to_self + path_cost_heuristic(n->x, n->y, goal->x, goal->y);
		if (n->state == UNVISITED) {
			n->state = VISITED;
			heap_add(h, n);
		}
	}
}

//Code specific to a rectangular grid, with diagonals

PNODE in_map(int x, int y, PNODE map, int numcols, int numrows)
{
	if (x >= 0 && x < numcols && y >= 0 && y < numrows)
		return &map[x + y*numcols];
	else
		return NULL;
}

int get_neighbors(PNODE map, int numcols, int numrows, PNODE current,
	PNODE neighbors[MAX_NUM_NEIGHBORS], float edge_costs[MAX_NUM_NEIGHBORS])
{
	int num_written = 0;
	for (int x = current->x - 1; x <= current->x + 1; x++) {
		for (int y = current->y - 1; y <= current->y + 1; y++) {
			PNODE n = in_map(x, y, map, numcols, numrows);

			if (!n || !n->traversible || n->state == CLOSED)
				continue;

			//Determine the cost of each edge (adjacent, diagonal, leak)
			float ecost = INFINITY;
			if (!(x == current->x || y == current->y)) {
				PNODE c1 = in_map(x, current->y, map, numcols, numrows);
				PNODE c2 = in_map(current->x, y, map, numcols, numrows);
				ecost = (c1->traversible || c2->traversible) ? DIAG_COST : LEAK_COST;
			} else {
				ecost = EDGE_COST;
			}
			neighbors[num_written] = n;
			edge_costs[num_written++] = ecost;
		}
	}
	return num_written;
}

NODE new_node(char c, int x, int y)
{
	return (NODE){(c == ' ') ? true : false, UNVISITED, x, y, INFINITY, INFINITY, NULL};
}

//Returns a pointer to a new map, free when done.
PNODE map_new(char *mapstr, int *numcols, int *numrows)
{
	int cols = *numcols = strchr(mapstr, '\n') - mapstr;
	int rows = *numrows = strlen(mapstr) / (cols + 1); //Don't count the '\n' chars
	printf("%i columns, %i rows\n", cols, rows);

	//Allocate and init
	PNODE map = malloc(cols * rows * sizeof(NODE));
	if (map == NULL)
		return NULL;

	for (int i = 0; i < rows*cols; i++)
		map[i] = new_node(mapstr[i%cols + (i/cols)*(cols+1)], i%cols, i/cols);
	return map;
}

void read_map_file(int argc, char **argv, char **mapstr)
{
	//Load in the map.
	char *filestr = NULL;
	int fd;
	if (argc > 1 && (fd = open(argv[1], O_RDONLY))) {
		struct stat buf;
		fstat(fd, &buf);
		filestr = (char *)malloc(buf.st_size + 1);
		read(fd, filestr, buf.st_size);
		close(fd);
		filestr[buf.st_size] = '\0';
		*mapstr = filestr;
	} else {
		*mapstr = strdup(test);
	}
	printf("%s\n", *mapstr);
}

int main(int argc, char **argv)
{
	char *mapstr = NULL;
	read_map_file(argc, argv, &mapstr);

	int numrows, numcols;
	int start_x = -1, start_y = -1, goal_x = -1, goal_y = -1;
	PNODE start_node = NULL, goal_node = NULL;
	PNODE map = map_new(mapstr, &numcols, &numrows);

	//Get start and goal coords from user
	do {
		printf("Enter valid start coordinates:\n");
		scanf("%i %i", &start_x, &start_y);
	} while (!(start_node = in_map(start_x, start_y, map, numcols, numrows)));
	do {
		printf("Enter valid goal coordinates:\n");
		scanf("%i %i", &goal_x, &goal_y);
	} while (!(goal_node = in_map(goal_x, goal_y, map, numcols, numrows)));

	printf("From (%i, %i) to (%i, %i)\n", start_x, start_y, goal_x, goal_y);

	start_node->cost_to_self = 0;
	start_node->cost_to_goal = path_cost_heuristic(start_x, start_y, goal_x, goal_y);

	HEAP *open_set = heap_new(numcols+numrows, compare_path_cost);
	heap_add(open_set, start_node);

	while (peek(open_set) != NULL) {
		PNODE current = heap_remove(open_set);
		if (current == goal_node)
			break;

		current->state = CLOSED;
		PNODE neighbors[MAX_NUM_NEIGHBORS];
		float edge_costs[MAX_NUM_NEIGHBORS];
		int num_neighbors = get_neighbors(map, numcols, numrows, current, neighbors, edge_costs);
		visit_neighbors(open_set, current, goal_node, neighbors, edge_costs, num_neighbors);
		//mapstr[current->x + current->y*(numcols + 1)] = direction_from_node(current);
	}

	for (PNODE current = goal_node; current != NULL; current = current->pi)
		mapstr[current->x + current->y*(numcols + 1)] = '*';

	printf("%s\n", mapstr);
	printf("Cost to goal node: %f\n", goal_node->cost_to_self);

	free(map);
	free(mapstr);
	heap_free(open_set);
	return 0;
}
