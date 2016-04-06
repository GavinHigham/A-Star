#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "queue.h"

char *test = {
"#    #   # #    #\n"
"  #    #   # #  #\n"
"    #      #   ##\n"
"  #    #        #\n"
"  #    #    #  ##\n"
"#    #      #   #\n"
};

typedef struct node NODE, *PNODE;
typedef struct node {
	bool is_block;
	float cost_to_self;
	float cost_to_goal;
	PNODE pi;
} NODE, *PNODE;

NODE new_node(char c)
{
	return (NODE){false, INFINITY, INFINITY, NULL};
}

//Returns a pointer to a new map, free when done.
void init_map(PNODE map, char *mapstr, int numcols, int numrows)
{
	for (int i = 0, j = 0; mapstr[i + j*(numcols+1)] != '\0'; i++) {
		char c = mapstr[i + j*(numcols+1)];
		if (c == '\n') {
			j++;
			i = 0;
			continue;
		}
		map[i + j*numcols] = new_node(c);
	}
}

float path_cost_heuristic(int dx, int dy)
{
	float x = abs(dx), y = abs(dy);
	return fmaf(fmin(x, y), sqrt(2) - 1, fmin(x, y));
}

int compare_path_cost(const void *node1, const void *node2)
{
	PNODE n1 = (PNODE)node1, n2 = (PNODE)node2;
	return (n1->cost_to_self + n1->cost_to_goal) - (n2->cost_to_self + n2->cost_to_goal);
}

int main(int argc, char **argv)
{
	char *mapstr = NULL;
	int fd;
	struct stat buf;
	//Load in the map.
	if (argc > 1 && (fd = open(argv[1], O_RDONLY))) {
		fstat(fd, &buf);
		char *mapstr = (char *)malloc(buf.st_size+1);
		read(fd, mapstr, buf.st_size);
		close(fd);
		mapstr[buf.st_size] = '\0';
	} else {
		mapstr = test;
	}
	int numcols = strchr(mapstr, '\n') - mapstr;
	int numrows = (strlen(mapstr) / numcols) - 1; //Don't count the '\n' chars
	//Allocate and init
	PNODE map = malloc(numcols * numrows * sizeof(NODE));
	init_map(map, mapstr, numcols, numrows);
	//Get start and goal coords
	int start_x = -1, start_y = -1, goal_x = -1, goal_y = -1;
	do {
		printf("Enter valid start coordinates:\n");
		scanf("%i %i", &start_x, &start_y);
	} while (start_x < 0 || start_x > numcols || start_y < 0 || start_y > numrows);
	do {
		printf("Enter valid goal coordinates:\n");
		scanf("%i %i", &goal_x, &goal_y);
	} while (goal_x < 0 || goal_x > numcols || goal_y < 0 || goal_y > numrows);
	PNODE start_node = &map[start_x + numcols*start_y];
	PNODE goal_node = &map[goal_x + numcols*goal_y];
	start_node->cost_to_self = 0;
	start_node->cost_to_goal = path_cost_heuristic(start_x-goal_x, start_y-goal_y);
	QUEUEP open_set = new_queue(numcols+numrows);
	heap_enqueue(open_set, start_node, compare_path_cost);

	free(map);
	return 0;
}
