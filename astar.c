#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "queue.h"

#define EDGE_COST 1
#define DIAG_COST sqrt(2)
#define LEAK_COST INFINITY

char test[] = {
"#    #   # #    #\n"
"  #    #   # #  #\n"
"    #      #   ##\n"
"  #    #        #\n"
"  #    #    #  ##\n"
"#    #      #   #\n"
};

enum node_state {CLOSED, UNVISITED, VISITED};
typedef struct node NODE, *PNODE;
typedef struct node {
	bool traversible;
	enum node_state state;
	int x, y;
	float cost_to_self;
	float cost_to_goal;
	PNODE pi;
} NODE, *PNODE;

NODE new_node(char c, int x, int y)
{
	printf("%2i %2i |", x, y);
	return (NODE){(c==' ')?true:false, UNVISITED, x, y, INFINITY, INFINITY, NULL};
}

//Returns a pointer to a new map, free when done.
void init_map(PNODE map, char *mapstr, int numcols, int numrows)
{
	int rowchars = numcols + 1;
	for (int i = 0; i < numrows*rowchars; i++)
		if (mapstr[i] != '\n')
			map[i%rowchars + (i/rowchars)*numcols] = new_node(mapstr[i], i%rowchars, i/rowchars);
}

float path_cost_heuristic(int x1, int y1, int x2, int y2)
{
	float x = abs(x1-x2), y = abs(y1-y2);
	return fmaf(fmin(x, y), sqrt(2) - 1, fmin(x, y));
}

int compare_path_cost(const void *node1, const void *node2)
{
	return ((PNODE)node2)->cost_to_self - ((PNODE)node1)->cost_to_self;
}

PNODE in_map(int x, int y, PNODE map, int numcols, int numrows)
{
	if (x >= 0 && x < numcols && y >= 0 && y < numrows)
		return &map[x + y*numcols];
	else
		return NULL;
}

void consider_neighbors(QUEUEP q, PNODE current, PNODE map, int numcols, int numrows)
{
	int x = current->x, y = current->y;
	int xo[] = {1, 1, 0, -1, -1, -1, 0, 1};
	int yo[] = {0, 1, 1, 1, 0, -1, -1, -1};
	for (int i = 0; i < 8; i++) {
		PNODE n = in_map(x+xo[i], y+yo[i], map, numcols, numrows);
		if (!n || !n->traversible || n->state == CLOSED)
			continue;
		float ecost;
		if (i%2) { //n is diagonal-adjacent
			PNODE c1 = in_map(x+xo[(i+1)%8], y+yo[(i+1)%8], map, numcols, numrows);
			PNODE c2 = in_map(x+xo[(i+7)%8], y+yo[(i+7)%8], map, numcols, numrows);
			ecost = (c1->traversible || c2->traversible) ? DIAG_COST : LEAK_COST;
		} else { //n is regular-adjacent
			ecost = EDGE_COST;
		}
		if (current->cost_to_self + ecost >= n->cost_to_self)
			continue;
		n->pi = current;
		n->cost_to_self = current->cost_to_self + ecost;
		n->cost_to_goal = current->cost_to_self + path_cost_heuristic(x, y, n->x, n->y);
		if (n->state == UNVISITED) {
			n->state = VISITED;
			heap_enqueue(q, n, compare_path_cost);
		}
	}
}

int main(int argc, char **argv)
{
	char *mapstr = NULL;
	bool mapstr_dynamic = false;
	int fd;
	struct stat buf;
	//Load in the map.
	if (argc > 1 && (fd = open(argv[1], O_RDONLY))) {
		fstat(fd, &buf);
		mapstr = (char *)malloc(buf.st_size+1);
		mapstr_dynamic = true;
		read(fd, mapstr, buf.st_size);
		close(fd);
		mapstr[buf.st_size] = '\0';
	} else {
		mapstr = test;
	}
	printf("%s\n", mapstr);
	int numcols = strchr(mapstr, '\n') - mapstr;
	int numrows = strlen(mapstr) / (numcols + 1); //Don't count the '\n' chars
	printf("%i columns, %i rows\n", numcols, numrows);
	//Allocate and init
	PNODE map = malloc(numcols * numrows * sizeof(NODE));
	if (map == NULL)
		return -1;
	init_map(map, mapstr, numcols, numrows);
	//Get start and goal coords
	int start_x = -1, start_y = -1, goal_x = -1, goal_y = -1;
	PNODE start_node, goal_node;
	do {
		printf("Enter valid start coordinates:\n");
		scanf("%i %i", &start_x, &start_y);
	} while (!(start_node = in_map(start_x, start_y, map, numcols, numrows)));
	do {
		printf("Enter valid goal coordinates:\n");
		scanf("%i %i", &goal_x, &goal_y);
	} while (!(goal_node = in_map(goal_x, goal_y, map, numcols, numrows)));
	printf("From (%i, %i) to (%i, %i)\n", start_x, start_y, goal_x, goal_y);
	printf("From (%i, %i) to (%i, %i)\n", start_node->x, start_node->y, goal_node->x, goal_node->y);
	start_node->cost_to_self = 0;
	start_node->cost_to_goal = path_cost_heuristic(start_x, start_y, goal_x, goal_y);
	QUEUEP open_set = new_queue(numcols+numrows);
	heap_enqueue(open_set, start_node, compare_path_cost);
	while (open_set->length > 0) {
		PNODE current = heap_dequeue(open_set, compare_path_cost);
		if (current == goal_node)
			break;
		current->state = CLOSED;
		consider_neighbors(open_set, current, map, numcols, numrows);
	}
	for (PNODE current = goal_node; current != NULL; current = current->pi)
		mapstr[current->x + current->y*(numcols + 1)] = '*';
	printf("%s\n", mapstr);
	printf("Cost to goal node: %f\n", goal_node->cost_to_self);

	free(map);
	free_queue(open_set);
	if (mapstr_dynamic)
		free(mapstr);
	return 0;
}
